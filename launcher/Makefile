all:../build/launcher ../build/daemon
	
../build/launcher : launcher.cpp common.hpp
	g++ launcher.cpp -o ../build/launcher -static
../build/daemon   : daemon.cpp   common.hpp
	g++ daemon.cpp -o  ../build/daemon -static