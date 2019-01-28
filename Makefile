CC=g++
CFLAGS=-Wall -std=c++11 -I./src -g

FILES=$(shell find ./src -type f -name "*.cpp" | grep -v "^./src/tools")
FILES_OBJECTS=${FILES:.cpp=.o}

# TOOLS_NAMES = $(shell find ./src/tools -type f -name "*.cpp" | sed 's/\.\/src\/tools\///g' | sed 's/\.cpp//g')
TOOLS_NAMES = zt test
TOOLS=${TOOLS_NAMES:%=./src/tools/%.cpp}
TOOLS_OBJECTS=${TOOLS_NAMES:%=./src/tools/%.o}
TOOLS_BINARIES=${TOOLS_NAMES:%=./bin/%}

.PHONY: all
all: $(TOOLS_BINARIES)

.PHONY: clean
clean:
	find ./src -name "*.o" -exec rm {} +

$(TOOLS_OBJECTS): $(TOOLS)
	$(CC) $(CFLAGS) -c $(subst .o,.cpp,$@) -o $@


$(TOOLS_BINARIES): $(FILES_OBJECTS) $(TOOLS_OBJECTS)
	$(CC) $(CFLAGS) $(FILES_OBJECTS) $(addsuffix .o, $(subst bin,src/tools,$@)) -o $@ -lz3

%.o: %.cpp
	$(CC) $(CFLAGS) -c ./$< -o $@
