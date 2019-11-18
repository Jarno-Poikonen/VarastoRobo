/*
	VarastoRobo master server version 0.4.0 2019-11-18 by Santtu Nyman.
*/

#ifndef _M_X64
#error Platform not supported!
#endif

#include <stddef.h>
#include <stdint.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "vrp_constants.h"
#include "vrp_file.h"
#include "vrp_configuration.h"
#include "jsonpl.h"
#include <stdio.h>
#include "vrp_test_client.h"
#include "ntdll_time.h"
#include "vrp_log.h"

#define VRP_MAX_MAP_HEIGHT 64
#define VRP_MAX_MAP_WIDTH 64
#define VRP_MAP_SIZE ((size_t)(((VRP_MAX_MAP_HEIGHT * VRP_MAX_MAP_WIDTH) + 7) / 8))
#define VRP_MAX_BLOCK_COUNT 256
#define VRP_MAX_PICKUP_LOCATION_COUNT 256
#define VRP_MAX_DEVICE_COUNT 32
#define VRP_DEVICE_IO_MEMORY_SIZE 0x100000
#define VRP_IO_IDLE 0
#define VRP_IO_READ 1
#define VRP_IO_WRITE 2
#define VRP_WAIT_ACCAPT 0x80000001
#define VRP_WAIT_EMERGENCY 0x80000002
#define VRP_WAIT_BROADCAST 0x80000003
#define VRP_CONNECTION_NEW 0
#define VRP_CONNECTION_SETUP 1
#define VRP_CONNECTION_IDLE 2
#define VRP_CONNECTION_SENDING_COMMAND 3
#define VRP_CONNECTION_WAITING_FOR_RESPONSE 4
#define VRP_CONNECTION_WAITING_FOR_COMMAND 5
#define VRP_CONNECTION_RESPONDING_TO_COMMAND 6
#define VRP_CONNECTION_DISCONNECT 7

typedef struct vrp_server_t
{
	struct sockaddr_in broadcast_address;
	struct sockaddr_in server_address;
	struct sockaddr_in emergency_address;
	DWORD system_tick_time;
	DWORD io_timeout;
	DWORD command_timeout;
	DWORD broadcast_delay;
	uint8_t status;
	uint8_t id;
	uint8_t map_height;
	uint8_t map_width;
	uint8_t min_temporal_id;
	uint8_t max_temporal_id;
	size_t block_count;
	uint8_t* map;
	struct
	{
		uint8_t x;
		uint8_t y;
	}* block_table;
	size_t device_count;
	struct
	{
		SOCKET sock;
		int io_state;
		int unprocessed_io;
		int connection_state;
		int control_overridden;
		DWORD io_begin_time;
		DWORD last_uppdate_time;
		DWORD connection_begin_time;
		uint8_t command;
		uint8_t type;
		uint8_t id;
		uint8_t state;
		uint8_t x;
		uint8_t y;
		uint8_t direction;
		uint32_t ip_address;
		uint8_t destination_x;
		uint8_t destination_y;
		uint8_t* io_memory;
		DWORD io_flags;
		DWORD io_transfered;
		WSABUF io_buffer;
		OVERLAPPED io_result;
	}*device_table;
	size_t pickup_location_count;
	struct
	{
		uint8_t id;
		uint8_t x;
		uint8_t y;
	}* pickup_location_table;
	int debug_no_emergency_listen;
	size_t page_size;
	size_t map_buffer_size;
	size_t block_buffer_size;
	size_t pickup_location_buffer_size;
	size_t device_buffer_size;
	size_t broadcast_io_buffer_size;
	size_t emergency_io_buffer_size;
	size_t device_io_buffer_size;
	size_t allocation_size;
	void* allocation_base;
	uint64_t last_broadcast_time;
	int broadcast_io_state;
	uint8_t* broadcast_io_memory;
	WSABUF broadcast_io_buffer;
	OVERLAPPED broadcast_io_result;
	int emergency_io_state;
	DWORD emergency_io_flags;
	uint8_t* emergency_io_memory;
	WSABUF emergency_io_buffer;
	OVERLAPPED emergency_io_result;
	SOCKET broadcast_sock;
	SOCKET emergency_sock;
	SOCKET listen_sock;
	HANDLE accept_event;
	HANDLE io_event_table[MAXIMUM_WAIT_OBJECTS];
	vrp_log_t log;
} vrp_server_t;

uint8_t vrp_get_temporal_device_id(vrp_server_t* server)
{
	if (server->min_temporal_id == VRP_ID_UNDEFINED || server->max_temporal_id == VRP_ID_UNDEFINED || server->min_temporal_id > server->max_temporal_id)
		return VRP_ID_UNDEFINED;
	uint8_t id_offset = server->min_temporal_id;
	uint8_t id_count = (server->max_temporal_id - server->min_temporal_id) + 1;
	for (uint8_t relative_id = 0; relative_id != id_count; ++relative_id)
	{
		int id_is_free = 1;
		for (size_t i = 0; id_is_free && i != server->device_count; ++i)
			if (server->device_table[i].id == id_offset + relative_id)
				id_is_free = 0;
		if (id_is_free)
			return id_offset + relative_id;
	}
	return VRP_ID_UNDEFINED;
}

DWORD vrp_get_system_tick()
{
	ULONG min_time_resulution_100ns;
	ULONG max_time_resulution_100ns;
	ULONG current_time_resulution_100ns;
	if ((NtQueryTimerResolution(&min_time_resulution_100ns, &max_time_resulution_100ns, &current_time_resulution_100ns) >> 31) || !current_time_resulution_100ns)
		return 1;
	else
		return (current_time_resulution_100ns + 9999) / 10000;
}

size_t vrp_get_message_size(const void* message)
{
	return (size_t)(((uint32_t) * (const uint8_t*)((uintptr_t)message + 1) << 0) |
		((uint32_t)*(const uint8_t*)((uintptr_t)message + 2) << 8) |
		((uint32_t)*(const uint8_t*)((uintptr_t)message + 3) << 16) |
		((uint32_t)*(const uint8_t*)((uintptr_t)message + 4) << 24));
}

