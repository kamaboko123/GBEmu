#Makefile

CC = g++
INCLUDE = -I include -I lib
CFLAGS = -std=c++20 -g -Wall -O0 `pkg-config.exe --libs sdl2 | sed -e "s/-mwindows//g"`
LDFLAGS = `pkg-config.exe --libs sdl2 | sed -e "s/-mwindows//g"`

TARGET_DIR = bin
TARGET = $(TARGET_DIR)/emu

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))

LIBS_DIR = lib
LIBS_DIRS = $(shell find $(LIBS_DIR) -maxdepth 1 -type d -not -path $(LIBS_DIR))
LIBS_SRC = $(shell find $(LIBS_DIR) -name *.cpp -type f -print | sed -e 's,/src/,/obj/,g')
LIBS_OBJ = $(shell find $(LIBS_DIR) -name *.cpp -type f -print | sed -e 's,/src/,/obj/,g' | sed -e 's/\.cpp$$/\.o/g')

define make_lib
	cd $1; make;
endef
define clean_lib
	cd $1; make clean;
endef

all: $(TARGET)

test: all
	cd test; make run;

run: all
	./$(TARGET) test/gb/0xe0_and_0xf0.gb

debug: all
	./$(TARGET) test/gb/0xe0_and_0xf0.gb --regs

clean:
	rm -rf $(TARGET_DIR)
	rm -rf $(OBJ_DIR)

clean_all:
	make clean
	make clean_libs

.PHONY: libs
$(TARGET): $(OBJ)
	make libs
	mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LIBS_OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

libs: 
	$(foreach x, $(LIBS_DIRS), $(call make_lib, $(x)))

clean_libs:
	$(foreach x, $(LIBS_DIRS), $(call clean_lib, $(x)))
