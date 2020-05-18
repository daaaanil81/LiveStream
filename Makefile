ACE_INCLUDE_PATH = -I/Volumes/ACE_BUILD/ACE_wrappers
ACE_LIB_PATH 	 = -L/Volumes/ACE_BUILD/ACE_wrappers/ace
LIBS 		 = $(ACE_LIB_PATH) -lACE -lssl -lm -lz -lcrypto  -pthread -lwebsockets
FLAGS		 = -ggdb -Wall $(ACE_INCLUDE_PATH)
FILES		 = ws-server
CPP		 = cpp

compile:
	g++ $(FLAGS) $(FILES).$(CPP) -o $(FILES) $(LIBS) 

clean:
	rm ws-server
