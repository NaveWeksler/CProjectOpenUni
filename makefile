test:
	rm main.o || true
	gcc *.c */*.c -c -Wall -g -ansi -pedantic
	gcc *.o -g -o main.o

	

	