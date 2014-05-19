all:
	mkdir -p bin
	g++ -Wall -o bin/opensnap-quicktile src/opensnap.cpp -lX11 `pkg-config gtk+-3.0 --cflags` `pkg-config gtk+-3.0 --libs` -Os
clean:
	rm bin/opensnap-quicktile
