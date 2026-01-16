# Compiler and flags
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O2

# -------- User-configurable --------
SRC     ?= game_files/example.c

OUT := $(basename $(notdir $(SRC)))

OUTDIR  := game_exes

T_PLATFORM ?= game_files/display/rl_display.c
SPRITES := game_files/sprites/sprites.c
PALETTES := game_files/sprites/palettes.c

# Include paths
INCLUDES := -I.

# Dynamic library file

LIB := game_files/display/libraylib_m.a

# -------- OS detection --------
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
		LIBS = -lm -lpthread -ldl -lrt -lX11
		LIB = game_files/display/libraylib_l.a
endif

ifeq ($(UNAME_S),Darwin)
    LIBS := \
            -framework OpenGL \
            -framework Cocoa \
            -framework IOKit \
            -framework CoreVideo
		
		OUT := $(OUT)_m
endif

# Full target path
TARGET := $(OUTDIR)/$(OUT)

# -------- Build rules --------
all: $(TARGET)

$(TARGET): $(SRC) $(T_PLATFORM) $(SPRITES) $(PALETTES)
	mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) $(T_PLATFORM) $(SPRITES) $(PALETTES) -o $(TARGET) $(LIB) $(LIBS)

play: $(TARGET)
	./$(OUTDIR)/$(OUT)

.PHONY: all clean

