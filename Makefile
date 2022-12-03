main: main.c day*.c
	gcc -o main main.c -lcurl

run: main
	./main $(day)
