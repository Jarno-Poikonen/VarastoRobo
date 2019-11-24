/*
	VarastoRobo common code 2019-11-10 by Santtu Nyman.
*/

#ifndef ROBO_WIN32_BROADCAST_H
#define ROBO_WIN32_BROADCAST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <windows.h>

typedef struct robo_win32_broadcast_device_t
{
	uint8_t type;
	uint8_t id;
	uint8_t x;
	uint8_t y;
	uint32_t ip_address;
} robo_win32_broadcast_device_t;

typedef struct robo_win32_broadcast_info_t
{
	uint32_t master_ip_address;
	uint8_t system_status;
	uint8_t master_id;
	uint8_t map_height;
	uint8_t map_width;
	uint8_t block_count;
	uint8_t device_count;
	uint8_t pickup_location_count;
	uint8_t* valid_map_locations;
	uint8_t* block_table;
	robo_win32_broadcast_device_t* device_table;
	uint8_t* pickup_location_table;
} robo_win32_broadcast_info_t;

typedef struct robo_win32_broadcast_t
{
	HANDLE stop_event;
	HANDLE closed_event;
	HANDLE wake_event;
	HANDLE mutex;
	DWORD error;
	DWORD sleep_time;
	robo_win32_broadcast_info_t info;
	uint8_t valid_map_location_buffer[2048];
	uint8_t block_buffer[64];
	robo_win32_broadcast_device_t device_buffer[64];
} robo_win32_broadcast_t;

DWORD robo_win32_find_master(robo_win32_broadcast_info_t** broadcast_info);

void robo_win32_free_broadcast_info(robo_win32_broadcast_info_t* broadcast_info);

DWORD robo_win32_create_broadcast_thread(volatile robo_win32_broadcast_t* configuration, DWORD ms_sleep_time, uint8_t system_status, uint8_t master_id, uint8_t map_height, uint8_t map_width, const uint8_t* valid_map_locations);

void robo_win32_stop_broadcast_thread(volatile robo_win32_broadcast_t* configuration);

void robo_win32_set_broadcast_master_id(volatile robo_win32_broadcast_t* configuration, uint8_t master_id);

void robo_win32_set_broadcast_sleep_time(volatile robo_win32_broadcast_t* configuration, uint32_t ms_sleep_time);

void robo_win32_set_system_status(volatile robo_win32_broadcast_t* configuration, uint8_t system_status);

int robo_win32_add_broadcast_block(volatile robo_win32_broadcast_t* configuration, uint8_t x, uint8_t y);

int robo_win32_remove_broadcast_block(volatile robo_win32_broadcast_t* configuration, uint8_t x, uint8_t y);

int robo_win32_add_broadcast_device(volatile robo_win32_broadcast_t* configuration, uint8_t device_type, uint8_t device_id, uint8_t x, uint8_t y, uint32_t device_ip);

int robo_win32_remove_broadcast_device_by_id(volatile robo_win32_broadcast_t* configuration, uint8_t device_id);

int robo_win32_remove_broadcast_device_by_ip(volatile robo_win32_broadcast_t* configuration, uint32_t device_ip);

#ifdef __cplusplus
}
#endif

#endif