#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "raylib.h"

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

int map(uint8_t reg)
{
	switch(reg)
	{
		case 0:
			return KEY_X;
		case 1:
			return KEY_ONE;
		case 2:
			return KEY_TWO;
		case 3:
			return KEY_THREE;
		case 4:
			return KEY_Q;
		case 5:
			return KEY_W;
		case 6:
			return KEY_E;
		case 7:
			return KEY_A;
		case 8:
			return KEY_S;
		case 9:
			return KEY_D;
		case 0xA:
			return KEY_Z;
		case 0xB:
			return KEY_C;
		case 0xC:
			return KEY_FOUR;
		case 0xD:
			return KEY_R;
		case 0xE:
			return KEY_F;
		case 0xF:
			return KEY_V;
	}
}

uint8_t unmap(int key)
{
	switch(key)
    {
        case KEY_X:
            return 0;
        case KEY_ONE:
            return 1;
        case KEY_TWO:
            return 2;
        case KEY_THREE:
            return 3;
        case KEY_Q:
            return 4;
        case KEY_W:
            return 5;
        case KEY_E:
            return 6;
        case KEY_A:
            return 7;
        case KEY_S:
            return 8;
        case KEY_D:
            return 9;
        case KEY_Z:
            return 0xA;
        case KEY_C:
            return 0xB;
        case KEY_FOUR:
            return 0xC;
        case KEY_R:
            return 0xD;
        case KEY_F:
            return 0xE;
        case KEY_V:
            return 0xF;
	}
}
