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

#ifndef PINKED_INPUT_STREAM_H
#define PINKED_INPUT_STREAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct input_stream {
	size_t length;
	size_t offset;
	void* data;
};

void ins_create(struct input_stream* in, size_t length, void* data);

size_t ins_available(struct input_stream* in);
int32_t ins_read32s(struct input_stream* in);
uint32_t ins_read32u(struct input_stream* in);
uint8_t ins_read8u(struct input_stream* in);
bool ins_read_string(struct input_stream* in, char* str, size_t length);

#endif
