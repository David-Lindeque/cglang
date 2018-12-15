SOURCES = $(shell find src -name '*.cpp')
HEADERS = $(shell find src -name '*.h')
LFILES = $(shell find src -name '*.l')
YFILES = $(shell find src -name '*.y')
OBJ = ${SOURCES:src/%.cpp=bin/%.o}
OBJ += ${LFILES:src/%.l=bin/%.l.o}
OBJ += ${YFILES:src/%.y=bin/%.y.o}
GENFILES = ${LFILES:src/%.l=bin/%.l.h}
GENFILES += ${LFILES:src/%.l=bin/%.l.cpp}
GENFILES += ${YFILES:src/%.y=bin/%.y.h}
GENFILES += ${YFILES:src/%.y=bin/%.y.cpp}
GENFILES += ${YFILES:src/%.y=bin/%.y.output}

CC = clang++ -stdlib=libc++ -I/usr/include -std=c++14 -g
FLEX = /usr/local/opt/flex/bin/flex
BISON = /usr/local/opt/bison/bin/bison

default: bin/cglang

bin/cglang: $(OBJ)
	$(CC) $^ -o $@

bin/%.o: src/%.cpp $(HEADERS)
	$(CC) -c $< -o $@
	
bin/%.y.cpp: src/%.y
	$(BISON) --defines=$(subst .cpp,.h,$@) --report=all -o $@ $<

bin/%.y.o: bin/%.y.cpp $(HEADERS)
	$(CC) -c $< -o $@

bin/%.l.cpp: src/%.l src/%.y bin/%.y.cpp $(HEADERS)
	$(FLEX) --header-file=$(subst .cpp,.h,$@) -o $@ $<

bin/%.l.o: bin/%.l.cpp $(HEADERS)
	$(CC) -c $< -o $@

bin/feature.lexer.o: src/feature.lexer.cpp bin/feature.y.cpp $(HEADERS)
	$(CC) -c $< -o $@

clean:
	rm -rf bin/cglang $(OBJ) $(GENFILES)