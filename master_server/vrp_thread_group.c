/*
	VarastoRobo master server version 1.1.0 2019-12-12 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#include "vrp_thread_group.h"
#include <windows.h>

typedef struct vrp_thread_group_t
{
	uint32_t atomic_running_thread_count;
	uint32_t calling_thread_id;
	uint32_t flags;
	uint32_t thread_count;
	void* thread_group_parameter;
	void (*thread_procedure)(void* thread_group_parameter, size_t thread_count, size_t thread_index);
	void (*thread_group_exit_procedure)(uint32_t flags, void* thread_group_parameter, size_t thread_count);
	HANDLE finished_event;
} vrp_thread_group_t;

DWORD CALLBACK vrp_thread_procedure(IN LPVOID parameter)
{
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);
	volatile vrp_thread_group_t* thread_group = (volatile vrp_thread_group_t*)((uintptr_t)parameter & ~(uintptr_t)(system_info.dwPageSize - 1));
	size_t thread_index = ((uintptr_t)parameter & (uintptr_t)(system_info.dwPageSize - 1));
	size_t thread_count = (size_t)thread_group->thread_count;
	void* thread_group_parameter = thread_group->thread_group_parameter;

	thread_group->thread_procedure(thread_group_parameter, thread_count, thread_index);

	uint32_t calling_thread_id = thread_group->calling_thread_id;
	uint32_t flags = thread_group->flags;
	void (*thread_group_exit_procedure)(uint32_t flags, void* thread_group_parameter, size_t thread_count) = thread_group->thread_group_exit_procedure;
	HANDLE finished_event = thread_group->finished_event;

	size_t thread_finish_index = (size_t)InterlockedDecrement((volatile LONG*)&thread_group->atomic_running_thread_count);
	if (flags & VRP_FLAG_BLOCK_UNTIL_FINISHED)
	{
		if (!thread_finish_index)
			SetEvent(finished_event);
		if (GetCurrentThreadId() != calling_thread_id)
			ExitThread(0);
		else
		{
			WaitForSingleObject(finished_event, INFINITE);
			CloseHandle(finished_event);
			VirtualFree((LPVOID)thread_group, 0, MEM_RELEASE);
			if (thread_group_exit_procedure)
				thread_group_exit_procedure(flags, thread_group_parameter, thread_count);
			return 0;
		}
		__assume(0);
	}
	else
	{
		if (!thread_finish_index)
		{
			VirtualFree((LPVOID)thread_group, 0, MEM_RELEASE);
			if (thread_group_exit_procedure)
				thread_group_exit_procedure(flags, thread_group_parameter, thread_count);
		}
		ExitThread(0);
		__assume(0);
	}
}

int vrp_create_thread_group(uint32_t flags, size_t thread_count, void* thread_group_parameter, void(*thread_procedure)(void* thread_group_parameter, size_t thread_count, size_t thread_index), void(*thread_group_exit_procedure)(uint32_t flags, void* thread_group_parameter, size_t thread_count))
{
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);
	if (!thread_count || thread_count > 0xFFFFFFFF || (uint32_t)thread_count >= system_info.dwPageSize)
		return EINVAL;
	size_t thread_group_stucture_size = (sizeof(vrp_thread_group_t) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t thread_handle_table_size = ((thread_count * sizeof(HANDLE)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t thread_group_buffer_size = ((thread_group_stucture_size + thread_handle_table_size) + ((size_t)system_info.dwPageSize - 1)) & ~((size_t)system_info.dwPageSize - 1);
	volatile vrp_thread_group_t* thread_group = (volatile vrp_thread_group_t*)VirtualAlloc(0, thread_group_buffer_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	HANDLE* thread_handle_table = (HANDLE*)((uintptr_t)thread_group + thread_group_stucture_size);
	if (!thread_group)
		return ENOMEM;

	thread_group->atomic_running_thread_count = (uint32_t)thread_count;
	thread_group->calling_thread_id = GetCurrentThreadId();
	thread_group->flags = flags;
	thread_group->thread_count = (uint32_t)thread_count;
	thread_group->thread_group_parameter = thread_group_parameter;
	thread_group->thread_procedure = thread_procedure;
	thread_group->thread_group_exit_procedure = thread_group_exit_procedure;
	if (flags & VRP_FLAG_BLOCK_UNTIL_FINISHED)
	{
		HANDLE finished_event = CreateEventW(0, FALSE, FALSE, 0);
		if (!finished_event)
		{
			VirtualFree((LPVOID)thread_group, 0, MEM_RELEASE);
			return EBUSY;
		}
		thread_group->finished_event = finished_event;
	}
	else
		thread_group->finished_event = 0;

	size_t new_thread_count = (flags & VRP_FLAG_BLOCK_UNTIL_FINISHED) ? (thread_count - 1) : thread_count;
	for (size_t i = 0; i != new_thread_count; ++i)
	{
		thread_handle_table[i] = CreateThread(0, 0, vrp_thread_procedure, (LPVOID)((uintptr_t)thread_group | i), CREATE_SUSPENDED, 0);
		if (!thread_handle_table[i])
		{
			for (size_t j = i; j--;)
			{
#pragma warning(suppress:6001)
#pragma warning(suppress:6258)
				TerminateThread(thread_handle_table[j], 0);
#pragma warning(default:6258)
				CloseHandle(thread_handle_table[j]);
#pragma warning(default:6001)
			}
			if (flags & VRP_FLAG_BLOCK_UNTIL_FINISHED)
				CloseHandle(thread_group->finished_event);
			VirtualFree((LPVOID)thread_group, 0, MEM_RELEASE);
			return EBUSY;
		}
	}
	for (size_t i = 0; i != new_thread_count; ++i)
	{
#pragma warning(suppress:6001)
		ResumeThread(thread_handle_table[i]);
		CloseHandle(thread_handle_table[i]);
#pragma warning(default:6001)
	}
	if (flags & VRP_FLAG_BLOCK_UNTIL_FINISHED)
		vrp_thread_procedure((LPVOID)((uintptr_t)thread_group | (thread_count - 1)));

	return 0;
}
