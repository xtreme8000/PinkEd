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
#include <string.h>

#include "output_stream.h"

#define INIT_GRANULARITY 4096

static void outs_ensure_available(struct output_stream* out, size_t bytes) {
	assert(out);

	// printf("%p %u %u %u\n", out, out->offset, bytes, out->length);

	if(out->offset + bytes >= out->length) {
		if(!out->data) {
			out->length = (bytes + INIT_GRANULARITY - 1) / INIT_GRANULARITY
				* INIT_GRANULARITY;
			out->data = malloc(out->length);
		} else {
			while(out->offset + bytes >= out->length)
				out->length *= 2;

			out->data = realloc(out->data, out->length);
		}

		assert(out->data);
	}
}

void outs_create(struct output_stream* out) {
	assert(out);

	out->offset = 0;
	out->length = 0;
	out->data = NULL;
}

void outs_destroy(struct output_stream* out) {
	assert(out);

	if(out->data)
		free(out->data);
}

void outs_write32s(struct output_stream* out, int32_t x) {
	assert(out);
	outs_ensure_available(out, sizeof(int32_t));

	((uint8_t*)out->data)[out->offset++] = x & 0xFF;
	((uint8_t*)out->data)[out->offset++] = (x >> 8) & 0xFF;
	((uint8_t*)out->data)[out->offset++] = (x >> 16) & 0xFF;
	((uint8_t*)out->data)[out->offset++] = (x >> 24) & 0xFF;
}

void outs_write32u(struct output_stream* out, uint32_t x) {
	assert(out);
	outs_ensure_available(out, sizeof(uint32_t));

	((uint8_t*)out->data)[out->offset++] = x & 0xFF;
	((uint8_t*)out->data)[out->offset++] = (x >> 8) & 0xFF;
	((uint8_t*)out->data)[out->offset++] = (x >> 16) & 0xFF;
	((uint8_t*)out->data)[out->offset++] = (x >> 24) & 0xFF;
}

void outs_write8u(struct output_stream* out, uint8_t x) {
	assert(out);
	outs_ensure_available(out, sizeof(uint8_t));

	((uint8_t*)out->data)[out->offset++] = x;
}
void outs_write_string(struct output_stream* out, char* str) {
	assert(out && str && strlen(str) < (1 << (sizeof(uint8_t) * 8)));
	outs_ensure_available(out, sizeof(uint8_t) + strlen(str));

	outs_write8u(out, strlen(str));

	while(*str)
		outs_write8u(out, *(str++));
}

void outs_save(struct output_stream* out, FILE* f) {
	assert(out && f);
	fwrite(out->data, out->offset, 1, f);
}
