virtualmem: virtualmem.o
	gcc -o virtualmem virtualmem.o

virtualmem.o: virtualmem.c virtualmem.h
	gcc -c -Wall virtualmem.c

