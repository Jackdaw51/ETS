# -------- Toolchain --------
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O2

# -------- Your sources --------
SRC     ?= game_files/example.c

OUT := $(basename $(notdir $(SRC)))

OUTDIR  := game_exes

T_PLATFORM ?= game_files/display/rl_display.c
SPRITES := game_files/sprites/sprites.c
PALETTES := game_files/sprites/palettes.c

INCLUDES := -I.

# -------- Raylib from MSYS2 (UCRT64) --------
# Change this if your msys2 is in a different folder
RAYLIB_ROOT := C:/msys64/ucrt64
RAYLIB_INC  := -I$(RAYLIB_ROOT)/include
RAYLIB_LIB  := -L$(RAYLIB_ROOT)/lib -lraylib

# Windows system libs needed by raylib
SYS_LIBS := -lopengl32 -lgdi32 -lwinmm

# Output
TARGET := $(OUTDIR)/$(OUT)

# -------- Build rules --------
all: $(TARGET)

$(TARGET): $(SRC) $(T_PLATFORM) $(SPRITES) $(PALETTES)
	mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) $(T_PLATFORM) $(SPRITES) $(PALETTES) -o $(TARGET) $(LIB) $(LIBS)

play: $(TARGET)
	./$(TARGET)

clean:
	@cmd /C if exist "$(OUTDIR)" rmdir /S /Q "$(OUTDIR)"

.PHONY: all clean

