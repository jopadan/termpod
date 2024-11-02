# termpod
Single-header C++ library for CRC-32/MPEG-2 checksummed POD{1,2,3,4,5,6}/EPD archive file formats used in Terminal Reality game engines

## Dependencies

Included as submodule:

- [libzippp](http://github.com/ctabin/libzippp)
- [stdtype](http://github.com/jopadan/stdtype)
- [crcle](http://github.com/jopadan/crcle)

## Status

**DONE**

- identified POD checksum ranges
- podorgana example to list and extract checksummed POD3 file contents

**TODO**

- clean up library interface and add other POD formats
- identify missing header fields in POD3-6
- reverse engineer CPod, CPodFile and CDemonPod used in BloodRayne
- implement compression used in POD4/5
- update [termpod Wiki](https://github.com/jopadan/termpod/wiki)

## Build & Run

```sh
cmake . --install-prefix=/usr && make install
./podorgana [-a] [-x] english.pod
```

## Usage

```cpp
/* enable audit log output */
tr::pod::type::audit::visible = true;

/* loop through POD3 file arguments */
while(optind < argc)
{
    /* create and parse pod 3 file */
    tr::pod3::file src(argv[optind++]);

    /* extract all entries */
    if(extract)
        for(size_t i = 0; i < src.entries.size(); i++)
		    src[i].extract();
}
```

## References
[Related Software](https://github.com/termpod/wiki/Related_Software)
