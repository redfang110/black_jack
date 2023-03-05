all: poker

poker: poker.o
	g++ -lncurses poker.o -o poker

poker.o: poker.cpp poker.h
	g++ -Wall -Werror -g poker.cpp -c

run:
	./poker

clean:
	rm -f *.o poker
