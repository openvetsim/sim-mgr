TARGETS= obj/simmgr obj/simpulse obj/simstatus.cgi

## -pthread (Posix Threads) is required where shared memory and/or multiple threads are used
CFLAGS=-pthread -Wall -g -ggdb

## librt is required by any C/C++ where shared memory and/or multiple threads are used
LDFLAGS=-lrt

## CGIBIN must be set for the location of the cgi-bin directory on your server
CGIBIN=/var/lib/cgi-bin
BIN=/usr/local/bin

default: obj/simstatus.cgi obj/simmgr obj/simpulse

obj/simstatus.cgi: src/simstatus.cpp src/sim-util.o include/simmgr.h
	g++ $(CPPFLAGS) $(CFLAGS) -o obj/simstatus.cgi src/simstatus.cpp src/sim-util.c $(LDFLAGS) -lcgicc

obj/simmgr: src/simmgr.cpp src/sim-util.c include/simmgr.h
	g++ $(CPPFLAGS) $(CFLAGS)  -lcgicc -o obj/simmgr src/simmgr.cpp  src/sim-util.c $(LDFLAGS)
	
obj/simpulse: src/simpulse.cpp src/sim-util.c include/simmgr.h
	g++ $(CPPFLAGS) $(CFLAGS)  -lcgicc -o obj/simpulse src/simpulse.cpp  src/sim-util.c $(LDFLAGS)
	
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
