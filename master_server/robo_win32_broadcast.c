/*
	VarastoRobo common code 2019-11-10 by Santtu Nyman.
*/

#include <stddef.h>
#include <stdint.h>
#include <Winsock2.h>
#include <windows.h>
#include "robo_win32_broadcast.h"

DWORD robo_win32_find_master(robo_win32_broadcast_info_t** broadcast_info)
{
	HANDLE heap = GetProcessHeap();
	if (!heap)
		return GetLastError();

	struct sockaddr_in any_address = { 0 };
	any_address.sin_family = AF_INET;
	any_address.sin_port = htons(1732);
	any_address.sin_addr.s_addr = INADDR_ANY;

	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
		return ERROR_NOT_SUPPORTED;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		WSACleanup();
		return ERROR_OPEN_FAILED;
	}

	static const BOOL broadcast_enable = TRUE;
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast_enable, sizeof(BOOL)) == -1)
	{
		closesocket(sock);
		WSACleanup();
		return ERROR_OPEN_FAILED;
	}

	if (bind(sock, (struct sockaddr*)&any_address, sizeof(struct sockaddr_in)) == -1)
	{
		closesocket(sock);
		WSACleanup();
		return ERROR_OPEN_FAILED;
	}

	struct sockaddr_in source_address;
	int source_address_size = sizeof(struct sockaddr_in);
	uint8_t buffer[512];

	int message_size = recvfrom(sock, (char*)buffer, sizeof(buffer), 0, (struct sockaddr*)&source_address, &source_address_size);
	if (message_size == -1)
	{
		closesocket(sock);
		WSACleanup();
		return ERROR_IO_DEVICE;
	}

	closesocket(sock);
	WSACleanup();

	if (message_size < 8)
		return ERROR_INVALID_DATA;
	
	uint16_t message_canstant = (uint16_t)buffer[0] | (uint16_t)(buffer[1] << 8);
	uint8_t system_status = buffer[2];
	uint8_t master_id = buffer[3];
	uint8_t map_height = buffer[4];
	uint8_t map_width = buffer[5];
	uint8_t block_count = buffer[6];
	uint8_t device_count = buffer[7];

	size_t map_memory_size = (((size_t)map_height * (size_t)map_width) + 7) / 8;
	size_t block_memory_size = (size_t)block_count * 2;
	size_t device_memory_size = (size_t)device_count * 8;

	if (message_canstant != 0x0701 || (8 + map_memory_size + block_memory_size + device_memory_size) != (size_t)message_size)
		return ERROR_INVALID_DATA;

	robo_win32_broadcast_info_t* info = (robo_win32_broadcast_info_t*)HeapAlloc(heap, 0, ((sizeof(robo_win32_broadcast_info_t) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1)) + ((size_t)device_count * sizeof(robo_win32_broadcast_device_t)) + map_memory_size + block_memory_size);
	if (!info)
		return ERROR_OUTOFMEMORY;

	info->device_table = (robo_win32_broadcast_device_t*)((UINT_PTR)info + ((sizeof(robo_win32_broadcast_info_t) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1)));
	info->valid_map_locations = (uint8_t*)((UINT_PTR)info + ((sizeof(robo_win32_broadcast_info_t) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1)) + ((size_t)device_count * sizeof(robo_win32_broadcast_device_t)));
	info->block_table = (uint8_t*)((UINT_PTR)info + ((sizeof(robo_win32_broadcast_info_t) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1)) + ((size_t)device_count * sizeof(robo_win32_broadcast_device_t)) + map_memory_size);

	info->master_ip_address = ntohl(source_address.sin_addr.s_addr);
	info->system_status = system_status;
	info->master_id = master_id;
	info->map_height = map_height;
	info->map_width = map_width;
	info->block_count = block_count;
	info->device_count = device_count;

	memcpy(info->valid_map_locations, buffer + 8, map_memory_size);
	memcpy(info->block_table, buffer + 8 + map_memory_size, block_memory_size);
	for (uint8_t i = 0; i != device_count; ++i)
	{
		uint8_t* buffer_device = buffer + 8 + map_memory_size + block_memory_size + ((size_t)i * 8);

		uint8_t device_type = buffer_device[0];
		uint8_t device_id = buffer_device[1];
		uint8_t device_x = buffer_device[2];
		uint8_t device_y = buffer_device[3];
		uint32_t device_ip_address = ((uint32_t)buffer_device[4] << 0) | ((uint32_t)buffer_device[5] << 8) | ((uint32_t)buffer_device[6] << 16) | ((uint32_t)buffer_device[7] << 24);
		
		info->device_table[i].type = device_type;
		info->device_table[i].id = device_id;
		info->device_table[i].x = device_x;
		info->device_table[i].y = device_y;
		info->device_table[i].ip_address = device_ip_address;
	}

	*broadcast_info = info;
	return 0;
}

