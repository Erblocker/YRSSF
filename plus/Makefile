all: ../build/std.so
	
../build/std.so : std.c
	gcc std.c ../core/lua/src/liblua.a -fPIC -shared -I ../core -o ../build/std.so
clean:
	-rm ../build/std.so