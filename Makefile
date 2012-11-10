all: sandbox

sandbox: sandbox.c
	gcc -Wall --ansi -o sandbox sandbox.c

clean:
	rm -f sandbox