void robo_win32_free_broadcast_info(robo_win32_broadcast_info_t* broadcast_info)
{
	HeapFree(GetProcessHeap(), 0, broadcast_info);
}

void wake_master_broadcast_thread(volatile robo_win32_broadcast_t* configuration)
{
	SetEvent(configuration->wake_event);
}

void end_master_broadcast_thread(volatile robo_win32_broadcast_t* configuration, DWORD error)
{
	configuration->error = error;
	MemoryBarrier();
	SetEvent(configuration->closed_event);
	WSACleanup();
	ExitThread(0);
}

DWORD master_broadcast_thread(volatile robo_win32_broadcast_t* configuration)
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		configuration->error = ERROR_NOT_SUPPORTED;
		MemoryBarrier();
		SetEvent(configuration->closed_event);
		ExitThread(0);
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
		end_master_broadcast_thread(configuration->closed_event, ERROR_OPEN_FAILED);

	static const BOOL broadcast_enable = TRUE;
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast_enable, sizeof(BOOL)) == -1)
	{
		closesocket(sock);
		end_master_broadcast_thread(configuration->closed_event, ERROR_NOT_SUPPORTED);
	}

	struct sockaddr_in broadcast_address = { 0 };
	broadcast_address.sin_family = AF_INET;
	broadcast_address.sin_port = htons(1732);
	broadcast_address.sin_addr.s_addr = INADDR_BROADCAST;

	uint8_t message[512];

	while (WaitForSingleObject(configuration->closed_event, 0) != WAIT_OBJECT_0)
	{
		if (WaitForSingleObject(configuration->mutex, INFINITE) == WAIT_OBJECT_0)
		{
			DWORD sleep_time = configuration->sleep_time;
			uint8_t map_height = configuration->info.map_height;
			uint8_t map_width = configuration->info.map_width;
			uint8_t block_count = configuration->info.block_count;
			uint8_t device_count = configuration->info.device_count;
			size_t map_bitmap_size = (((size_t)map_height * (size_t)map_width) + 7) / 8;
			size_t block_array_size = (size_t)block_count * 2;
			size_t device_array_size = (size_t)device_count * 8;
			size_t packet_size = 8 + map_bitmap_size + block_array_size + device_array_size;
			if (packet_size <= 512)
			{
				message[0] = 0x01;
				message[1] = 0x07;
				message[2] = configuration->info.system_status;
				message[3] = configuration->info.master_id;
				message[4] = map_height;
				message[5] = map_width;
				message[6] = block_count;
				message[7] = device_count;
				memcpy(message + 8, configuration->info.valid_map_locations, map_bitmap_size);
				memcpy(message + 8 + map_bitmap_size, configuration->info.block_table, block_array_size);
				for (uint8_t i = 0; i != device_count; ++i)
				{
					uint8_t device_type = configuration->info.device_table[i].type;
					uint8_t device_id = configuration->info.device_table[i].id;
					uint8_t device_x = configuration->info.device_table[i].x;
					uint8_t device_y = configuration->info.device_table[i].y;
					uint32_t device_ip_address = configuration->info.device_table[i].ip_address;
					message[8 + map_bitmap_size + block_array_size + (i * 8) + 0] = device_type;
					message[8 + map_bitmap_size + block_array_size + (i * 8) + 1] = device_id;
					message[8 + map_bitmap_size + block_array_size + (i * 8) + 2] = device_x;
					message[8 + map_bitmap_size + block_array_size + (i * 8) + 3] = device_y;
					message[8 + map_bitmap_size + block_array_size + (i * 8) + 4] = (uint8_t)(device_ip_address & 0xFF);
					message[8 + map_bitmap_size + block_array_size + (i * 8) + 5] = (uint8_t)((device_ip_address >> 8) & 0xFF);
					message[8 + map_bitmap_size + block_array_size + (i * 8) + 6] = (uint8_t)((device_ip_address >> 16) & 0xFF);
					message[8 + map_bitmap_size + block_array_size + (i * 8) + 7] = (uint8_t)((device_ip_address >> 24) & 0xFF);
				}
			}
			ReleaseMutex(configuration->mutex);
			if (packet_size <= 512)
				sendto(sock, (const char*)message, (int)packet_size, 0, (struct sockaddr*)&broadcast_address, sizeof(struct sockaddr_in));
			WaitForSingleObject(configuration->wake_event, sleep_time);
		}
	}

	closesocket(sock);
	end_master_broadcast_thread(configuration->closed_event, 0);
	return 0;
}

