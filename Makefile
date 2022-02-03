#Makefile

CC = g++
INCLUDE = -I include
CFLAGS = -std=c++20 -g -Wall -O0 `pkg-config.exe --libs sdl2 | sed -e "s/-mwindows//g"` `pkg-config --cflags gtk+-3.0`  
ifeq ($(OS),Windows_NT)
LDFLAGS = `pkg-config.exe --libs sdl2 | sed -e "s/-mwindows//g"` `pkg-config --libs gtk+-3.0`
else
LDFLAGS = -lopengl32 -lglu32 -lm -lmingw32 -lSDL2main -lSDL2
endif

TARGET_DIR = bin
TARGET = $(TARGET_DIR)/emu

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))

all: $(TARGET)

test: all
	cd test; make run;

run: all
	./$(TARGET) test/gb/0xe0_and_0xf0.gb

debug: all
	./$(TARGET) test/gb/0xe0_and_0xf0.gb --regs

clean:
ifeq ($(OS),Windows_NT)
	cmd.exe /C rmdir /s /q $(TARGET_DIR)
	cmd.exe /C rmdir /s /q $(OBJ_DIR)
	cd test; make clean;
else
	rm -rf $(TARGET_DIR)
	rm -rf $(OBJ_DIR)
endif

$(TARGET): $(OBJ)
ifeq ($(OS),Windows_NT)
	cmd.exe /C if not exist $(TARGET_DIR) mkdir $(TARGET_DIR)
else
	mkdir -p $(TARGET_DIR)
endif
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
ifeq ($(OS),Windows_NT)
	cmd.exe /C if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
else
	mkdir -p $(OBJ_DIR)
endif
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
