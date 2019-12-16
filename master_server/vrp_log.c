/*
	VarastoRobo master server version 1.1.1 2019-12-16 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#include "vrp_log.h"
#include "vrp_file.h"
#include <stdio.h>

void vrp_close_log(vrp_log_t* log)
{
	if (log->handle != INVALID_HANDLE_VALUE)
	{
		FlushFileBuffers(log->handle);
		CloseHandle(log->handle);
		log->handle = INVALID_HANDLE_VALUE;
	}
	if (log->line_table)
	{
		VirtualFree(log->line_table, 0, MEM_RELEASE);
		log->line_table = 0;
	}
}

DWORD vrp_read_log(vrp_log_t* log, uint32_t first_line, uint32_t line_count, size_t buffer_size, char* buffer, uint32_t* line_read_count, size_t* data_size)
{
	if (first_line >= log->line_count)
		return ERROR_FILE_NOT_FOUND;
	if (first_line + line_count > log->line_count)
		line_count = log->line_count - first_line;
	uint64_t offset = (uint64_t)(log->line_table[first_line].offset);
	DWORD total_size = (log->line_table[first_line + line_count - 1].offset - (uint64_t)(log->line_table[first_line].offset)) + ((uint64_t)log->line_table[first_line + line_count - 1].size);
	if ((size_t)total_size > buffer_size)
		return ERROR_INSUFFICIENT_BUFFER;
	if (!SetFilePointerEx(log->handle, *(LARGE_INTEGER*)&offset, 0, FILE_BEGIN))
		return GetLastError();
	DWORD log_file_read;
	if (!ReadFile(log->handle, buffer, total_size, &log_file_read, 0))
		return GetLastError();
	if (log_file_read != total_size)
		return ERROR_IO_DEVICE;
	*line_read_count = line_count;
	*data_size = (size_t)log_file_read;
	return 0;
}

DWORD vrp_write_log_entry(vrp_log_t* log, const char* line)
{
	char line_buffer[0x100];
	const size_t time_stamp_size = 21;
	const uint64_t relative_offset = 0;
	uint64_t offset;
	DWORD written_size;
	SYSTEMTIME time;
	DWORD line_size = (DWORD)lstrlenA(line);
	if (line_size > (sizeof(line_buffer) - time_stamp_size))
		return ERROR_INVALID_PARAMETER;
	GetLocalTime(&time);
	line_buffer[0] = '\n';
	line_buffer[1] = '0' + (char)(((time.wYear) / 1000) % 10);
	line_buffer[2] = '0' + (char)(((time.wYear) / 100) % 10);
	line_buffer[3] = '0' + (char)(((time.wYear) / 10) % 10);
	line_buffer[4] = '0' + (char)(((time.wYear) / 1) % 10);
	line_buffer[5] = '-';
	line_buffer[6] = '0' + (char)(((time.wMonth) / 10) % 10);
	line_buffer[7] = '0' + (char)(((time.wMonth) / 1) % 10);
	line_buffer[8] = '-';
	line_buffer[9] = '0' + (char)(((time.wDay) / 10) % 10);
	line_buffer[10] = '0' + (char)(((time.wDay) / 1) % 10);
	line_buffer[11] = ' ';
	line_buffer[12] = '0' + (char)(((time.wHour) / 10) % 10);
	line_buffer[13] = '0' + (char)(((time.wHour) / 1) % 10);
	line_buffer[14] = ':';
	line_buffer[15] = '0' + (char)(((time.wMinute) / 10) % 10);
	line_buffer[16] = '0' + (char)(((time.wMinute) / 1) % 10);
	line_buffer[17] = ':';
	line_buffer[18] = '0' + (char)(((time.wSecond) / 10) % 10);
	line_buffer[19] = '0' + (char)(((time.wSecond) / 1) % 10);
	line_buffer[20] = ' ';
	memcpy(line_buffer + time_stamp_size, line, line_size);
	if ((((size_t)log->line_count + 1) * sizeof(*log->line_table)) > log->line_table_memory_commited)
	{
		if ((((size_t)log->line_count + 1) * sizeof(*log->line_table)) > log->line_table_memory_reserved)
			return ERROR_OUTOFMEMORY;
		if (!VirtualAlloc((void*)((uintptr_t)log->line_table + log->line_table_memory_commited), log->line_table_memory_granularity, MEM_COMMIT, PAGE_READWRITE))
			return ERROR_OUTOFMEMORY;
		log->line_table_memory_commited += log->line_table_memory_granularity;
	}
	if (!SetFilePointerEx(log->handle, *(LARGE_INTEGER*)&relative_offset, (LARGE_INTEGER*)&offset, FILE_END))
		return GetLastError();
	if (!WriteFile(log->handle, line_buffer, (DWORD)time_stamp_size + line_size, &written_size, 0))
		return GetLastError();
	log->line_table[log->line_count].offset = (uint32_t)(offset + 1);
	log->line_table[log->line_count].size = (uint32_t)((time_stamp_size - 1) + line_size);
	log->line_count++;
	return 0;
}

DWORD vrp_open_log_file(vrp_log_t* log, size_t reserved_memory_size, size_t memory_granularity, int debug_print_logs)
{
	SYSTEM_INFO sytem_info;
	GetSystemInfo(&sytem_info);

	uint8_t log_buffer[1024];
	uint64_t offset = 0;
	uint64_t file_size;
	uint32_t line_count = 1;
	DWORD error = vrp_open_file_from_program_directory(L"vrp_master_log.txt", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, &log->handle);

	if (error)
		return error;

	if (!GetFileSizeEx(log->handle, (LARGE_INTEGER*)&file_size) || !SetFilePointerEx(log->handle, *(LARGE_INTEGER*)&offset, 0, FILE_BEGIN))
	{
		error = GetLastError();
		CloseHandle(log->handle);
		log->handle = INVALID_HANDLE_VALUE;
		return error;
	}

	while (offset != file_size)
	{
		DWORD file_read_size;
		if (ReadFile(log->handle, log_buffer, (DWORD)(((file_size - offset) < sizeof(log_buffer)) ? (file_size - offset) : sizeof(log_buffer)), &file_read_size, 0))
		{
			for (DWORD relative_offset = 0; relative_offset != file_read_size; ++relative_offset)
				if (log_buffer[relative_offset] == (uint8_t)'\n')
					++line_count;
			offset += (uint64_t)file_read_size;
		}
		else
		{
			error = GetLastError();
			CloseHandle(log->handle);
			log->handle = INVALID_HANDLE_VALUE;
			return error;
		}
	}

	log->line_table_memory_granularity = (memory_granularity + ((size_t)sytem_info.dwPageSize - 1)) & ~((size_t)sytem_info.dwPageSize - 1);
	log->line_table_memory_reserved = (reserved_memory_size + (log->line_table_memory_granularity - 1)) & ~(log->line_table_memory_granularity - 1);
	log->line_table = (void*)VirtualAlloc(0, log->line_table_memory_reserved, MEM_RESERVE, PAGE_NOACCESS);
	if (!log->line_table)
	{
		error = GetLastError();
		CloseHandle(log->handle);
		log->handle = INVALID_HANDLE_VALUE;
		return error;
	}

	log->line_table_memory_commited = ((line_count * sizeof(*log->line_table)) + (log->line_table_memory_granularity - 1)) & ~(log->line_table_memory_granularity - 1);
	if (!VirtualAlloc(log->line_table, log->line_table_memory_commited, MEM_COMMIT, PAGE_READWRITE))
	{
		error = GetLastError();
		VirtualFree(log->line_table, 0, MEM_RELEASE);
		log->line_table = 0;
		CloseHandle(log->handle);
		log->handle = INVALID_HANDLE_VALUE;
		return error;
	}

	offset = 0;
	if (!SetFilePointerEx(log->handle, *(LARGE_INTEGER*)&offset, 0, FILE_BEGIN))
	{
		error = GetLastError();
		VirtualFree(log->line_table, 0, MEM_RELEASE);
		log->line_table = 0;
		CloseHandle(log->handle);
		log->handle = INVALID_HANDLE_VALUE;
		return error;
	}

	uint32_t current_line = 0;
	uint32_t current_line_offset = 0;
	uint32_t current_line_size = 0;
	int return_character = 0;
	while (offset != file_size)
	{
		DWORD file_read_size;
		if (ReadFile(log->handle, log_buffer, (DWORD)(((file_size - offset) < sizeof(log_buffer)) ? (file_size - offset) : sizeof(log_buffer)), &file_read_size, 0))
		{
			__assume(file_read_size <= sizeof(log_buffer));
			for (DWORD relative_offset = 0; relative_offset != file_read_size; ++relative_offset)
			{
				__assume(relative_offset < file_read_size);
				if (log_buffer[relative_offset] == (uint8_t)'\n')
				{
					__assume(current_line < line_count);
					log->line_table[current_line].offset = current_line_offset;
					log->line_table[current_line].size = current_line_size - (uint32_t)return_character;
					++current_line;
					current_line_offset = (uint32_t)(offset + relative_offset + 1);
					current_line_size = 0;
				}
				else
				{
					if (log_buffer[relative_offset] == (uint8_t)'\r')
						return_character = 1;
					else
						return_character = 0;
					++current_line_size;
				}
			}
			offset += (uint64_t)file_read_size;
		}
		else
		{
			error = GetLastError();
			VirtualFree(log->line_table, 0, MEM_RELEASE);
			log->line_table = 0;
			CloseHandle(log->handle);
			log->handle = INVALID_HANDLE_VALUE;
			return error;
		}
	}

	__assume(current_line < line_count);
	log->line_table[current_line].offset = current_line_offset;
	log->line_table[current_line].size = current_line_size - (uint32_t)return_character;

	log->line_count = line_count;

	if (debug_print_logs)
	{
		for (uint32_t i = 0; i != log->line_count; i++)
		{
			offset = (uint64_t)log->line_table[i].offset;
			SetFilePointerEx(log->handle, *(LARGE_INTEGER*)&offset, 0, FILE_BEGIN);
			DWORD line_read_size;
			if (!ReadFile(log->handle, log_buffer, (log->line_table[i].size < (sizeof(log_buffer) - 1)) ? log->line_table[i].size : (sizeof(log_buffer) - 1), &line_read_size, 0))
				line_read_size = 0;
			log_buffer[line_read_size] = 0;
			printf("LINE %lu \"%s\"\n", i, (const char*)log_buffer);
		}
		printf("Total %lu lines\n", log->line_count);
	}

	return 0;
}