DWORD robo_win32_create_broadcast_thread(volatile robo_win32_broadcast_t* configuration, DWORD ms_sleep_time, uint8_t system_status, uint8_t master_id, uint8_t map_height, uint8_t map_width, const uint8_t* valid_map_locations)
{
	if (!ms_sleep_time || map_height > 127 || map_width > 127)
		return ERROR_INVALID_PARAMETER;

	memset(configuration, 0, sizeof(robo_win32_broadcast_t));
	configuration->sleep_time = ms_sleep_time;
	configuration->info.master_ip_address = INADDR_BROADCAST;
	configuration->info.system_status = system_status;
	configuration->info.master_id = master_id;
	configuration->info.map_height = map_height;
	configuration->info.map_width = map_width;
	configuration->info.valid_map_locations = configuration->valid_map_location_buffer;
	configuration->info.block_table = configuration->block_buffer;
	configuration->info.device_table = configuration->device_buffer;
	memcpy(configuration->info.valid_map_locations, valid_map_locations, (((size_t)map_height * (size_t)map_width) + 7) / 8);
	
	DWORD error;

	HANDLE stop_event = CreateEventW(0, FALSE, FALSE, 0);
	if (!stop_event)
	{
		error = GetLastError();
		return error;
	}

	HANDLE closed_event = CreateEventW(0, FALSE, FALSE, 0);
	if (!closed_event)
	{
		error = GetLastError();
		CloseHandle(stop_event);
		return error;
	}

	HANDLE wake_event = CreateEventW(0, FALSE, FALSE, 0);
	if (!wake_event)
	{
		error = GetLastError();
		CloseHandle(closed_event);
		CloseHandle(stop_event);
		return error;
	}

	HANDLE mutex = CreateMutexW(0, FALSE, 0);
	if (!mutex)
	{
		error = GetLastError();
		CloseHandle(wake_event);
		CloseHandle(closed_event);
		CloseHandle(stop_event);
		return error;
	}

	configuration->stop_event = stop_event;
	configuration->closed_event = closed_event;
	configuration->wake_event = wake_event;
	configuration->mutex = mutex;
	configuration->error = 0;
	
	HANDLE thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)master_broadcast_thread, configuration, 0, 0);
	if (!thread)
	{
		error = GetLastError();
		CloseHandle(mutex);
		CloseHandle(wake_event);
		CloseHandle(closed_event);
		CloseHandle(stop_event);
		return error;
	}
	CloseHandle(thread);
	return 0;
}

void robo_win32_stop_broadcast_thread(volatile robo_win32_broadcast_t* configuration)
{
	SetEvent(configuration->stop_event);
	wake_master_broadcast_thread(configuration);
	WaitForSingleObject(configuration->closed_event, INFINITE);
	CloseHandle(configuration->mutex);
	CloseHandle(configuration->wake_event);
	CloseHandle(configuration->closed_event);
	CloseHandle(configuration->stop_event);
}

void robo_win32_set_broadcast_master_id(volatile robo_win32_broadcast_t* configuration, uint8_t master_id)
{
	WaitForSingleObject(configuration->mutex, INFINITE);
	configuration->info.master_id = master_id;
	ReleaseMutex(configuration->mutex);
	wake_master_broadcast_thread(configuration);
}

void robo_win32_set_broadcast_sleep_time(volatile robo_win32_broadcast_t* configuration, uint32_t ms_sleep_time)
{
	WaitForSingleObject(configuration->mutex, INFINITE);
	configuration->sleep_time = ms_sleep_time;
	ReleaseMutex(configuration->mutex);
	wake_master_broadcast_thread(configuration);
}

void robo_win32_set_system_status(volatile robo_win32_broadcast_t* configuration, uint8_t system_status)
{
	WaitForSingleObject(configuration->mutex, INFINITE);
	configuration->info.system_status = system_status;
	ReleaseMutex(configuration->mutex);
	wake_master_broadcast_thread(configuration);
}

