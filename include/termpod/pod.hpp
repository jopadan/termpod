#pragma once
#include <cstdint>
#include <cstddef>
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
#include <crcle/crcle.hpp>

namespace tr::pod
{
	/* pod version */
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
	/* string manipulation */
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
	/* audit types */
	namespace audit
	{
		bool visible = false;
		enum class action
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
			char             name[256];
			int32_t      old_timestamp;
			uint32_t          old_size;
			int32_t      new_timestamp;
			uint32_t          new_size;
		};

		std::pair<enum action, const char*> str[3] = { { action::add, "          Add" }, { action::rem, "       Remove" }, { action::chg, "       Change" } };
		const char* print(const struct entry& src)
		{
			static char dst[1024] = { '\0' };
			sprintf(dst, "[AUD] %s %.8X %.8X %s %s\n[AUD] %s %.8X %.8X %13u %s\n[AUD] %s %.8X %.8X %13u %s\n", string::ctime(&src.timestamp), -1, -1, str[(size_t)src.action].second, src.user, string::ctime(&src.old_timestamp), -1, -1, src.old_size, src.name, string::ctime(&src.new_timestamp), -1, -1, src.new_size, src.name);
			return visible ? (const char*)dst : "";
		}
	};

	/* pod archive types */
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
		struct depend
		{
			uint8_t       unknown[264];
		};
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
	};
	template<>
	struct archive<pod4>
	{
		using header       = archive<pod3>::header;
		using extra_header = archive<pod3>::extra_header;

		struct entry
		{
			uint32_t      names_offset;
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

	/* checksum types */
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

	/*
	checksum: crc32::mpeg2
	============================================================================
	| POD2 : offset=sizeof(ident)+sizeof(checksum) count=sizeof(file  )-offset |
	| POD3 : offset=sizeof(ident)+sizeof(checksum) count=sizeof(header)-offset |
	| POD4 : offset=sizeof(ident)+sizeof(checksum) count=sizeof(header)-offset |
	| POD5 : offset=sizeof(ident)+sizeof(checksum) count=sizeof(header)-offset |
	| EPD  : offset=sizeof(header)                 count=sizeof(file  )-offset |
	============================================================================
	checksum: none
	============================================================================
	| POD6 : offset=sizeof(header)                 count=sizeof(file  )-offset |
	| POD1 : offset=sizeof(header)=4+80            count=sizeof(file  )-offset |
	============================================================================
	*/
	const std::pair<uint32_t, enum section> range[last] =
	{
		{ 0u                                                                            , section::none   },
		{ sizeof(archive<pod1>::header)                                                 , section::file   },
		{ sizeof(archive<pod2>::header::ident) + sizeof(archive<pod2>::header::checksum), section::file   },
		{ sizeof(archive<pod3>::header::ident) + sizeof(archive<pod3>::header::checksum), section::header },
		{ sizeof(archive<pod4>::header::ident) + sizeof(archive<pod4>::header::checksum), section::header },
		{ sizeof(archive<pod5>::header::ident) + sizeof(archive<pod5>::header::checksum), section::header },
		{ sizeof(archive<pod6>::header)                                                 , section::file   },
		{ sizeof(archive<epd>::header)                                                  , section::file   },
		{ sizeof(archive<epd1>::header)                                                 , section::file   },
		{ sizeof(archive<epd2>::header)                                                 , section::file   },
	};

	template<enum version version>
	constexpr inline size_t section_size(size_t size)
	{
		switch(range[version].second)
		{
			case section::entry:
			case section::file:
				return size;
			case section::header:
				return sizeof(archive<version>::header);
			case section::none:
			default:
				return 0;
		}
	}

	template<enum version version>
	constexpr inline size_t section_offset()
	{
		switch(range[version].second)
		{
			case section::header:
			case section::file:
				return range[version].first;
			case section::entry:
			case section::none:
			default:
				return 0;
		}
	}

	template<enum version version>
	constexpr inline uint32_t checksum(uint8_t* buf, size_t size)
	{
		return crc32::mpeg2::compute(buf + section_offset<version>(), section_size<version>(size) - section_offset<version>());
	}

	/* file entry */
	struct entry
	{
		char*        name;
		int32_t timestamp;
		uint32_t checksum;
		uint32_t   offset;
		uint32_t     size;
		uint8_t*     data;
		entry() : name(nullptr), timestamp(-1), checksum(-1), size(0), data(nullptr) { }
		~entry() { free(data); }
		bool extract(std::filesystem::path dst = ".")
		{
			const std::filesystem::path od = dst / name;
			std::filesystem::create_directories(od.parent_path());
			FILE* of = fopen(od.c_str(), "wb");
			if(of == nullptr)
				return false;
			size_t written = fwrite(data, size, 1, of);
			fclose(of);
			if(written != 1)
			{
				fprintf(stderr, "[ERR] extracting %s\n", od.c_str());
				return false;
			}
			struct utimbuf ot = { (time_t)timestamp, (time_t)timestamp };
			return (utime(od.c_str(), &ot) == 0);
		}
	};
	struct file
	{
		std::filesystem::path name;
		uint32_t size;
		int32_t timestamp;
		uint32_t checksum;

		struct pod::archive<pod3>::header* header;

		std::vector<struct pod::entry> entries;
		std::vector<struct pod::archive<pod3>::depend> depends;
		std::vector<struct pod::audit::entry> audits;

		file() : size(0), checksum(-1), header(nullptr) { }
		file(std::filesystem::path filename) : name(filename), size(std::filesystem::file_size(filename)), checksum(-1), header(nullptr)
		{
			if(verify_file())
			{
				header = (struct pod::archive<pod3>::header*)calloc(sizeof(struct pod::archive<pod3>::header), 1);

				FILE* fp = fopen(name.c_str(), "rb");
				if(fread((uint8_t*)header, sizeof(struct pod::archive<pod3>::header), 1, fp) == 1 && verify_header())
				{
					fseek(fp, header->entry_offset, SEEK_SET);
					struct entry* dict = (struct entry*)calloc(header->entry_count, sizeof(struct entry));
					if(fread(dict, sizeof(struct entry), header->entry_count,fp) != header->entry_count)
					{
						free(dict);
						fprintf(stderr, "[ERR] Could not read entries!\n");
						header->entry_count = 0;
					}
					long base = ftell(fp);
					fseek(fp, header->audit_offset(), SEEK_CUR);
					struct pod::audit::entry* audit_dict = (pod::audit::entry*)calloc(header->audit_count, sizeof(pod::audit::entry));
					ssize_t error = 0;
					if((error = fread(audit_dict, sizeof(pod::audit::entry), header->audit_count, fp)) != header->audit_count)
					{
						free(audit_dict);
						fprintf(stderr, "[ERR] Could not read audit entries: %zu/%zu!\n", error, header->audit_count);
						header->audit_count = 0;
					}
					entries.resize(header->entry_count);
					for(uint32_t i = 0; i < header->entry_count; i++)
					{
						fseek(fp, base + header->names_offset(), SEEK_SET);
						entries[i].name = strdup(pod::string::fgets(256, fp));
						entries[i].timestamp = dict[i].timestamp;
						entries[i].checksum  = dict[i].checksum;
						entries[i].offset    = dict[i].offset;
						entries[i].size      = dict[i].size;
						entries[i].data = (uint8_t*)calloc(dict[i].size, 1);
						fseek(fp, dict[i].offset, SEEK_SET);
						if(fread(entries[i].data, dict[i].size, 1, fp) != 1)
						{
							fprintf(stderr, "[ERR] Could not read entry %u %s of size %u at offset %u!\n", i, entries[i].name, entries[i].size, entries[i].offset);
							entries[i].size = 0;
							continue;
						}
					}
					audits.resize(header->audit_count);
					for(uint32_t i = 0;i < header->audit_count; i++)
						memcpy(&audits[i], &audit_dict[i], sizeof(struct pod::audit::entry));
					if(dict != nullptr)
						free(dict);
					if(audit_dict != nullptr)
						free(audit_dict);
				}
				fclose(fp);
				if(entries.size() > 0)
					verify_entries();
				print();
			}
		}
		void print()
		{
			printf("[NFO] %s checksum   offset          size name\n\n", pod::string::ctime(&timestamp));
			for(uint32_t i = 0; i < entries.size(); i++)
				printf("[ENT] %s %.8X %.8X %13u %s\n", pod::string::ctime(&entries[i].timestamp), entries[i].checksum, entries[i].offset, entries[i].size, entries[i].name);
			if(pod::audit::visible)
				for(uint32_t i = 0; i < audits.size(); i++)
					printf("%s\n", pod::audit::print(audits[i]));
			printf("\n[HDR] %s %.8X %.8X %13zu %s %s %s %s\n", pod::string::ctime(&timestamp), header->checksum, 0, sizeof(struct pod::archive<pod3>::header), pod::ident[pod::id(header->ident)].first, header->comment, header->author, header->copyright);
			printf("[FLE] %s %.8X %.8X %13u %s\n", pod::string::ctime(&timestamp), checksum, 0, size, name.c_str());
			printf("[CNT] %s %.8X %.8X %13zu %zu\n", pod::string::ctime(&timestamp), -1, -1, entries.size(), audits.size());
		}
		~file() { if(header != nullptr) free(header); }
		bool verify_file()
		{
			checksum = -1;
			if(std::filesystem::exists(name) && size > 0)
			{
				timestamp = pod::string::ftime(name.c_str());
				FILE* fp = fopen(name.c_str(), "rb");
				uint8_t* buf = (uint8_t*)calloc(size, 1);
				if(fread(buf, size, 1, fp) == 1)
					checksum = crc32::mpeg2::compute(buf, size);
				fclose(fp);
				free(buf);
			}
			if(checksum == (uint32_t)-1)
			{
				fprintf(stderr, "[ERR] file %s does not exist or is empty\n", name.c_str());
			}
			return checksum != (uint32_t)-1;
		}
		bool verify_header()
		{
			uint32_t chksum = -1;
			chksum = crc32::mpeg2::compute((uint8_t*)header + 8, sizeof(struct pod::archive<pod3>::header) - 8);
			if(chksum != header->checksum)
				fprintf(stderr, "[ERR] CRC-32/MPEG-2 checksum verification failed for %s\n", name.c_str());
			return chksum == header->checksum;
		}
		bool verify_entries()
		{
			for(uint32_t i=0; i < entries.size(); i++)
			{
				uint32_t chksum = crc32::mpeg2::compute(entries[i].data, entries[i].size);
				if(chksum != entries[i].checksum)
				{
					fprintf(stderr, "[ERR] CRC-32/MPEG-2 checksum verification failed for %s of size %u in %s\n", entries[i].name, entries[i].size, name.c_str());
					return false;
				}
			}
			return true;
		}
		struct pod::entry& operator[](uint32_t i) { return entries[i]; }
	};
};

