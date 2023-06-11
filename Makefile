main: main.c day*.c vec.h aoc.h vec2.h types.h
	gcc -o main main.c -lcurl

run: main
	./main $(day)
