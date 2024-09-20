#pragma once
#include <stdtype/stdtype.hpp>
#include <crcle/crcle.hpp>
#include <sys/stat.h>
#include <utime.h>
#include "libzippp.h"

using namespace libzippp;

namespace tr
{
	namespace pod
	{
		namespace string
		{
			u32<1> ceil(u32<1> size)
			{
				static const std::array<u32<1>,12> sizes = { 4, 8, 12, 16, 32, 48, 64, 80, 96, 128, 256, 264 };
				std::array<u32<1>,12>::const_iterator dst = sizes.begin();
				while(size > *dst) dst++;
				return *dst;
			}
			str fgets(u32<1> size, FILE* stream)
			{
				if(size == 0)
					return nullptr;

				str dst = (str)calloc(ceil(size), 1);

				for(u32<1> i = 0; i < size; i++)
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

			str ctime(const t32<1>* time)
			{
				time_t t = (time_t)*time;
				str dst = std::ctime(&t);
				dst[strcspn(dst, "\n")] = '\0';
				return dst;
			}
			t32<1> ftime(const char* filename)
			{
				struct stat sb;
				return stat(filename, &sb) != -1 ? (t32<1>)sb.st_mtime : -1;
			}
		};
		namespace type
		{

			using cmt     = c8<80>;

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
				last = 8,
			};

			const std::pair<const char*, const char*> ident[last] =
			{
				{"NONE", "\0NONE"},
				{"POD1", "\0POD1"},
				{"POD2", "POD2\0"},
				{"POD3", "POD3\0"},
				{"POD4", "POD4\0"},
				{"POD5", "POD5\0"},
				{"POD6", "POD6\0"},
				{"EPD" , "dtxe\0"}
			};

			enum version verify(const c8<4> magic)
			{
				for(int i = pod2; i < last; i++)
					if(strncmp(ident[i].second, &magic[0], 4) == 0)
						return (enum version)i;
				return pod1;
			}

			struct entry
			{
				str name;
				t32<1> timestamp;
				u32<1> checksum;
				u32<1> offset;
				u32<1> size;
				buf    data;
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

			/* audit entry types */
			namespace audit
			{
				bool visible = false;
				enum act : u32<1>
				{
					add = 0,
					rem = 1,
					chg = 2,
				};
				std::pair<enum act, const char*> str[3] = { { add, "          Add" }, { rem, "       Remove" }, { chg, "       Change" } };
				struct entry
				{
					c8<32>  user;
					t32<1>  timestamp;
					act     action;
					c8<256> path;
					t32<1>  old_timestamp;
					u32<1>  old_size;
					t32<1>  new_timestamp;
					u32<1>  new_size;
				};
				const char* print(const struct entry& src)
				{
					static char dst[1024] = { '\0' };
					sprintf(dst, "[AUD] %s %.8X %.8X %s %s\n[AUD] %s %.8X %.8X %13u %s\n[AUD] %s %.8X %.8X %13u %s\n", string::ctime(&src.timestamp), -1, -1, str[src.action].second, src.user, string::ctime(&src.old_timestamp), -1, -1, src.old_size, src.path, string::ctime(&src.new_timestamp), -1, -1, src.new_size, src.path);
					return visible ? (const char*)dst : "";
				}
			};
		};


	};
	using namespace pod;
	namespace pod3
	{
		struct header
		{
			c8<4>  ident;
			u32<1> checksum;
			c8<80> comment;
			u32<1> entry_count;
			u32<1> audit_count;
			u32<1> revision;
			u32<1> priority;
			c8<80> author;
			c8<80> copyright;
			u32<1> entry_offset;
			u32<1> pad10c;
			u32<1> audit_offset;
			u32<1> neutral_element;
			u32<1> inverse_element;
			u32<1> pad11c;
		};
		struct entry
		{
			u32<1> path_offset;
			u32<1> size;
			u32<1> offset;
			t32<1> timestamp;
			u32<1> checksum;
		};
		struct file
		{
			std::filesystem::path name;
			u32<1> size;
			t32<1> timestamp;
			u32<1> checksum;

			struct header* header;

			std::vector<struct pod::type::entry> entries;
			std::vector<struct pod::type::audit::entry> audits;

