# Makefile
CC = g++
INCLUDE = -I include -I ./lib/img/include
CFLAGS = -std=c++11 -g -Wall -O0 `pkg-config --libs sdl2 | sed -e "s/-mwindows//g"`

SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(SRC:.cpp=.o)))

all: $(OBJ)

clean:
	rm -rf $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
