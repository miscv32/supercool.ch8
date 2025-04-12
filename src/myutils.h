#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "raylib.h"
#include "myutils.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

unsigned int get_display_index(uint8_t x, uint8_t y);
uint8_t get_display_y(unsigned int index);

uint8_t get_display_x(unsigned int index);

int map(uint8_t reg);
uint8_t unmap(int key);