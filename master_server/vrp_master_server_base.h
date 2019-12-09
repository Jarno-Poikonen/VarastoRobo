/*
	VarastoRobo master server version 0.9.3 2019-12-09 by Santtu Nyman.
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
#include "ntdll_time.h"
#include "vrp_log.h"
#include "vrp_ip_addresses_info.h"
#include <assert.h>

#define VRP_MAX_MAP_HEIGHT 64
#define VRP_MAX_MAP_WIDTH 64
#define VRP_MAP_SIZE ((size_t)(((VRP_MAX_MAP_HEIGHT * VRP_MAX_MAP_WIDTH) + 7) / 8))
#define VRP_MAX_BLOCK_COUNT 256
#define VRP_MAX_PICKUP_LOCATION_COUNT 128
#define VRP_MAX_PRODUCT_ORDER_COUNT 128
#define VRP_MAX_DEVICE_COUNT 32
#define VRP_MAX_IMMEDIATE_PATH_LENGTH 8
#define VRP_LOG_ENTRY_BUFFER_SIZE 0x400
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
#define VRP_ORDER_NOT_AVAILABLE 0
#define VRP_ORDER_IN_STORAGE 1
#define VRP_ORDER_PICKUP 2
#define VRP_ORDER_ON_MOVE 3
#define VRP_ORDER_FINAL_WAITING 4

typedef struct vrp_device_t
{
	SOCKET sock;
	int io_state;
	int unprocessed_io;
	int connection_state;
	DWORD io_begin_time;
	DWORD last_uppdate_time;
	DWORD connection_begin_time;
	DWORD last_moved;
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
	uint8_t home_x;
	uint8_t home_y;
	uint8_t carried_product_confidence;
	uint8_t carried_product_id;
	uint8_t* io_memory;
	DWORD io_flags;
	DWORD io_transfered;
	WSABUF io_buffer;
	OVERLAPPED io_result;
	size_t immediate_path_length;
	struct
	{
		uint8_t x;
		uint8_t y;
	} immediate_path[VRP_MAX_IMMEDIATE_PATH_LENGTH];
#ifndef _NDEBUG
	uint8_t debug_controling_device_id;
	int debug_priority;
#endif
} vrp_device_t;

typedef struct vrp_map_state_t
{
	int path_finder_state;
	uint8_t valid_location;
	uint8_t blocked;
	uint8_t used_by_device_id;
} vrp_map_state_t;

typedef struct vrp_block_t
{
	DWORD last_uppdate_time;
	uint8_t x;
	uint8_t y;
} vrp_block_t;

typedef struct vrp_idle_location_t
{
	uint8_t x;
	uint8_t y;
} vrp_idle_location_t;

typedef struct vrp_pickup_location_t
{
	uint8_t id;
	uint8_t load_x;
	uint8_t load_y;
	uint8_t direction;
	uint8_t entry_x;
	uint8_t entry_y;
} vrp_pickup_location_t;

typedef struct vrp_product_order_t
{
	int order_status;
	uint32_t order_number;
	DWORD placement_time;
	DWORD pickup_time;
	uint8_t product_id;
	uint8_t transport_device_id;
	uint8_t destination_x;
	uint8_t destination_y;
	uint8_t ur5_pickup_complete;
	uint8_t gopigo_pickup_complete;
} vrp_product_order_t;

typedef struct vrp_server_t
{
	struct sockaddr_in broadcast_address;
	struct sockaddr_in server_address;
	struct sockaddr_in emergency_address;
	DWORD time;
	DWORD time_resolution;
	DWORD io_timeout;
	DWORD command_timeout;
	DWORD broadcast_delay;
	DWORD idle_status_query_delay;
	DWORD product_pickup_status_query_delay;
	DWORD acceptable_product_mask;
	DWORD block_expiration_time;
	DWORD wait_for_path_timeout;
	DWORD product_not_available_timeout;
	uint32_t carried_product_confidence_max;
	uint32_t carried_product_confidence_pickup_limit;
	uint8_t status;
	uint8_t id;
	uint8_t map_height;
	uint8_t map_width;
	uint8_t min_temporal_id;
	uint8_t max_temporal_id;
	uint8_t* map_bitmap;
	vrp_map_state_t* map_state;
	size_t block_count;
	vrp_block_t* block_table;
	size_t idle_location_count;
	vrp_idle_location_t* idle_location_table;
	size_t device_count;
	vrp_device_t* device_table;
	size_t pickup_location_count;
	vrp_pickup_location_t* pickup_location_table;
	size_t product_order_count;
	vrp_product_order_t* product_order_table;
	int debug_no_emergency_listen;
	int debug_no_broadcast;
	size_t page_size;
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
	int broadcast_immediately;
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
	char* log_entry_buffer;
#ifndef _NDEBUG
	vrp_device_t* debug_device_table[VRP_MAX_DEVICE_COUNT];
#endif
#ifndef _NDEBUG
	vrp_product_order_t* debug_product_order_table[VRP_MAX_PRODUCT_ORDER_COUNT];
#endif
} vrp_server_t;

uint64_t vrp_get_valid_device_entries(const vrp_server_t* server);

int vrp_calculate_device_movement_priority(const vrp_server_t* server, uint8_t device_id);

int vrp_get_next_block_expiration_time(const vrp_server_t* server, uint32_t* expiration_time);

int vrp_remove_all_expired_blocks(vrp_server_t* server);

int vrp_add_block(vrp_server_t* server, uint8_t x, uint8_t y);

int vrp_remove_block(vrp_server_t* server, uint8_t x, uint8_t y);

uint32_t vrp_create_product_order_number(const vrp_server_t* server, uint8_t optional_client_id);

size_t vrp_get_order_index_by_number(const vrp_server_t* server, uint32_t order_number);

int vrp_remove_product_order(vrp_server_t* server, uint32_t order_number);

size_t vrp_create_product_order(vrp_server_t* server, uint8_t product_id, uint8_t x, uint8_t y, uint8_t optional_client_id);

int vrp_choose_product_order_destination(const vrp_server_t* server, size_t coordinate_count, const uint8_t* coordinate_table, uint8_t* destination_x, uint8_t* destination_y);

size_t vrp_get_nonstarted_product_order_index(const vrp_server_t* server);

size_t vrp_get_order_index_of_transport_device(const vrp_server_t* server, size_t device_index);

int vrp_is_valid_product_id(const vrp_server_t* server, uint8_t product_id, int is_not_undefined);

uint8_t vrp_get_temporal_device_id(const vrp_server_t* server);

size_t vrp_get_device_index_by_id(const vrp_server_t* server, uint8_t id);

size_t vrp_get_controlling_device_index(const vrp_server_t* server, uint8_t controled_device_id);

size_t vrp_get_pickup_location_index_by_id(const vrp_server_t* server, uint8_t pickup_location_id);

DWORD vrp_get_system_tick_resolution();

size_t vrp_get_message_size(const void* message);

int vrp_is_device_timeout_reached(const vrp_server_t* server, size_t device_index);

void vrp_shutdown_device_connection(vrp_server_t* server, size_t i);

int vrp_read(vrp_server_t* server, size_t device_index, size_t offset, size_t size);

int vrp_write(vrp_server_t* server, size_t device_index, size_t offset, size_t size);

size_t vrp_finish_io(vrp_server_t* server, size_t device_index, int* io_type);

size_t vrp_message_transfer_incomplete(const vrp_server_t* server, size_t device_index, int io_type);

int vrp_process_possible_emergency(vrp_server_t* server);

size_t vrp_get_free_device_slot(const vrp_server_t* server);

size_t vrp_get_device_index_by_io_event(const vrp_server_t* server, HANDLE io_event);

size_t vrp_wait_for_io(vrp_server_t* server);

BOOL WINAPI vrp_ctr_c_close_process_routine(DWORD parameter);

BOOL vrp_set_exit_flush_handle(HANDLE handle);

size_t vrp_get_page_size();

void vrp_close_server_instance(vrp_server_t* server);

DWORD vrp_create_server_instance(vrp_server_t** server_instance, const char** error_information_text);

void vrp_remove_device(vrp_server_t* server, size_t i);

size_t vrp_send_system_broadcast_message(vrp_server_t* server);

void vrp_finish_send_system_broadcast_message(vrp_server_t* server);

int vrp_accept_incoming_connection(vrp_server_t* server);

#ifdef __cplusplus
}
#endif

#endif