
src = $(wildcard src/*.cc)
c_src = $(wildcard src/*.c)
obj = $(src:.cc=.o) $(c_src:.c=.o)
dep = $(obj:.o=.d)
bin = faros

dbg = -g
opt = -O0
inc = -Isrc -Isrc/shaders -Isrc/math

CXX = g++
CC = gcc
CXXFLAGS = -pedantic -Wall $(dbg) $(opt) $(inc)
CFLAGS = $(CXXFLAGS)
LDFLAGS = -lGL -lGLU -lglut -lGLEW -limago -ltreestore

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.cc
	@$(CPP) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(bin) $(dep)
