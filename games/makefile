# Compiler and flags
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O2

# -------- User-configurable --------
SRC     ?= game_files/example.c
OUT     ?= game
OUTDIR  := game_exes

T_PLATFORM ?= game_files/display/rl_display.c
SPRITES := game_files/sprites/sprites.c

# Include paths
INCLUDES := -Ithird_party 

# -------- OS detection --------
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    LIBS := $(shell pkg-config --cflags --libs raylib) -lm
endif

ifeq ($(UNAME_S),Darwin)
    LIBS := -lraylib \
            -framework OpenGL \
            -framework Cocoa \
            -framework IOKit \
            -framework CoreVideo
endif

ifeq ($(OS),Windows_NT)
    OUT := $(OUT).exe
    LIBS := -lraylib -lopengl32 -lgdi32 -lwinmm
endif

# Full target path
TARGET := $(OUTDIR)/$(OUT)

# -------- Build rules --------
all: $(TARGET)

$(TARGET): $(SRC) $(T_PLATFORM) $(SPRITES)
	mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) $(T_PLATFORM) $(SPRITES) -o $(TARGET) $(LIBS)

play: $(TARGET)
	./$(OUTDIR)/$(OUT)



.PHONY: all clean

