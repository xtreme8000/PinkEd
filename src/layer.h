/*
	Copyright (c) 2022 ByteBit/xtreme8000

	This file is part of PinkEd.

	PinkEd is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	PinkEd is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with PinkEd.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PINKED_LAYER_H
#define PINKED_LAYER_H

#include "chunk.h"
#include "hashtable.h"
#include "input_stream.h"
#include "output_stream.h"

enum layer_blend_mode {
	KEEP_NONE = 0,
	KEEP_AIR = 1,
	KEEP_SPECIAL = 2,
	SUBTRACT_SOLID = 3,
};

struct layer {
	int x, y, z;
	size_t sx, sy, sz;
	char name[17];
	bool selected;
	HashTable chunks;
	enum layer_blend_mode blend;
};

void layer_create(struct layer* l, int x, int y, int z);
void layer_destroy(struct layer* l);

void layer_set_air(struct layer* l, int x, int y, int z);
void layer_set_solid(struct layer* l, int x, int y, int z, struct color color);

bool layer_is_solid(struct layer* l, int x, int y, int z);
struct color layer_get_color(struct layer* l, int x, int y, int z);

bool layer_read(struct layer* l, struct input_stream* in);
void layer_write(struct layer* l, struct output_stream* out);

void layer_render(struct layer* l);

#endif
