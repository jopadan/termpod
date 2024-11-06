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
		char* gets(char* buf)
		{
			uint32_t size = strnlen(buf, 264);
			if(size == 0)
				return nullptr;

			char* dst = (char*)calloc(ceil(size), 1);

			for(uint32_t i = 0; i < size; i++)
			{
				dst[i] = buf[i];
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
	struct archive;

	template<enum version version>
	constexpr inline uint32_t checksum(const uint8_t* buf, size_t size);

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
	namespace depend
	{
		struct entry
		{
			uint8_t       unknown[264];
		};
	};
	template<>
	struct archive<pod3>
	{
		struct header
		{
			/* 0x0000 */ char          ident[4];
			/* 0x0004 */ uint32_t      checksum;
			/* 0x0008 */ char       comment[80]; 
			/* 0x0058 */ uint32_t   entry_count;
			/* 0x005c */ uint32_t   audit_count;
			/* 0x0060 */ uint32_t      revision;
			/* 0x0064 */ uint32_t      priority;
			/* 0x0068 */ char        author[80];
			/* 0x00B8 */ char     copyright[80];
			/* 0x0108 */ uint32_t  entry_offset;
			/* 0x010c */ uint32_t     entry_crc;
			/* 0x0110 */ uint32_t    names_size;
			/* 0x0114 */ uint32_t depends_count;
			/* 0x0118 */ uint32_t   depends_crc;
			/* 0x011c */ uint32_t    audits_crc;
			constexpr inline uint32_t checksum_offset(          ) { return sizeof(checksum) + sizeof(ident); }
			constexpr inline uint32_t  entries_offset(          ) { return entry_offset; }
			constexpr inline uint32_t    names_offset(          ) { return entry_offset + entry_count * sizeof(struct entry); }
			constexpr inline uint32_t  depends_offset(          ) { return names_size + names_offset(); }
			constexpr inline uint32_t   audits_offset(          ) { return depends_offset() + depends_count * sizeof(struct depend::entry); }
			constexpr inline  bool checksum_verify(uint8_t* buf) { return    checksum == crc32::mpeg2::compute(&buf[checksum_offset()], sizeof(struct header) - sizeof(checksum) - sizeof(ident)); } 
			constexpr inline  bool  entries_verify(uint8_t* buf) { return   entry_crc == crc32::mpeg2::compute(&buf[ entries_offset()],   entry_count * sizeof(struct entry)); }
			constexpr inline  bool  depends_verify(uint8_t* buf) { return depends_crc == crc32::mpeg2::compute(&buf[ depends_offset()], depends_count * sizeof(struct depend::entry)); }
			constexpr inline  bool   audits_verify(uint8_t* buf) { return  audits_crc == crc32::mpeg2::compute(&buf[  audits_offset()],   audit_count * sizeof(struct  audit::entry)); }
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
		constexpr static bool verify(const uint8_t* buf, size_t len)
		{
			uint32_t checksum = (uint32_t)-1;
			if(id((const char*)buf) == pod3)
				checksum = pod::checksum<pod3>(buf, len);
			if(checksum != (uint32_t)-1)
				return *(uint32_t*)(buf + 4) == checksum;
			return false;
		}
		constexpr static bool verify(std::filesystem::path src)
		{
			FILE* fi     = nullptr;
			size_t len  = 0;
			uint8_t* buf = nullptr;
			bool status = false;
			if(std::filesystem::exists(src) && ((len = std::filesystem::file_size(src)) > 0))
			{
				if(((fi = fopen(src.c_str(), "rb")) != nullptr))
				{
					if(((buf = (uint8_t*)std::malloc(len)) != nullptr) && fread(buf, len, 1, fi) == 1)
					{
						status = verify(buf, len);
						free(buf);
					}
					fclose(fi);
				}
			}
			if(!status) fprintf(stderr, "Error opening/reading file %s\n", src.c_str());
			return status;
		}
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
		{ sizeof(struct archive<pod1>::header)                                          , section::file   },
		{ sizeof(archive<pod2>::header::ident) + sizeof(archive<pod2>::header::checksum), section::file   },
		{ sizeof(archive<pod3>::header::ident) + sizeof(archive<pod3>::header::checksum), section::header },
		{ sizeof(archive<pod4>::header::ident) + sizeof(archive<pod4>::header::checksum), section::header },
		{ sizeof(archive<pod5>::header::ident) + sizeof(archive<pod5>::header::checksum), section::header },
		{ sizeof(struct archive<pod6>::header)                                          , section::file   },
		{ sizeof(struct archive<epd>::header)                                           , section::file   },
		{ sizeof(struct archive<epd1>::header)                                          , section::file   },
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
				return sizeof(struct archive<version>::header);
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
	constexpr inline uint32_t checksum(const uint8_t* buf, size_t size)
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
		~entry() { if(name) free(name); }
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
		uint32_t checksum;
		int32_t timestamp;
		std::vector<uint8_t> data;
		FILE* fp;

		archive<pod3>::header* hdr;
		std::vector<struct pod::entry> entries;

		file() : size(0), checksum(-1), hdr(nullptr) { }
		file(std::filesystem::path filename) : name(filename), size(std::filesystem::file_size(filename)), checksum(-1), hdr(nullptr)
		{
			switch(verify_file())
			{
				case pod1:
					break;
				case pod2:
					break;
				case epd1:
				case epd2:
					break;
				case pod6:
					break;
				case pod3:
					{
						hdr = reinterpret_cast<archive<pod3>::header*>(&data[0]);
						entries.resize(hdr->entry_count);
						archive<pod3>::entry* dict = reinterpret_cast<archive<pod3>::entry*>(&data[hdr->entry_offset]);

						if(hdr->checksum_verify(&data[0]) && hdr->entries_verify(&data[0]) && hdr->depends_verify(&data[0]) && hdr->audits_verify(&data[0]))
							printf("[ERR] POD3/4/5 checksum verification failed for %s!\n", name.c_str());

						for(uint32_t i = 0; i < hdr->entry_count; i++)
						{
							entries[i].name      = pod::string::gets(reinterpret_cast<char*>(&data[hdr->names_offset() + dict[i].names_offset]));
							entries[i].offset    = dict[i].offset;
							entries[i].timestamp = dict[i].timestamp;
							entries[i].checksum  = dict[i].checksum;
							entries[i].size      = dict[i].size;
							entries[i].data      = &data[entries[i].offset];
						}
						print();
					}
					break;
				case pod4:
					break;
				case pod5:
					break;
				case none:
				default:
					break;
			}
		}
		constexpr void print()
		{
			printf("[NFO] %s checksum   offset          size name\n\n", pod::string::ctime(&timestamp));
			for(uint32_t i = 0; i < entries.size(); i++)
				printf("[ENT] %s %.8X %.8X %13u %s\n", pod::string::ctime(&entries[i].timestamp), entries[i].checksum, entries[i].offset, entries[i].size, entries[i].name);
			if(pod::audit::visible)
				for(uint32_t i = 0; i < hdr->audit_count; i++)
					printf("%s\n", pod::audit::print(reinterpret_cast<struct audit::entry*>(&data[hdr->audits_offset()])[i]));
			printf("\n[HDR] %s %.8X %.8X %13zu %s %s %s %s\n", pod::string::ctime(&timestamp), hdr->checksum, 0, sizeof(struct pod::archive<pod3>::header), pod::ident[pod::id(hdr->ident)].first, hdr->comment, hdr->author, hdr->copyright);
			printf("[FLE] %s %.8X %.8X %13u %s\n", pod::string::ctime(&timestamp), checksum, 0, size, name.c_str());
			printf("[CNT] %s %.8X %.8X %13u %u\n\n\n", pod::string::ctime(&timestamp), -1, -1, (uint32_t)entries.size(), hdr->audit_count);
		}
		~file() {  }
		enum version verify_file()
		{
			checksum             = -1;
			enum version version = version::none;
			if(std::filesystem::exists(name) && size > 0)
			{
				timestamp = pod::string::ftime(name.c_str());
				data.resize(size);
				if((fp = fopen(name.c_str(), "rb")) != nullptr)
				{
					if(fread(&data[0], size, 1, fp) == 1)
					{
						version = id((const char*)&data[0]);
						checksum = pod::archive<pod3>::verify(&data[0], size);
					}
					fclose(fp);
				}
				return version;
			}
			fprintf(stderr, "[ERR] file %s does not exist or is empty\n", name.c_str());
			return version;
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
/*
	struct pod : std::vector<uint8_t>
	{

		struct header_pod1& header_pod1() { return * (struct header_pod1*)(*this).data(); }
		struct entry_pod1&   entry_pod1(number i) { return * (struct  entry_pod1*)((*this).data() + sizeof(struct header_pod1) + i * sizeof(struct entry_pod1)); }
		
		std::vector<uint8_t> read(struct entry_pod1& src) { return std::vector<uint8_t>((*this).begin() + src.offset, (*this).begin() + src.offset + src.size); }
		bool write(struct entry_pod1& src)
		{
			std::filesystem::path dst(src.name.data());
			std::filesystem::create_directories(dst.parent_path());
			FILE* fo = fopen(dst.c_str(), "wb");
			if(fo)
			{
				fwrite((*this).data() + src.offset, src.size, 1, fo);
				fclose(fo);
				return true;
			}
			return false;
		}
		bool extract()
		{
			for(number i = 0; i < header_pod1().count; i++)
				if(!write(entry_pod1(i))) return false;
			return true;
		}
		void list()
		{
			number data_size = 0;
			for(number i = 0; i < header_pod1().count; i++)
			{
				data_size += entry_pod1(i).size;
				printf("%13u %32s\n", entry_pod1(i).size, entry_pod1(i).name.data());
			}
			printf("\n%13u %32u\n", data_size, header_pod1().count);
		}
	};
*/

