# termpod
Single-header C++ library for CRC-32/MPEG-2 checksummed POD{1,2,3,4,5,6}/EPD archive file formats used in Terminal Reality game engines

## Dependencies

Included as submodule:

- [libzippp](http://github.com/ctabin/libzippp)
- [crcle](http://github.com/jopadan/crcle)

## Status

**DONE**

- identified POD checksum ranges
- podorgana example to list and extract checksummed POD3 file contents

**TODO**

- clean up library interface and add other POD formats
- identify extra_header uint32_t field and its use
- identify dependency data structure of length 264
- implement compression used in POD4/5
- update [termpod Wiki](https://github.com/jopadan/termpod/wiki)

## Build & Run

```sh
cmake . --install-prefix=/usr && make install
```
```sh
podorgana [-a] [-x] world.pod

[NFO] Wed Apr 10 21:56:31 2024 checksum   offset          size name

[ENT] Sat Aug  3 20:24:24 2002 697DCAE2 00000120           480 DATA/HEINRICH.SLT
[ENT] Tue May 28 21:10:32 2002 26FF5785 00000300           480 DATA/MASTER_SOLDIER.SLT
[ENT] Thu Jun 27 17:25:20 2002 CF1B24FF 000004E0          2528 DATA/SVETLANA.SLT
[ENT] Thu Jul  4 16:27:30 2002 79B89A95 00000EC0           486 DATA/TWIN.SLT
[ENT] Thu Aug 22 01:44:32 2002 E6346ACD 000010A6           180 MODELS/BUTCHERESS.JUG
[ENT] Thu Aug 22 01:48:52 2002 7D90C6A6 0000115A           182 MODELS/MENTOR.JUG
[ENT] Tue Aug 27 00:04:48 2002 AE4800DF 00001210          4052 MODELS/RAYNE.JUG
[ENT] Thu Aug  1 00:29:10 2002 397BA6B2 000021E4          1121 MODELS/TWIN.JUG
[ENT] Wed Dec 11 16:25:22 2002 EA38594D 00002645       1228347 WORLD/AR_GH_SUBBAY.MSN
[ENT] Wed Dec 11 16:27:24 2002 78B77CD1 0012E480         37883 WORLD/AR_GH_SUBBAY.SCB

[HDR] Wed Apr 10 21:56:31 2024 FA924A77 00000000           288 POD3 Missions and scripts  
[FLE] Wed Apr 10 21:56:31 2024 70F4A969 00000000       1869532 world.pod
[CNT] Wed Apr 10 21:56:31 2024 FFFFFFFF FFFFFFFF            10 0 1901


```

## Usage

```cpp
/* enable audit log output */
tr::pod::audit::visible = true;

/* loop through POD3 file arguments */
while(optind < argc)
{
    /* extract all entries */
	while(optind < argc)
	{
        /* create and parse pod 3 file */
		tr::pod::file src(argv[optind++]);
		if(extract)
			for(size_t i = 0; i < src.entries.size(); i++)
				src[i].extract();
	}
}
```

## References
[Related Software](https://github.com/jopadan/termpod/wiki/Related-Software)
