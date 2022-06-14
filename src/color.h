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

#ifndef PINKED_COLOR_H
#define PINKED_COLOR_H

#include <stdint.h>

struct color {
	uint8_t red, green, blue;
};

struct color color_blend(struct color a, struct color b, float s);

#endif
