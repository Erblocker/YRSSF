/*
 * lwan - simple web server
 * Copyright (c) 2016 Leandro A. F. Pereira <leandro@hardinfo.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#pragma once

/* API available in Glibc/Linux, but possibly not elsewhere */
#define HAS_ACCEPT4
/* #undef HAS_CLOCK_GETTIME */
#define HAS_MEMPCPY
#define HAS_MEMRCHR
#define HAS_PIPE2
#define HAS_PTHREADBARRIER
#define HAS_RAWMEMCHR

/* Compiler builtins for specific CPU instruction support */
#define HAVE_BUILTIN_CLZLL
/* #undef HAVE_BUILTIN_CPU_INIT */
/* #undef HAVE_BUILTIN_IA32_CRC32 */
/* #undef HAVE_BUILTIN_MUL_OVERFLOW */

/* C11 _Static_assert() */
#define HAVE_STATIC_ASSERT

/* Libraries */
/* #undef HAVE_LUA */
/* #undef HAVE_ZOPFLI */

/* Valgrind support for coroutines */
#define USE_VALGRIND

