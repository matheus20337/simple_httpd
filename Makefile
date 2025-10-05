TARGET := hyperboreattp
CFLAGS := -Wextra -Wall -Werror
LFLAGS :=

hyperboreattp: main.o
	gcc $(CFLAGS) main.o $(LFLAGS) -o $(TARGET)

main.o: src/main.c
	gcc -c $(CFLAGS) $< $(LFLAGS)


.PHONY: clean

clean:
	rm -v main.o $(TARGET)
