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
#include <string.h>

#include "layer.h"

#define HT_CHUNK_KEY(x, y, z)                                                  \
	(int[3]) {                                                                 \
		(x >= 0) ? (x / LAYER_CHUNK_SIZE) : ((x + 1) / LAYER_CHUNK_SIZE - 1),  \
			(y >= 0) ? (y / LAYER_CHUNK_SIZE) :                                \
					   ((y + 1) / LAYER_CHUNK_SIZE - 1),                       \
			(z >= 0) ? (z / LAYER_CHUNK_SIZE) :                                \
					   ((z + 1) / LAYER_CHUNK_SIZE - 1)                        \
	}
#define LOOKUP_CHUNK(l, x, y, z) ht_lookup(&l->chunks, HT_CHUNK_KEY(x, y, z))
#define LOCAL_CHUNK_COORD(x) (x & (LAYER_CHUNK_SIZE - 1))

static int chunk_coords_compare(void* a, void* b, size_t key_size) {
	assert(a && b && key_size == sizeof(int[3]));
	int* A = (int*)a;
	int* B = (int*)b;

	return A[0] != B[0] || A[1] != B[1] || A[2] != B[2];
}

static uint32_t int_hash(uint32_t x) {
	x = ((x >> 16) ^ x) * 0x45D9F3B;
	x = ((x >> 16) ^ x) * 0x45D9F3B;
	x = (x >> 16) ^ x;
	return x;
}

static size_t chunk_coords_hash(void* a, size_t key_size) {
	assert(a && key_size == sizeof(int[3]));

	int32_t* A = (int32_t*)a;
	return int_hash(A[0]) ^ int_hash(A[1]) ^ int_hash(A[2]);
}

void layer_create(struct layer* l, int x, int y, int z) {
	assert(l);

	l->x = x;
	l->y = y;
	l->z = z;
	l->sx = l->sy = l->sz = 0;
	l->selected = false;
	l->blend = KEEP_NONE;

	strcpy(l->name, "New layer");

	ht_setup(&l->chunks, sizeof(int[3]), sizeof(struct layer_chunk), 256);
	l->chunks.compare = chunk_coords_compare;
	l->chunks.hash = chunk_coords_hash;
}

static bool layer_destroy_chunks_callback(void* key, void* value, void* user) {
	layer_chunk_destroy((struct layer_chunk*)value);
	return true;
}

void layer_destroy(struct layer* l) {
	assert(l);

	ht_iterate(&l->chunks, NULL, layer_destroy_chunks_callback);
	ht_destroy(&l->chunks);
}

bool layer_is_solid(struct layer* l, int x, int y, int z) {
	assert(l);

	struct layer_chunk* c = LOOKUP_CHUNK(l, x, y, z);

	if(!c)
		return false;

	return layer_chunk_is_solid(c, LOCAL_CHUNK_COORD(x), LOCAL_CHUNK_COORD(y),
								LOCAL_CHUNK_COORD(z));
}

struct color layer_get_color(struct layer* l, int x, int y, int z) {
	assert(l);

	struct layer_chunk* c = LOOKUP_CHUNK(l, x, y, z);

	assert(c);

	return layer_chunk_get_color(c, LOCAL_CHUNK_COORD(x), LOCAL_CHUNK_COORD(y),
								 LOCAL_CHUNK_COORD(z));
}

void layer_set_air(struct layer* l, int x, int y, int z) {
	assert(l);

	struct layer_chunk* c = LOOKUP_CHUNK(l, x, y, z);

	if(c) {
		layer_chunk_set_air(c, LOCAL_CHUNK_COORD(x), LOCAL_CHUNK_COORD(y),
							LOCAL_CHUNK_COORD(z));

		if(!c->solid_blocks) {
			layer_chunk_destroy(c);
			ht_erase(&l->chunks, HT_CHUNK_KEY(x, y, z));
		}
	}
}

void layer_set_solid(struct layer* l, int x, int y, int z, struct color color) {
	assert(l);

	struct layer_chunk* c = LOOKUP_CHUNK(l, x, y, z);

	if(c) {
		layer_chunk_set_solid(c, LOCAL_CHUNK_COORD(x), LOCAL_CHUNK_COORD(y),
							  LOCAL_CHUNK_COORD(z), color);
	} else {
		struct layer_chunk c2;
		int* key = HT_CHUNK_KEY(x, y, z);
		layer_chunk_init(&c2, key[0], key[1], key[2]);
		layer_chunk_set_solid(&c2, LOCAL_CHUNK_COORD(x), LOCAL_CHUNK_COORD(y),
							  LOCAL_CHUNK_COORD(z), color);

		ht_insert(&l->chunks, key, &c2);
	}
}

bool layer_read(struct layer* l, struct input_stream* in) {
	assert(l && in);

	if(ins_available(in) < 6 * sizeof(int32_t))
		return false;

	l->selected = false;

	l->x = ins_read32s(in);
	l->y = ins_read32s(in);
	l->z = ins_read32s(in);

	l->sx = ins_read32u(in);
	l->sy = ins_read32u(in);
	l->sz = ins_read32u(in);

	if(!ins_read_string(in, l->name, sizeof(l->name)))
		return false;

	if(ins_available(in) < sizeof(int32_t) + sizeof(uint8_t))
		return false;

	l->blend = (enum layer_blend_mode)ins_read8u(in);
	size_t read_chunks = ins_read32u(in);

	ht_setup(&l->chunks, sizeof(int) * 3, sizeof(struct layer_chunk), 256);

	for(size_t k = 0; k < read_chunks; k++) {
		struct layer_chunk c;

		if(!layer_chunk_read(&c, in)) {
			layer_destroy(l);
			return false;
		}

		ht_insert(&l->chunks, (int[3]) {c.x, c.y, c.z}, &c);
	}

	return true;
}

static bool layer_write_chunks_callback(void* key, void* value, void* user) {
	struct layer_chunk* c = (struct layer_chunk*)value;
	assert(c->solid_blocks > 0);
	layer_chunk_write(c, (struct output_stream*)user);

	return true;
}

void layer_write(struct layer* l, struct output_stream* out) {
	assert(l && out);

	outs_write32s(out, l->x);
	outs_write32s(out, l->y);
	outs_write32s(out, l->z);

	outs_write32u(out, l->sx);
	outs_write32u(out, l->sy);
	outs_write32u(out, l->sz);

	outs_write_string(out, l->name);
	outs_write8u(out, l->blend);
	outs_write32u(out, l->chunks.size);

	ht_iterate(&l->chunks, out, layer_write_chunks_callback);
}

static bool layer_render_chunks_callback(void* key, void* value, void* user) {
	layer_chunk_render((struct layer_chunk*)value);
	return true;
}

void layer_render(struct layer* l) {
	assert(l);
	ht_iterate(&l->chunks, NULL, layer_render_chunks_callback);
}