int robo_win32_add_broadcast_block(volatile robo_win32_broadcast_t* configuration, uint8_t x, uint8_t y)
{
	WaitForSingleObject(configuration->mutex, INFINITE);
	uint8_t block_count = configuration->info.block_count;
	if (block_count == 64)
	{
		ReleaseMutex(configuration->mutex);
		return 0;
	}
	configuration->info.block_table[2 * block_count] = x;
	configuration->info.block_table[2 * block_count + 1] = y;
	configuration->info.block_count = block_count + 1;
	ReleaseMutex(configuration->mutex);
	wake_master_broadcast_thread(configuration);
	return 1;
}

int robo_win32_remove_broadcast_block(volatile robo_win32_broadcast_t* configuration, uint8_t x, uint8_t y)
{
	WaitForSingleObject(configuration->mutex, INFINITE);
	uint8_t block_count = configuration->info.block_count;
	for (uint8_t i = 0; i != block_count; ++i)
		if (configuration->info.block_table[2 * i] == x && configuration->info.block_table[2 * i + 1] == y)
		{
			if (i != block_count - 1)
			{
				configuration->info.block_table[2 * i] = configuration->info.block_table[2 * (block_count - 1)];
				configuration->info.block_table[2 * i + 1] = configuration->info.block_table[2 * (block_count - 1) + 1];
			}
			configuration->info.block_count = block_count - 1;
			ReleaseMutex(configuration->mutex);
			wake_master_broadcast_thread(configuration);
			return 1;
		}
	ReleaseMutex(configuration->mutex);
	return 0;
}

int robo_win32_add_broadcast_device(volatile robo_win32_broadcast_t* configuration, uint8_t device_type, uint8_t device_id, uint8_t x, uint8_t y, uint32_t device_ip)
{
	WaitForSingleObject(configuration->mutex, INFINITE);
	uint8_t device_count = configuration->info.device_count;
	if (device_count == 64)
	{
		ReleaseMutex(configuration->mutex);
		return 0;
	}
	configuration->info.device_table[device_count].type = device_type;
	configuration->info.device_table[device_count].id = device_id;
	configuration->info.device_table[device_count].x = x;
	configuration->info.device_table[device_count].y = y;
	configuration->info.device_table[device_count].ip_address = device_ip;
	configuration->info.device_count = device_count + 1;
	ReleaseMutex(configuration->mutex);
	wake_master_broadcast_thread(configuration);
	return 1;
}

int robo_win32_remove_broadcast_device_by_id(volatile robo_win32_broadcast_t* configuration, uint8_t device_id)
{
	WaitForSingleObject(configuration->mutex, INFINITE);
	uint8_t device_count = configuration->info.device_count;
	for (uint8_t i = 0; i != device_count; ++i)
		if (configuration->info.device_table[i].id == device_id)
		{
			if (i != device_count - 1)
			{
				configuration->info.device_table[i].type = configuration->info.device_table[device_count - 1].type;
				configuration->info.device_table[i].id = configuration->info.device_table[device_count - 1].id;
				configuration->info.device_table[i].x = configuration->info.device_table[device_count - 1].x;
				configuration->info.device_table[i].y = configuration->info.device_table[device_count - 1].y;
				configuration->info.device_table[i].ip_address = configuration->info.device_table[device_count - 1].ip_address;
			}
			configuration->info.device_count = device_count - 1;
			ReleaseMutex(configuration->mutex);
			wake_master_broadcast_thread(configuration);
			return 1;
		}
	ReleaseMutex(configuration->mutex);
	return 0;
}

int robo_win32_remove_broadcast_device_by_ip(volatile robo_win32_broadcast_t* configuration, uint32_t device_ip)
{
	WaitForSingleObject(configuration->mutex, INFINITE);
	uint8_t device_count = configuration->info.device_count;
	for (uint8_t i = 0; i != device_count; ++i)
		if (configuration->info.device_table[i].ip_address == device_ip)
		{
			if (i != device_count - 1)
			{
				configuration->info.device_table[i].type = configuration->info.device_table[device_count - 1].type;
				configuration->info.device_table[i].id = configuration->info.device_table[device_count - 1].id;
				configuration->info.device_table[i].x = configuration->info.device_table[device_count - 1].x;
				configuration->info.device_table[i].y = configuration->info.device_table[device_count - 1].y;
				configuration->info.device_table[i].ip_address = configuration->info.device_table[device_count - 1].ip_address;
			}
			configuration->info.device_count = device_count - 1;
			ReleaseMutex(configuration->mutex);
			wake_master_broadcast_thread(configuration);
			return 1;
		}
	ReleaseMutex(configuration->mutex);
	return 0;
}