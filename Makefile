TARGETS= obj/simmgr obj/simpulse obj/simstatus.cgi obj/scenario

## -pthread (Posix Threads) is required where shared memory and/or multiple threads are used
CFLAGS=-pthread -Wall -g -ggdb -rdynamic
CXXFLAGS = -pthread -Wall -g -ggdb -rdynamic -std=c++11

## librt is required by any C/C++ where shared memory and/or multiple threads are used
LDFLAGS=-lrt

## CGIBIN must be set for the location of the cgi-bin directory on your server
CGIBIN=/var/lib/cgi-bin
BIN=/usr/local/bin

default: obj/simstatus.cgi obj/simmgr obj/simpulse obj/scenario

obj/scenario: src/scenario.cpp obj/llist.o obj/sim-util.o obj/sim-parse.o obj/llist.o
	g++ $(CPPFLAGS)-I/usr/include/libxml2  $(CXXFLAGS) -o obj/scenario src/scenario.cpp obj/sim-util.o obj/sim-parse.o obj/llist.o $(LDFLAGS) -lxml2
	
obj/tinyxml2.o: src/tinyxml2/tinyxml2.cpp src/tinyxml2/tinyxml2.h
	g++ $(CPPFLAGS) $(CXXFLAGS) -g -c -o obj/tinyxml2.o src/tinyxml2/tinyxml2.cpp

obj/sim-util.o: src/sim-util.c include/simmgr.h
	g++ $(CPPFLAGS) $(CXXFLAGS) -g -c -o obj/sim-util.o src/sim-util.c

obj/sim-parse.o: src/sim-parse.c include/simmgr.h
	g++ $(CPPFLAGS) $(CXXFLAGS) -g -c -o obj/sim-parse.o src/sim-parse.c

obj/sim-log.o: src/sim-log.c include/simmgr.h
	g++ $(CPPFLAGS) $(CXXFLAGS) -g -c -o obj/sim-log.o src/sim-log.c

obj/llist.o: src/llist.c include/llist.h
	g++ $(CPPFLAGS) $(CXXFLAGS) -g -c -o obj/llist.o src/llist.c
	
obj/simstatus.cgi: src/simstatus.cpp obj/sim-util.o obj/sim-parse.o include/simmgr.h
	g++ $(CPPFLAGS) $(CXXFLAGS) -o obj/simstatus.cgi src/simstatus.cpp obj/sim-util.o obj/sim-parse.o $(LDFLAGS) -lcgicc

obj/simmgr: src/simmgr.cpp obj/sim-log.o obj/sim-util.o include/simmgr.h
	g++ $(CPPFLAGS) $(CXXFLAGS)  -lcgicc -o obj/simmgr src/simmgr.cpp  obj/sim-log.o obj/sim-util.o $(LDFLAGS)
	
obj/simpulse: src/simpulse.cpp obj/sim-util.o include/simmgr.h
	g++ $(CPPFLAGS) $(CXXFLAGS)  -lcgicc -o obj/simpulse src/simpulse.cpp  obj/sim-util.o $(LDFLAGS)
	
install: check $(TARGETS) installDaemon
	sudo cp -u obj/simstatus.cgi $(CGIBIN)
	sudo chown simmgr:simmgr $(CGIBIN)/simstatus.cgi
	sudo chmod u+s $(CGIBIN)/simstatus.cgi
	sudo cp -u obj/simmgr $(BIN)
	sudo chown simmgr:simmgr $(BIN)/simmgr
	sudo chmod u+s $(BIN)/simmgr
	sudo cp -u obj/simpulse $(BIN)
	sudo chown simmgr:simmgr $(BIN)/simpulse
	sudo chmod u+s $(BIN)/simpulse
	sudo cp -u obj/scenario $(BIN)
	sudo chown simmgr:simmgr $(BIN)/scenario
	sudo chmod u+s $(BIN)/scenario
	
installDaemon:
	sudo cp -u simmgr.init /etc/init.d/simmgr
	sudo update-rc.d simmgr defaults
	
removeDaemon:
	sudo update-rc.d -f simmgr remove

clean:
	rm obj/*

## Check for the installed tools we require
check:
	@if [ ! -e /usr/bin/g++ ]; then echo "GNU C++ Compiler (g++) is not found in /usr/bin\n"; exit ; fi;
	@if [ ! -e /usr/lib/libcgicc.so ]; then echo "CGICC Library libcgicc.so is not found in /usr/lib\n"; exit ; fi;
