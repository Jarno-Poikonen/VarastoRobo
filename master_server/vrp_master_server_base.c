/*
	VarastoRobo master server version 0.9.0 2019-12-04 by Santtu Nyman.
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

int vrp_calculate_device_movement_priority(const vrp_server_t* server, uint8_t device_id)
{
	int device_priority = 0;
	size_t device_index = vrp_get_device_index_by_id(server, device_id);
	if (device_index == (size_t)~0)
	{
		assert(device_priority == 0);
		return device_priority;
	}
	size_t order_index = vrp_get_order_index_of_transport_device(server, vrp_get_device_index_by_id(server, device_id));
	if (order_index == (size_t)~0)
	{
		device_priority = (int)(0xFF - device_id);
		assert((device_priority > 0) && (device_priority < 0x4000));
		return device_priority;
	}
	device_priority = 0x4000 | ((int)(0x3F - (order_index & 0x3F)) << 8) | (int)(0xFF - device_id);
	assert(device_priority > 0x4000);
	return device_priority;
}

int vrp_add_block(vrp_server_t* server, uint8_t x, uint8_t y)
{
	if (server->block_count == VRP_MAX_BLOCK_COUNT)
		return 0;

	if (!((server->map_bitmap[((size_t)y * (size_t)server->map_width + (size_t)x) / 8] >> (int)(((size_t)y * (size_t)server->map_width + (size_t)x) % 8)) & 1))
		return 0;

	for (size_t i = 0; i != server->device_count; ++i)
		if ((server->device_table[i].type == VRP_DEVICE_TYPE_GOPIGO) &&
			(server->device_table[i].x == x) &&
			(server->device_table[i].y == y))
			return 0;

	for (size_t i = 0; i != server->block_count; ++i)
		if ((server->block_table[i].x == x) &&
			(server->block_table[i].y == y))
			return 0;

	server->block_table[server->block_count].x = x;
	server->block_table[server->block_count].y = y;
	server->block_count++;
	return 1;
}

int vrp_remove_block(vrp_server_t* server, uint8_t x, uint8_t y)
{
	for (size_t i = 0; i != server->block_count; ++i)
		if ((server->block_table[i].x == x) &&
			(server->block_table[i].y == y))
		{
			if (i != (server->block_count - 1))
				memcpy(server->block_table + i, server->block_table + (server->block_count - 1), sizeof(*server->block_table));
			server->block_count--;
			return 1;
		}

	return 0;
}

uint32_t vrp_create_product_order_number(vrp_server_t* server, uint8_t optional_client_id)
{
	return ((uint32_t)optional_client_id << 24) | ((uint32_t)server->product_order_count << 16) | ((uint32_t)server->time & 0x0000FFFF);
}

size_t vrp_get_order_index_by_number(vrp_server_t* server, uint32_t order_number)
{
	if (!order_number)
		return (size_t)~0;

	for (size_t i = 0; i != server->product_order_count; ++i)
		if (server->product_order_table[i].order_number == order_number)
			return i;

	return (size_t)~0;
}

int vrp_remove_product_order(vrp_server_t* server, uint32_t order_number)
{
	if (!order_number)
		return 0;

	for (size_t i = 0; i != server->product_order_count; ++i)
		if (server->product_order_table[i].order_number == order_number)
		{
			if (i != server->product_order_count - 1)
				memcpy(server->product_order_table + i, server->product_order_table + (server->product_order_count - 1), sizeof(*server->product_order_table));

			server->product_order_count--;
			return 1;
		}

	return 0;
}

size_t vrp_create_product_order(vrp_server_t* server, uint8_t product_id, uint8_t x, uint8_t y, uint8_t optional_client_id)
{
	if (server->product_order_count == VRP_MAX_PRODUCT_ORDER_COUNT || x >= server->map_width || y >= server->map_height)
		return (size_t)~0;

	size_t i = server->product_order_count;
	server->product_order_table[i].order_status = VRP_ORDER_IN_STORAGE;
	server->product_order_table[i].order_number = vrp_create_product_order_number(server, optional_client_id);
	server->product_order_table[i].placement_time = server->time;
	server->product_order_table[i].product_id = product_id;
	server->product_order_table[i].transport_device_id = VRP_ID_UNDEFINED;
	server->product_order_table[i].destination_x = x;
	server->product_order_table[i].destination_y = y;
	server->product_order_table[i].ur5_pickup_complete = 0;
	server->product_order_table[i].gopigo_pickup_complete = 0;

	server->product_order_count++;
	return i;
}

int vrp_choose_product_order_destination(vrp_server_t* server, size_t coordinate_count, const uint8_t* coordinate_table, uint8_t* destination_x, uint8_t* destination_y)
{
	for (size_t i = 0; i != coordinate_count; ++i)
	{
		uint8_t x = coordinate_table[i * 2 + 0];
		uint8_t y = coordinate_table[i * 2 + 1];
		int destination_is_free = 1;
		for (size_t j = 0; destination_is_free && j != server->product_order_count; ++j)
			if (server->product_order_table[i].destination_x == x && server->product_order_table[i].destination_y == y)
				destination_is_free = 0;
		if (destination_is_free)
		{
			*destination_x = x;
			*destination_y = y;
			return 1;
		}
	}
	if (coordinate_count)
	{
		*destination_x = coordinate_table[0];
		*destination_y = coordinate_table[1];
		return 1;
	}
	return 0;
}

size_t vrp_get_nonstarted_product_order_index(vrp_server_t* server)
{
	if (!server->product_order_count)
		return (size_t)~0;

	for (size_t i = 0; i != server->product_order_count; ++i)
		if ((server->product_order_table[i].order_status == VRP_ORDER_IN_STORAGE) && (server->product_order_table[i].transport_device_id == VRP_ID_UNDEFINED))
			return i;
	return (size_t)~0;
}

size_t vrp_get_order_index_of_transport_device(vrp_server_t* server, size_t device_index)
{
	for (size_t i = 0; i != server->product_order_count; ++i)
	{
		uint8_t device_id = server->device_table[device_index].id;
		uint8_t transport_device_id = server->product_order_table[i].transport_device_id;
		if (transport_device_id == device_id)
			return i;
	}
	return (size_t)~0;
}

int vrp_is_valid_product_id(const vrp_server_t* server, uint8_t product_id, int is_not_undefined)
{
	if ((product_id > 3) && (product_id != VRP_PRODUCT_TYPE_UNDEFINED))
		return 0;

	if ((product_id == VRP_PRODUCT_TYPE_UNDEFINED) && is_not_undefined)
		return 0;
	
	return ((uint8_t)server->acceptable_product_mask & (uint8_t)(1 << product_id)) ? 1 : 0;
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

size_t vrp_get_pickup_location_index_by_id(vrp_server_t* server, uint8_t pickup_location_id)
{
	for (size_t i = 0; i != server->pickup_location_count; ++i)
		if (server->pickup_location_table[i].id == pickup_location_id)
			return i;
	return (size_t)~0;
}

DWORD vrp_get_system_tick_resolution()
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

int vrp_is_device_timeout_reached(vrp_server_t* server, size_t device_index)
{
	assert(server->device_table[device_index].sock != INVALID_SOCKET);

	if (server->device_table[device_index].io_state == VRP_IO_READ)
	{
		if (server->device_table[device_index].connection_state == VRP_CONNECTION_WAITING_FOR_COMMAND)
		{
			assert(server->device_table[device_index].type == VRP_DEVICE_TYPE_CLIENT);
			return 0;
		}
		else if (server->device_table[device_index].connection_state == VRP_CONNECTION_NEW)
		{
			if ((server->time - server->device_table[device_index].io_begin_time) > server->io_timeout)
				return 1;
		}
		else if (server->device_table[device_index].io_state == VRP_IO_READ)
		{
			if ((server->time - server->device_table[device_index].io_begin_time) > server->command_timeout)
				return 1;
		}
	}
	else if (server->device_table[device_index].io_state == VRP_IO_WRITE)
	{
		if ((server->time - server->device_table[device_index].io_begin_time) > server->io_timeout)
			return 1;
	}

	return 0;
}

void vrp_shutdown_device_connection(vrp_server_t* server, size_t i)
{
	shutdown(server->device_table[i].sock, SD_BOTH);
	server->device_table[i].connection_state = VRP_CONNECTION_DISCONNECT;
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
		server->device_table[device_index].io_begin_time = server->time;
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
		server->device_table[device_index].io_begin_time = server->time;
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
	if ((server->emergency_io_state == VRP_IO_READ) && WSAGetOverlappedResult(server->emergency_sock, &server->emergency_io_result, &io_transfered, FALSE, &server->emergency_io_flags) && io_transfered)
	{
		if ((io_transfered >= 8) && (server->emergency_io_memory[0] == 0x01) && (server->emergency_io_memory[1] == 0x07) && !server->emergency_io_memory[2] && (server->emergency_io_memory[3] != server->id))
		{



			server->status = 0;
			server->broadcast_immediately = 1;
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
	server->time = NtGetTickCount();

	if (!server->debug_no_emergency_listen && server->emergency_io_state == VRP_IO_IDLE)
	{
		assert(server->emergency_io_buffer_size >= 0x200);

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

	DWORD max_wait_time = !server->debug_no_broadcast ? (((server->time - server->last_broadcast_time) < server->broadcast_delay) ? (server->time - server->last_broadcast_time) : (server->broadcast_io_state == VRP_IO_WRITE ? server->broadcast_delay : 0)) : server->broadcast_delay;
	DWORD event_index = WaitForMultipleObjects(io_event_count, server->io_event_table, FALSE, max_wait_time);
	
	server->time = NtGetTickCount();

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

void vrp_close_server_instance(vrp_server_t* server)
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
	VirtualFree(server, 0, MEM_RELEASE);
	WSACleanup();
}

DWORD vrp_create_server_instance(vrp_server_t** server_instance, const char** error_information_text)
{
#ifndef _NDEBUG
	*server_instance = 0;
	*error_information_text = 0;
#endif

	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		*error_information_text = "Error failed to initialize Windows socket library";
		return ERROR_NOT_SUPPORTED;
	}

	vrp_configuration_t* configuration;
	DWORD error = vrp_load_master_configuration(&configuration);

	if (error)
	{
		WSACleanup();
		*error_information_text = "Error failed to load server configuration json file";
		return error;
	}

	if (configuration->block_count > VRP_MAX_BLOCK_COUNT || configuration->pickup_location_count > VRP_MAX_PICKUP_LOCATION_COUNT)
	{
		vrp_free_master_configuration(configuration);
		WSACleanup();
		*error_information_text = "Error server configuration is invalid";
		return ERROR_INVALID_DATA;
	}

	size_t page_size = vrp_get_page_size();

	size_t required_main_part_buffer_size = (sizeof(vrp_server_t) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_map_bitmap_buffer_size = ((size_t)((((size_t)configuration->map_height * (size_t)configuration->map_width) + 7) / 8) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_map_state_buffer_size = (((size_t)configuration->map_height * (size_t)configuration->map_width * sizeof(vrp_map_state_t)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_block_buffer_size = ((size_t)(VRP_MAX_BLOCK_COUNT * sizeof(vrp_block_t)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_idle_location_table_buffer_size = ((((size_t)configuration->idle_location_count * sizeof(vrp_idle_location_t)) / 8) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_pickup_location_buffer_size = ((size_t)(VRP_MAX_PICKUP_LOCATION_COUNT * sizeof(vrp_pickup_location_t)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_device_buffer_size = ((size_t)(VRP_MAX_DEVICE_COUNT * sizeof(vrp_device_t)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	size_t required_product_order_table_buffer_size = ((size_t)(VRP_MAX_PRODUCT_ORDER_COUNT * sizeof(vrp_product_order_t)) + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
	
	size_t main_part_buffer_size;
	size_t map_bitmap_buffer_size;
	size_t map_state_buffer_size;
	size_t block_buffer_size;
	size_t idle_location_table_buffer_size;
	size_t pickup_location_buffer_size;
	size_t device_table_buffer_size;
	size_t product_order_table_buffer_size;
	size_t broadcast_io_buffer_size;
	size_t emergency_io_buffer_size;
	size_t log_entry_buffer_size;
	size_t device_io_buffer_size;
	size_t total_allocation_size;
	
	if ((required_main_part_buffer_size + required_map_bitmap_buffer_size + required_map_state_buffer_size + required_block_buffer_size + required_idle_location_table_buffer_size + required_pickup_location_buffer_size) <= page_size)
	{
		main_part_buffer_size = required_main_part_buffer_size;
		map_bitmap_buffer_size = required_map_bitmap_buffer_size;
		map_state_buffer_size = required_map_state_buffer_size;
		block_buffer_size = required_block_buffer_size;
		idle_location_table_buffer_size = required_idle_location_table_buffer_size;
		pickup_location_buffer_size = page_size - (main_part_buffer_size + map_bitmap_buffer_size + map_state_buffer_size + block_buffer_size + idle_location_table_buffer_size);
	}
	else
	{
		main_part_buffer_size = (required_main_part_buffer_size + (page_size - 1)) & ~(page_size - 1);
		map_bitmap_buffer_size = (required_map_bitmap_buffer_size + (page_size - 1)) & ~(page_size - 1);
		map_state_buffer_size = (required_map_state_buffer_size + (page_size - 1)) & ~(page_size - 1);
		block_buffer_size = (required_block_buffer_size + (page_size - 1)) & ~(page_size - 1);
		idle_location_table_buffer_size = (required_idle_location_table_buffer_size + (page_size - 1)) & ~(page_size - 1);
		pickup_location_buffer_size = (required_pickup_location_buffer_size + (page_size - 1)) & ~(page_size - 1);
	}
	device_table_buffer_size = (required_device_buffer_size + (page_size - 1)) & ~(page_size - 1);
	product_order_table_buffer_size = (required_product_order_table_buffer_size + (page_size - 1)) & ~(page_size - 1);
	if (page_size >= 0x400 + VRP_LOG_ENTRY_BUFFER_SIZE)
	{
		broadcast_io_buffer_size = 0x200;
		emergency_io_buffer_size = 0x200;
		log_entry_buffer_size = page_size - (broadcast_io_buffer_size + emergency_io_buffer_size);
	}
	else
	{
		broadcast_io_buffer_size = (0x200 + (page_size - 1)) & ~(page_size - 1);
		emergency_io_buffer_size = (0x200 + (page_size - 1)) & ~(page_size - 1);
		log_entry_buffer_size = (VRP_LOG_ENTRY_BUFFER_SIZE + (page_size - 1)) & ~(page_size - 1);
	}
	device_io_buffer_size = (VRP_DEVICE_IO_MEMORY_SIZE + (page_size - 1)) & ~(page_size - 1);

	total_allocation_size =
		main_part_buffer_size +
		map_bitmap_buffer_size +
		map_state_buffer_size +
		block_buffer_size +
		idle_location_table_buffer_size +
		pickup_location_buffer_size +
		device_table_buffer_size +
		product_order_table_buffer_size +
		broadcast_io_buffer_size +
		emergency_io_buffer_size +
		log_entry_buffer_size +
		(VRP_MAX_DEVICE_COUNT * device_io_buffer_size);
	vrp_server_t* server = (vrp_server_t*)VirtualAlloc(0, total_allocation_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!server)
	{
		vrp_free_master_configuration(configuration);
		WSACleanup();
		*error_information_text = "Error memory allocation failed";
		return ERROR_OUTOFMEMORY;
	}

	server->page_size = page_size;
	server->main_part_buffer_size = main_part_buffer_size;
	server->map_bitmap_buffer_size = map_bitmap_buffer_size;
	server->map_state_buffer_size = map_state_buffer_size;
	server->block_buffer_size = block_buffer_size;
	server->idle_location_table_buffer_size = idle_location_table_buffer_size;
	server->pickup_location_buffer_size = pickup_location_buffer_size;
	server->device_table_buffer_size = device_table_buffer_size;
	server->product_order_table_buffer_size = product_order_table_buffer_size;
	server->broadcast_io_buffer_size = broadcast_io_buffer_size;
	server->emergency_io_buffer_size = emergency_io_buffer_size;
	server->log_entry_buffer_size = log_entry_buffer_size;
	server->device_io_buffer_size = device_io_buffer_size;
	server->total_allocation_size = total_allocation_size;

	server->map_bitmap = (uint8_t*)((UINT_PTR)server + main_part_buffer_size);
	server->map_state = (void*)((UINT_PTR)server->map_bitmap + map_bitmap_buffer_size);
	server->block_table = (void*)((UINT_PTR)server->map_state + map_state_buffer_size);
	server->idle_location_table = (void*)((UINT_PTR)server->block_table + block_buffer_size);
	server->pickup_location_table = (void*)((UINT_PTR)server->idle_location_table + idle_location_table_buffer_size);
	server->device_table = (void*)((UINT_PTR)server->pickup_location_table + pickup_location_buffer_size);
	server->product_order_table = (void*)((UINT_PTR)server->device_table + device_table_buffer_size);
	server->broadcast_io_memory = (uint8_t*)((UINT_PTR)server->product_order_table + product_order_table_buffer_size);
	server->emergency_io_memory = (uint8_t*)((UINT_PTR)server->broadcast_io_memory + broadcast_io_buffer_size);
	server->log_entry_buffer = (char*)((UINT_PTR)server->emergency_io_memory + emergency_io_buffer_size);
	for (size_t offset = main_part_buffer_size + map_bitmap_buffer_size + map_state_buffer_size + block_buffer_size + idle_location_table_buffer_size + pickup_location_buffer_size + device_table_buffer_size + product_order_table_buffer_size + broadcast_io_buffer_size + emergency_io_buffer_size + log_entry_buffer_size, i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
	{
		server->device_table[i].io_memory = (uint8_t*)((UINT_PTR)server + offset + (i * device_io_buffer_size));
#ifndef _NDEBUG
		server->debug_device_table[i] = server->device_table + i;
#endif
	}

#ifndef _NDEBUG
	for (size_t i = 0; i != VRP_MAX_PRODUCT_ORDER_COUNT; ++i)
		server->debug_product_order_table[i] = server->product_order_table + i;
#endif

	server->log.handle = INVALID_HANDLE_VALUE;
	server->emergency_sock = INVALID_SOCKET;
	server->broadcast_sock = INVALID_SOCKET;
	server->listen_sock = INVALID_SOCKET;

	server->broadcast_immediately = 0;

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

	error = vrp_open_log_file(&server->log, 0x200000000, 0x100000, 0);
	if (error)
	{
		vrp_free_master_configuration(configuration);
		VirtualFree(server, 0, MEM_RELEASE);
		WSACleanup();
		*error_information_text = "Error failed to open or create log file";
		return error;
	}
	SetConsoleCtrlHandler(vrp_ctr_c_close_process_routine, TRUE);
	vrp_set_exit_flush_handle(server->log.handle);

	server->emergency_io_state = VRP_IO_IDLE;
	server->emergency_io_result.hEvent = CreateEventW(0, TRUE, FALSE, 0);
	if (!server->emergency_io_result.hEvent)
	{
		error = GetLastError();
		vrp_free_master_configuration(configuration);
		vrp_close_server_instance(server);
		*error_information_text = "Error failed to create system event object";
		return error;
	}

	server->last_broadcast_time = 0;
	server->broadcast_io_state = VRP_IO_IDLE;
	server->broadcast_io_result.hEvent = CreateEventW(0, TRUE, FALSE, 0);
	if (!server->broadcast_io_result.hEvent)
	{
		error = GetLastError();
		vrp_free_master_configuration(configuration);
		vrp_close_server_instance(server);
		*error_information_text = "Error failed to create system event object";
		return error;
	}

	server->accept_event = CreateEventW(0, TRUE, FALSE, 0);
	if (!server->accept_event)
	{
		error = GetLastError();
		vrp_free_master_configuration(configuration);
		vrp_close_server_instance(server);
		*error_information_text = "Error failed to create system event object";
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
			vrp_close_server_instance(server);
			*error_information_text = "Error failed to create emergency UDP socket";
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
			vrp_close_server_instance(server);
			*error_information_text = "Error failed to initialize emergency socket";
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
			vrp_close_server_instance(server);
			*error_information_text = "Error failed to create broadcast UDP socket";
			return error;
		}

		if (setsockopt(server->broadcast_sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast_enable, sizeof(BOOL)))
		{
			error = ERROR_OPEN_FAILED;
			vrp_free_master_configuration(configuration);
			vrp_close_server_instance(server);
			*error_information_text = "Error failed to initialize broadcast socket";
			return error;
		}
	}

	server->listen_sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (server->listen_sock == INVALID_SOCKET)
	{
		error = ERROR_OPEN_FAILED;
		vrp_free_master_configuration(configuration);
		vrp_close_server_instance(server);
		*error_information_text = "Error failed to create server TCP socket";
		return error;
	}

	if (bind(server->listen_sock, (SOCKADDR*)&server->server_address, sizeof(SOCKADDR)) == SOCKET_ERROR ||
		listen(server->listen_sock, SOMAXCONN) == SOCKET_ERROR ||
		WSAEventSelect(server->listen_sock, server->accept_event, FD_ACCEPT) == SOCKET_ERROR)
	{
		error = ERROR_OPEN_FAILED;
		vrp_free_master_configuration(configuration);
		vrp_close_server_instance(server);
		*error_information_text = "Error failed to initialize server TCP socket";
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
			vrp_close_server_instance(server);
			*error_information_text = "Error failed to create system event object";
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

	server->time_resolution = vrp_get_system_tick_resolution();
	server->io_timeout = configuration->io_ms_timeout;
	server->command_timeout = configuration->command_ms_timeout;
	server->broadcast_delay = configuration->broadcast_ms_delay;
	server->idle_status_query_delay = configuration->idle_status_query_delay;
	server->product_pickup_status_query_delay = configuration->product_pickup_status_query_delay;
	server->acceptable_product_mask = configuration->acceptable_product_mask;
	server->carried_product_confidence_max = configuration->carried_product_confidence_max;
	server->carried_product_confidence_pickup_limit = configuration->carried_product_confidence_pickup_limit;
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
		server->pickup_location_table[i].id = configuration->pickup_location_table[i * 4 + 0];
		server->pickup_location_table[i].load_x = configuration->pickup_location_table[i * 4 + 1];
		server->pickup_location_table[i].load_y = configuration->pickup_location_table[i * 4 + 2];
		server->pickup_location_table[i].direction = configuration->pickup_location_table[i * 4 + 3];
		if (server->pickup_location_table[i].load_x < server->map_width &&
			server->pickup_location_table[i].load_y < server->map_height &&
			server->pickup_location_table[i].direction < 4)
		{
			switch (server->pickup_location_table[i].direction)
			{
				case VRP_DIRECTION_RIGHT:
					server->pickup_location_table[i].entry_x = server->pickup_location_table[i].load_x - 1;
					server->pickup_location_table[i].entry_y = server->pickup_location_table[i].load_y;
					break;
				case VRP_DIRECTION_UP:
					server->pickup_location_table[i].entry_x = server->pickup_location_table[i].load_x;
					server->pickup_location_table[i].entry_y = server->pickup_location_table[i].load_y - 1;
					break;
				case VRP_DIRECTION_LEFT:
					server->pickup_location_table[i].entry_x = server->pickup_location_table[i].load_x + 1;
					server->pickup_location_table[i].entry_y = server->pickup_location_table[i].load_y;
					break;
				case VRP_DIRECTION_DOWN:
					server->pickup_location_table[i].entry_x = server->pickup_location_table[i].load_x;
					server->pickup_location_table[i].entry_y = server->pickup_location_table[i].load_y + 1;
					break;
				default:
					break;
			}
		}
		else
		{
			server->pickup_location_table[i].load_x = VRP_COORDINATE_UNDEFINED;
			server->pickup_location_table[i].load_y = VRP_COORDINATE_UNDEFINED;
			server->pickup_location_table[i].direction = VRP_DIRECTION_UNDEFINED;
			server->pickup_location_table[i].entry_x = VRP_COORDINATE_UNDEFINED;
			server->pickup_location_table[i].entry_y = VRP_COORDINATE_UNDEFINED;
		}
	}

	vrp_free_master_configuration(configuration);

	char startup_log_entry[128];
	sprintf(startup_log_entry, "Server started with status %lu and id %lu at address %lu.%lu.%lu.%lu", server->status, server->id,
		((server->server_address.sin_addr.s_addr) >> 0) & 0xFF, ((server->server_address.sin_addr.s_addr) >> 8) & 0xFF, ((server->server_address.sin_addr.s_addr) >> 16) & 0xFF, ((server->server_address.sin_addr.s_addr) >> 24) & 0xFF);
	vrp_write_log_entry(&server->log, startup_log_entry);

	server->time = NtGetTickCount();
	*server_instance = server;
	*error_information_text = "";
	server->broadcast_immediately = 1;
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
	vrp_shutdown_device_connection(server, i);
	closesocket(server->device_table[i].sock);
	server->device_table[i].sock = INVALID_SOCKET;
	server->device_count--;
}

size_t vrp_send_system_broadcast_message(vrp_server_t* server)
{
	assert(server->broadcast_io_buffer_size >= 0x200);

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
		server->last_broadcast_time = server->time;
		server->broadcast_immediately = 0;
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
			server->device_table[i].connection_begin_time = server->time;
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
			server->device_table[i].carried_product_confidence = 0;
			server->device_table[i].carried_product_id = 0;
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