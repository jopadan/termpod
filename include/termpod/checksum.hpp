#pragma once

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

namespace tr::pod
{

enum section
{
     none   = 0,
     file   = 1,
     header = 2,
     extra  = 3,
     entry  = 4,
     depend = 5,
     audit  = 6,
};

const std::pair<u32<1>, enum section> range[last] =
{
     { 0u                                                                             , section::none   },
     { sizeof(tr::archive<pod1>::header)                                              , section::file   },
     { sizeof(tr::archive<pod2>::header::ident) + sizeof(tr::pod<2>::header::checksum), section::file   },
     { sizeof(tr::archive<pod3>::header::ident) + sizeof(tr::pod<3>::header::checksum), section::header },
     { sizeof(tr::archive<pod4>::header::ident) + sizeof(tr::pod<4>::header::checksum), section::header },
     { sizeof(tr::archive<pod5>::header::ident) + sizeof(tr::pod<5>::header::checksum), section::header },
     { sizeof(tr::archive<pod6>::header)                                              , section::file   },
     { sizeof(tr::archive<epd1>::header)                                              , section::file   },
     { sizeof(tr::archive<epd2>::header)                                              , section::file   },
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
               return sizeof(tr::archive<version>::header);
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
constexpr inline u32<1> checksum(uint8_t* buf, size_t size)
{
     return crc32::mpeg2::compute(buf + section_offset<version>(), section_size<version>(size) - section_offset<version>());
}

};

