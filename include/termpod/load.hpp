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
*/
};

