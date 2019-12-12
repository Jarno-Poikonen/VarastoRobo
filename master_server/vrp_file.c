/*
	VarastoRobo master server version 1.1.0 2019-12-12 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "vrp_file.h"

void vrp_free_file_data(LPVOID file_data)
{
	HeapFree(GetProcessHeap(), 0, file_data);
}

DWORD vrp_load_file(const WCHAR* file_name, SIZE_T* file_size, LPVOID* file_data)
{
	HANDLE heap = GetProcessHeap();
	if (!heap)
		return GetLastError();
	DWORD error;
	SIZE_T size;
	HANDLE handle = CreateFileW(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if (handle == INVALID_HANDLE_VALUE)
		return  GetLastError();
#ifdef _WIN64
	if (!GetFileSizeEx(handle, (LARGE_INTEGER*)&size))
	{
		error = GetLastError();
		CloseHandle(handle);
		return error;
	}
#else
	ULARGE_INTEGER raw_size;
	if (!GetFileSizeEx(handle, (LARGE_INTEGER*)&raw_size))
	{
		error = GetLastError();
		CloseHandle(handle);
		return error;
	}
	if (raw_size.HighPart)
	{
		CloseHandle(handle);
		return ERROR_FILE_TOO_LARGE;
	}
	size = (SIZE_T)raw_size.LowPart;
#endif
	UINT_PTR data = (UINT_PTR)HeapAlloc(heap, 0, size);
	if (!data)
	{
		error = ERROR_OUTOFMEMORY;
		CloseHandle(handle);
		return error;
	}
	for (SIZE_T file_read = 0; file_read != size;)
	{
		DWORD read_result;
		if (ReadFile(handle, (LPVOID)(data + file_read), (DWORD)((size - file_read) < 0x80000000 ? (size - file_read) : 0x80000000), &read_result, 0))
			file_read += (SIZE_T)read_result;
		else
		{
			error = GetLastError();
			HeapFree(heap, 0, (LPVOID)data);
			CloseHandle(handle);
			return error;
		}
	}
	CloseHandle(handle);
	*file_size = size;
	*file_data = (LPVOID)data;
	return 0;
}

static WCHAR* vrp_create_file_path_from_program_directory(const WCHAR* file_name)
{
	HANDLE heap = GetProcessHeap();
	if (!heap)
		return 0;
	SIZE_T file_name_length = (SIZE_T)lstrlenW(file_name);
	if ((SIZE_T)INT_MAX < file_name_length)
		return 0;
	SIZE_T program_directory_name_length = (MAX_PATH + 1);
	if (((program_directory_name_length + file_name_length) < program_directory_name_length) || ((program_directory_name_length + file_name_length) > 0xFFFFFFFF) || (((program_directory_name_length + file_name_length) * sizeof(WCHAR)) / sizeof(WCHAR)) != (program_directory_name_length + file_name_length))
		return 0;
	SIZE_T name_buffer_size = (program_directory_name_length + file_name_length) * sizeof(WCHAR);
	WCHAR* name = (WCHAR*)HeapAlloc(heap, 0, name_buffer_size);
	if (!name)
		return 0;
	for (DWORD error = ERROR_INSUFFICIENT_BUFFER; error == ERROR_INSUFFICIENT_BUFFER;)
	{
		SIZE_T module_file_name_length = (SIZE_T)GetModuleFileNameW(0, name, (DWORD)program_directory_name_length);
		if (module_file_name_length == program_directory_name_length)
		{
			if (((program_directory_name_length + 1024) < program_directory_name_length) || (program_directory_name_length > 0xFFFFFFFF))
			{
				HeapFree(heap, 0, name);
				return 0;
			}
			program_directory_name_length += 1024;
			if (((program_directory_name_length + file_name_length) < program_directory_name_length) || ((program_directory_name_length + file_name_length) > 0xFFFFFFFF) || (((program_directory_name_length + file_name_length) * sizeof(WCHAR)) / sizeof(WCHAR)) != (program_directory_name_length + file_name_length))
			{
				HeapFree(heap, 0, name);
				return 0;
			}
			name_buffer_size = (program_directory_name_length + file_name_length) * sizeof(WCHAR);
			WCHAR* new_name = (WCHAR*)HeapReAlloc(heap, 0, name, name_buffer_size);
			if (!new_name)
			{
				HeapFree(heap, 0, name);
				return 0;
			}
			name = new_name;
		}
		else if (module_file_name_length < program_directory_name_length)
		{
			program_directory_name_length = module_file_name_length;
			error = 0;
		}
		else
		{
			HeapFree(heap, 0, name);
			return 0;
		}
	}
	while (program_directory_name_length && name[program_directory_name_length - 1] != L'\\' && name[program_directory_name_length - 1] != L'/')
		--program_directory_name_length;
	memcpy(name + program_directory_name_length, file_name, (file_name_length + 1) * sizeof(WCHAR));
	return name;
}

DWORD vrp_open_file_from_program_directory(const WCHAR* file_name, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE* file_handle)
{
	HANDLE heap = GetProcessHeap();
	if (!heap)
		ERROR_OUTOFMEMORY;
	WCHAR* name = vrp_create_file_path_from_program_directory(file_name);
	if (!name)
		ERROR_OUTOFMEMORY;
	HANDLE handle = CreateFileW(name, dwDesiredAccess, dwShareMode, 0, dwCreationDisposition, dwFlagsAndAttributes, 0);
	DWORD error = (handle == INVALID_HANDLE_VALUE) ? GetLastError() : 0;
	HeapFree(heap, 0, name);
	*file_handle = handle;
	return error;
}

DWORD vrp_load_program_directory_file(const WCHAR* file_name, SIZE_T* file_size, LPVOID* file_data)
{
	HANDLE heap = GetProcessHeap();
	if (!heap)
		ERROR_OUTOFMEMORY;
	WCHAR* name = vrp_create_file_path_from_program_directory(file_name);
	if (!name)
		ERROR_OUTOFMEMORY;
	DWORD error = vrp_load_file(name, file_size, file_data);
	HeapFree(heap, 0, name);
	return error;
}

DWORD vrp_store_file(const WCHAR* file_name, SIZE_T file_size, LPVOID file_data)
{
	DWORD error;
	HANDLE handle = CreateFileW(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (handle == INVALID_HANDLE_VALUE)
		return  GetLastError();
	for (SIZE_T file_written = 0; file_written != file_size;)
	{
		DWORD write_result;
		if (WriteFile(handle, (LPVOID)((UINT_PTR)file_data + file_written), (DWORD)(file_size - file_written < 0x80000000 ? (DWORD)(file_size - file_written) : 0x80000000), &write_result, 0))
			file_written += (SIZE_T)write_result;
		else
		{
			error = GetLastError();
			CloseHandle(handle);
			return error;
		}
	}
	if (!FlushFileBuffers(handle))
	{
		error = GetLastError();
		CloseHandle(handle);
		return error;
	}
	CloseHandle(handle);
	return 0;
}

DWORD vrp_load_json_from_file(const WCHAR* file_name, jsonpl_value_t** json_content)
{
	HANDLE heap = GetProcessHeap();
	if (!heap)
		return GetLastError();
	SIZE_T file_size;
	LPVOID file_data;
	DWORD error = vrp_load_file(file_name, &file_size, &file_data);
	if (error)
		return error;
	SIZE_T json_content_size = jsonpl_parse_text(file_size, (const char*)file_data, 0, 0);
	if (!json_content_size)
	{
		HeapFree(heap, 0, file_data);
		return ERROR_INVALID_DATA;
	}
	jsonpl_value_t* content = (jsonpl_value_t*)HeapAlloc(heap, 0, json_content_size);
	if (!content)
	{
		HeapFree(heap, 0, file_data);
		return ERROR_OUTOFMEMORY;
	}
	jsonpl_parse_text(file_size, (const char*)file_data, json_content_size, content);
	HeapFree(heap, 0, file_data);
	*json_content = content;
	return 0;
}

DWORD vrp_load_json_from_program_directory_file(const WCHAR* file_name, jsonpl_value_t** json_content)
{
	HANDLE heap = GetProcessHeap();
	if (!heap)
		return GetLastError();
	SIZE_T file_size;
	LPVOID file_data;
	DWORD error = vrp_load_program_directory_file(file_name, &file_size, &file_data);
	if (error)
		return error;
	SIZE_T json_content_size = jsonpl_parse_text(file_size, (const char*)file_data, 0, 0);
	if (!json_content_size)
	{
		HeapFree(heap, 0, file_data);
		return ERROR_INVALID_DATA;
	}
	jsonpl_value_t* content = (jsonpl_value_t*)HeapAlloc(heap, 0, json_content_size);
	if (!content)
	{
		HeapFree(heap, 0, file_data);
		return ERROR_OUTOFMEMORY;
	}
	jsonpl_parse_text(file_size, (const char*)file_data, json_content_size, content);
	HeapFree(heap, 0, file_data);
	*json_content = content;
	return 0;
}

#ifdef __cplusplus
}
#endif