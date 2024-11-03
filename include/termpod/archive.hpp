#pragma once
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <utility>
#include <array>
#include <vector>
#include <sys/stat.h>
#include <utime.h>

namespace tr::pod
{
	enum version
	{
		none = 0,
		pod1 = 1,
		pod2 = 2,
		pod3 = 3,
		pod4 = 4,
		pod5 = 5,
		pod6 = 6,
		epd  = 7,
		epd1 = 7,
		epd2 = 9,
		last = 10,
	};

	static constexpr const std::pair<const char*, const char*> ident[last] =
	{
		{"NONE", "\0NONE"},
		{"POD1", "\0POD1"},
		{"POD2", "POD2\0"},
		{"POD3", "POD3\0"},
		{"POD4", "POD4\0"},
		{"POD5", "POD5\0"},
		{"POD6", "POD6\0"},
		{"EPD",  "dtxe\0"},
		{"EPD1", "dtxe\0"},
		{"EPD2", "tsal\0"},
	};

	static constexpr inline enum version id(const char magic[4])
	{
		for(int i = pod2; i < last; i++)
			if(strncmp(ident[i].second, &magic[0], 4) == 0)
				return (enum version)i;
		return pod1;
	}

	enum class section
	{
		none   = 0,
		file   = 1,
		header = 2,
		extra  = 3,
		entry  = 4,
		depend = 5,
		audit  = 6,
	};

	namespace audit
	{
		enum action
		{
			add = 0,
			rem = 1,
			chg = 2,
		};
		struct entry
		{
			char              user[32];
			int32_t          timestamp;
			enum action         action;
			char             path[256];
			int32_t      old_timestamp;
			uint32_t          old_size;
			int32_t      new_timestamp;
			uint32_t          new_size;
		};
	};
	namespace string
	{
		uint32_t ceil(uint32_t size)
		{
			static const std::array<uint32_t,12>  sizes = { 4, 8, 12, 16, 32, 48, 64, 80, 96, 128, 256, 264 };
			std::array<uint32_t,12>::const_iterator dst = sizes.begin();
			while(size > *dst) dst++;
			return *dst;
		}
		char* fgets(uint32_t size, FILE* stream)
		{
			if(size == 0)
				return nullptr;

			char* dst = (char*)calloc(ceil(size), 1);

			for(uint32_t i = 0; i < size; i++)
			{
				dst[i] = fgetc(stream);
				if(dst[i] == EOF)
					dst[i] = '\0';
				if(dst[i] == '\0')
					break;
				if(dst[i] == '\\')
					dst[i] = '/';
			}
			if(strlen(dst) == 0)
			{
				free(dst);
				dst = nullptr;
			}
			return dst;
		}

		char* ctime(const int32_t* time)
		{
			time_t t = (time_t)*time;
			char* dst = std::ctime(&t);
			dst[strcspn(dst, "\n")] = '\0';
			return dst;
		}
		int32_t ftime(const char* filename)
		{
			struct stat sb;
			return stat(filename, &sb) != -1 ? (int32_t)sb.st_mtime : -1;
		}
	};

	template<enum version>
	struct archive
	{
		archive(const std::filesystem::path& src)
		{
		}
		~archive() { }

		virtual bool extract(const std::filesystem::path& dst, size_t i = -1) = 0;
		virtual std::vector<uint8_t> get(size_t i) = 0;
		virtual size_t find(const std::filesystem::path& name) = 0;
	};

        /* POD1 Style types */
	template<>
	struct archive<pod1>
	{
		struct header
		{
			uint32_t       entry_count;
			char           comment[80];
		};
		struct entry
		{
			char              name[32];
			uint32_t              size;
			uint32_t            offset;
		};
	};
	template<>
	struct archive<epd1>
	{
		struct header
		{
			char              ident[4];
			char          comment[256];
			uint32_t       entry_count;
			uint32_t           version;
			uint32_t          checksum;
		};

		struct entry
		{
			char              name[64];
			uint32_t              size;
			uint32_t            offset;
			uint32_t         timestamp;
			uint32_t          checksum;
		};
	};
	template<>
	struct archive<epd2>
	{
		using header = archive<epd1>::header;
		using entry  = archive<epd1>::entry;
	};
	template<>
	struct archive<pod2>
	{
		struct header
		{
			char              ident[4];
			uint32_t          checksum;
			char           comment[80];
			uint32_t       entry_count;
			uint32_t       audit_count;
		};

		struct entry
		{
			uint32_t      names_offset;
			uint32_t              size;
			uint32_t            offset;
			int32_t          timestamp;
			uint32_t          checksum;
		};
	};
	template<>
	struct archive<pod6>
	{
		struct header
		{
			char              ident[4];
			uint32_t       entry_count;
			uint32_t           version;
			uint32_t      entry_offset;
			uint32_t       names_count;
		};
		struct entry
		{
			uint32_t      names_offset;
			uint32_t              size;
			uint32_t            offset;
			uint32_t      uncompressed;
			uint32_t compression_level;
			uint32_t              zero;
		};
	};
        /* POD3 Style types */
	template<>
	struct archive<pod3>
	{
		struct header
		{
			char              ident[4];
			uint32_t          checksum;
			char           comment[80];
			uint32_t       entry_count;
			uint32_t       audit_count;
			uint32_t          revision;
			uint32_t          priority;
			char            author[80];
			char         copyright[80];
			uint32_t      entry_offset;
			uint32_t         entry_crc;
			uint32_t        names_size;
			uint32_t     depends_count;
			uint32_t       depends_crc;
			uint32_t         audit_crc;
              constexpr uint32_t names_offset() { return entry_offset + entry_count * sizeof(struct entry); }
	      constexpr uint32_t audit_offset() { return names_size + names_offset() + depends_count * sizeof(struct depend); }
		};
		struct extra_header : header
		{
			uint32_t            pad120;
		};
		struct entry
		{
			uint32_t      names_offset;
			uint32_t              size;
			uint32_t            offset;
			int32_t          timestamp;
			uint32_t          checksum;
		};
		struct depend
		{
			uint8_t       unknown[264];
		};
	};
	template<>
	struct archive<pod4>
	{
		using header       = archive<pod3>::header;
		using extra_header = archive<pod3>::extra_header;

		struct entry
		{
			uint32_t       path_offset;
			uint32_t   compressed_size;
			uint32_t            offset;
			uint32_t uncompressed_size;
			uint32_t compression_level;
			int32_t          timestamp;
			uint32_t          checksum;
		};
	};
	template<>
	struct archive<pod5>
	{
		struct header : archive<pod3>::header
		{
			uint8_t   next_archive[80];
		};
		struct extra_header : header
		{
			uint32_t            pad124;
		};
		using entry = archive<pod4>::entry;
	};
};

