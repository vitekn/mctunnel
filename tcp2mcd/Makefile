RM = rm -rf

SHARED_INCLUDE_PATH = ../../
PATH_PREFIX = /
TARGET = tcp2mcd

CXXFLAGS = -I$(SHARED_INCLUDE_PATH) -g -Wall -O3 -std=c++0x -fpermissive -DPATH_PREFIX=\"$(PATH_PREFIX)\" -DAPP_NAME=\"$(TARGET)\" 
DEPENDFLAG = -MD
LDFLAGS = -lconfig -L ../../libzet/
LIBS = -pthread -lpthread -lconfig -lm -lrt

SOURCES = $(wildcard *.cpp) $(wildcard ../../libzet/*.cpp)
OBJECTS_DIR = ../objects/
OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	g++ -o $@ $^ $(LIBS)

$(OBJECTS): %.o: %.cpp %.d
	g++ $(CXXFLAGS) -c -o $@ $<

%.d: %.cpp Makefile
	g++ $(CXXFLAGS) -c $(DEPENDFLAG) $< > $@

include $(wildcard *.d)

.PHONY: clean

clean:
	$(RM) *.d $(OBJECTS) $(TARGET) ../../libzet/*.d ../../libzet/*.o *.o *.*~ ../../libzet/*.*~ *~ ../../libzet/*~
