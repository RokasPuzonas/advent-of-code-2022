main: main.c
	gcc -o main main.c -lcurl

run: main
	./main $(day)
