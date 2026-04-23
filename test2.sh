#! /bin/sh -uex
CFLAGS="-Iapi/include"
gcc $CFLAGS -c -g test2.c 
./build/dwarf-query/enum_dwarf_query --format=c enum_2.o > enum_test2.c
gcc $CFLAGS -c -g enum_test2.c -Iapi/include
gcc $CFLAGS -g -o test2.exe test2.c enum_test2.c api/src/enum_desc.c -Iapi/include
./test2.exe
