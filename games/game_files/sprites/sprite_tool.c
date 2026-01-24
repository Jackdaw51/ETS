#include "palettes.h"
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>

#define TRUE 1
#define FALSE 0

#define MAX_SPRITE_LINES 10000
#define MAX_SPRITE_LINE_LEN 10000

typedef unsigned char boolean;

int main(int argc, char** argv){
	int width,height,channels;
	int palette_index = 0;

	if(argc < 3){
		printf("Usage: ./sprite_tool <FILEPATH> <OUTPUT_NAME>\n");
		return 1;
	}

	if(argc == 4){
		palette_index = atoi(argv[3]);
	}

	char *file_path = argv[1];
	char *output_name = argv[2];

	// make sure the sprite doesn't already exist

	FILE *header_read = fopen("sprites.h","r");

	size_t lines = 0;
	char* c = malloc(MAX_SPRITE_LINE_LEN* sizeof(char));
	char* comparison_string = malloc(MAX_SPRITE_LINE_LEN * sizeof(char));

	sprintf(comparison_string,"extern const Sprite %s_sprite;\n",output_name);

	while (fgets(c,100*sizeof(char),header_read) != NULL) {
		if(strcmp(c,comparison_string) == 0){
			printf("This sprite name already exists, please use a different one\n");
			return 1;
		}
	}

	fclose(header_read);
	free(c);
	free(comparison_string);

	// Turn the png into a 2bpp array
	
	FILE *header = fopen("sprites.h","a");
	FILE *impl = fopen("sprites.c","a");

	char (*header_output)[MAX_SPRITE_LINE_LEN] = malloc(sizeof(char)*MAX_SPRITE_LINES*MAX_SPRITE_LINE_LEN); 
	char (*impl_output)[MAX_SPRITE_LINE_LEN] = malloc(sizeof(char)*MAX_SPRITE_LINES*MAX_SPRITE_LINE_LEN); 

	int header_out_len = 0;
	int impl_out_len = 0;

	if (header == NULL) {
		printf("Unable to open the header file\n");
		return 1;
	}

	if (impl == NULL) {
		printf("Unable to open the c file\n");
		return 1;
	}

	unsigned char *pixels = stbi_load(
		file_path,
		&width,
		&height,
		&channels,
		4
	);

	if((width > 160) || (height > 128)){
		printf("Sprite is too large");
		return 1;
	}

	if(!pixels){
		printf("Failed to load image\n");
		return 1;
	}

	int x = 0, y = 0;

  sprintf(header_output[header_out_len++],"\nextern const Sprite %s_sprite;\n",output_name);
  sprintf(header_output[header_out_len++],"extern const uint8_t %s[];\n",output_name);
  sprintf(impl_output[impl_out_len++],"const uint8_t %s[] = {\n",output_name);

  int unfound_counter = 0;
  for(int pal = 0; pal < PALETTE_ARR_LEN; pal++){
    char (*scratch_space)[MAX_SPRITE_LINE_LEN] = malloc(sizeof(char)*MAX_SPRITE_LINES*MAX_SPRITE_LINE_LEN);
    int scratch_space_len = 0;

    int unfound_counter = 0;
      
    unsigned char *allowed_p = PaletteArray[pal].colors;
    int counter = 0;
    unsigned char temp_space = 255;
	printf("Height: %d, width: %d\n", height, width);
	fflush(stdout);

    for(int row_ptr = 0; row_ptr < height; row_ptr++){
      for(int col_ptr = 0; col_ptr < width; col_ptr++){
        unsigned char *pixel = pixels + (row_ptr * width + col_ptr) * 4;
        unsigned char r = pixel[0];
        unsigned char g = pixel[1];
        unsigned char b = pixel[2];
        unsigned char a = pixel[3];
		printf("Pixel %d,%d - RGBA: %d,%d,%d,%d\n", col_ptr, row_ptr, r, g, b, a);
		fflush(stdout);

        boolean allowed = FALSE;
        unsigned char index = 0;

        if(a==0){ // Always allow transparent
          allowed = TRUE;
          index = 3;
        }
        for(unsigned char i = 0; i < 3; i++){
          int base = i*4;
          if(r==allowed_p[base] && g==allowed_p[base+1] && b==allowed_p[base+2] && a==allowed_p[base+3]){
            allowed = TRUE;
            index = i;
          }
        }
        if(!allowed){
          unfound_counter++;

          if(unfound_counter == PALETTE_ARR_LEN){
            printf("Could not find a valid palette to match the png");
            return 1;
          }
          goto next_palette;
        }
		printf("Unfound counter: %d - Valid palette: %d\n", unfound_counter, pal);
		fflush(stdout);

        unsigned char shifted_index = (3-index) << ((3-counter)*2);
        temp_space = temp_space & (shifted_index^255);
        
        if(counter == 3){
          sprintf(scratch_space[scratch_space_len++],"%d,",temp_space);
          counter = 0;
          temp_space = 255;
        } else {
          counter++;
        }
      //	fprintf(impl,"%d,",index);
      }

      if(counter != 0){
          sprintf(scratch_space[scratch_space_len++],"%d,",temp_space);
          counter = 0;
          temp_space = 255;
      }
      sprintf(scratch_space[scratch_space_len++],"\n");
    }

    for(int ssp = 0; ssp < scratch_space_len; ssp++){
      sprintf(impl_output[impl_out_len++],"%s",scratch_space[ssp]);
    }

    next_palette:

      ;
  }

	sprintf(impl_output[impl_out_len++],"};\n");
	sprintf(impl_output[impl_out_len++],"const Sprite %s_sprite = {%d,%d,%s};\n\n",output_name,height,width,output_name);


	for(int i = 0; i < header_out_len; i++){
		fprintf(header,"%s",header_output[i]);	
	}

	for(int i = 0; i < impl_out_len; i++){
		fprintf(impl,"%s",impl_output[i]);	
	}

	fclose(header);
	fclose(impl);
}
