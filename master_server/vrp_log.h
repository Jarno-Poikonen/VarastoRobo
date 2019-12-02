/*
	VarastoRobo master server version 0.5.0 2019-11-26 by Santtu Nyman.
*/

#ifndef VRP_LOG_H
#define VRP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <Windows.h>

typedef struct vrp_log_t
{
	HANDLE handle;
	uint32_t line_count;
	size_t line_table_memory_granularity;
	size_t line_table_memory_reserved;
	size_t line_table_memory_commited;
	struct
	{
		uint32_t offset;
		uint32_t size;
	}* line_table;
} vrp_log_t;

void vrp_close_log(vrp_log_t* log);

DWORD vrp_read_log(vrp_log_t* log, uint32_t first_line, uint32_t line_count, size_t buffer_size, char* buffer, uint32_t* line_read_count, size_t* data_size);

DWORD vrp_write_log_entry(vrp_log_t* log, const char* line);

DWORD vrp_open_log_file(vrp_log_t* log, size_t reserved_memory_size, size_t memory_granularity, int debug_print_logs);

#ifdef __cplusplus
}
#endif

#endif