all:../build/YRSSF
	
../build/YRSSF:main.cpp key.hpp.gch rwmutex.hpp.gch
	g++ main.cpp -pthread ./leveldb/out-static/libleveldb.a ./leveldb/out-static/libmemenv.a ./lua/src/liblua.a ./lwan/build/common/liblwan.a ./zlib/libz.a ./aes/aes.o ./sqlite/.libs/libsqlite3.a -ldl -I ./lua/src -I ./lwan/common -I ./leveldb/include -I ./zlib -I ./aes -I ./ -std=c++0x -ljpeg -lssl -lcrypto -o ../build/YRSSF
key.hpp.gch : key.hpp
	g++ key.hpp
rwmutex.hpp.gch : rwmutex.hpp
	g++ rwmutex.hpp