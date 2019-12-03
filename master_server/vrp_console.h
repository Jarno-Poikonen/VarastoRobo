/*
	VarastoRobo master server version 0.8.0 2019-12-03 by Santtu Nyman.
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