			file() : size(0), checksum(-1), header(nullptr) { }
			file(std::filesystem::path filename) : name(filename), size(std::filesystem::file_size(filename)), checksum(-1), header(nullptr)
			{
				if(verify_file())
				{
					header = (struct header*)calloc(sizeof(struct header), 1);

					FILE* fp = fopen(name.c_str(), "rb");
					if(fread((uint8_t*)header, sizeof(struct header), 1, fp) == 1 && verify_header())
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
						fseek(fp, header->audit_offset, SEEK_CUR);
						struct pod::type::audit::entry* audit_dict = (pod::type::audit::entry*)calloc(header->audit_count, sizeof(pod::type::audit::entry));
						if(fread(audit_dict, sizeof(pod::type::audit::entry), header->audit_count, fp) != header->audit_count)
						{
							 free(audit_dict);
							 fprintf(stderr, "[ERR] Could not read audit entries!\n");
							 header->audit_count = 0;
						}
						entries.resize(header->entry_count);
						for(u32<1> i = 0; i < header->entry_count; i++)
						{
							fseek(fp, base + dict[i].path_offset, SEEK_SET);
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
						for(u32<1> i = 0;i < header->audit_count; i++)
							memcpy(&audits[i], &audit_dict[i], sizeof(struct pod::type::audit::entry));
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
				for(u32<1> i = 0; i < entries.size(); i++)
					printf("[ENT] %s %.8X %.8X %13u %s\n", pod::string::ctime(&entries[i].timestamp), entries[i].checksum, entries[i].offset, entries[i].size, entries[i].name);
				if(pod::type::audit::visible)
					for(u32<1> i = 0; i < audits.size(); i++)
						printf("%s\n", pod::type::audit::print(audits[i]));
				printf("\n[HDR] %s %.8X %.8X %13zu %s %s %s %s\n", pod::string::ctime(&timestamp), header->checksum, 0, sizeof(struct header), pod::type::ident[pod::type::verify(header->ident)].first, header->comment, header->author, header->copyright);
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
				if(checksum == (u32<1>)-1)
					fprintf(stderr, "[ERR] file %s does not exist or is empty\n", name.c_str());
				return checksum != (u32<1>)-1;
			}
			bool verify_header()
			{
				u32<1> chksum = -1;
				chksum = crc32::mpeg2::compute((uint8_t*)header + 8, sizeof(struct header) - 8);
				if(chksum != header->checksum)
					fprintf(stderr, "[ERR] CRC-32/MPEG-2 checksum verification failed for %s\n", name.c_str());
				return chksum == header->checksum;
			}
			bool verify_entries()
			{
				for(u32<1> i=0; i < entries.size(); i++)
				{
					u32<1> chksum = crc32::mpeg2::compute(entries[i].data, entries[i].size);
					if(chksum != entries[i].checksum)
					{
						fprintf(stderr, "[ERR] CRC-32/MPEG-2 checksum verification failed for %s of size %u in %s\n", entries[i].name, entries[i].size, name.c_str());
						return false;
					}
				}
				return true;
			}
			struct pod::type::entry& operator[](u32<1> i) { return entries[i]; }
		};
	};

	using namespace pod;
	namespace pod6
	{
		struct header
		{
			c8<4> ident;
			u32<1> entry_count;
			u32<1> version;
			u32<1> entry_offset;
			u32<1> path_count;
		};
		struct entry
		{
			u32<1> path_offset;
			u32<1> size;
			u32<1> offset;
			u32<1> uncompressed;
			u32<1> compression_level;
			u32<1> zero;
		};
		struct file
		{
			struct header header;
			buf contents;
			std::vector<struct entry> entries;
			str paths;
		};
	};
/* TODO implement other formats */
	namespace epd
	{
		struct header
		{
			c8<4> ident;
			c8<256> comment;
			u32<1> entry_count;
			u32<1> version;
			u32<1> checksum;
		};

		struct entry
		{
			c8<64> name;
			u32<1>  size;
			u32<1>  offset;
			u32<1>  timestamp;
			u32<1>  checksum;
		};
	};
	namespace pod1
	{
		struct header
		{
			u32<1> entry_count;
			c8<80> comment;
		};
		struct entry
		{
			c8<1> name[32];
			u32<1> size;
			u32<1> offset;
		};

		struct file : std::vector<uint8_t>
		{
			struct header* header;
			struct entry* entries;
			u32<1> checksum;
			u32<1> data_size;
			u32<1> data_start;

			file(std::filesystem::path filename)
			{
				if(std::filesystem::exists(filename))
				{
					(*this).resize(std::filesystem::file_size(filename));
					header = (struct header*)(*this).data();
					entries = (struct entry*)(*this).data() + sizeof(struct header);
					FILE* fp = fopen(filename.c_str(), "rb");
					data_size = fread((*this).data(), (*this).size(), 1, fp) == 1 ? (*this).size() - header->entry_count * sizeof(struct entry) : -1;
					data_start = data_size != (u32<1>)-1 ? (*this).size() - data_size : -1;
					fclose(fp);
				}
			}
			bool verify(uint8_t* buf)
			{
				checksum = crc32::mpeg2::compute((*this).data() + sizeof(struct header), (*this).size() - sizeof(struct header));
				return true;
			}

