MODULES=matrixjxj.so \
		keyboardjxj.so \
		matrixhoneywell.so \
		matrixflmix.so \
		matrixcz08.so \
		keyboardhoneywell.so


CC=g++
TARGETS=test modules

CPPFLAGS=-g -I../libet -I../include -I../

LDFLAGS=-L../lib -lgtest_main -lpthread -llog4cplus -ltinyxml -let -ldl 

all:$(MODULES) 

$(MODULES):%.so:%.cpp $(OBJECTS)
	$(CC) $(CPPFLAGS) -shared -fPIC -o $@ ../*.o $^ $(LDFLAGS)

clean:
	rm -rf $(MODULES) 
