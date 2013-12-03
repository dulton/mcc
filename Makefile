OBJECTS=moduleinfo.o \
		keyboardmanager.o \
		matrixmanager.o \
	    serialport.o	
MODULES=testkeyboard.so \
		matrixjxj.so \
		keyboardjxj.so


CC=g++
TARGETS=test libmcc.so libmcc.a

CPPFLAGS=-g -I./libet -I./include

LDFLAGS=-L./lib -lgtest_main -lpthread -llog4cplus -ltinyxml -let -ldl -Wl,-rpath ./lib


all:$(TARGETS) modules

$(OBJECTS):%.o:%.cpp %.h
	$(CC) -c $(CPPFLAGS) $< -o $@ 

test:test.cc $(OBJECTS)
	$(CC) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY:modules
modules: $(OBJECTS)
	cd modules && make

#$(MODULES):%.so:%.cpp $(OBJECTS)
#	$(CC) $(CPPFLAGS) -shared -fPIC -o $@ $^ $(LDFLAGS)

libmcc.so:$(OBJECTS)
	$(CC) $(CPPFLAGS) -shared -fPIC -o $@ $^ $(LDFLAGS)

libmcc.a:$(OBJECTS)
	$(AR) rv $@ $^ 

clean:
	cd modules && make clean
	rm -rf $(OBJECTS) core $(TARGETS) 
