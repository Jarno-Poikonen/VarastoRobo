/*
	VarastoRobo master server version 1.1.0 2019-12-12 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
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
#include "ntdll_time.h"
#include "vrp_master_server_types.h"
#include <assert.h>

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