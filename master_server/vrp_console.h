/*
	VarastoRobo master server version 1.1.1 2019-12-16 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#ifndef VRP_CONSOLE_H
#define VRP_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <windows.h>

DWORD vrp_print(const WCHAR* text);

DWORD vrp_print_utf8(const CHAR* text);

#ifdef __cplusplus
}
#endif

#endif