/*
	VarastoRobo master server version 1.0.0 2019-12-10 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#ifndef VRP_PATH_LOGIC_H
#define VRP_PATH_LOGIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vrp_master_server_base.h"

void vrp_coordinate_from_direction(uint8_t base_x, uint8_t base_y, int direction, uint8_t* x, uint8_t* y);

int vrp_is_cell_open(const vrp_server_t* server, int x, int y);

void vrp_initialize_path_finder(vrp_server_t* server, int exclude_x, int exclude_y, int device_ignore_distance, int ignore_all_devices_below_priority);

int vrp_create_path_map(vrp_server_t* server, int target_x, int target_y, int do_not_care_if_target_cell_is_not_open);

int vrp_calculate_direction_to_coordinate(int base_x, int base_y, int target_x, int target_y);

void vrp_calculate_coordinate_form_direction(int base_x, int base_y, int direction, uint8_t* x, uint8_t* y);

int vrp_calculate_immediate_path_to_target(vrp_server_t* server, size_t device_index);

int vrp_is_coordinate_idle_location(const vrp_server_t* server, int x, int y);

int vrp_deprecated_get_idle_location_for_device(const vrp_server_t* server, size_t device_index, uint8_t* x, uint8_t* y);

int vrp_get_idle_location_for_device(const vrp_server_t* server, size_t device_index, uint8_t* x, uint8_t* y);

int vrp_get_device_distance_to_pickup_location_entry(vrp_server_t* server, size_t device_index, size_t* pickup_location_index);

size_t vrp_get_device_index_for_free_transport(vrp_server_t* server);

size_t vrp_get_nearest_pickup_location_index_for_device(vrp_server_t* server, size_t device_index);

size_t vrp_is_device_on_pickup_load_location(vrp_server_t* server, size_t device_index);

size_t vrp_get_pickup_location_index_by_load_coordinate(const vrp_server_t* server, int x, int y);

size_t vrp_get_pickup_location_index_by_entry_coordinate(const vrp_server_t* server, int x, int y);

size_t vrp_extended_get_transport_device_index_by_coordinate(const vrp_server_t* server, int x, int y, size_t* product_order_index);

int vrp_get_any_transport_device_from_pickup_load_location(vrp_server_t* server, size_t* device_index, size_t* pickup_location_index);

int vrp_is_device_likely_to_colide(const vrp_server_t* server, size_t device_this_index, size_t device_other_index, size_t time_step_count, int do_not_care_if_this_device_has_higher_priority, uint8_t* x, uint8_t* y);

size_t vrp_get_likely_to_colide_device_index(const vrp_server_t* server, size_t device_this_index, int do_not_care_if_this_device_has_higher_priority, uint8_t* x, uint8_t* y);

int vrp_is_coordinate_in_any_immediate_path(const vrp_server_t* server, size_t excluded_device_index, size_t in_time_steps, uint8_t x, uint8_t y);

int vrp_pick_most_prefarable_direction_to_move(const vrp_server_t* server, size_t device_index, int right_open, int up_open, int left_open, int down_open);

int vrp_calculate_direction_to_make_way_for_other_devices(vrp_server_t* server, size_t device_index);

int vrp_is_pickup_location_blocked(const vrp_server_t* server, size_t pickup_location_index);

int vrp_calculate_immediate_path_any_open_direction(vrp_server_t* server, size_t device_index);

#ifdef __cplusplus
}
#endif

#endif