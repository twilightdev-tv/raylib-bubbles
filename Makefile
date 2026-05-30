all:
	$(CC) ./main.c -lraylib -o bubbles

run: all
	./bubbles
