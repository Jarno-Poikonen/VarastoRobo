/*
	VarastoRobo master server version 1.1.0 2019-12-12 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#ifndef VRP_FILE_H
#define VRP_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <windows.h>
#include "jsonpl.h"

void vrp_free_file_data(LPVOID file_data);

DWORD vrp_load_file(const WCHAR* file_name, SIZE_T* file_size, LPVOID* file_data);

DWORD vrp_open_file_from_program_directory(const WCHAR* file_name, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE* file_handle);

DWORD vrp_load_program_directory_file(const WCHAR* file_name, SIZE_T* file_size, LPVOID* file_data);

DWORD vrp_store_file(const WCHAR* file_name, SIZE_T file_size, LPVOID file_data);

DWORD vrp_load_json_from_file(const WCHAR* file_name, jsonpl_value_t** json_content);

DWORD vrp_load_json_from_program_directory_file(const WCHAR* file_name, jsonpl_value_t** json_content);

#ifdef __cplusplus
}
#endif

#endif