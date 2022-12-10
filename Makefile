main: main.c day*.c vec.h aoc.h point.h
	gcc -o main main.c -lcurl

run: main
	./main $(day)
