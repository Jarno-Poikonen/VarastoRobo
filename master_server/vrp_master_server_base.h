/*
	VarastoRobo master server version 0.4.3 2019-11-19 by Santtu Nyman.
*/

#ifndef VRP_MASTER_SERVER_BASE_H
#define VRP_MASTER_SERVER_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

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
#include "vrp_ip_addresses_info.h"
#include <assert.h>

#define VRP_MAX_MAP_HEIGHT 64
#define VRP_MAX_MAP_WIDTH 64
#define VRP_MAP_SIZE ((size_t)(((VRP_MAX_MAP_HEIGHT * VRP_MAX_MAP_WIDTH) + 7) / 8))
#define VRP_MAX_BLOCK_COUNT 256
#define VRP_MAX_PICKUP_LOCATION_COUNT 256
#define VRP_MAX_PRODUCT_ORDER_COUNT 128
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
#define VRP_CONNECTION_REMOTE_COMMAND 8

typedef struct vrp_device_t
{
	SOCKET sock;
	int io_state;
	int unprocessed_io;
	int connection_state;
	DWORD io_begin_time;
	DWORD last_uppdate_time;
	DWORD connection_begin_time;
	uint8_t control_target_id;
	uint8_t executing_remote_command;
	uint8_t remote_command_finished;
	uint8_t command;
	uint8_t type;
	uint8_t id;
	uint8_t state;
	uint8_t x;
	uint8_t y;
	uint8_t direction;
	uint32_t ip_address;
	uint8_t move_to_x;
	uint8_t move_to_y;
	uint8_t destination_x;
	uint8_t destination_y;
	uint8_t* io_memory;
	DWORD io_flags;
	DWORD io_transfered;
	WSABUF io_buffer;
	OVERLAPPED io_result;
} vrp_device_t;

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
	uint8_t* map_bitmap;
	struct
	{
		int path_finder_state;
		uint8_t valid_location;
		uint8_t blocked;
		uint8_t used_by_device_id;
	}* map_state;
	size_t block_count;
	struct
	{
		uint8_t x;
		uint8_t y;
	}* block_table;
	size_t idle_location_count;
	struct
	{
		uint8_t x;
		uint8_t y;
	}* idle_location_table;
	size_t device_count;
	vrp_device_t* device_table;
	size_t pickup_location_count;
	struct
	{
		uint8_t id;
		uint8_t x;
		uint8_t y;
	}* pickup_location_table;
	size_t product_order_count;
	struct
	{
		int order_status;
		DWORD placement_time;
		uint8_t product_id;
		uint8_t product_holder_device_id;
		uint8_t destination_x;
		uint8_t destination_y;
		uint8_t client_device_id;
	}* product_order_table;
	int debug_no_emergency_listen;
	int debug_no_broadcast;
	size_t page_size;
	size_t map_bitmap_buffer_size;
	size_t map_state_buffer_size;
	size_t block_buffer_size;
	size_t idle_location_table_buffer_size;
	size_t pickup_location_buffer_size;
	size_t device_table_buffer_size;
	size_t product_order_table_buffer_size;
	size_t broadcast_io_buffer_size;
	size_t emergency_io_buffer_size;
	size_t device_io_buffer_size;
	size_t allocation_size;
	void* allocation_base;
	DWORD last_broadcast_time;
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
#ifndef _NDEBUG
	vrp_device_t* debug_device_table[VRP_MAX_DEVICE_COUNT];
#endif
} vrp_server_t;

uint64_t vrp_get_valid_device_entries(vrp_server_t* server);

uint8_t vrp_get_temporal_device_id(vrp_server_t* server);

size_t vrp_get_device_index_by_id(vrp_server_t* server, uint8_t id);

size_t vrp_get_controlling_device_index(vrp_server_t* server, uint8_t controled_device_id);

DWORD vrp_get_system_tick();

size_t vrp_get_message_size(const void* message);

int vrp_read(vrp_server_t* server, size_t device_index, size_t offset, size_t size);

int vrp_write(vrp_server_t* server, size_t device_index, size_t offset, size_t size);

size_t vrp_finish_io(vrp_server_t* server, size_t device_index, int* io_type);

size_t vrp_message_transfer_incomplete(vrp_server_t* server, size_t device_index, int io_type);

int vrp_process_possible_emergency(vrp_server_t* server);

size_t vrp_get_free_device_slot(vrp_server_t* server);

size_t vrp_get_device_index_by_io_event(vrp_server_t* server, HANDLE io_event);

size_t vrp_wait_for_io(vrp_server_t* server);

BOOL WINAPI vrp_ctr_c_close_process_routine(DWORD parameter);

BOOL vrp_set_exit_flush_handle(HANDLE handle);

size_t vrp_get_page_size();

void vrp_server_close(vrp_server_t* server);

DWORD vrp_server_setup(vrp_server_t* server, int* error_hint);

void vrp_remove_device(vrp_server_t* server, size_t i);

size_t vrp_send_system_broadcast_message(vrp_server_t* server);

void vrp_finish_send_system_broadcast_message(vrp_server_t* server);

int vrp_accept_incoming_connection(vrp_server_t* server);

#ifdef __cplusplus
}
#endif

#endif