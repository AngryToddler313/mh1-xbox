/*
 * xbox_compat.h - forced into every C file of the Xbox build except
 * xbox_libc.c and xbox_log.c (tools/build_xbox.py), game C included. Each
 * name is renamed before <stdio.h> declares it, so the declarations and calls
 * go to the replacements in xbox_libc.c:
 * - fopen: the PC code joins paths with '/', the Xbox kernel wants '\'.
 * - the printf family: nxdk's pdclib has no floating point (%f/%e/%g are
 *   skipped without taking their argument, so the rest of the line is garbage).
 */
#ifndef XBOX_COMPAT_H
#define XBOX_COMPAT_H
#define fopen xbox_fopen
#define printf xbox_printf
#define fprintf xbox_fprintf
#define sprintf xbox_sprintf
#define snprintf xbox_snprintf
#define vprintf xbox_vprintf
#define vfprintf xbox_vfprintf
#define vsprintf xbox_vsprintf
#define vsnprintf xbox_vsnprintf
#endif
