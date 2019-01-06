CC=gcc
CXX=g++
MKDIR=mkdir -p
RM=rm -f
RMDIR=rm -rf

# CPPFLAGS=-g $(shell root-config --cflags)
# LDFLAGS=-g $(shell root-config --ldflags)
# LDLIBS=$(shell root-config --libs)
CPPFLAGS=-std=c++17 -I../../include -I../../PEGTL/include -MMD -Wall -Wno-psabi -Wno-nonnull-compare -fno-delete-null-pointer-checks -Ddebuggable_cast=static_cast -Ddebuggable_pointer_cast=std::static_pointer_cast
LDFLAGS=-L. -Wl,-rpath,. -Wl,-rpath-link,. -Lbuild/gcc -Wl,-rpath,build/gcc
LDLIBS=-Wl,--as-needed -latomic -pthread -Wl,--no-as-needed -ljemalloc

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SOURCES_SHARED=$(wildcard ../../src/Zeni/*.cpp)
SOURCES_CONCURRENCY=$(call rwildcard, ../../ConcRete/src/Zeni/Concurrency, *.cpp) $(SOURCES_SHARED)
SOURCES_RETE=$(call rwildcard, ../../ConcRete/src/Zeni/Rete, *.cpp) $(SOURCES_SHARED)
SOURCES_CARLI=$(call rwildcard, ../../src/carli, *.cpp) $(SOURCES_SHARED)
SOURCES_ADVENT=$(call rwildcard, ../../src/advent, *.cpp) $(SOURCES_SHARED)
SOURCES_BLOCKS_WORLD_2=$(call rwildcard, ../../src/blocks_world_2, *.cpp) $(SOURCES_SHARED)
SOURCES_MOUNTAIN_CAR=$(call rwildcard, ../../src/mountain_car, *.cpp) $(SOURCES_SHARED)
SOURCES_PUDDLE_WORLD=$(call rwildcard, ../../src/puddle_world, *.cpp) $(SOURCES_SHARED)
SOURCES_TAXICAB=$(call rwildcard, ../../src/taxicab, *.cpp) $(SOURCES_SHARED)

OBJS_CONCURRENCY=$(addprefix Concurrency/, $(subst .cpp,.o,$(notdir $(SOURCES_CONCURRENCY))))
OBJS_RETE=$(addprefix Rete/, $(subst .cpp,.o,$(notdir $(SOURCES_RETE))))
OBJS_CARLI=$(addprefix Carli/, $(subst .cpp,.o,$(notdir $(SOURCES_CARLI))))
OBJS_ADVENT=$(addprefix Advent/, $(subst .cpp,.o,$(notdir $(SOURCES_ADVENT))))
OBJS_BLOCKS_WORLD_2=$(addprefix Blocks_World_2/, $(subst .cpp,.o,$(notdir $(SOURCES_BLOCKS_WORLD_2))))
OBJS_MOUNTAIN_CAR=$(addprefix Mountain_Car/, $(subst .cpp,.o,$(notdir $(SOURCES_MOUNTAIN_CAR))))
OBJS_PUDDLE_WORLD=$(addprefix Puddle_World/, $(subst .cpp,.o,$(notdir $(SOURCES_PUDDLE_WORLD))))
OBJS_TAXICAB=$(addprefix Taxicab/, $(subst .cpp,.o,$(notdir $(SOURCES_TAXICAB))))

BINARIES=libConcurrency.so libRete.so libCarli.so ../../advent ../../blocks_world_2 ../../mountain_car ../../puddle_world ../../taxicab
DIRECTORIES=Concurrency Rete Carli Advent Blocks_World_2 Mountain_Car Puddle_World Taxicab
OBJS=$(OBJS_CONCURRENCY) $(OBJS_RETE) $(OBJS_CARLI) $(OBJS_ADVENT) $(OBJS_BLOCKS_WORLD_2) $(OBJS_MOUNTAIN_CAR) $(OBJS_PUDDLE_WORLD) $(OBJS_TAXICAB)
SOURCES=$(call rwildcard, ../../src, *.cpp)
DEPENDENCIES=$(call rwildcard, ., *.d)

debug: CPPFLAGS+=-ggdb -Og
debug: LDFLAGS+=-ggdb -Og
debug: all

release: CPPFLAGS+=-O3 -DNDEBUG
release: all

.PHONY: all clean directories

all: directories libConcurrency.so libRete.so libCarli.so ../../advent ../../blocks_world_2 ../../mountain_car ../../puddle_world ../../taxicab

directories: $(DIRECTORIES)

$(DIRECTORIES):
	$(MKDIR) $(DIRECTORIES)

-include $(DEPENDENCIES)

libConcurrency.so: $(OBJS_CONCURRENCY)
	$(CXX) $(LDFLAGS) -shared -o $@ $(OBJS_CONCURRENCY) $(LDLIBS)

Concurrency/%.o:
	$(CXX) $(CPPFLAGS) -fPIC -o $@ -I../../ConcRete/include -c $(filter %$(subst .o,.cpp,/$(notdir $@)), $(SOURCES_CONCURRENCY))

libRete.so: libConcurrency.so $(OBJS_RETE)
	$(CXX) $(LDFLAGS) -shared -o $@ -lConcurrency $(OBJS_RETE) $(LDLIBS)

Rete/%.o:
	$(CXX) $(CPPFLAGS) -fPIC -o $@ -I../../ConcRete/include -I../../ConcRete/PEGTL/include -c $(filter %$(subst .o,.cpp,/$(notdir $@)), $(SOURCES_RETE))

libCarli.so: $(OBJS_CARLI)
	$(CXX) $(LDFLAGS) -shared -o $@ $(OBJS_CARLI) $(LDLIBS)

Carli/%.o:
	$(CXX) $(CPPFLAGS) -fPIC -o $@ -c $(filter %$(subst .o,.cpp,/$(notdir $@)), $(SOURCES_CARLI))

../../advent: libCarli.so $(OBJS_ADVENT)
	$(CXX) $(LDFLAGS) -o $@ -lCarli $(OBJS_ADVENT) $(LDLIBS)

Advent/%.o:
	$(CXX) $(CPPFLAGS) -o $@ -c -I../../src -I../../src/advent_env $(filter %$(subst .o,.cpp,/$(notdir $@)), $(SOURCES_ADVENT))

../../blocks_world_2: libCarli.so $(OBJS_BLOCKS_WORLD_2)
	$(CXX) $(LDFLAGS) -o $@ -lCarli $(OBJS_BLOCKS_WORLD_2) $(LDLIBS)

Blocks_World_2/%.o:
	$(CXX) $(CPPFLAGS) -o $@ -c -I../../src -I../../src/blocks_world_2_env $(filter %$(subst .o,.cpp,/$(notdir $@)), $(SOURCES_BLOCKS_WORLD_2))

../../mountain_car: libCarli.so $(OBJS_MOUNTAIN_CAR)
	$(CXX) $(LDFLAGS) -o $@ -lCarli $(OBJS_MOUNTAIN_CAR) $(LDLIBS)

Mountain_Car/%.o:
	$(CXX) $(CPPFLAGS) -o $@ -c -I../../src -I../../src/mountain_car_env $(filter %$(subst .o,.cpp,/$(notdir $@)), $(SOURCES_MOUNTAIN_CAR))

../../puddle_world: libCarli.so $(OBJS_PUDDLE_WORLD)
	$(CXX) $(LDFLAGS) -o $@ -lCarli $(OBJS_PUDDLE_WORLD) $(LDLIBS)

Puddle_World/%.o:
	$(CXX) $(CPPFLAGS) -o $@ -c -I../../src -I../../src/puddle_world_env $(filter %$(subst .o,.cpp,/$(notdir $@)), $(SOURCES_PUDDLE_WORLD))

../../taxicab: libCarli.so $(OBJS_TAXICAB)
	$(CXX) $(LDFLAGS) -o $@ -lCarli $(OBJS_TAXICAB) $(LDLIBS)

Taxicab/%.o:
	$(CXX) $(CPPFLAGS) -o $@ -c -I../../src -I../../src/taxicab_env $(filter %$(subst .o,.cpp,/$(notdir $@)), $(SOURCES_TAXICAB))

clean:
	$(RMDIR) $(DIRECTORIES) $(BINARIES) $(DEPENDENCIES)
