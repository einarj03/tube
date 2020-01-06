tube: main.o tube.o
	g++ -Wall -g main.o tube.o -o tube

main.o: main.cpp tube.h
	g++ -Wall -c main.cpp

tube.o: tube.cpp tube.h
	g++ -Wall -c tube.cpp