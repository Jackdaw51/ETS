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

	if(argc != 3){
		printf("Usage: ./sprite_tool <FILEPATH> <OUTPUT_NAME>\n");
		return 1;
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

	// 111 is a magic number which should never come up :)
	int allowed_colors[] = {0,85,111,255};
	int counter = 0;
	unsigned char temp_space = 0;

	sprintf(header_output[header_out_len++],"\nextern const Sprite %s_sprite;\n",output_name);
	sprintf(header_output[header_out_len++],"extern const uint8_t %s[];\n",output_name);
	sprintf(impl_output[impl_out_len++],"const uint8_t %s[] = {\n",output_name);

	for(int row_ptr = 0; row_ptr < height; row_ptr++){
		for(int col_ptr = 0; col_ptr < width; col_ptr++){
			unsigned char *pixel = pixels + (row_ptr * width + col_ptr) * 4;
			unsigned char r = pixel[0];
			unsigned char g = pixel[1];
			unsigned char b = pixel[2];
			unsigned char a = pixel[3];

			boolean allowed = FALSE;
			unsigned char index = 0;

			if(a == 0){
				allowed = TRUE;
				index = 2;
			} else {

				for(unsigned char i = 0; i < 4; i++){
					if(r == allowed_colors[i]){
						allowed = TRUE; 
						index = i;
					}  
				}
			}

			if(!allowed){
				printf("forbidden color detected at pixel: (%d,%d), please use a valid png\n",col_ptr,row_ptr);
				return 1;
			}

			if ((r != g) && (r != b)){
				printf("forbidden color detected at pixel: (%d,%d), please use a valid png\n",col_ptr,row_ptr);
				return 1;
			}

			unsigned char shifted_index = index << ((3-counter)*2);
			temp_space = temp_space | shifted_index;
			
			if(counter == 3){
				sprintf(impl_output[impl_out_len++],"%d,",temp_space);
				counter = 0;
				temp_space = 0;
			} else {
				counter++;
			}
		//	fprintf(impl,"%d,",index);
		}

		if(counter != 0){
				sprintf(impl_output[impl_out_len++],"%d,",counter);
				counter = 0;
				temp_space = 0;
		}
		sprintf(impl_output[impl_out_len++],"\n");
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
