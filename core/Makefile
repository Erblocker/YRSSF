all:../build/YRSSF
	
../build/YRSSF:\
    classes.hpp.gch       \
    client.hpp.gch        \
    crypt.hpp.gch         \
    daemon.hpp.gch        \
    global.hpp.gch        \
    func.hpp.gch          \
    live.hpp.gch          \
    nint.hpp.gch          \
    server-base.hpp.gch   \
    server.hpp.gch        \
    webserver.hpp.gch     \
    ysdb.hpp.gch          \
    ysconnection.hpp.gch  \
    key.hpp.gch           \
    rwmutex.hpp.gch       \
    base64.hpp.gch        \
    main.cpp              \
    sqlite/sqlite3.o      \
    ./zlib/libz.a         \
    ./aes/aes.o           \
    ./lua/src/liblua.a    \
    ./leveldb/out-static/libleveldb.a \
    ./lwan/build/common/liblwan.a
	g++ main.cpp -pthread ./leveldb/out-static/libleveldb.a ./leveldb/out-static/libmemenv.a ./lua/src/liblua.a ./lwan/build/common/liblwan.a ./zlib/libz.a ./aes/aes.o ./sqlite/sqlite3.o -ldl -I ./lua/src -I ./lwan/common -I ./leveldb/include -I ./zlib -I ./aes -I ./ -std=c++0x -ljpeg -lssl -lcrypto -o ../build/YRSSF
./zlib/libz.a :
	cd zlib       && make
./lwan/build/common/liblwan.a:
	-mkdir core/lwan/build
	cd core/lwan/build && cmake ../   && make	
./leveldb/out-static/libleveldb.a :
	cd leveldb    && make
./aes/aes.o :
	cd aes   && make
./lua/src/liblua.a :
	cd lua && make linux
sqlite/sqlite3.o : sqlite/sqlite3.c
	cd sqlite && gcc sqlite3.c -c
key.hpp.gch : key.hpp
	g++ key.hpp
rwmutex.hpp.gch : rwmutex.hpp
	g++ rwmutex.hpp
base64.hpp.gch : base64.hpp
	g++ base64.hpp
lwan-build-config.h.gch : lwan-build-config.h
	g++ lwan-build-config.h
classes.hpp.gch : classes.hpp
	g++ classes.hpp -std=c++0x
client.hpp.gch : client.hpp
	g++ client.hpp -I ./leveldb/include -std=c++0x
crypt.hpp.gch : crypt.hpp
	g++ crypt.hpp -std=c++0x
daemon.hpp.gch : daemon.hpp
	g++ daemon.hpp -I ./leveldb/include -std=c++0x
func.hpp.gch : func.hpp
	g++ func.hpp -std=c++0x
global.hpp.gch : global.hpp
	g++ global.hpp -std=c++0x
live.hpp.gch : live.hpp
	g++ live.hpp -I ./leveldb/include -std=c++0x
nint.hpp.gch : nint.hpp
	g++ nint.hpp -std=c++0x
server-base.hpp.gch : server-base.hpp
	g++ server-base.hpp -std=c++0x
server.hpp.gch : server.hpp
	g++ server.hpp -I ./leveldb/include -std=c++0x
webserver.hpp.gch : webserver.hpp
	g++ webserver.hpp -I ./lwan/common -I ./leveldb/include -std=c++0x
ysconnection.hpp.gch : ysconnection.hpp
	g++ ysconnection.hpp -I ./leveldb/include -std=c++0x
ysdb.hpp.gch : ysdb.hpp
	g++ ysdb.hpp -I ./leveldb/include -std=c++0x
clean:
	-rm *.gch
	-rm ../build/YRSSF
	-rm sqlite/sqlite3.o
	cd zlib    && make clean
	cd aes     && make clean
	cd lua     && make clean
	cd leveldb && make clean
	-rm -rf lwan/build