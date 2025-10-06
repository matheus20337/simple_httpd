TARGET := hyperboreattp
CFLAGS := -Wextra -Wall -Werror
LFLAGS :=

hyperboreattp: main.o server.o utils.o request.o router.o
	gcc $(CFLAGS) $^ $(LFLAGS) -o $(TARGET)

main.o: src/main.c src/server.h
	gcc -c $(CFLAGS) $< $(LFLAGS)

server.o: src/server.c src/server.h src/utils.h src/request.h
	gcc -c $(CFLAGS) $< $(LFLAGS)

request.o: src/request.c src/request.h
	gcc -c $(CFLAGS) $< $(LFLAGS)

utils.o: src/utils.c
	gcc -c $(CFLAGS) $< $(LFLAGS)

router.o: src/router.c src/server.h 
	gcc -c $(CFLAGS) $< $(LFLAGS)

.PHONY: clean

clean:
	rm -v main.o server.o utils.o request.o $(TARGET)
