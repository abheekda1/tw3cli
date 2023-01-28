.PHONY: clean

tw3cli: main.o w3sc.o
	gcc main.o w3sc.o -o tw3cli -lpthread

tw3cli-bak: bak.o w3sc.o
	gcc bak.o w3sc.o -o tw3cli -lpthread

bak.o: src/bak.c
	gcc -c src/bak.c

main.o: src/main.c
	gcc -c src/main.c

w3sc.o: src/w3sc.c src/w3sc.h
	gcc -c src/w3sc.c

clean:
	rm *.o tw3cli