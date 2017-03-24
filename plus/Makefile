all: ../build/sqlfilter.so
	
../build/sqlfilter.so : sqlfilter.c
	gcc sqlfilter.c ../core/lua/src/liblua.a -fPIC -shared -I ../core -o ../build/sqlfilter.so
clean:
	-rm ../build/sqlfilter.so