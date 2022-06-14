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

#ifndef PINKED_LAYER_CHUNK_H
#define PINKED_LAYER_CHUNK_H

#include <GLES2/gl2.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "color.h"
#include "input_stream.h"
#include "output_stream.h"

// must be power of 2
#define LAYER_CHUNK_SIZE 16

struct layer_chunk_block {
	bool solid;
	struct color color;
};

struct layer_chunk {
	int x, y, z;
	size_t solid_blocks;
	struct layer_chunk_block* blocks;
	struct {
		bool has_vbo;
		bool vbo_dirty;
		GLuint vbo;
		size_t vertices;
	} render;
};

void layer_chunk_init(struct layer_chunk* c, int x, int y, int z);
void layer_chunk_destroy(struct layer_chunk* c);

bool layer_chunk_is_solid(struct layer_chunk* c, int x, int y, int z);
struct color layer_chunk_get_color(struct layer_chunk* c, int x, int y, int z);

void layer_chunk_set_air(struct layer_chunk* c, int x, int y, int z);
void layer_chunk_set_solid(struct layer_chunk* c, int x, int y, int z,
						   struct color color);

bool layer_chunk_read(struct layer_chunk* c, struct input_stream* in);
void layer_chunk_write(struct layer_chunk* c, struct output_stream* out);

void layer_chunk_render(struct layer_chunk* c);

#endif
