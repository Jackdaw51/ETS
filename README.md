# Embedded together strong


## Simulation Tools

In the games folder you will find tools for:

- Compiling the game code
- Drawing onto a window (default size equal to the one we will have physically)
- Transforming pixel art into code

Note : When you use the simulation graphics lib the slider value goes from [0,1023]

By default it starts at 0 (top)
But if you need to change it for testing purposes it is the variable "slider_value" in 
the <rl_display.c> file


### Windows "users" !

So we don't have to deal with windows build system x.x you should use wsl when compiling the project. If you do not have it already follow these steps


```powershell

# Install wsl ubuntu

wsl --install -d Ubuntu

# open wsl

wsl

```

After this you will have a bash terminal

```bash

# update your wsl package manager
sudo apt update

# install the build system and dependencies
sudo apt install -y \
  build-essential \
  libx11-dev \
  libxrandr-dev \
  libxi-dev \
  libxinerama-dev \
  libxcursor-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev \
  libasound2-dev \
  libpulse-dev \
  libudev-dev \
  libdrm-dev


# cd to the games folder of ETS and run this to compile and run
make play

# Note that after this you can just write "wsl" in the vscode terminal and you're good to go!
```

### Compiling Code

In the folder there is a makefile which is setup to be crossplatform.
It is used as so

```bash
# This will compile the example.c file

make

# This will compile a chosen file 
# (note that it needs the full path so /game_files/FILENAME)

make SRC=<FILENAME> 

# Adding play will make the executable run after compiling 

make play SRC=<FILENAME>
```

### Drawing

A template.c file is included in the game_files/ folder, note that certain functions must be called in order because of the dependency on the raylib library. So try to stick to the template.  The template also represents the typical game loop of Input -> Physics -> Drawing.


The drawing functions accept the enum TWOS_COLORS, which include black, white, dark gray and transparent. This is so that you do not need to care about the internals of the color implementation.

**THE COORDINATE SYSTEM HAS THE POINT (0,0) IN THE TOP LEFT. THIS MEANS ADDING TO X GOES RIGHT, AND ADDING TO Y GOES DOWN!**


### Sprites/Textures

One of the drawing functions is for textures. 
The way to use this is:

```c
// This must be outside of the game loop (outside of the while)
TextureHandle handle = load_texture_from_sprite(sprite.height,sprite.width,sprite.data);

while(...){
// This is inside the game loop

    draw_texture(x,y,handle);

}
```

#### Generating Sprites

- Sprites must contain only 3 specific colors
- You Must specify the 3 colors by defining a Palette and adding it to the PaletteArray (See the palette.c and palette.h files)
- The size must also be maximum 160x128 (the whole screen)

1. Get a png of a sprite using any pixel art app/site. 
I used https://www.piskelapp.com/p/create/sprite/ while testing.

2. Transform the png into code by using the sprite_tool in the sprites folder



```bash

# To generate the sprite variable
./sprite_tool pngs/<PNGNAME> <SPRITE_VARIABLE_NAME>

# You can delete sprites from code to clean up by using sprite_remover.py
python sprite_remover.py <SPRITE_VARIABLE_NAME>

# or 
python3 sprite_remover.py <SPRITE_VARIABLE_NAME>

```

3. Use the sprite in your code, easy! :)


