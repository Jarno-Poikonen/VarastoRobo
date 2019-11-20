/*
	VarastoRobo master server version 0.4.2 2019-11-20 by Santtu Nyman.
*/

#ifndef VRP_PATH_LOGIC_H
#define VRP_PATH_LOGIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vrp_master_server_base.h"

void vrp_initialize_path_finder(vrp_server_t* server, int exclude_x, int exclude_y);

int vrp_create_path_map(vrp_server_t* server, int target_x, int target_y);

int vrp_calculate_direction_to_target(vrp_server_t* server, int current_x, int current_y, int target_x, int target_y);

#ifdef __cplusplus
}
#endif

#endif