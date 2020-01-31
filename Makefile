CC=g++ -I/usr/local/Cellar/boost/1.69.0/include
CFLAGS=-Wall -std=c++11 -I./src -g

FILES=$(shell find ./src -type f -name "*.cpp" | grep -v "^./src/tools"|grep -v "^./src/zlib")
FILES_OBJECTS=${FILES:.cpp=.o}

# TOOLS_NAMES = $(shell find ./src/tools -type f -name "*.cpp" | sed 's/\.\/src\/tools\///g' | sed 's/\.cpp//g')
# TOOLS_NAMES = zt snap clst_ahc ncd revqs
TOOLS_NAMES = snap zt qs
TOOLS=${TOOLS_NAMES:%=./src/tools/%.cpp}
TOOLS_OBJECTS=${TOOLS_NAMES:%=./src/tools/%.o}
TOOLS_BINARIES=${TOOLS_NAMES:%=./bin/%}

.PHONY: all
all: $(TOOLS_BINARIES)

.PHONY: clean
clean:
	find ./src -path ./src/alglib -prune -o -name "*.o" -exec rm {} +

$(TOOLS_OBJECTS): $(TOOLS)
	$(CC) $(CFLAGS) -c $(subst .o,.cpp,$@) -o $@


$(TOOLS_BINARIES): $(FILES_OBJECTS) $(TOOLS_OBJECTS)
	$(CC) $(CFLAGS) $(FILES_OBJECTS) $(addsuffix .o, $(subst bin,src/tools,$@)) -o $@ -lz3 -lz

%.o: %.cpp
	$(CC) $(CFLAGS) -c ./$< -o $@
