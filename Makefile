main: main.c
	mkdir -p build
	gcc -o build/main main.c -lcurl

run: main
	./build/main
