/*
	VarastoRobo master server version 1.1.0 2019-12-12 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#ifndef VRP_LOG_H
#define VRP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vrp_master_server_types.h"
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <Windows.h>

void vrp_close_log(vrp_log_t* log);

DWORD vrp_read_log(vrp_log_t* log, uint32_t first_line, uint32_t line_count, size_t buffer_size, char* buffer, uint32_t* line_read_count, size_t* data_size);

DWORD vrp_write_log_entry(vrp_log_t* log, const char* line);

DWORD vrp_open_log_file(vrp_log_t* log, size_t reserved_memory_size, size_t memory_granularity, int debug_print_logs);

#ifdef __cplusplus
}
#endif

#endif