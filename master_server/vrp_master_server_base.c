/*
	VarastoRobo master server version 0.4.3 2019-11-20 by Santtu Nyman.
*/

#include "vrp_master_server_base.h"

uint64_t vrp_get_valid_device_entries(vrp_server_t* server)
{
	__assume(VRP_MAX_DEVICE_COUNT <= 64);
	uint64_t valid_device_entry_mask = 0;
	for (int i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
		if (server->device_table[i].sock != INVALID_SOCKET)
			valid_device_entry_mask |= ((uint64_t)1 << (uint64_t)i);
#ifndef _NDEBUG
	int set_bit_count = 0;
	for (int i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
		if ((int)((uint64_t)valid_device_entry_mask >> (uint64_t)i) & 1)
		{
			assert(server->device_table[i].io_result.hEvent);
			++set_bit_count;
		}
	assert(set_bit_count == server->device_count);
#endif
	return valid_device_entry_mask;
}

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

size_t vrp_get_device_index_by_id(vrp_server_t* server, uint8_t id)
{
	if (id == VRP_ID_UNDEFINED)
		return (size_t)~0;
	for (size_t i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
		if (server->device_table[i].sock != INVALID_SOCKET && server->device_table[i].id == id)
			return i;
	return (size_t)~0;
}

size_t vrp_get_controlling_device_index(vrp_server_t* server, uint8_t controled_device_id)
{
	for (size_t i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
		if (server->device_table[i].sock != INVALID_SOCKET && server->device_table[i].control_target_id == controled_device_id)
			return i;
	return (size_t)~0;
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
		((uint32_t) * (const uint8_t*)((uintptr_t)message + 2) << 8) |
		((uint32_t) * (const uint8_t*)((uintptr_t)message + 3) << 16) |
		((uint32_t) * (const uint8_t*)((uintptr_t)message + 4) << 24));
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
	DWORD max_wait_time = !server->debug_no_broadcast ? (((wait_begin_time - server->last_broadcast_time) < server->broadcast_delay) ? (wait_begin_time - server->last_broadcast_time) : (server->broadcast_io_state == VRP_IO_WRITE ? server->broadcast_delay : 0)) : server->broadcast_delay;
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

BOOL WINAPI vrp_ctr_c_close_process_routine(DWORD parameter)
{
	WCHAR buffer[(sizeof(HANDLE) * 2) + 1];
	const size_t buffer_length = sizeof(buffer) / sizeof(WCHAR);
	HANDLE log_file_handle = INVALID_HANDLE_VALUE;
	if (GetEnvironmentVariableW(L"MASTER_LOG_FILE_HANDLE", buffer, (DWORD)buffer_length) == (sizeof(HANDLE) * 2))
	{
		int valid_string = 1;
		for (int i = 0; valid_string && i != (sizeof(HANDLE) * 2); ++i)
			if (!(buffer[i] >= L'0' && buffer[i] <= L'9') && !(buffer[i] >= L'A' && buffer[i] <= L'F'))
				valid_string = 0;
		if (valid_string)
		{
			log_file_handle = (HANDLE)0;
			for (int i = 0; valid_string && i != (sizeof(HANDLE) * 2); ++i)
				if (buffer[i] <= L'9')
					log_file_handle = (HANDLE)(((UINT_PTR)log_file_handle << 4) | (UINT_PTR)(buffer[i] - L'0'));
				else
					log_file_handle = (HANDLE)(((UINT_PTR)log_file_handle << 4) | ((UINT_PTR)10 + (UINT_PTR)(buffer[i] - L'A')));
		}
	}
	if (log_file_handle != INVALID_HANDLE_VALUE)
		FlushFileBuffers(log_file_handle);
	ExitProcess(0);
}

BOOL vrp_set_exit_flush_handle(HANDLE handle)
{
	WCHAR hex_table[16] = { L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F' };
	WCHAR buffer[(sizeof(HANDLE) * 2) + 1];
	for (int i = 0; i != sizeof(HANDLE) * 2; ++i)
		buffer[i] = hex_table[((UINT_PTR)handle >> ((((sizeof(HANDLE) * 2) - 1) - i) * 4)) & 0xF];
	buffer[sizeof(HANDLE) * 2] = 0;
	return SetEnvironmentVariableW(L"MASTER_LOG_FILE_HANDLE", buffer);
}

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

DWORD vrp_server_setup(vrp_server_t* server, int* error_hint)
{
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		*error_hint = __LINE__;
		return ERROR_NOT_SUPPORTED;
	}

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
		*error_hint = -1;
		return error;
	}

	if (configuration->block_count > VRP_MAX_BLOCK_COUNT || configuration->pickup_location_count > VRP_MAX_PICKUP_LOCATION_COUNT)
	{
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		*error_hint = __LINE__;
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

	if (server->broadcast_address.sin_addr.s_addr == INADDR_BROADCAST && server->server_address.sin_addr.s_addr == INADDR_ANY)
	{
		printf("default address configuration\n");
		uint32_t subnet_mask = 0xFFFFFFFF;
		vrp_get_host_ip_address(&server->server_address.sin_addr.s_addr, &subnet_mask);
		server->broadcast_address.sin_addr.s_addr = (server->server_address.sin_addr.s_addr & subnet_mask) | ~subnet_mask;
	}

	server->emergency_address.sin_family = AF_INET;
	server->emergency_address.sin_port = server->broadcast_address.sin_port;
	server->emergency_address.sin_addr.s_addr = server->server_address.sin_addr.s_addr;

	server->page_size = vrp_get_page_size();

	size_t required_map_bitmap_buffer_size = ((size_t)((((size_t)configuration->map_height * (size_t)configuration->map_width) + 7) / 8) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_map_state_buffer_size = (((size_t)configuration->map_height * (size_t)configuration->map_width * sizeof(*server->map_state)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_block_buffer_size = ((size_t)(VRP_MAX_BLOCK_COUNT * sizeof(*server->block_table)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_idle_location_table_buffer_size = ((((size_t)configuration->idle_location_count * sizeof(*server->idle_location_table)) / 8) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_pickup_location_buffer_size = ((size_t)(VRP_MAX_PICKUP_LOCATION_COUNT * sizeof(*server->pickup_location_table)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_device_buffer_size = ((size_t)(VRP_MAX_DEVICE_COUNT * sizeof(*server->device_table)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_product_order_table_buffer_size = ((size_t)(VRP_MAX_PRODUCT_ORDER_COUNT * sizeof(*server->product_order_table)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	if ((required_map_bitmap_buffer_size + required_map_state_buffer_size + required_block_buffer_size + required_idle_location_table_buffer_size + required_pickup_location_buffer_size) <= server->page_size)
	{
		server->map_bitmap_buffer_size = required_map_bitmap_buffer_size;
		server->map_state_buffer_size = required_map_state_buffer_size;
		server->block_buffer_size = required_block_buffer_size;
		server->idle_location_table_buffer_size = required_idle_location_table_buffer_size;
		server->pickup_location_buffer_size = server->page_size - (server->map_bitmap_buffer_size + server->map_state_buffer_size + server->block_buffer_size + server->idle_location_table_buffer_size);
	}
	else
	{
		server->map_bitmap_buffer_size = (required_map_bitmap_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
		server->map_state_buffer_size = (required_map_state_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
		server->block_buffer_size = (required_block_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
		server->idle_location_table_buffer_size = (required_idle_location_table_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
		server->pickup_location_buffer_size = (required_pickup_location_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
	}
	server->device_table_buffer_size = (required_device_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
	server->product_order_table_buffer_size = (required_product_order_table_buffer_size + (server->page_size - 1)) & ~(server->page_size - 1);
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

	server->allocation_size =
		server->map_bitmap_buffer_size +
		server->map_state_buffer_size +
		server->block_buffer_size +
		server->idle_location_table_buffer_size +
		server->pickup_location_buffer_size +
		server->device_table_buffer_size +
		server->product_order_table_buffer_size +
		server->broadcast_io_buffer_size +
		server->emergency_io_buffer_size +
		(VRP_MAX_DEVICE_COUNT * server->device_io_buffer_size);
	server->allocation_base = VirtualAlloc(0, server->allocation_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!server->allocation_base)
	{
		WSACleanup();
		*error_hint = __LINE__;
		return ERROR_OUTOFMEMORY;
	}

	server->map_bitmap = (uint8_t*)server->allocation_base;
	server->map_state = (void*)((UINT_PTR)server->map_bitmap + server->map_bitmap_buffer_size);
	server->block_table = (void*)((UINT_PTR)server->map_state + server->map_state_buffer_size);
	server->idle_location_table = (void*)((UINT_PTR)server->block_table + server->block_buffer_size);
	server->pickup_location_table = (void*)((UINT_PTR)server->idle_location_table + server->idle_location_table_buffer_size);
	server->device_table = (void*)((UINT_PTR)server->pickup_location_table + server->pickup_location_buffer_size);
	server->product_order_table = (void*)((UINT_PTR)server->device_table + server->device_table_buffer_size);
	server->broadcast_io_memory = (uint8_t*)((UINT_PTR)server->product_order_table + server->product_order_table_buffer_size);
	server->emergency_io_memory = (uint8_t*)((UINT_PTR)server->broadcast_io_memory + server->broadcast_io_buffer_size);
	for (size_t offset = server->map_bitmap_buffer_size + server->map_state_buffer_size + server->block_buffer_size + server->idle_location_table_buffer_size + server->pickup_location_buffer_size + server->device_table_buffer_size + server->product_order_table_buffer_size + server->broadcast_io_buffer_size + server->emergency_io_buffer_size, i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
	{
		server->device_table[i].io_memory = (uint8_t*)((UINT_PTR)server->allocation_base + offset + (i * server->device_io_buffer_size));
#ifndef _NDEBUG
		server->debug_device_table[i] = server->device_table + i;
#endif
	}

	error = vrp_open_log_file(&server->log, 0x200000000, 0x100000, 0);
	if (error)
	{
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		*error_hint = __LINE__;
		return error;
	}
	vrp_set_exit_flush_handle(server->log.handle);

	server->emergency_io_state = VRP_IO_IDLE;
	server->emergency_io_result.hEvent = CreateEventW(0, TRUE, FALSE, 0);
	if (!server->emergency_io_result.hEvent)
	{
		error = GetLastError();
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		*error_hint = __LINE__;
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
		*error_hint = __LINE__;
		return error;
	}

	server->accept_event = CreateEventW(0, TRUE, FALSE, 0);
	if (!server->accept_event)
	{
		error = GetLastError();
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		*error_hint = __LINE__;
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
			*error_hint = __LINE__;
			return error;
		}

		const BOOL reuse_address_enable = TRUE;
		setsockopt(server->emergency_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_address_enable, sizeof(BOOL));
		// this is not needed to be successful, but helps when running clients on same machine

		if (setsockopt(server->emergency_sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast_enable, sizeof(BOOL)) ||
			bind(server->emergency_sock, (struct sockaddr*)&server->emergency_address, sizeof(struct sockaddr_in)))
		{
			error = ERROR_OPEN_FAILED;
			vrp_free_master_configuration(configuration);
			vrp_server_close(server);
			*error_hint = __LINE__;
			return error;
		}
	}

	if (configuration->debug_no_broadcast)
		server->debug_no_broadcast = 1;
	else
	{
		server->broadcast_sock = WSASocketW(AF_INET, SOCK_DGRAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
		if (server->broadcast_sock == INVALID_SOCKET)
		{
			error = ERROR_OPEN_FAILED;
			vrp_free_master_configuration(configuration);
			vrp_server_close(server);
			*error_hint = __LINE__;
			return error;
		}

		if (setsockopt(server->broadcast_sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast_enable, sizeof(BOOL)))
		{
			error = ERROR_OPEN_FAILED;
			vrp_free_master_configuration(configuration);
			vrp_server_close(server);
			*error_hint = __LINE__;
			return error;
		}
	}

	server->listen_sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (server->listen_sock == INVALID_SOCKET)
	{
		error = ERROR_OPEN_FAILED;
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		*error_hint = __LINE__;
		return error;
	}

	if (bind(server->listen_sock, (SOCKADDR*)&server->server_address, sizeof(SOCKADDR)) == SOCKET_ERROR ||
		listen(server->listen_sock, SOMAXCONN) == SOCKET_ERROR ||
		WSAEventSelect(server->listen_sock, server->accept_event, FD_ACCEPT) == SOCKET_ERROR)
	{
		error = ERROR_OPEN_FAILED;
		vrp_free_master_configuration(configuration);
		vrp_server_close(server);
		*error_hint = __LINE__;
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
			*error_hint = __LINE__;
			return error;
		}
	}

	server->block_count = configuration->block_count;
	for (size_t i = 0; i != configuration->block_count; ++i)
	{
		server->block_table[i].x = configuration->block_table[i * 2 + 0];
		server->block_table[i].y = configuration->block_table[i * 2 + 1];
	}

	server->idle_location_count = configuration->idle_location_count;
	for (size_t i = 0; i != configuration->idle_location_count; ++i)
	{
		server->idle_location_table[i].x = configuration->idle_location_table[i * 2 + 0];
		server->idle_location_table[i].y = configuration->idle_location_table[i * 2 + 1];
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

	memcpy(server->map_bitmap, configuration->map, ((server->map_height * server->map_width) + 7) / 8);

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

	*error_hint = 0;
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
	memcpy(server->broadcast_io_memory + 8, server->map_bitmap, map_size);
	for (size_t i = 0; i != server->block_count; ++i)
	{
		server->broadcast_io_memory[8 + map_size + (i * 2) + 0] = server->block_table[i].x;
		server->broadcast_io_memory[8 + map_size + (i * 2) + 1] = server->block_table[i].y;
	}

	for (size_t c = 0, i = 0; c != server->device_count; ++i)
		if (server->device_table[i].sock != INVALID_SOCKET)
		{
			server->broadcast_io_memory[8 + map_size + block_array_size + (c * 8) + 0] = server->device_table[i].type;
			server->broadcast_io_memory[8 + map_size + block_array_size + (c * 8) + 1] = server->device_table[i].id;
			server->broadcast_io_memory[8 + map_size + block_array_size + (c * 8) + 2] = server->device_table[i].x;
			server->broadcast_io_memory[8 + map_size + block_array_size + (c * 8) + 3] = server->device_table[i].y;
			server->broadcast_io_memory[8 + map_size + block_array_size + (c * 8) + 4] = (uint8_t)((server->device_table[i].ip_address >> 0) & 0xFF);
			server->broadcast_io_memory[8 + map_size + block_array_size + (c * 8) + 5] = (uint8_t)((server->device_table[i].ip_address >> 8) & 0xFF);
			server->broadcast_io_memory[8 + map_size + block_array_size + (c * 8) + 6] = (uint8_t)((server->device_table[i].ip_address >> 16) & 0xFF);
			server->broadcast_io_memory[8 + map_size + block_array_size + (c * 8) + 7] = (uint8_t)((server->device_table[i].ip_address >> 24) & 0xFF);
			++c;
		}

	server->broadcast_io_buffer.buf = server->broadcast_io_memory;
	server->broadcast_io_buffer.len = (ULONG)packet_size;
	server->broadcast_io_result.Internal = 0;
	server->broadcast_io_result.InternalHigh = 0;
	server->broadcast_io_result.Offset = 0;
	server->broadcast_io_result.OffsetHigh = 0;

	if (!WSASendTo(server->broadcast_sock, &server->broadcast_io_buffer, 1, 0, 0, (const struct sockaddr*) & server->broadcast_address, sizeof(struct sockaddr_in), &server->broadcast_io_result, 0) ||
		WSAGetLastError() == WSA_IO_PENDING)
	{
		server->broadcast_io_state = VRP_IO_WRITE;
		server->last_broadcast_time = NtGetTickCount();
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
			server->device_table[i].control_target_id = VRP_ID_UNDEFINED;
			server->device_table[i].remote_command_finished = 0;
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
			server->device_table[i].move_to_x = VRP_COORDINATE_UNDEFINED;
			server->device_table[i].move_to_y = VRP_COORDINATE_UNDEFINED;
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