CC      = g++
Options = -std=c++11 -Wall -fexceptions

ifeq ($(OS), Windows_NT)
  # Options += -municode
endif
#-------------------------------------------------------------------------------

Includes   =
Libraries  = 
LibInclude = 
#-------------------------------------------------------------------------------

Version = -DMAJOR_VERSION=1 -DMINOR_VERSION=5

Objects = 

ifeq ($(OS), Windows_NT)
  Resources =
endif

Headers = *.h
#-------------------------------------------------------------------------------

.PHONY: clean all
.SECONDARY:

ifeq ($(OS), Windows_NT)
all: bin/Drill2Gerber.exe
else
all: bin/Drill2Gerber
endif

clean:
	rm -rf obj
	rm -rf bin
#-------------------------------------------------------------------------------

# Binaries

bin/Drill2Gerber: main.cpp main.h
	mkdir -p bin
	$(CC) $(Options) $(Version) $(Includes) $< -s -o $@

bin/Drill2Gerber.exe: main.cpp main.h $(Resources)
	mkdir -p bin
	$(CC) $(Options) $(Version) $(Includes) $< -s $(Resources) -o $@
#-------------------------------------------------------------------------------

# Objects

obj/%.o: %.cpp $(Headers)
	mkdir -p $(@D)
	$(CC) $(Options) $(Version) $(Defines) -c $(Includes) $< -o $@
#-------------------------------------------------------------------------------

# Resources

obj/%.res: %.rc
	mkdir -p $(@D)
	windres.exe -J rc -O coff -i $< -o $@
#-------------------------------------------------------------------------------

