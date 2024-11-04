#pragma once
#include <crcle/crcle.hpp>
#include <sys/stat.h>
#include <utime.h>

namespace tr
{
	};

	namespace pod
	{


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
						for(uint32_t i = 0; i < header->entry_count; i++)
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
				for(uint32_t i = 0; i < entries.size(); i++)
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

