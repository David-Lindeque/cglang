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

ifeq ($(OS),Windows_NT)
    # CCFLAGS += -D WIN32
    # ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
    #     CCFLAGS += -D AMD64
    # else
    #     ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
    #         CCFLAGS += -D AMD64
    #     endif
    #     ifeq ($(PROCESSOR_ARCHITECTURE),x86)
    #         CCFLAGS += -D IA32
    #     endif
    # endif
	CC = cc.exe
	FLEX = flex.exe
	BISON = bison.exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
		CC = g++ -std=c++14
		BISON = bison
		FLEX = flex
        # CCFLAGS += -D LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
		# CC = clang++ -stdlib=libc++ -I/usr/include -std=c++14 -g
		CC = g++ -std=c++14
		BISON = /usr/local/opt/bison/bin/bison
		FLEX = /usr/local/opt/flex/bin/flex
        # CCFLAGS += -D OSX
    endif
    # UNAME_P := $(shell uname -p)
    # ifeq ($(UNAME_P),x86_64)
    #     CCFLAGS += -D AMD64
    # endif
    # ifneq ($(filter %86,$(UNAME_P)),)
    #     CCFLAGS += -D IA32
    # endif
    # ifneq ($(filter arm%,$(UNAME_P)),)
    #     CCFLAGS += -D ARM
    # endif
endif

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