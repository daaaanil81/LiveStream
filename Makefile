ACE_INCLUDE_PATH = -I/Volumes/ACE_BUILD/ACE_wrappers
ACE_LIB_PATH 	 = -L/Volumes/ACE_BUILD/ACE_wrappers/ace
LIBS 		 = $(ACE_LIB_PATH) -lACE -lssl -lm -lz -lcrypto  -pthread -lwebsockets
FLAGS		 = -ggdb -Wall $(ACE_INCLUDE_PATH) 
BIN		 = ws-server
FILES		 = 
CPP		 = cpp

compile:
	g++ $(FLAGS) $(BIN).$(CPP) $(FILES) -o $(BIN) $(LIBS)
	install_name_tool -change @rpath/libACE.dylib /Volumes/ACE_BUILD/ACE_wrappers/ace/libACE.dylib $(BIN)
clean:
	rm $(BIN)
	rm -rf $(BIN).dSYM
