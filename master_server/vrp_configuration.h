/*
	VarastoRobo master server version 1.0.0 2019-12-10 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#ifndef VRP_CONFIGURATION_H
#define VRP_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <windows.h>
#include "vrp_file.h"
#include "jsonpl.h"

typedef struct vrp_configuration_t
{
	uint32_t broadcast_ms_delay;
	uint32_t io_ms_timeout;
	uint32_t command_ms_timeout;
	uint32_t on_wire_broadcast_ip_address;
	uint32_t on_wire_server_ip_address;
	uint32_t network_prefix_length;
	uint32_t idle_status_query_delay;
	uint32_t product_pickup_status_query_delay;
	uint32_t acceptable_product_mask;
	uint32_t carried_product_confidence_max;
	uint32_t carried_product_confidence_pickup_limit;
	uint32_t block_expiration_time;
	uint32_t wait_for_path_ms_timeout;
	uint32_t product_not_available_ms_timeout;
	uint8_t master_id;
	uint8_t system_status;
	uint8_t min_temporal_id;
	uint8_t max_temporal_id;
	uint8_t map_height;
	uint8_t map_width;
	uint8_t debug_no_broadcast;
	uint8_t debug_no_emergency_listen;
	uint8_t* map;
	size_t block_count;
	uint8_t* block_table;
	size_t pickup_location_count;
	uint8_t* pickup_location_table;
	size_t idle_location_count;
	uint8_t* idle_location_table;
} vrp_configuration_t;

DWORD vrp_load_master_configuration(vrp_configuration_t** master_configuration);

void vrp_free_master_configuration(vrp_configuration_t* master_configuration);

#ifdef __cplusplus
}
#endif

#endif