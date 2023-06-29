/******************************************************************************
 * Copyright (C) 2015, 2023
 * Younghyung Cho. <yhcting77@gmail.com>
 * All rights reserved.
 *
 * This file is part of ylib
 *
 * This program is licensed under the FreeBSD license
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the FreeBSD Project.
 *****************************************************************************/

/*****************************************************************************
 *
 * NOTE
 * ----
 * * Integer overflow / underflow are NOT considered.
 *  (Make sure using signed integer within 32bit.)
 * * NOT thread-safe.
 *
 *
 * Naming convention(common)
 * -------------------------
 * By default, This module assumes 2D coordinates.
 * For 3D, '3' is appended as suffix
 * ex. struct ypoint3;  // point struct for 3D coord
 *
 * Function naming convention
 * --------------------------
 * Single character abbreviation.
 * p : Point
 * l : Line
 * b : Band
 * r : Rectangle
 * g : Region
 *
 *****************************************************************************/

#pragma once

#include <limits.h>
#include "../common.h"

struct ypoint;
struct yline;
struct yband;
struct yrect;
struct yrgn;

/* Value around INT_MIN and INT_MAX MUST NOT be used */
#define GEO_INVALID_MIN_VALUE INT_MIN
#define GEO_INVALID_MAX_VALUE INT_MAX

enum optype {
	OP_INTERSECT,
	OP_DIFF,
	OP_UNION
};
