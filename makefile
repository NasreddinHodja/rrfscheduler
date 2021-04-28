CC=+gcc

scheduler: scheduler.c
	$(CC) $(CFLAGS) scheduler.c -o scheduler
	./scheduler
