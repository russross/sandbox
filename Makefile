all: sandbox

sandbox: sandbox.c
	gcc -static -Wall --ansi -o sandbox sandbox.c

clean:
	rm -f sandbox
