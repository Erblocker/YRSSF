all:../build/lock
	
../build/lock:main.cpp
	g++ main.cpp -static -pthread -std=c++0x -o ../build/lock
hook:hook.cpp
	g++ hook.cpp -fPIC -shared -o hook.so
clean:
	-rm hook.so
	-rm ../build/lock