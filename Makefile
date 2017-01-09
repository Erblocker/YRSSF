all:
	cd core/leveldb    && make
	mkdir core/lwan/build
	cd core/lwan/build && cmake ../ && make
	cd core/lua        && make linux
	cd core/zlib       && make
	cd core/sqlite     && make
clean:
	cd core/leveldb    && make clean
	cd core/lwan/build && make clean
	rm -rf core/lwan/build
	cd core/lua        && make clean
	cd core/zlib       && make clean
	cd core/sqlite     && make clean
	rm core/YRSSF
YRSSF-linux:
	cd core && make
lock:
	