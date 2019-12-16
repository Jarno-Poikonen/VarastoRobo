#include "vrp_master_server_types.h"
#include <Windows.h>
#include <stdio.h>

size_t get_page_size()
{
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);
	return (size_t)system_info.dwPageSize;
}

vrp_server_t* map_view_of_server()
{
	size_t page_size = get_page_size();
	
	HANDLE handle = OpenFileMappingW(FILE_MAP_READ, FALSE, L"Local\\VRP_MASTER_SERVER");
	if (!handle)
		return 0;

	vrp_server_t* server_view = (vrp_server_t*)MapViewOfFile(handle, FILE_MAP_READ, 0, 0, (sizeof(vrp_server_header_t) + (page_size - 1)) & ~(page_size - 1));
	if (!server_view)
	{
		CloseHandle(handle);
		return 0;
	}

	size_t server_view_size = server_view->header.file_mapping_size;

	UnmapViewOfFile(server_view);
	server_view = (vrp_server_t*)MapViewOfFile(handle, FILE_MAP_READ, 0, 0, server_view_size);
	CloseHandle(handle);
	if (!server_view)
		return 0;
	
	return server_view;
}

void translate_server_view(const vrp_server_t* view, vrp_server_t* translated_view)
{
	CopyMemory(translated_view, view, view->header.file_mapping_size);

	uintptr_t remote_base = (uintptr_t)view->header.file_mapping_address;
	uintptr_t local_base = (uintptr_t)translated_view;
	translated_view->header.file_mapping_address = local_base;
	translated_view->map_bitmap = (uint8_t*)((uintptr_t)translated_view->map_bitmap - remote_base + local_base);
	translated_view->map_state = (vrp_map_state_t*)((uintptr_t)translated_view->map_state - remote_base + local_base);
	translated_view->block_table = (vrp_block_t*)((uintptr_t)translated_view->block_table - remote_base + local_base);
	translated_view->idle_location_table = (vrp_idle_location_t*)((uintptr_t)translated_view->idle_location_table - remote_base + local_base);
	translated_view->device_table = (vrp_device_t*)((uintptr_t)translated_view->device_table - remote_base + local_base);
	translated_view->pickup_location_table = (vrp_pickup_location_t*)((uintptr_t)translated_view->pickup_location_table - remote_base + local_base);
	translated_view->product_order_table = (vrp_product_order_t*)((uintptr_t)translated_view->product_order_table - remote_base + local_base);
	translated_view->broadcast_io_memory = (uint8_t*)((uintptr_t)translated_view->broadcast_io_memory - remote_base + local_base);
	translated_view->broadcast_io_buffer.buf = translated_view->broadcast_io_buffer.buf ? (CHAR*)((uintptr_t)translated_view->broadcast_io_buffer.buf - remote_base + local_base) : 0;
	translated_view->emergency_io_memory = (uint8_t*)((uintptr_t)translated_view->emergency_io_memory - remote_base + local_base);
	translated_view->emergency_io_buffer.buf = translated_view->emergency_io_buffer.buf ? (CHAR*)((uintptr_t)translated_view->emergency_io_buffer.buf - remote_base + local_base) : 0;
	translated_view->log_entry_buffer = (char*)((uintptr_t)translated_view->log_entry_buffer - remote_base + local_base);
	translated_view->log.line_table = 0;// not mapped
	for (size_t i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
		translated_view->debug_device_table[i] = (vrp_device_t*)((uintptr_t)translated_view->debug_device_table[i] - remote_base + local_base);
	for (size_t i = 0; i != VRP_MAX_PRODUCT_ORDER_COUNT; ++i)
		translated_view->debug_product_order_table[i] = (vrp_product_order_t*)((uintptr_t)translated_view->debug_product_order_table[i] - remote_base + local_base);

	for (size_t i = 0; i != VRP_MAX_DEVICE_COUNT; ++i)
	{
		translated_view->device_table[i].io_memory = (uint8_t*)((uintptr_t)translated_view->device_table[i].io_memory - remote_base + local_base);
		translated_view->device_table[i].io_buffer.buf = translated_view->device_table[i].io_buffer.buf ? (CHAR*)((uintptr_t)translated_view->device_table[i].io_buffer.buf - remote_base + local_base) : 0;
	}
}

int main()
{
	vrp_server_t* remote_view = map_view_of_server();
	if (!remote_view)
	{
		printf("Failed to map server memory");
		return EXIT_FAILURE;
	}

	vrp_server_t* view = (vrp_server_t*)VirtualAlloc(0, remote_view->header.file_mapping_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!view)
	{
		printf("Failed to allocate memory");
		UnmapViewOfFile(remote_view);
		return EXIT_FAILURE;
	}

	translate_server_view(remote_view, view);

	printf("Opened view of server process %lu at ip %lu.%lu.%lu.%lu\n",
		(unsigned long)view->header.server_process_id,
		(unsigned long)((view->server_address.sin_addr.s_addr >> 0) & 0xFF),
		(unsigned long)((view->server_address.sin_addr.s_addr >> 8) & 0xFF),
		(unsigned long)((view->server_address.sin_addr.s_addr >> 16) & 0xFF),
		(unsigned long)((view->server_address.sin_addr.s_addr >> 24) & 0xFF));

	for (;;)
	{
		DebugBreak();
		translate_server_view(remote_view, view);
	}
	
	return EXIT_FAILURE;
}