# Makefile

AS = rgbasm 
ASFLAGS = -L

LD = rgblink
LDFLAGS=

ASM_SRC_DIR = asm
ASM_SRC = $(wildcard $(ASM_SRC_DIR)/*.asm)
OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(notdir $(ASM_SRC:.asm=.o)))

TARGET_DIR = gb
TARGET = $(addprefix $(TARGET_DIR)/, $(notdir $(ASM_SRC:.asm=.gb)))

.PRECIOUS: $(OBJ)

run: all
	python test.py

all: $(TARGET)

$(TARGET_DIR)/%.gb: $(OBJ_DIR)/%.o
	mkdir -p $(TARGET_DIR)
	$(LD) -o $@ $^
	rgbfix -v -p 0xFF $@

$(OBJ_DIR)/%.o: $(ASM_SRC_DIR)/%.asm
	mkdir -p $(OBJ_DIR)
	$(AS) $(ASFLAGS) -o $@ $^

clean:
	rm -rf $(TARGET_DIR)
	rm -rf $(OBJ_DIR)
