#pragma once
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <array>
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace tr
{
	struct tdf
	{
		enum logic : uint8_t
		{
			invisible = 0,
			visible   = 1,
			not_boss  = 2,
		};

		struct entry
		{
			std::filesystem::path          lvl;
			struct
			{
				std::array<int32_t, 3> pos;
				logic                  vis;
				std::filesystem::path  tex[2];
			} entrance;
			struct
			{
				std::array<int32_t, 3> pos;
				logic                  vis;
				std::filesystem::path  tex[2];
			} exit;
			bool                           chamber_exit;
		};
		std::filesystem::path name;
		std::vector<struct entry> entries;

		tdf(std::filesystem::path src)
		{
		        char buf[3][2][32] = { '\0' };
			uint8_t chamber = 0;
			uint32_t count  = 0;
			bool error = false;
			name = src;
			FILE* fp = fopen(name.c_str(), "r");
			if(fp != NULL)
			{
				if(fscanf(fp, "%u\n", &count) != 1)
				{
				   std::cerr << "ERROR: reading TDF file entries count" << name << std::endl;
				   entries.clear();
				}
				else
				{
					entries.resize(count);
					for(size_t i = 0; i < entries.size(); i++)
					{
						if(fscanf(fp, "%32s\n", buf[0][0]) != 1)
						   std::cerr << "ERROR: reading TDF file entry level " << i << " " << name << std::endl;
						else if(fscanf(fp, "%d,%d,%d\n", &entries[i].entrance.pos[0], &entries[i].entrance.pos[1], &entries[i].entrance.pos[2]) != 3)
						   std::cerr << "ERROR: reading TDF file entry entrance position " << i << " " << name << std::endl;
						else if(fscanf(fp, "%d,%d,%d\n", &entries[i].exit.pos[0], &entries[i].exit.pos[1], &entries[i].exit.pos[2]) != 3)
						   std::cerr << "ERROR: reading TDF file entry exit position " << i << " " << name << std::endl;
						else if(fscanf(fp, "%hhu\n", (uint8_t*)&entries[i].entrance.vis) != 1)
						   std::cerr << "ERROR: reading TDF file entry entrance visibility " << i << " " << name << std::endl;
						else if(fscanf(fp, "%32s\n", buf[1][0]) != 1)
						   std::cerr << "ERROR: reading TDF file entry entrance texture[0] " << i << " " << name << std::endl;
						else if(fscanf(fp, "%32s\n", buf[1][1]) != 1)
						   std::cerr << "ERROR: reading TDF file entry entrance texture[1]" << i << " " << name << std::endl;
						else if(fscanf(fp, "%hhu\n", (uint8_t*)&entries[i].exit.vis) != 1)
						   std::cerr << "ERROR: reading TDF file entry exit visibility " << i << " " << name << std::endl;
						else if(fscanf(fp, "%32s\n", buf[2][0]) != 1)
						   std::cerr << "ERROR: reading TDF file entry exit texture[0] " << i << " " << name << std::endl;
						else if(fscanf(fp, "%32s\n", buf[2][1]) != 1)
						   std::cerr << "ERROR: reading TDF file entry exit texture[1] " << i << " " << name << std::endl;
						else if(fscanf(fp, "%hhu\n", (uint8_t*)&chamber) != 1)
						   std::cerr << "ERROR: reading TDF file entry chamber exit " << i << " " << name << std::endl;
						else
						{
							entries[i].chamber_exit = chamber != 0u ? true : false;
							for(size_t i = 0; i < 3; i++)
							{
								std::replace(buf[i][0], buf[i][0] + strlen(buf[i][0]), '\\', '/');
								std::replace(buf[i][1], buf[i][1] + strlen(buf[i][1]), '\\', '/');
							}
							entries[i].lvl = buf[0][0];
							entries[i].entrance.tex[0] = buf[1][0];
							entries[i].entrance.tex[1] = buf[1][1];
							entries[i].exit.tex[0] = buf[2][0];
							entries[i].exit.tex[1] = buf[2][1];
							continue;
						}
						std::cerr << "ERROR: reading TDF file entry " << i << " " << name << std::endl;
						entries.clear();
						break;
					}
				}
				fclose(fp);
			}
		}
		void print()
		{
			for(size_t i = 0; i < entries.size(); i++)
			{
				std::cout << "   level.nme: " << entries[i].lvl << std::endl;
				printf("entrance.pos: [%d,%d,%d]\n", entries[i].entrance.pos[0], entries[i].entrance.pos[1], entries[i].entrance.pos[2]);
				printf("    exit.pos: [%d,%d,%d]\n", entries[i].exit.pos[0], entries[i].exit.pos[1], entries[i].exit.pos[2]);
				std::cout << "entrance.tex: " << entries[i].entrance.tex[0] << ", " << entries[i].entrance.tex[1] << std::endl;
				std::cout << "    exit.tex: " << entries[i].exit.tex[0] << ", " << entries[i].exit.tex[1] << std::endl;
				std::cout << std::endl;
			}
		}
		bool write(std::filesystem::path dst)
		{
			FILE* fo = fopen(dst.c_str(), "w");
			if(fo)
			{
				fprintf(fo, "%zu\n", entries.size());
				for(size_t i = 0; i < entries.size(); i++)
				{
					fprintf(fo, "%s\n", entries[i].lvl.c_str());
					fprintf(fo, "%d,%d,%d\n", entries[i].entrance.pos[0], entries[i].entrance.pos[1], entries[i].entrance.pos[2]);
					fprintf(fo, "%d,%d,%d\n", entries[i].exit.pos[0], entries[i].exit.pos[1], entries[i].exit.pos[2]);
					fprintf(fo, "%hhu\n", entries[i].entrance.vis);
					fprintf(fo, "%s\n", entries[i].entrance.tex[0].c_str());
					fprintf(fo, "%s\n", entries[i].entrance.tex[1].c_str());
					fprintf(fo, "%hhu\n", entries[i].exit.vis);
					fprintf(fo, "%s\n", entries[i].exit.tex[0].c_str());
					fprintf(fo, "%s\n", entries[i].exit.tex[1].c_str());
					fprintf(fo, "%hhu\n", !entries[i].chamber_exit ? 0u : 1u);
				}
			}
			fflush(fo);
			fclose(fo);
			return true;
		}
		~tdf()
		{
		}
	};
};

