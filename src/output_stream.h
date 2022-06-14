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

#ifndef PINKED_OUTPUT_STREAM_H
#define PINKED_OUTPUT_STREAM_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct output_stream {
	size_t length;
	size_t offset;
	void* data;
};

void outs_create(struct output_stream* out);

void outs_write32s(struct output_stream* out, int32_t x);
void outs_write32u(struct output_stream* out, uint32_t x);
void outs_write8u(struct output_stream* out, uint8_t x);
void outs_write_string(struct output_stream* out, char* str);
void outs_save(struct output_stream* out, FILE* f);

#endif