int vrp_read(vrp_server_t* server, size_t device_index, size_t offset, size_t size)
{
	server->device_table[device_index].io_flags = 0;
	server->device_table[device_index].io_buffer.buf = (CHAR*)(server->device_table[device_index].io_memory + offset);
	server->device_table[device_index].io_buffer.len = (ULONG)size;
	server->device_table[device_index].io_result.Internal = 0;
	server->device_table[device_index].io_result.InternalHigh = 0;
	server->device_table[device_index].io_result.Offset = 0;
	server->device_table[device_index].io_result.OffsetHigh = 0;
	if (!WSARecv(server->device_table[device_index].sock,
		&server->device_table[device_index].io_buffer, 1, 0,
		&server->device_table[device_index].io_flags,
		&server->device_table[device_index].io_result, 0) ||
		WSAGetLastError() == WSA_IO_PENDING)
	{
		server->device_table[device_index].io_begin_time = NtGetTickCount();
		server->device_table[device_index].io_state = VRP_IO_READ;
		return 1;
	}
	else
		return 0;
}

int vrp_write(vrp_server_t* server, size_t device_index, size_t offset, size_t size)
{
	server->device_table[device_index].io_flags = 0;
	server->device_table[device_index].io_buffer.buf = (CHAR*)(server->device_table[device_index].io_memory + offset);
	server->device_table[device_index].io_buffer.len = (ULONG)size;
	server->device_table[device_index].io_result.Internal = 0;
	server->device_table[device_index].io_result.InternalHigh = 0;
	server->device_table[device_index].io_result.Offset = 0;
	server->device_table[device_index].io_result.OffsetHigh = 0;
	if (!WSASend(server->device_table[device_index].sock,
		&server->device_table[device_index].io_buffer, 1, 0,
		server->device_table[device_index].io_flags,
		&server->device_table[device_index].io_result, 0) ||
		WSAGetLastError() == WSA_IO_PENDING)
	{
		server->device_table[device_index].io_begin_time = NtGetTickCount();
		server->device_table[device_index].io_state = VRP_IO_WRITE;
		return 1;
	}
	else
		return 0;
}

size_t vrp_finish_io(vrp_server_t* server, size_t device_index, int* io_type)
{
	BOOL io_successful = WSAGetOverlappedResult(server->device_table[device_index].sock,
		&server->device_table[device_index].io_result,
		&server->device_table[device_index].io_transfered,
		FALSE,
		&server->device_table[device_index].io_flags) &&
		server->device_table[device_index].io_transfered;
	ResetEvent(server->device_table[device_index].io_result.hEvent);
	*io_type = (int)server->device_table[device_index].io_state;
	server->device_table[device_index].io_buffer.buf = (CHAR*)((uintptr_t)server->device_table[device_index].io_buffer.buf + (size_t)server->device_table[device_index].io_transfered);
	server->device_table[device_index].io_state = VRP_IO_IDLE;
	return io_successful ? (size_t)((uintptr_t)server->device_table[device_index].io_buffer.buf - (uintptr_t)server->device_table[device_index].io_memory) : 0;
}

size_t vrp_message_transfer_incomplete(vrp_server_t* server, size_t device_index, int io_type)
{
	size_t transfer_size = (size_t)((uintptr_t)server->device_table[device_index].io_buffer.buf - (uintptr_t)server->device_table[device_index].io_memory);
	size_t message_size = vrp_get_message_size(server->device_table[device_index].io_memory);
	if (io_type == VRP_IO_READ)
	{
		// message_size is undefined if received < 5
		if (transfer_size < 5 || transfer_size < (5 + message_size))
		{
			if (transfer_size < 5)
				return server->device_io_buffer_size - transfer_size;
			else if ((5 + message_size) <= server->device_io_buffer_size)
				return (5 + message_size) - transfer_size;
			else
				return (size_t)~0;
		}
		else if (transfer_size == (5 + message_size))
			return 0;
		else
			return (size_t)~0;
	}
	else if (io_type == VRP_IO_WRITE)
	{
		if (server->device_table[device_index].io_memory[0] == VRP_MESSAGE_SBM)
			return 0;
		else
		{
			if (transfer_size < (5 + message_size))
				return (5 + message_size) - transfer_size;
			else if (transfer_size == (5 + message_size))
				return 0;
			else
				return (size_t)~0;
		}
	}
	else
		return (size_t)~0;
}

int vrp_process_possible_emergency(vrp_server_t* server)
{
	int emergency_stop = 0;
	DWORD io_transfered;
	if (WSAGetOverlappedResult(server->emergency_sock, &server->emergency_io_result, &io_transfered, FALSE, &server->emergency_io_flags) && io_transfered)
	{
		if (io_transfered >= 8 && server->emergency_io_memory[0] == 0x01 && server->emergency_io_memory[1] == 0x07 && server->emergency_io_memory[2] != server->id && server->emergency_io_memory[3] == 0x00)
		{
			server->status = 0;
			server->last_broadcast_time = 0;
			emergency_stop = 1;
		}
	}
	ResetEvent(server->emergency_io_result.hEvent);
	server->emergency_io_state = VRP_IO_IDLE;
	return emergency_stop;
}

size_t vrp_get_free_device_slot(vrp_server_t* server)
{
	// assume slot exists
	for (size_t i = 0;; ++i)
		if (server->device_table[i].sock == INVALID_SOCKET)
			return i;
}

size_t vrp_get_device_index_by_io_event(vrp_server_t* server, HANDLE io_event)
{
	// assume device exists
	for (size_t i = 0;; ++i)
		if (server->device_table[i].io_result.hEvent == io_event)
			return i;
}

