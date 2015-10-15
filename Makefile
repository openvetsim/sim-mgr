TARGETS= obj/simmgr obj/simstatus.cgi

## -pthread (Posix Threads) is required where shared memory and/or multiple threads are used
CFLAGS=-pthread -Wall -g -ggdb

## librt is required by any C/C++ where shared memory and/or multiple threads are used
LDFLAGS=-lrt

default: obj/simstatus.cgi obj/simmgr 

obj/simstatus.cgi: src/simstatus.cpp src/sim-util.o include/simmgr.h
	g++ $(CPPFLAGS) $(CFLAGS) -o obj/simstatus.cgi src/simstatus.cpp src/sim-util.c $(LDFLAGS) -lcgicc

obj/simmgr: src/simmgr.cpp src/sim-util.c include/simmgr.h
	g++ $(CPPFLAGS) $(CFLAGS)  -lcgicc -o obj/simmgr src/simmgr.cpp  src/sim-util.c $(LDFLAGS)
	
	
install:
	sudo cp -u obj/simstatus.cgi /var/lib/cgi-bin
	sudo chown simmgr:simmgr /var/lib/cgi-bin/simstatus.cgi
	sudo chmod u+s /var/lib/cgi-bin/simstatus.cgi
	sudo cp -u obj/simmgr /usr/local/bin
	sudo chown root:root /usr/local/bin/simmgr
	sudo chmod u+s /usr/local/bin/simmgr	
	
clean:
	rm obj/*