			uint8_t* operator[](uint32_t i)
			{
				uint8_t* pos = (*this).data() + entries[i].offset;
				return pos;
			}
		};
	};
	namespace pod2
	{
		struct header
		{
			c8<4>  ident;
			u32<1> checksum;
			c8<80> comment;
			u32<1> entry_count;
			u32<1> audit_count;
		};

		struct entry
		{
			u32<1> path_offset;
			u32<1> size;
			u32<1> offset;
			t32<1> timestamp;
			u32<1> checksum;
		};

		struct file
		{
			std::filesystem::path name;
			u32<1> size;
			u32<1> checksum;

			struct header* header;

			std::vector<struct pod::type::entry> entries;
			std::vector<struct pod::type::audit::entry> audits;

			file() : size(0), checksum(-1), header(nullptr) { }
			file(std::filesystem::path filename) : name(filename), size(std::filesystem::file_size(filename)), checksum(-1), header(nullptr)
			{
				if(verify_file())
				{
					header = (struct header*)calloc(sizeof(struct header), 1);

					FILE* fp = fopen(name.c_str(), "rb");
					if(fread((uint8_t*)header, sizeof(struct header), 1, fp) == 1 && verify_header())
					{
						long base = ftell(fp);
						struct entry* dict = (struct entry*)calloc(header->entry_count, sizeof(struct entry));
						fread(dict, header->entry_count, sizeof(struct entry),fp);
						struct pod::type::audit::entry* audit_dict = (pod::type::audit::entry*)calloc(header->audit_count, sizeof(pod::type::audit::entry));
						fread(audit_dict, header->audit_count, sizeof(pod::type::audit::entry),fp);
						entries.resize(header->entry_count);
						for(u32<1> i = 0; i < header->entry_count; i++)
						{
							fseek(fp, base + dict[i].path_offset, SEEK_SET);
							entries[i].name = pod::string::fgets(256, fp);
							entries[i].timestamp = dict[i].timestamp;
							entries[i].checksum  = dict[i].checksum;
							entries[i].size      = dict[i].size;
							entries[i].data = (uint8_t*)calloc(dict[i].size, 1);
							fseek(fp, dict[i].offset, SEEK_SET);
							fread(entries[i].data, dict[i].size, 1, fp);
						}
						free(dict);
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
				for(u32<1> i = 0; i < entries.size(); i++)
					printf("[CHK]      %.8X %13u %s\n", entries[i].checksum, entries[i].size, entries[i].name);
				printf("\n[CHK]      %.8X %13zu %s %s\n", header->checksum, sizeof(struct header), pod::type::ident[pod::type::verify(header->ident)].first, header->comment);
				printf("[CHK]      %.8X %13u %s\n", checksum, size, name.c_str());
				printf("[CNT] %13zu %13zu\n", entries.size(), audits.size());
			}
			~file() { if(header != nullptr) free(header); }
			bool verify_file()
			{
				checksum = -1;
				if(std::filesystem::exists(name) && size > 0)
				{
					FILE* fp = fopen(name.c_str(), "rb");
					uint8_t* buf = (uint8_t*)calloc(size, 1);
					if(fread(buf, size, 1, fp) == 1)
						checksum = crc32::mpeg2::compute(buf, size);
					fclose(fp);
					free(buf);
				}
				if(checksum == (u32<1>)-1)
					fprintf(stderr, "[ERR] file %s does not exist or is empty\n", name.c_str());
				return checksum != (u32<1>)-1;
			}
			bool verify_header()
			{
				u32<1> chksum = -1;
				if(std::filesystem::exists(name) && size > 0)
				{
					FILE* fp = fopen(name.c_str(), "rb");
					uint8_t* buf = (uint8_t*)calloc(size, 1);
					if(fread(buf, size, 1, fp) == 1)
						checksum = crc32::mpeg2::compute(buf + 8, size -8);
					fclose(fp);
					free(buf);
			 	}
				if(chksum != header->checksum)
					fprintf(stderr, "[ERR] CRC-32/MPEG-2 checksum verification failed for %s\n", name.c_str());
				return chksum == header->checksum;
			}
			bool verify_entries()
			{
				for(u32<1> i=0; i < entries.size(); i++)
				{
					u32<1> chksum = crc32::mpeg2::compute(entries[i].data, entries[i].size);
					if(chksum != entries[i].checksum)
					{
						fprintf(stderr, "[ERR] CRC-32/MPEG-2 checksum verification failed for %s of size %u in %s\n", entries[i].name, entries[i].size, name.c_str());
						return false;
					}
				}
				return true;
			}
			struct pod::type::entry& operator[](u32<1> i) { return entries[i]; }
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
};