size_t vrp_wait_for_io(vrp_server_t* server)
{
	if (!server->debug_no_emergency_listen && server->emergency_io_state == VRP_IO_IDLE)
	{
		server->emergency_io_flags = 0;
		server->emergency_io_buffer.buf = (CHAR*)server->emergency_io_memory;
		server->emergency_io_buffer.len = 0x200;
		server->emergency_io_result.Internal = 0;
		server->emergency_io_result.InternalHigh = 0;
		server->emergency_io_result.Offset = 0;
		server->emergency_io_result.OffsetHigh = 0;
		if (!WSARecvFrom(server->emergency_sock,
			&server->emergency_io_buffer, 1, 0, &server->emergency_io_flags,
			0, 0,
			&server->emergency_io_result, 0) ||
			WSAGetLastError() == WSA_IO_PENDING)
			server->emergency_io_state = VRP_IO_READ;
		else
			server->emergency_io_state = VRP_IO_IDLE;
	}

	DWORD io_event_count = 1;
	DWORD io_event_accept = 0;
	DWORD io_event_emergency = WAIT_TIMEOUT;
	DWORD io_event_broadcast = WAIT_TIMEOUT;
	server->io_event_table[io_event_accept] = server->accept_event;
	if (server->emergency_io_state == VRP_IO_READ)
	{
		io_event_emergency = io_event_count;
		++io_event_count;
		server->io_event_table[io_event_emergency] = server->emergency_io_result.hEvent;
	}
	if (server->broadcast_io_state == VRP_IO_WRITE)
	{
		io_event_broadcast = io_event_count;
		++io_event_count;
		server->io_event_table[io_event_broadcast] = server->broadcast_io_result.hEvent;
	}
	for (DWORD device_count = 0, i = 0; device_count != (DWORD)server->device_count; ++i)
		if (server->device_table[i].sock != INVALID_SOCKET)
		{
			if (server->device_table[i].io_state == VRP_IO_READ || server->device_table[i].io_state == VRP_IO_WRITE)
			{
				server->io_event_table[io_event_count] = server->device_table[i].io_result.hEvent;
				++io_event_count;
			}
			++device_count;
		}

	DWORD wait_begin_time = NtGetTickCount();
	DWORD max_wait_time = ((wait_begin_time - server->last_broadcast_time) < server->broadcast_delay) ? (wait_begin_time - server->last_broadcast_time) : (server->broadcast_io_state == VRP_IO_WRITE ? server->broadcast_delay : 0);
	DWORD event_index = WaitForMultipleObjects(io_event_count, server->io_event_table, FALSE, max_wait_time);

	if (event_index < MAXIMUM_WAIT_OBJECTS)
	{
		if (event_index == io_event_accept)
			return (size_t)VRP_WAIT_ACCAPT;
		else if (event_index == io_event_emergency)
			return (size_t)VRP_WAIT_EMERGENCY;
		else if (event_index == io_event_broadcast)
			return (size_t)VRP_WAIT_BROADCAST;
		else
			return vrp_get_device_index_by_io_event(server, server->io_event_table[event_index]);
	}
	else if (event_index == WAIT_TIMEOUT)
		return (size_t)WAIT_TIMEOUT;
	else
		return (size_t)WAIT_FAILED;
}

BOOL WINAPI vrp_ctr_c_close_process_routine(DWORD parameter) { ExitProcess(0); }

size_t vrp_get_page_size() { SYSTEM_INFO system_info; GetSystemInfo(&system_info); return (size_t)system_info.dwPageSize; }

void vrp_server_close(vrp_server_t* server)
{
	if (server->device_table)
		for (size_t i = VRP_MAX_DEVICE_COUNT; i--;)
			if (server->device_table[i].io_result.hEvent)
			{
				CloseHandle(server->device_table[i].io_result.hEvent);
				server->device_table[i].io_result.hEvent = 0;
			}
	if (server->listen_sock != INVALID_SOCKET)
	{
		closesocket(server->listen_sock);
		server->listen_sock = INVALID_SOCKET;
	}
	if (server->broadcast_sock != INVALID_SOCKET)
	{
		closesocket(server->broadcast_sock);
		server->broadcast_sock = INVALID_SOCKET;
	}
	if (server->emergency_sock != INVALID_SOCKET)
	{
		closesocket(server->emergency_sock);
		server->emergency_sock = INVALID_SOCKET;
	}
	if (server->accept_event)
	{
		CloseHandle(server->accept_event);
		server->accept_event = 0;
	}
	if (server->broadcast_io_result.hEvent)
	{
		CloseHandle(server->broadcast_io_result.hEvent);
		server->broadcast_io_result.hEvent = 0;
	}
	if (server->emergency_io_result.hEvent)
	{
		CloseHandle(server->emergency_io_result.hEvent);
		server->emergency_io_result.hEvent = 0;
	}
	vrp_close_log(&server->log);
	if (server->allocation_base)
	{
		VirtualFree(server->allocation_base, 0, MEM_RELEASE);
		server->allocation_base = 0;
	}
	WSACleanup();
}

