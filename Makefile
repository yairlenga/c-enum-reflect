default: all

CFLAGS = -g -Wall -Werror

all: enum_test1

enum_test1.o: enum_desc.h enum_refl.h enum_desc_def.h

enum_test1: enum_test1.o enum_reflect.o

clean:
	rm -f *.o enum_test1
