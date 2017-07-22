all:build/launcher build/YRSSF build/live-client
	
clean:
	-rm build/YRSSF
	-rm build/launcher
	-rm build/daemon
	-rm build/live-client
	cd core && make clean
build/YRSSF:YRSSF-linux
	
build/live-client:
	cd live && make
build/launcher:
	cd launcher        && make
YRSSF-linux:
	cd core && make
build/lock:
	cd lock && make
atulocher:
	git clone http://git.oschina.net/cgoxopx/atulocher
	cd atulocher && make