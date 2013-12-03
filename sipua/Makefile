CFLAGS=-Wall -g
LDFLAGS+=`pkg-config --cflags --libs libpjproject` -lpthread

sipua: sipua.c
	gcc $(CFLAGS) sipua.c -o sipua $(LDFLAGS)

clean:
	rm -rf sip
