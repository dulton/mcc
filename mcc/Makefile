OBJECTS=mccrunner.o

TARGETS=mcc

CPPFLAGS=-g -I../include -I../libet  

LDFLAGS=-L../lib 


all:$(TARGETS)

mcc:$(OBJECTS)
	$(CC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(TARGETS) $(OBJECTS) core

