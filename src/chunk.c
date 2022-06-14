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

#include <assert.h>
#include <stdlib.h>

#include "chunk.h"

#define LAYER_CHUNK_INDEX(x, y, z)                                             \
	((x) + ((y)*LAYER_CHUNK_SIZE + (z)) * LAYER_CHUNK_SIZE)

void layer_chunk_init(struct layer_chunk* c, int x, int y, int z) {
	assert(c);

	c->render.has_vbo = false;
	c->render.vbo_dirty = true;
	c->x = x;
	c->y = y;
	c->z = z;
	c->solid_blocks = 0;
	c->blocks = malloc(LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE
					   * sizeof(struct layer_chunk_block));
	assert(c->blocks);

	// don't feel like using memset() here
	for(size_t k = 0;
		k < LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE; k++)
		c->blocks[k].solid = false;
}

void layer_chunk_destroy(struct layer_chunk* c) {
	assert(c);

	if(c->render.has_vbo)
		glDeleteBuffers(1, &c->render.vbo);

	free(c->blocks);
}

bool layer_chunk_is_solid(struct layer_chunk* c, int x, int y, int z) {
	assert(c && x >= 0 && y >= 0 && z >= 0 && x < LAYER_CHUNK_SIZE
		   && y < LAYER_CHUNK_SIZE && z < LAYER_CHUNK_SIZE);

	if(!c->solid_blocks)
		return false;

	return c->blocks[LAYER_CHUNK_INDEX(x, y, z)].solid;
}

struct color layer_chunk_get_color(struct layer_chunk* c, int x, int y, int z) {
	assert(c && x >= 0 && y >= 0 && z >= 0 && x < LAYER_CHUNK_SIZE
		   && y < LAYER_CHUNK_SIZE && z < LAYER_CHUNK_SIZE);

	return c->blocks[LAYER_CHUNK_INDEX(x, y, z)].color;
}

void layer_chunk_set_air(struct layer_chunk* c, int x, int y, int z) {
	assert(c && x >= 0 && y >= 0 && z >= 0 && x < LAYER_CHUNK_SIZE
		   && y < LAYER_CHUNK_SIZE && z < LAYER_CHUNK_SIZE);

	if(layer_chunk_is_solid(c, x, y, z)) {
		c->solid_blocks--;
		c->blocks[LAYER_CHUNK_INDEX(x, y, z)].solid = false;
	}
}

void layer_chunk_set_solid(struct layer_chunk* c, int x, int y, int z,
						   struct color color) {
	assert(c && x >= 0 && y >= 0 && z >= 0 && x < LAYER_CHUNK_SIZE
		   && y < LAYER_CHUNK_SIZE && z < LAYER_CHUNK_SIZE);

	if(!layer_chunk_is_solid(c, x, y, z)) {
		c->blocks[LAYER_CHUNK_INDEX(x, y, z)].solid = true;
		c->solid_blocks++;
	}

	c->blocks[LAYER_CHUNK_INDEX(x, y, z)].color = color;
}

void layer_chunk_write(struct layer_chunk* c, struct output_stream* out) {
	assert(c && out);

	if(c->solid_blocks) {
		outs_write32s(out, c->x);
		outs_write32s(out, c->y);
		outs_write32s(out, c->z);

		for(size_t k = 0;
			k < LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE; k++) {
			outs_write8u(out, c->blocks[k].solid);
			outs_write8u(out, c->blocks[k].color.red);
			outs_write8u(out, c->blocks[k].color.green);
			outs_write8u(out, c->blocks[k].color.blue);
		}
	}
}

bool layer_chunk_read(struct layer_chunk* c, struct input_stream* in) {
	assert(c && in);

	if(ins_available(in) < 3 * sizeof(int32_t)
		   + LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE * 4
			   * sizeof(uint8_t))
		return false;

	c->blocks = malloc(LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE
					   * sizeof(struct layer_chunk_block));

	c->x = ins_read32s(in);
	c->y = ins_read32s(in);
	c->z = ins_read32s(in);

	c->solid_blocks = 0;

	for(size_t k = 0;
		k < LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE * LAYER_CHUNK_SIZE; k++) {
		c->blocks[k] = (struct layer_chunk_block) {
            .solid = ins_read8u(in),
				.color = (struct color) {
					.red = ins_read8u(in),
					.green = ins_read8u(in),
					.blue = ins_read8u(in),
				},
			};

		if(c->blocks[k].solid)
			c->solid_blocks++;
	}

	return true;
}

void layer_chunk_render(struct layer_chunk* c) {
	assert(c);

	if(!c->render.has_vbo) {
		glGenBuffers(1, &c->render.vbo);
		c->render.has_vbo = true;
	}

	glBindBuffer(GL_ARRAY_BUFFER, c->render.vbo);

	if(c->render.vbo_dirty) {
		c->render.vbo_dirty = false;

		struct output_stream vertices;
		outs_create(&vertices);

		c->render.vertices = 0;

		for(size_t x = 0; x < LAYER_CHUNK_SIZE; x++) {
			for(size_t y = 0; y < LAYER_CHUNK_SIZE; y++) {
				for(size_t z = 0; z < LAYER_CHUNK_SIZE; z++) {
					if(c->blocks[LAYER_CHUNK_INDEX(x, y, z)].solid) {
						outs_write8u(&vertices, x);
						outs_write8u(&vertices, y);
						outs_write8u(&vertices, z);
						c->render.vertices++;
					}
				}
			}
		}

		glBufferData(GL_ARRAY_BUFFER, vertices.length, vertices.data,
					 GL_STATIC_DRAW);
	}

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, 0, NULL);
	glDrawArrays(GL_POINTS, 0, c->render.vertices);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
