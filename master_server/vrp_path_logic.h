/*
	VarastoRobo master server version 0.4.2 2019-11-20 by Santtu Nyman.
*/

#ifndef VRP_PATH_LOGIC_H
#define VRP_PATH_LOGIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vrp_master_server_base.h"

void vrp_coordinate_from_direction(uint8_t base_x, uint8_t base_y, int direction, uint8_t* x, uint8_t* y);

int vrp_is_cell_open(const vrp_server_t* server, int x, int y);

void vrp_initialize_path_finder(vrp_server_t* server, int exclude_x, int exclude_y);

int vrp_create_path_map(vrp_server_t* server, int target_x, int target_y);

int vrp_calculate_direction_to_target(vrp_server_t* server, int current_x, int current_y, int target_x, int target_y);

int vrp_is_coordinate_idle_location(const vrp_server_t* server, int x, int y);

int vrp_get_idle_location_for_device(const vrp_server_t* server, size_t device_index, uint8_t* x, uint8_t* y);

int vrp_get_device_distance_to_pickup_location(vrp_server_t* server, size_t device_index, size_t* pickup_location_index);

size_t vrp_get_device_index_for_free_transport(vrp_server_t* server);

size_t vrp_get_nearest_pickup_location_index_for_device(vrp_server_t* server, size_t device_index);

size_t vrp_is_device_on_pickup_location(vrp_server_t* server, size_t device_index);

int vrp_get_any_transport_device_from_pickup_location(vrp_server_t* server, size_t* device_index, size_t* pickup_location_index);

#ifdef __cplusplus
}
#endif

#endif