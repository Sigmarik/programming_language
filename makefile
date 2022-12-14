CC = g++

CFLAGS = -I./ -D _DEBUG -ggdb3 -std=c++2a -O0 -Wall -Wextra -Weffc++\
-Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations\
-Wcast-align -Wchar-subscripts -Wconditionally-supported\
-Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral\
-Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op\
-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith\
-Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo\
-Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn\
-Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default\
-Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast\
-Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers\
-Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector\
-fcheck-new\
-fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging\
-fno-omit-frame-pointer -fPIE -fsanitize=address,bool,${strip \
}bounds,enum,float-cast-overflow,float-divide-by-zero,${strip \
}integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,${strip \
}returns-nonnull-attribute,shift,signed-integer-overflow,undefined,${strip \
}unreachable,vla-bound,vptr\
-pie -Wlarger-than=65536 -Wstack-usage=8192

BLD_FOLDER = build
TEST_FOLDER = test
ASSET_FOLDER = assets

FRONT_BLD_NAME = frontend
BACK_BLD_NAME = backend
INVERTER_BLD_NAME = restoration
BLD_VERSION = 0.1
BLD_PLATFORM = linux
BLD_TYPE = dev
BLD_FORMAT = .out

BLD_SUFFIX = _v$(BLD_VERSION)_$(BLD_TYPE)_$(BLD_PLATFORM)$(BLD_FORMAT)
FRONT_BLD_FULL_NAME = $(FRONT_BLD_NAME)$(BLD_SUFFIX)
BACK_BLD_FULL_NAME = $(BACK_BLD_NAME)$(BLD_SUFFIX)
INVERTER_BLD_FULL_NAME = $(INVERTER_BLD_NAME)$(BLD_SUFFIX)

all: asset frontend backend inverter

LIB_OBJECTS = argparser.o logger.o debug.o alloc_tracker.o file_helper.o bin_tree.o speaker.o grammar.o util.o

FRONTEND_OBJECTS = frontend.o frontend_utils.o common_utils.o $(LIB_OBJECTS)
frontend: $(FRONTEND_OBJECTS)
	mkdir -p $(BLD_FOLDER)
	$(CC) $(FRONTEND_OBJECTS) $(CFLAGS) -o $(BLD_FOLDER)/$(FRONT_BLD_FULL_NAME)

BACKEND_OBJECTS = backend.o backend_utils.o common_utils.o $(LIB_OBJECTS)
backend: $(BACKEND_OBJECTS)
	mkdir -p $(BLD_FOLDER)
	$(CC) $(BACKEND_OBJECTS) $(CFLAGS) -o $(BLD_FOLDER)/$(BACK_BLD_FULL_NAME)

INVERTER_OBJECTS = inverter.o inverter_utils.o common_utils.o $(LIB_OBJECTS)
inverter: $(INVERTER_OBJECTS)
	mkdir -p $(BLD_FOLDER)
	$(CC) $(INVERTER_OBJECTS) $(CFLAGS) -o $(BLD_FOLDER)/$(INVERTER_BLD_FULL_NAME)

asset:
	mkdir -p $(BLD_FOLDER)
	cp -r $(ASSET_FOLDER)/. $(BLD_FOLDER)

test: asset tree binary execute restore

execute:
	cd build && ./assembler.out bin.instr
	cd build && ./processor.out a.bin -I0

run: asset
	cd $(BLD_FOLDER) && exec ./$(FRONT_BLD_FULL_NAME) $(FILE)
	cd $(BLD_FOLDER) && exec ./$(BACK_BLD_FULL_NAME)
	make execute

tree:
	cd $(BLD_FOLDER) && exec ./$(FRONT_BLD_FULL_NAME) $(ARGS)

binary:
	cd $(BLD_FOLDER) && exec ./$(BACK_BLD_FULL_NAME) $(ARGS)

restore:
	cd $(BLD_FOLDER) && exec ./$(INVERTER_BLD_FULL_NAME) $(ARGS)

install:
	apt-get install espeak -y
	sudo apt install graphviz

frontend.o:
	$(CC) $(CFLAGS) -c src/frontend.cpp

backend.o:
	$(CC) $(CFLAGS) -c src/backend.cpp

inverter.o:
	$(CC) $(CFLAGS) -c src/inverter.cpp

frontend_utils.o:
	$(CC) $(CFLAGS) -c src/utils/frontend_utils.cpp

backend_utils.o:
	$(CC) $(CFLAGS) -c src/utils/backend_utils.cpp

inverter_utils.o:
	$(CC) $(CFLAGS) -c src/utils/inverter_utils.cpp

common_utils.o:
	$(CC) $(CFLAGS) -c src/utils/common_utils.cpp

alloc_tracker.o:
	$(CC) $(CFLAGS) -c lib/alloc_tracker/alloc_tracker.cpp

argparser.o:
	$(CC) $(CFLAGS) -c lib/util/argparser.cpp

logger.o:
	$(CC) $(CFLAGS) -c lib/util/dbg/logger.cpp

debug.o:
	$(CC) $(CFLAGS) -c lib/util/dbg/debug.cpp

file_helper.o:
	$(CC) $(CFLAGS) -c lib/file_helper.cpp

bin_tree.o:
	$(CC) $(CFLAGS) -c lib/grammar/bin_tree.cpp

speaker.o:
	$(CC) $(CFLAGS) -c lib/speaker.cpp

grammar.o:
	$(CC) $(CFLAGS) -c lib/grammar/grammar.cpp

util.o:
	$(CC) $(CFLAGS) -c lib/util/util.cpp

clean:
	rm -rf *.o

rmbld:
	rm -rf $(BLD_FOLDER)
	rm -rf $(TEST_FOLDER)

rm: clean rmbld