DWORD vrp_server_setup(vrp_server_t* server)
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
		return ERROR_NOT_SUPPORTED;

	memset(server, 0, sizeof(vrp_server_t));
	server->log.handle = INVALID_HANDLE_VALUE;
	server->emergency_sock = INVALID_SOCKET;
	server->broadcast_sock = INVALID_SOCKET;
	server->listen_sock = INVALID_SOCKET;

	vrp_configuration_t* configuration;
	DWORD error = vrp_load_master_configuration(&configuration);

	if (error)
	{
		vrp_server_close(server);
		return error;
	}

	if (configuration->block_count > VRP_MAX_BLOCK_COUNT || configuration->pickup_location_count > VRP_MAX_PICKUP_LOCATION_COUNT)
	{
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		return ERROR_INVALID_DATA;
	}

	SetConsoleCtrlHandler(vrp_ctr_c_close_process_routine, TRUE);

	const BOOL broadcast_enable = TRUE;

	server->broadcast_address.sin_family = AF_INET;
	server->broadcast_address.sin_port = htons(1732);
	server->broadcast_address.sin_addr.s_addr = configuration->on_wire_broadcast_ip_address;

	server->server_address.sin_family = AF_INET;
	server->server_address.sin_port = htons(1739);
	server->server_address.sin_addr.s_addr = configuration->on_wire_server_ip_address;

	server->emergency_address.sin_family = AF_INET;
	server->emergency_address.sin_port = htons(1732);
	server->emergency_address.sin_addr.s_addr = INADDR_ANY;

	if (server->server_address.sin_addr.s_addr == INADDR_ANY)
	{
		if (server->broadcast_address.sin_addr.s_addr == INADDR_BROADCAST && server->server_address.sin_addr.s_addr == INADDR_ANY)
			printf("default address configuration\n");

		struct addrinfo host_address_hints;
		memset(&host_address_hints, 0, sizeof(struct addrinfo));
		host_address_hints.ai_family = AF_UNSPEC;
		host_address_hints.ai_socktype = SOCK_STREAM;
		host_address_hints.ai_protocol = IPPROTO_TCP;
		struct addrinfo* host_address;
		if (!getaddrinfo("", 0, &host_address_hints, &host_address))
		{
			for (struct addrinfo* address = host_address; address; address = address ? address->ai_next : 0)
				if (address->ai_family == AF_INET)
				{
					server->server_address.sin_addr = ((struct sockaddr_in*)address->ai_addr)->sin_addr;
					address = 0;
				}
			freeaddrinfo(host_address);
		}
	}

	server->page_size = vrp_get_page_size();
	size_t required_map_buffer_size = ((size_t)VRP_MAP_SIZE + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_block_buffer_size = ((size_t)(VRP_MAX_BLOCK_COUNT * sizeof(*server->block_table)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_pickup_location_buffer_size = ((size_t)(VRP_MAX_PICKUP_LOCATION_COUNT * sizeof(*server->pickup_location_table)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_device_buffer_size = ((size_t)(VRP_MAX_DEVICE_COUNT * sizeof(*server->device_table)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	if (required_map_buffer_size + required_block_buffer_size + required_pickup_location_buffer_size <= server->page_size)
	{
		server->map_buffer_size = required_map_buffer_size;
		server->block_buffer_size = required_block_buffer_size;
		server->pickup_location_buffer_size = server->page_size - (server->map_buffer_size + server->block_buffer_size);
	}
	else
	{
		server->map_buffer_size = (required_map_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
		server->block_buffer_size = (required_block_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
		server->pickup_location_buffer_size = (required_pickup_location_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
	}
	server->device_buffer_size = (required_device_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
	if (server->page_size >= 0x400)
	{
		server->broadcast_io_buffer_size = server->page_size / 2;
		server->emergency_io_buffer_size = server->page_size - server->broadcast_io_buffer_size;
	}
	else
	{
		server->broadcast_io_buffer_size = (0x200 + (server->page_size - 1)) & ~(server->page_size - 1);
		server->emergency_io_buffer_size = (0x200 + (server->page_size - 1)) & ~(server->page_size - 1);
	}
	server->device_io_buffer_size = (VRP_DEVICE_IO_MEMORY_SIZE + (server->page_size - 1)) & ~(server->page_size - 1);

	server->allocation_size = server->map_buffer_size +
		server->block_buffer_size +
		server->pickup_location_buffer_size +
		server->device_buffer_size +
		server->broadcast_io_buffer_size +
		server->emergency_io_buffer_size +
		(VRP_MAX_DEVICE_COUNT * server->device_io_buffer_size);
	server->allocation_base = VirtualAlloc(0, server->allocation_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!server->allocation_base)
		return ERROR_OUTOFMEMORY;

	server->map = (uint8_t*)server->allocation_base;
	server->block_table = (void*)((UINT_PTR)server->allocation_base + server->map_buffer_size);
	server->pickup_location_table = (void*)((UINT_PTR)server->allocation_base + server->map_buffer_size + server->block_buffer_size);
	server->device_table = (void*)((UINT_PTR)server->allocation_base + server->map_buffer_size + server->block_buffer_size + server->pickup_location_buffer_size);
	server->broadcast_io_memory = (uint8_t*)((UINT_PTR)server->allocation_base + server->map_buffer_size + server->block_buffer_size + server->pickup_location_buffer_size + server->device_buffer_size);
	server->emergency_io_memory = (uint8_t*)((UINT_PTR)server->allocation_base + server->map_buffer_size + server->block_buffer_size + server->pickup_location_buffer_size + server->device_buffer_size + server->broadcast_io_buffer_size);
	for (size_t offset = server->map_buffer_size + server->block_buffer_size + server->pickup_location_buffer_size + server->device_buffer_size + server->broadcast_io_buffer_size + server->emergency_io_buffer_size, i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
		server->device_table[i].io_memory = (uint8_t*)((UINT_PTR)server->allocation_base + offset + (i * server->device_io_buffer_size));

	error = vrp_open_log_file(&server->log, 0x200000000, 0x100000, 0);
	if (error)
	{
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		return error;
	}

	server->emergency_io_state = VRP_IO_IDLE;
	server->emergency_io_result.hEvent = CreateEventW(0, TRUE, FALSE, 0);
	if (!server->emergency_io_result.hEvent)
	{
		error = GetLastError();
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		return error;
	}

	server->last_broadcast_time = 0;
	server->broadcast_io_state = VRP_IO_IDLE;
	server->broadcast_io_result.hEvent = CreateEventW(0, TRUE, FALSE, 0);
	if (!server->broadcast_io_result.hEvent)
	{
		error = GetLastError();
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		return error;
	}

	server->accept_event = CreateEventW(0, TRUE, FALSE, 0);
	if (!server->accept_event)
	{
		error = GetLastError();
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		return error;
	}

	if (configuration->debug_no_emergency_listen)
		server->debug_no_emergency_listen = 1;
	else
	{
		server->emergency_sock = WSASocketW(AF_INET, SOCK_DGRAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
		if (server->emergency_sock == INVALID_SOCKET)
		{
			error = ERROR_OPEN_FAILED;
			vrp_free_master_configuration(configuration);
			vrp_server_close(server);
			return error;
		}

		const BOOL reuse_address_enable = TRUE;
		setsockopt(server->emergency_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_address_enable, sizeof(BOOL));
		// this is not needed to be successful, but helps when running clients on same machine

		int wsa_error_1 = WSAGetLastError();

		if (setsockopt(server->emergency_sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast_enable, sizeof(BOOL)) ||
			bind(server->emergency_sock, (struct sockaddr*)&server->emergency_address, sizeof(struct sockaddr_in)))
		{
			int wsa_error_2 = WSAGetLastError();

			error = ERROR_OPEN_FAILED;
			vrp_free_master_configuration(configuration);
			vrp_server_close(server);
			return error;
		}
	}

	server->broadcast_sock = WSASocketW(AF_INET, SOCK_DGRAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	if (server->broadcast_sock == INVALID_SOCKET)
	{
		error = ERROR_OPEN_FAILED;
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		return error;
	}

	if (setsockopt(server->broadcast_sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast_enable, sizeof(BOOL)))
	{
		error = ERROR_OPEN_FAILED;
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		return error;
	}

	server->listen_sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (server->listen_sock == INVALID_SOCKET)
	{
		error = ERROR_OPEN_FAILED;
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		return error;
	}

	if (bind(server->listen_sock, (SOCKADDR*)&server->server_address, sizeof(SOCKADDR)) == SOCKET_ERROR ||
		listen(server->listen_sock, SOMAXCONN) == SOCKET_ERROR ||
		WSAEventSelect(server->listen_sock, server->accept_event, FD_ACCEPT) == SOCKET_ERROR)
	{
		error = ERROR_OPEN_FAILED;
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		return error;
	}

	for (size_t i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
	{
		server->device_table[i].sock = INVALID_SOCKET;
		server->device_table[i].io_result.hEvent = CreateEventW(0, TRUE, FALSE, 0);
		if (!server->device_table[i].io_result.hEvent)
		{
			error = GetLastError();
			vrp_free_master_configuration(configuration);
			vrp_server_close(server);
			return error;
		}
	}

	server->block_count = configuration->block_count;
	for (size_t i = 0; i != configuration->block_count; ++i)
	{
		server->block_table[i].x = configuration->block_table[i * 2 + 0];
		server->block_table[i].y = configuration->block_table[i * 2 + 1];
	}

	server->system_tick_time = vrp_get_system_tick();
	server->io_timeout = configuration->io_ms_timeout;
	server->command_timeout = configuration->command_ms_timeout;
	server->broadcast_delay = configuration->broadcast_ms_delay;
	server->status = configuration->system_status;
	server->id = configuration->master_id;
	server->map_height = configuration->map_height;
	server->map_width = configuration->map_width;
	server->min_temporal_id = configuration->min_temporal_id;
	server->max_temporal_id = configuration->max_temporal_id;

	memcpy(server->map, configuration->map, ((server->map_height * server->map_width) + 7) / 8);

	server->pickup_location_count = configuration->pickup_location_count;
	for (size_t i = 0; i != configuration->pickup_location_count; ++i)
	{
		server->pickup_location_table[i].id = configuration->pickup_location_table[i * 3 + 0];
		server->pickup_location_table[i].x = configuration->pickup_location_table[i * 3 + 1];
		server->pickup_location_table[i].y = configuration->pickup_location_table[i * 3 + 2];
	}

	vrp_free_master_configuration(configuration);

	char startup_log_entry[128];
	sprintf(startup_log_entry, "Server started with status %lu and id %lu at address %lu.%lu.%lu.%lu", server->status, server->id,
		((server->server_address.sin_addr.s_addr) >> 0) & 0xFF, ((server->server_address.sin_addr.s_addr) >> 8) & 0xFF, ((server->server_address.sin_addr.s_addr) >> 16) & 0xFF, ((server->server_address.sin_addr.s_addr) >> 24) & 0xFF);
	vrp_write_log_entry(&server->log, startup_log_entry);

	return 0;
}

void vrp_remove_device(vrp_server_t* server, size_t i)
{
	if (server->device_table[i].connection_state != VRP_CONNECTION_DISCONNECT)
	{
		char log_entry_buffer[128];
		sprintf(log_entry_buffer, "Device %lu connection failed and disconnected from address %lu.%lu.%lu.%lu", server->device_table[i].id, ((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
		vrp_write_log_entry(&server->log, log_entry_buffer);
	}
	closesocket(server->device_table[i].sock);
	server->device_table[i].sock = INVALID_SOCKET;
	server->device_count--;
}

size_t vrp_send_system_broadcast_message(vrp_server_t* server)
{
	size_t map_size = (((size_t)server->map_height * (size_t)server->map_width) + 7) / 8;
	size_t block_array_size = server->block_count * 2;
	size_t device_array_size = server->device_count * 8;
	size_t packet_size = 8 + map_size + block_array_size + device_array_size;

	if (packet_size > 512)
	{
		server->broadcast_io_state = VRP_IO_IDLE;
		return 0;
	}

	server->broadcast_io_memory[0] = 0x01;
	server->broadcast_io_memory[1] = 0x07;
	server->broadcast_io_memory[2] = server->status;
	server->broadcast_io_memory[3] = server->id;
	server->broadcast_io_memory[4] = server->map_height;
	server->broadcast_io_memory[5] = server->map_width;
	server->broadcast_io_memory[6] = (uint8_t)server->block_count;
	server->broadcast_io_memory[7] = (uint8_t)server->device_count;
	memcpy(server->broadcast_io_memory + 8, server->map, map_size);
	for (size_t i = 0; i != server->block_count; ++i)
	{
		server->broadcast_io_memory[8 + map_size + (i * 2) + 0] = server->block_table[i].x;
		server->broadcast_io_memory[8 + map_size + (i * 2) + 1] = server->block_table[i].y;
	}
	for (size_t i = 0; i != server->device_count; ++i)
	{
		server->broadcast_io_memory[8 + map_size + block_array_size + (i * 8) + 0] = server->device_table[i].type;
		server->broadcast_io_memory[8 + map_size + block_array_size + (i * 8) + 1] = server->device_table[i].id;
		server->broadcast_io_memory[8 + map_size + block_array_size + (i * 8) + 2] = server->device_table[i].x;
		server->broadcast_io_memory[8 + map_size + block_array_size + (i * 8) + 3] = server->device_table[i].y;
		server->broadcast_io_memory[8 + map_size + block_array_size + (i * 8) + 4] = (uint8_t)((server->device_table[i].ip_address >> 0) & 0xFF);
		server->broadcast_io_memory[8 + map_size + block_array_size + (i * 8) + 5] = (uint8_t)((server->device_table[i].ip_address >> 8) & 0xFF);
		server->broadcast_io_memory[8 + map_size + block_array_size + (i * 8) + 6] = (uint8_t)((server->device_table[i].ip_address >> 16) & 0xFF);
		server->broadcast_io_memory[8 + map_size + block_array_size + (i * 8) + 7] = (uint8_t)((server->device_table[i].ip_address >> 24) & 0xFF);
	}

	server->broadcast_io_buffer.buf = server->broadcast_io_memory;
	server->broadcast_io_buffer.len = (ULONG)packet_size;
	server->broadcast_io_result.Internal = 0;
	server->broadcast_io_result.InternalHigh = 0;
	server->broadcast_io_result.Offset = 0;
	server->broadcast_io_result.OffsetHigh = 0;

	if (!WSASendTo(server->broadcast_sock, &server->broadcast_io_buffer, 1, 0, 0, (const struct sockaddr*)&server->broadcast_address, sizeof(struct sockaddr_in), &server->broadcast_io_result, 0) ||
		WSAGetLastError() == WSA_IO_PENDING)
	{
		server->broadcast_io_state = VRP_IO_WRITE;
		server->last_broadcast_time = GetTickCount64();
		return packet_size;
	}
	else
	{
		server->broadcast_io_state = VRP_IO_IDLE;
		return 0;
	}
}

void vrp_finish_send_system_broadcast_message(vrp_server_t* server)
{
	DWORD io_transfered;
	DWORD io_flags = 0;
	if (!WSAGetOverlappedResult(server->broadcast_sock, &server->broadcast_io_result, &io_transfered, FALSE, &io_flags) && io_transfered)
		server->last_broadcast_time = 0;
	ResetEvent(server->emergency_io_result.hEvent);
	server->broadcast_io_state = VRP_IO_IDLE;
}

int vrp_accept_incoming_connection(vrp_server_t* server)
{
	struct sockaddr_in client_address;
	int client_address_size = sizeof(struct sockaddr_in);
	SOCKET client_sock = accept(server->listen_sock, (struct sockaddr*)&client_address, &client_address_size);
	ResetEvent(server->accept_event);
	if (client_sock != INVALID_SOCKET)
	{
		if (client_address.sin_family == AF_INET && server->device_count != VRP_MAX_DEVICE_COUNT)
		{
			size_t i = vrp_get_free_device_slot(server);
			server->device_table[i].sock = client_sock;
			server->device_table[i].io_state = VRP_IO_IDLE;
			server->device_table[i].unprocessed_io = VRP_IO_IDLE;
			server->device_table[i].connection_state = VRP_CONNECTION_NEW;
			server->device_table[i].control_overridden = 0;
			server->device_table[i].io_begin_time = 0;
			server->device_table[i].last_uppdate_time = 0;
			server->device_table[i].connection_begin_time = NtGetTickCount();
			server->device_table[i].command = VRP_MESSAGE_UNDEFINED;
			server->device_table[i].type = VRP_DEVICE_TYPE_UNDEFINED;
			server->device_table[i].id = VRP_ID_UNDEFINED;
			server->device_table[i].state = VRP_STATE_UNDEFINED;
			server->device_table[i].x = VRP_COORDINATE_UNDEFINED;
			server->device_table[i].y = VRP_COORDINATE_UNDEFINED;
			server->device_table[i].direction = VRP_DIRECTION_UNDEFINED;
			server->device_table[i].ip_address = ntohl(client_address.sin_addr.s_addr);
			server->device_table[i].destination_x = VRP_COORDINATE_UNDEFINED;
			server->device_table[i].destination_y = VRP_COORDINATE_UNDEFINED;
			if (vrp_read(server, i, 0, server->device_io_buffer_size))
			{
				server->device_table[i].io_begin_time = server->device_table[i].connection_begin_time;
				server->device_count++;
				return 1;
			}
			else
			{
				closesocket(server->device_table[i].sock);// drop connection if socket stops working
				server->device_table[i].sock = INVALID_SOCKET;
			}
		}
		else
			closesocket(client_sock);// drop connection if too many devices
	}
	return 0;
}

int vrp_process_device(vrp_server_t* server, size_t i)
{
	char log_entry_buffer[256];
	DWORD current_time = NtGetTickCount();

	if ((((server->device_table[i].io_state == VRP_IO_WRITE) || ((server->device_table[i].io_state == VRP_IO_READ) && (server->device_table[i].connection_state == VRP_CONNECTION_NEW))) && (current_time - server->device_table[i].io_begin_time) > server->io_timeout) ||
		((server->device_table[i].io_state == VRP_IO_READ) && (current_time - server->device_table[i].io_begin_time) > server->command_timeout))
		return 0;
	
	if (server->device_table[i].unprocessed_io != VRP_IO_IDLE)
	{
		if (server->device_table[i].unprocessed_io == VRP_IO_WRITE)
		{
			if (server->device_table[i].connection_state == VRP_CONNECTION_SENDING_COMMAND)
			{
				server->device_table[i].connection_state = VRP_CONNECTION_WAITING_FOR_RESPONSE;
				if (!vrp_read(server, i, 0, server->device_io_buffer_size))
					return 0;
			}
			else if (server->device_table[i].connection_state == VRP_CONNECTION_SETUP)
			{
				server->device_table[i].connection_state = VRP_CONNECTION_WAITING_FOR_RESPONSE;
				if (!vrp_read(server, i, 0, server->device_io_buffer_size))
					return 0;
			}
			else if (server->device_table[i].connection_state == VRP_CONNECTION_RESPONDING_TO_COMMAND)
			{
				if (server->device_table[i].command == VRP_MESSAGE_CCM)
				{
					shutdown(server->device_table[i].sock, SD_BOTH);
					sprintf(log_entry_buffer, "Device %lu(QT-client) disconnected from address %lu.%lu.%lu.%lu", server->device_table[i].id, ((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
					vrp_write_log_entry(&server->log, log_entry_buffer);
					server->device_table[i].connection_state = VRP_CONNECTION_DISCONNECT;
					return 0;
				}

				server->device_table[i].command = VRP_MESSAGE_UNDEFINED;
				server->device_table[i].connection_state = VRP_CONNECTION_WAITING_FOR_COMMAND;
				if (!vrp_read(server, i, 0, server->device_io_buffer_size))
					return 0;
			}
			else
				return 0;
		}
		else
		{
			if (server->device_table[i].connection_state == VRP_CONNECTION_WAITING_FOR_RESPONSE)
			{
				if ((server->device_table[i].io_memory[0] != VRP_MESSAGE_WFM) || (server->device_table[i].io_memory[5] != server->device_table[i].command))
					return 0;

				server->device_table[i].last_uppdate_time = NtGetTickCount();

				server->device_table[i].x = server->device_table[i].io_memory[8];
				server->device_table[i].y = server->device_table[i].io_memory[9];
				server->device_table[i].direction = server->device_table[i].io_memory[10];
				server->device_table[i].state = server->device_table[i].io_memory[11];

				if (server->device_table[i].command == VRP_MESSAGE_CCM)
				{
					shutdown(server->device_table[i].sock, SD_BOTH);
					sprintf(log_entry_buffer, "Device %lu disconnected from address %lu.%lu.%lu.%lu", server->device_table[i].id, ((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
					vrp_write_log_entry(&server->log, log_entry_buffer);
					server->device_table[i].connection_state = VRP_CONNECTION_DISCONNECT;
					return 0;
				}
				else if (server->device_table[i].command == VRP_MESSAGE_SCM)
				{
					sprintf(log_entry_buffer, "Device %lu connected with type %lu and status %lu from address %lu.%lu.%lu.%lu", server->device_table[i].id, server->device_table[i].type, server->device_table[i].state,
						((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
					vrp_write_log_entry(&server->log, log_entry_buffer);
				}

				server->device_table[i].command = VRP_MESSAGE_UNDEFINED;
				server->device_table[i].connection_state = VRP_CONNECTION_IDLE;
			}
			else if (server->device_table[i].connection_state == VRP_CONNECTION_NEW)
			{
				if (server->device_table[i].io_memory[0] != VRP_MESSAGE_NCM)
					return 0;

				server->device_table[i].last_uppdate_time = NtGetTickCount();

				if (server->device_table[i].io_memory[6] != VRP_ID_UNDEFINED)
					server->device_table[i].id = server->device_table[i].io_memory[6];
				else
				{
					server->device_table[i].id = vrp_get_temporal_device_id(server);
					if (server->device_table[i].id == VRP_ID_UNDEFINED)
						return 0;
				}

				server->device_table[i].type = server->device_table[i].io_memory[5];
				server->device_table[i].x = server->device_table[i].io_memory[7];
				server->device_table[i].y = server->device_table[i].io_memory[8];
				server->device_table[i].direction = server->device_table[i].io_memory[9];
				server->device_table[i].state = server->device_table[i].io_memory[9];

				server->device_table[i].connection_state = VRP_CONNECTION_SETUP;

				server->device_table[i].command = VRP_MESSAGE_SCM;
				server->device_table[i].io_memory[0] = VRP_MESSAGE_SCM;
				server->device_table[i].io_memory[1] = 2;
				server->device_table[i].io_memory[2] = 0;
				server->device_table[i].io_memory[3] = 0;
				server->device_table[i].io_memory[4] = 0;
				server->device_table[i].io_memory[5] = 0x00;// do not change the controlling device
				server->device_table[i].io_memory[6] = server->device_table[i].id;

				if (server->device_table[i].type == VRP_DEVICE_TYPE_CLIENT)
				{
					sprintf(log_entry_buffer, "Device %lu(QT-client) connected with type %lu and status %lu from address %lu.%lu.%lu.%lu", server->device_table[i].id, server->device_table[i].type, server->device_table[i].state,
						((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
					vrp_write_log_entry(&server->log, log_entry_buffer);

					server->device_table[i].control_overridden = 1;
					server->device_table[i].command = VRP_MESSAGE_SCM;
					server->device_table[i].connection_state = VRP_CONNECTION_RESPONDING_TO_COMMAND;
					server->device_table[i].io_memory[5] = 0x01;
				}

				if (!vrp_write(server, i, 0, 7))
					return 0;
			}
			else if (server->device_table[i].connection_state == VRP_CONNECTION_WAITING_FOR_COMMAND)
			{
				server->device_table[i].command = server->device_table[i].io_memory[0];

				sprintf(log_entry_buffer, "Received command %lu from device %lu at address %lu.%lu.%lu.%lu", server->device_table[i].command, server->device_table[i].id,
					((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
				vrp_write_log_entry(&server->log, log_entry_buffer);

				size_t wfm_size = 12;
				uint8_t wfm_error = VRP_ERROR_NOT_SUPPORTED;
				uint8_t wfm_atomic = 1;
				uint8_t lines_read = VRP_DIRECTION_UNDEFINED;
				switch (server->device_table[i].command)
				{
					case VRP_MESSAGE_CCM:
					{
						wfm_error = VRP_ERROR_SUCCESS;
						break;
					}
					case VRP_MESSAGE_SQM:
					{
						wfm_error = VRP_ERROR_SUCCESS;
						break;
					}
					case VRP_MESSAGE_RLM:
					{
						uint32_t log_line_offset = ((uint32_t)server->device_table[i].io_memory[7] << 0) | ((uint32_t)server->device_table[i].io_memory[8] << 8) | ((uint32_t)server->device_table[i].io_memory[9] << 16) | ((uint32_t)server->device_table[i].io_memory[10] << 24);
						uint32_t log_line_count = (uint32_t)server->device_table[i].io_memory[6];
						if (server->device_table[i].io_memory[5])
						{
							if ((server->log.line_count - 1) < (log_line_offset + log_line_count))
								log_line_offset = 0;
							else
								log_line_offset = (server->log.line_count - 1) - (log_line_offset + log_line_count);
						}
						uint32_t log_lines_read;
						size_t log_size_read;
						DWORD log_error = vrp_read_log(&server->log, log_line_offset, log_line_count, server->device_io_buffer_size - 12, server->device_table[i].io_memory + 12, &log_lines_read, &log_size_read);
						if (!log_error)
						{
							wfm_error = VRP_ERROR_SUCCESS;
							if (log_lines_read < (uint8_t)log_line_count)
								wfm_atomic = 0;
							lines_read = (uint8_t)log_lines_read;
							wfm_size = 12 + log_size_read;
						}
						else if (wfm_error == ERROR_FILE_NOT_FOUND)
							wfm_error = VRP_ERROR_INVALID_PARAMETER;
						else
							wfm_error = VRP_ERROR_DEVICE_MALFUNCTION;
						break;
					}
					case VRP_MESSAGE_RCM:
					{
						break;
					}
					case VRP_MESSAGE_POM:
					{
						break;
					}
					case VRP_MESSAGE_SSM:
					{
						if (server->status == 2)
						{
							server->status = 1;
							wfm_error = VRP_ERROR_SUCCESS;
						}
						else
							wfm_error = VRP_ERROR_INVALID_PARAMETER;
						sprintf(log_entry_buffer, "Received startup command from device %lu at address %lu.%lu.%lu.%lu", server->device_table[i].id,
							((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
						vrp_write_log_entry(&server->log, log_entry_buffer);
						break;
					}
					case VRP_MESSAGE_UFM:
					{
						if (!server->status)
						{
							server->status = 1;
							wfm_error = VRP_ERROR_SUCCESS;
						}
						else
							wfm_error = VRP_ERROR_INVALID_PARAMETER;
						sprintf(log_entry_buffer, "Received unfreeze command from device %lu at address %lu.%lu.%lu.%lu", server->device_table[i].id,
							((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
						vrp_write_log_entry(&server->log, log_entry_buffer);
						break;
					}
					case VRP_MESSAGE_SHM:
					{
						if (server->status == 1)
						{
							server->status = 2;
							wfm_error = VRP_ERROR_SUCCESS;
						}
						else
							wfm_error = VRP_ERROR_INVALID_PARAMETER;
						sprintf(log_entry_buffer, "Received shutdown command from device %lu at address %lu.%lu.%lu.%lu", server->device_table[i].id,
							((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
						vrp_write_log_entry(&server->log, log_entry_buffer);
						break;
					}
					default:
						break;
				}
				server->device_table[i].io_memory[0] = VRP_MESSAGE_WFM;
				server->device_table[i].io_memory[1] = ((uint32_t)(wfm_size - 5) >> 0) & 0xFF;
				server->device_table[i].io_memory[2] = ((uint32_t)(wfm_size - 5) >> 8) & 0xFF;
				server->device_table[i].io_memory[3] = ((uint32_t)(wfm_size - 5) >> 16) & 0xFF;
				server->device_table[i].io_memory[4] = ((uint32_t)(wfm_size - 5) >> 24) & 0xFF;
				server->device_table[i].io_memory[5] = server->device_table[i].command;
				server->device_table[i].io_memory[6] = wfm_error;
				server->device_table[i].io_memory[7] = wfm_atomic;
				server->device_table[i].io_memory[8] = VRP_COORDINATE_UNDEFINED;
				server->device_table[i].io_memory[9] = VRP_COORDINATE_UNDEFINED;
				server->device_table[i].io_memory[10] = lines_read;;
				server->device_table[i].io_memory[11] = server->status;
				if (!vrp_write(server, i, 0, wfm_size))
					return 0;
				server->device_table[i].connection_state = VRP_CONNECTION_RESPONDING_TO_COMMAND;
			}
			else
				return 0;
		}
		server->device_table[i].unprocessed_io = VRP_IO_IDLE;
	}
	
	if (server->device_table[i].connection_state == VRP_CONNECTION_IDLE)
	{
		
		if (current_time - server->device_table[i].connection_begin_time > 120000)
		{
			server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;

			server->device_table[i].command = VRP_MESSAGE_CCM;
			server->device_table[i].io_memory[0] = server->device_table[i].command;
			server->device_table[i].io_memory[1] = 0;
			server->device_table[i].io_memory[2] = 0;
			server->device_table[i].io_memory[3] = 0;
			server->device_table[i].io_memory[4] = 0;

			if (!vrp_write(server, i, 0, 5))
				return 0;
		}
		else if (current_time - server->device_table[i].last_uppdate_time > 20000)
		{
			server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;

			server->device_table[i].command = VRP_MESSAGE_SQM;
			server->device_table[i].io_memory[0] = server->device_table[i].command;
			server->device_table[i].io_memory[1] = 0;
			server->device_table[i].io_memory[2] = 0;
			server->device_table[i].io_memory[3] = 0;
			server->device_table[i].io_memory[4] = 0;

			if (!vrp_write(server, i, 0, 5))
				return 0;
		}
	}

	return 1;
}

int main(int argc, char* argv)
{
	printf("Starting VarastoRobo master 0.4.0...\n");
	vrp_server_t server;
	DWORD error = vrp_server_setup(&server);

	if (error)
	{
		printf("vrp_server_setup failed. error code %lu\n", error);
		return EXIT_FAILURE;
	}
	else
		printf("server running at address %lu.%lu.%lu.%lu\n", ((server.server_address.sin_addr.s_addr) >> 0) & 0xFF, ((server.server_address.sin_addr.s_addr) >> 8) & 0xFF, ((server.server_address.sin_addr.s_addr) >> 16) & 0xFF, ((server.server_address.sin_addr.s_addr) >> 24) & 0xFF);

	//vrp_create_test_client();

	for (;;)
	{
		size_t i = vrp_wait_for_io(&server);
		vrp_accept_incoming_connection(&server);
		if (i == VRP_WAIT_EMERGENCY)
			vrp_process_possible_emergency(&server);
		else if (i == VRP_WAIT_BROADCAST)
			vrp_finish_send_system_broadcast_message(&server);
		else if (i < VRP_MAX_DEVICE_COUNT)
		{
			int io_type;
			size_t total_transfered = vrp_finish_io(&server, i, &io_type);
			size_t continue_size = total_transfered != (size_t)~0 ? continue_size = vrp_message_transfer_incomplete(&server, i, io_type) : (size_t)~0;
			if (continue_size && continue_size < (size_t)~0)
			{
				if (io_type == VRP_IO_READ)
					if (!vrp_read(&server, i, total_transfered, continue_size))
						continue_size = (size_t)~0;
				else
					if (!vrp_write(&server, i, total_transfered, continue_size))
						continue_size = (size_t)~0;
			}
			if (!continue_size)
				server.device_table[i].unprocessed_io = io_type;
			else if (continue_size == (size_t)~0)
				vrp_remove_device(&server, i);
		}
		for (size_t j = 0; j != VRP_MAX_DEVICE_COUNT; ++j)
			if (server.device_table[j].sock != INVALID_SOCKET)
				if(!vrp_process_device(&server, j))
					vrp_remove_device(&server, j);
		if (server.broadcast_io_state == VRP_IO_IDLE && (GetTickCount64() - server.last_broadcast_time) > server.broadcast_delay)
			vrp_send_system_broadcast_message(&server);
	}
	Sleep(INFINITE);
	return 0;
}