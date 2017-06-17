all:../build/live-client
	
../build/live-client : main.cpp shot.hpp
	g++ main.cpp -static \
	 -Wl,-Bdynamic,-lSDL -o ../build/live-client
android : main.cpp shot.hpp
	g++ main.cpp -static \
	 -Wl,-dynamic-linker=/system/lib/ld-linux.so.3 \
	 -Wl,-Bdynamic,-lSDL -o ../build/live-client