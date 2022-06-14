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

#include "input_stream.h"

void ins_create(struct input_stream* in, size_t length, void* data) {
	assert(in && length > 0 && data);

	in->offset = 0;
	in->length = length;
	in->data = data;
}

size_t ins_available(struct input_stream* in) {
	assert(in);
	return in->length - in->offset;
}

int32_t ins_read32s(struct input_stream* in) {
	assert(in && ins_available(in) >= sizeof(int32_t));

	int32_t v = ((int32_t*)in->data)[in->offset];
	in->offset += sizeof(int32_t);
	return v;
}

uint32_t ins_read32u(struct input_stream* in) {
	assert(in && ins_available(in) >= sizeof(uint32_t));

	uint32_t v = ((uint32_t*)in->data)[in->offset];
	in->offset += sizeof(uint32_t);
	return v;
}

uint8_t ins_read8u(struct input_stream* in) {
	assert(in && ins_available(in) >= sizeof(uint8_t));

	return ((uint8_t*)in->data)[in->offset++];
}

void ins_skip(struct input_stream* in, size_t bytes) {
	assert(in && ins_available(in) >= bytes);
	in->offset += bytes;
}

bool ins_read_string(struct input_stream* in, char* str, size_t length) {
	if(ins_available(in) < sizeof(uint8_t))
		return false;

	size_t available = ins_read8u(in);

	if(ins_available(in) < available)
		return false;

	size_t min_length = (length < available) ? length : available;

	strncpy(str, (char*)in->data + in->offset, min_length);
	str[min_length - 1] = 0;

	ins_skip(in, available - min_length);

	return true;
}
