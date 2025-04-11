/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#define COLOUR_OFF  CLITERAL(Color){ 139, 172, 15, 255 }  
#define COLOUR_ON  CLITERAL(Color){ 48, 98, 48, 255 }  

unsigned int get_display_index(uint8_t x, uint8_t y)
{
	return (y * 64) + x;
}

uint8_t get_display_y(unsigned int index)
{
	return index / 64;
}

uint8_t get_display_x(unsigned int index)
{
	return index % 64;
}

int main (int argc, char* argv[])
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	SetTargetFPS(60);

	// Create the window and OpenGL context
	InitWindow(1280, 640, "supercoolchip8"); // 64x32 CHIP-8 pixels

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");
	
	uint8_t ram[4096] = {0};
	
	uint8_t font[0x50] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	memcpy(&ram[0x50], &font, sizeof(uint8_t)*0x50);
	
	FILE* rom_file_ptr = fopen("IBM Logo.ch8", "r");

	if (!rom_file_ptr)
	{
		perror("Failed to open file");
		CloseWindow();
	}
	for(int c = 0, k = 0; (c = fgetc(rom_file_ptr)) != EOF && k < 4096 - 0x200; k++)
	{
		ram[0x200 + k] = (uint8_t)(c);
	}

	bool display[2048] = {0};

	uint16_t index_register = 0; 

	uint16_t stack[256];

	uint8_t stack_pointer = 0;

	uint8_t delay_timer;
	uint8_t sound_timer;

	uint16_t program_counter = 0x200;
	uint8_t instructions_counter = 0; // how many instructions have been executed this frame
	uint8_t instructions_per_frame = 12; // make this customisable 
	uint16_t current_instruction = 0;

	uint8_t registers[16];
	uint8_t flags = 0;

	Color colours[2] = {COLOUR_OFF, COLOUR_ON};
	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
		// things we need to do every frame
		// redraw display
		// decrement delay and sound timers
		// Execute some (configurable) number of instructions. 12 is a reasonable default I think

		// fetch
		current_instruction = ram[program_counter] << 8 | ram[program_counter + 1];
		program_counter += 2;

		// decode + execute
		uint8_t first_nibble = (uint8_t)(current_instruction >> 12) & 0xF;
		uint8_t second_nibble = (uint8_t)(current_instruction >> 8) & 0xF;
		uint8_t third_nibble = (uint8_t)(current_instruction >> 4) & 0xF;
		uint8_t fourth_nibble = (uint8_t)(current_instruction) & 0xF;
		switch (first_nibble)
		{
			case 0:
				switch (second_nibble)
				{
					case 0:
						switch (third_nibble)
						{
							case 0xE:
							switch (fourth_nibble)
							{
								case 0:
									memset(&display, 0, sizeof(bool)*2048);
									break;
							}
							break;
						}
					break;
				}
			break;
			case 1:
				program_counter = (uint16_t)(second_nibble << 8) | (uint16_t)(third_nibble << 4) | (uint16_t)(fourth_nibble);
				break;
			case 6:
				registers[second_nibble] = (third_nibble << 4) | fourth_nibble; 
				break;
			case 7:
				registers[second_nibble] += (third_nibble << 4) | fourth_nibble;
				break;
			case 0xA:
				index_register = (uint16_t)(second_nibble << 8) | (uint16_t)(third_nibble << 4) | (uint16_t)(fourth_nibble);
				break;
			case 0xD:
				uint8_t x = registers[second_nibble] & 63;
				uint8_t y = registers[third_nibble] & 31;
				uint8_t n = fourth_nibble;
				flags = 0;
				for (int i = 0; i < n; i++)
				{
					uint8_t sprite_row = ram[index_register + i];
					for(int j = 0; j < 8; j++)
					{
						bool sprite_pixel = (bool)((sprite_row >> (7 - j)) & 1);
						bool* screen_pixel_ptr = &display[get_display_index(x+j,y)];
						if (*screen_pixel_ptr && sprite_pixel)
						{
							*screen_pixel_ptr = 0;
							flags = 1;
						}
						else if (*screen_pixel_ptr == 0 && sprite_pixel == 1)
						{
							*screen_pixel_ptr = 1;
						}
						if (x+j == 63) break;
					}
					y++;
					if (y == 32) break;
				}
				break;
		}

		// drawing
		BeginDrawing();

		// draw based on the display buffer
		for (int i = 0; i < 2048; i++)
		{
			uint8_t x = get_display_x(i);
			uint8_t y = get_display_y(i);
			DrawRectangle(x*20, y*20, 20, 20, colours[display[i]]);
		}
		
		// end the frame and get ready for the next one (display frame, poll input, etc...)
		EndDrawing();
	}

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
