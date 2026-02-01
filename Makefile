default: all

CFLAGS = -g -Wall -Werror

all: enum_test1

enum_test1.o: enum_reflect.h

enum_test1: enum_test1.o enum_reflect.o

clean:
	rm -f *.o enum_test1
