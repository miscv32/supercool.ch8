#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "myutils.h"

#include "resource_dir.h"

#define COLOUR_OFF  CLITERAL(Color){ 139, 172, 15, 255 }  
#define COLOUR_ON  CLITERAL(Color){ 48, 98, 48, 255 } 

#define VX (registers[second_nibble])
#define VY (registers[third_nibble])
#define VF (registers[0xF])
#define N (fourth_nibble)
#define NN ((third_nibble << 4) | fourth_nibble)
#define NNN ((uint16_t)(second_nibble << 8) | (uint16_t)(third_nibble << 4) | (uint16_t)(fourth_nibble))

#define Q_VF_RESET
#define Q_MEMORY 
#define Q_DISPLAY_WAIT
#define Q_CLIPPING
// #define Q_SHIFTING
// #define Q_JUMPING

int randomSample(float gain) {

    return rand();
}

void generateSamples(void * buffer, unsigned int frames) {

    /* This audio callback fills a mono buffer with random
    32-bit integer samples.*/

    short * samples = (short *)buffer;

    for (unsigned int i; i<frames; i++) samples[i] = randomSample(1.0f);
}


int main (int argc, char* argv[])
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	SetTargetFPS(60);

	InitWindow(1280, 640, "supercoolchip8"); // 1 CHIP-8 pixel = 20x20 raylib rectangle

	SearchAndSetResourceDir("resources");
	
	InitAudioDevice();

	AudioStream stream = LoadAudioStream(48000, 32, 1);

    SetAudioStreamBufferSizeDefault(48000*5);
	SetAudioStreamVolume(stream, 0.5f);
    AttachAudioStreamProcessor(stream, generateSamples);

	Sound beep = LoadSound("beep.wav");

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
	
	FILE* rom_file_ptr = fopen("7-beep.ch8", "rb");

	if (!rom_file_ptr)
	{
		perror("Failed to open file");
		CloseWindow();
	}

	for(int c = 0, k = 0; (c = fgetc(rom_file_ptr)) != EOF && k < (4096 - 0x200); k++)
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
	uint16_t current_instruction = 0;

	uint8_t registers[16] = {0};
	uint8_t VF_tmp = 0;
	Color colours[2] = {COLOUR_OFF, COLOUR_ON};

	static bool key_state[16] = {0};
	
	int FX0A_key_to_release = -1;
	while (!WindowShouldClose())	
	{
		bool draw_now = false;

		for (int instructions_counter = 0; instructions_counter < 10 && !draw_now; instructions_counter++)
		{	
			for (int i = 0; i < 16; i++) {
				int raylib_key = map(i);
				bool current_state = IsKeyDown(raylib_key);
				if (FX0A_key_to_release == i && current_state == 0 && key_state[i] == 1) // if we've just released a key that FX0A is waiting for to be released, go to the next instruction (i.e. skip the FX0A that would otherwise have been executed)
				{
					program_counter += 2;
					FX0A_key_to_release = -1;
				}
				key_state[i] = current_state;
				
			}
	
			current_instruction = ram[program_counter] << 8 | ram[program_counter + 1];
			program_counter += 2;
			uint8_t first_nibble = (uint8_t)(current_instruction >> 12) & 0xF;
			uint8_t second_nibble = (uint8_t)(current_instruction >> 8) & 0xF;
			uint8_t third_nibble = (uint8_t)(current_instruction >> 4) & 0xF;
			uint8_t fourth_nibble = (uint8_t)(current_instruction) & 0xF;

			switch (first_nibble)
			{
				case 0:
					switch (NNN)
					{
						case 0x0E0:
							memset(&display, 0, sizeof(bool)*2048);
							break;
						case 0x0EE:
							program_counter = stack[stack_pointer]; // stack underflow? who is she?
							stack[stack_pointer] = 0;
							stack_pointer--;
							break;
					}
					break;
				case 1:
					program_counter = NNN;
					break;
				case 2:
					stack_pointer++;
					stack[stack_pointer] = program_counter; // stack overflow? who is he?
					program_counter = NNN;
					break;
				case 3:		
					if (VX == NN) program_counter += 2;
					break;
				case 4:
					if (VX != NN) program_counter += 2;
					break;
				case 5:
					if (VX == VY) program_counter += 2;
					break;
				case 6:					
					VX = NN; 
					break;
				case 7:
					VX += NN;
					break;
				case 8:
					switch (fourth_nibble)
					{
						case 0:
							VX = VY;
							break;
						case 1:
							VX |= VY;
							#ifdef Q_VF_RESET
							VF = 0;
							#endif
							break;
						case 2:
							VX &= VY;
							#ifdef Q_VF_RESET
							VF = 0;
							#endif
							break;
						case 3:
							VX = VX ^ VY;
							#ifdef Q_VF_RESET
							VF = 0;
							#endif
							break;
						case 4:	
							if (((uint16_t)(VX) + (uint16_t)(VY)) > 255) VF_tmp = 1;
							else VF_tmp = 0;
							VX += VY;
							VF = VF_tmp;
							break;
						case 5:
							VF_tmp = (VX >= VY) ? 1 : 0;
							VX = VX - VY;
							VF = VF_tmp;
							break;
						case 6:
							#ifndef Q_SHIFTING
							VX = VY;
							#endif
							VF_tmp = VX & 1;
							VX = VX >> 1;
							VF = VF_tmp;
							break;
						case 7:	
							VF_tmp = (VY >= VX) ? 1 : 0;
							VX = VY - VX;
							VF = VF_tmp;
							break;	
						case 0xE:
							#ifndef Q_SHIFTING
							VX = VY;
							#endif
							VF_tmp = (VX >> 7) & 1;
							VX = VX << 1;
							VF = VF_tmp;
							break;
					}
					break;
				case 9:
					if (VX != VY) 
						{
							program_counter += 2;
						}
					break;
				case 0xA:
					index_register = NNN;
					break;
				case 0xB:
					uint8_t offset = registers[0];
					#ifdef Q_JUMPING
					offset = VX;
					#endif
					program_counter = NNN + offset;
					break;
				case 0xC:
					VX = NN & GetRandomValue(0, UINT8_MAX);
					break;
				case 0xD:
					uint8_t x = VX % 64;
					uint8_t y = VY % 32;
					VF = 0;
					for (int i = 0; i < N; i++)
					{
						uint8_t sprite_row = ram[index_register + i];
						for(int j = 0; j < 8; j++)
						{
							bool sprite_pixel = (bool)((sprite_row >> (7 - j)) & 1);
							bool* screen_pixel_ptr = &display[get_display_index((x+j) % 64, y % 32)];
							if (*screen_pixel_ptr && sprite_pixel)
							{
								*screen_pixel_ptr = 0;
								VF = 1;
							}
							else if (*screen_pixel_ptr == 0 && sprite_pixel == 1)
							{
								*screen_pixel_ptr = 1;
							}
							#ifdef Q_CLIPPING
							if (x+j == 63) break;
							#endif
						}
						y++;
						#ifdef Q_CLIPPING
						if (y == 32) break;
						#endif
					}
					#ifdef Q_DISPLAY_WAIT
					draw_now = true;
					#endif
					break;
				case 0xE:
					switch (NN)
					{
						case 0x9E:
							if (key_state[VX & 0xF]) program_counter += 2;
							break;
						case 0xA1:
							if (!key_state[VX & 0xF]) program_counter += 2;
							break;
					}
					break;
				case 0xF:
					switch (NN)
					{
						case 0x07:
							VX = delay_timer;
							break;
						case 0x15:
							delay_timer = VX;
							break;
						case 0x18:
							sound_timer = VX;
							PlayAudioStream(stream);
							break;
						case 0x1E:
							if (((uint16_t)(VX) + (uint16_t)(index_register)) > 255) VF_tmp = 1;
							index_register += VX;
							VF = VF_tmp;
							break;
						case 0x0A:
							{
								bool any_key_pressed = false;
								for (int i = 0; i < 16; i++) 
								{
									if (key_state[i]) 
									{
										VX = i;
										any_key_pressed = true;
										FX0A_key_to_release = i;
										break;
									}
								}

								if (!any_key_pressed) 
								{
									program_counter -= 2;
								}
								else if (FX0A_key_to_release != -1 && key_state[FX0A_key_to_release])
								{
									program_counter -= 2;
								}
							}
							break;
						case 0x29:
							index_register = 0x50 +((VX & 0xF) * 5);
							break;
						case 0x33:
							ram[index_register + 2] = VX % 10;
							VX = VX / 10;
							ram[index_register + 1] = VX % 10;
							VX = VX / 10;
							ram[index_register] = VX % 10;
							break;
						case 0x55:
							memcpy(&ram[index_register], registers, sizeof(uint8_t)*(second_nibble+1));
							#ifdef Q_MEMORY
							index_register += second_nibble + 1;
							#endif
							break;
						case 0x65:
							memcpy(registers, &ram[index_register], sizeof(uint8_t)*(second_nibble+1));
							#ifdef Q_MEMORY
							index_register += second_nibble + 1;
							#endif
							break;		
					}
					break;
			}
		}



		BeginDrawing();

		for (int i = 0; i < 2048; i++)
		{
			uint8_t x = get_display_x(i);
			uint8_t y = get_display_y(i);
			DrawRectangle(x*20, y*20, 20, 20, colours[display[i]]);
		}
		
		EndDrawing();
		if (delay_timer > 0) delay_timer -= 1;
		if (sound_timer > 0) sound_timer -= 1;
		if (sound_timer = 0) StopAudioStream(stream);
	}
	UnloadAudioStream(stream);
    DetachAudioStreamProcessor(stream, generateSamples);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}
