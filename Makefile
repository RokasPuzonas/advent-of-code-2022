main: main.c day*.c vec.h aoc.h
	gcc -o main main.c -lcurl

run: main
	./main $(day)
