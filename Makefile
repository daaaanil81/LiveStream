export ACE_ROOT=/home/daaaanil81/ACE/ACE_wrappers

FILES=ws-server
CPP=cpp
OBJ=o

compile:
	g++ -ggdb -Wall  $(FILES).$(CPP) -o $(FILES) -lACE
