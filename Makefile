TARGET := hyperboreattp
CFLAGS := -Wextra -Wall -Werror
LFLAGS :=

hyperboreattp: main.o server.o utils.o
	gcc $(CFLAGS) $^ $(LFLAGS) -o $(TARGET)

main.o: src/main.c
	gcc -c $(CFLAGS) $< $(LFLAGS)

server.o: src/server.c
	gcc -c $(CFLAGS) $< $(LFLAGS)

utils.o: src/utils.c
	gcc -c $(CFLAGS) $< $(LFLAGS)

.PHONY: clean

clean:
	rm -v main.o server.o utils.o $(TARGET)
