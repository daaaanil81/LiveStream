FLAGS 	= -ggdb -Wall 
FILES	= ws-server
CPP	= cpp
OBJ	= o

compile:
	g++ $(FLAGS) $(FILES).$(CPP) -o $(FILES) -lACE

clean:
	rm ws-server
