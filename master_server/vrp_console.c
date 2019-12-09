/*
	VarastoRobo master server version 0.9.3 2019-12-09 by Santtu Nyman.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "vrp_console.h"

DWORD vrp_print(const WCHAR* text)
{
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!output)
		return ERROR_NOT_SUPPORTED;
	DWORD text_length = (DWORD)lstrlenW(text);
	DWORD text_written;
	return WriteConsoleW(output, text, text_length, &text_written, 0) ? 0 : GetLastError();
}

DWORD vrp_print_utf8(const CHAR* text)
{
	WCHAR line_buffer[128];
	int required_lenght = MultiByteToWideChar(CP_UTF8, 0, text, -1, 0, 0);
	if (!required_lenght)
		return GetLastError();
	HANDLE heap = GetProcessHeap();
	if (!heap)
		return GetLastError();
	WCHAR* wide_text = (required_lenght <= 128) ? line_buffer : (WCHAR*)HeapAlloc(heap, 0, (SIZE_T)required_lenght * sizeof(WCHAR));
	if (!wide_text)
		return ERROR_OUTOFMEMORY;
	DWORD error;
	if (MultiByteToWideChar(CP_UTF8, 0, text, -1, wide_text, required_lenght) != required_lenght)
	{
		error = GetLastError();
		if (wide_text != line_buffer)
			HeapFree(heap, 0, wide_text);
		return error;
	}
	error = vrp_print(wide_text);
	if (wide_text != line_buffer)
		HeapFree(heap, 0, wide_text);
	return 0;
}

#ifdef __cplusplus
}
#endif