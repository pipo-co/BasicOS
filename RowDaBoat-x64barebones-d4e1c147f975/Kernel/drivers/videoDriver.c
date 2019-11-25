//videoDriver.c
#include <videoDriver.h>

unsigned int horizontalPixelCount = 1024;
unsigned int verticalPixelCount = 768;
unsigned int numberOfColorBytes = 3;

struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;

	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;

	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));

struct vbe_mode_info_structure * screenData = (void*)0x5C00;

void init_VM_Driver() {
	numberOfColorBytes = screenData->bpp / 8;
	verticalPixelCount = screenData->height;
	horizontalPixelCount = screenData->width;
}

void drawPixel(unsigned int x, unsigned int y, int color){
	
	//Agregado de los casteos
    char* screen = (char *) ( (uint64_t) screenData->framebuffer); 
    unsigned where = (x + y*horizontalPixelCount) * numberOfColorBytes;
    screen[where] = color & 255;              // BLUE
    screen[where + 1] = (color >> 8) & 255;   // GREEN
    screen[where + 2] = (color >> 16) & 255;  // RED
}

void copyPixel(unsigned int xFrom, unsigned int yFrom, unsigned int xTo, unsigned int yTo){
	
	char* screen = (char *) ( (uint64_t) screenData->framebuffer);
	
	unsigned from = (xFrom + yFrom*horizontalPixelCount) * numberOfColorBytes;
	unsigned to = (xTo + yTo*horizontalPixelCount) * numberOfColorBytes;
	
	screen[to] = screen[from]; //red
	screen[to+1] = screen[from+1]; //green
	screen[to+2] = screen[from+2]; //blue
}

void drawChar(int x, int y, char character, int fontColor, int backgroundColor){
	
	if(x < 0 || x > horizontalPixelCount - CHAR_WIDTH || y < 0 || y > verticalPixelCount - CHAR_HEIGHT){
		return;
	}

	int aux_x = x;
	int aux_y = y;

	char bitIsPresent;

	unsigned char * toDraw = charBitmap(character);

	for (int i = 0; i < CHAR_HEIGHT; i++){
		for (int j = 0; j < CHAR_WIDTH; j++){
			bitIsPresent = (1 << (CHAR_WIDTH - 1 - j)) & toDraw[i]; //Ver pie de la funcion

			if(bitIsPresent)
				drawPixel(aux_x, aux_y, fontColor);
			else
				drawPixel(aux_x, aux_y, backgroundColor);

				aux_x++;
		}
		aux_x = x;
		aux_y++;
	}
}
//En la implementacion provista no se le restaba 1 en "CHAR_WIDTH - 1 - j", observamos que esto 
// hacia que no se imprima de manera completa el caracter, ya que no imprimia una de las filas 
// correspondientes.

int horPixelCount(){
	return horizontalPixelCount;
}

int verPixelCount(){
	return verticalPixelCount;
}
