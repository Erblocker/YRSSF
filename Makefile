all:
	cd core/leveldb    && make
	cmake              core/lwan
	cd core/lwan       && make
	cd core/lua        && make linux
	cd core/zlib       && make
	cd core/sqlite     && make
clean:
	cd core/leveldb    && make clean
	cd core/lwan       && make clean
	cd core/lua        && make clean
	cd core/zlib       && make clean
	cd core/sqlite     && make clean
	rm core/YRSSF
YRSSF-linux:
	cd core && make
lock:
	