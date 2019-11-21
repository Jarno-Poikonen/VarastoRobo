/*
	VarastoRobo master server version 0.4.2 2019-11-20 by Santtu Nyman.
*/

#include "vrp_path_logic.h"

#define VRP_PATH_BLOCK -3
#define VRP_PATH_UNREACHABLE -2
#define VRP_PATH_OPEN -1
#define VRP_PATH_TARGET 0

void vrp_initialize_path_finder(vrp_server_t* server, int exclude_x, int exclude_y)
{
	__assume(
		(server->map_height <= VRP_MAX_MAP_HEIGHT) &&
		(server->map_width <= VRP_MAX_MAP_WIDTH));

	int h = (int)server->map_height;
	int w = (int)server->map_width;

	for (int y = 0; y != h; ++y)
		for (int x = 0; x != w; ++x)
		{
			int cell_index = y * w + x;
			server->map_state[cell_index].path_finder_state = ((server->map_bitmap[cell_index / 8] >> (cell_index % 8)) & 1) ? VRP_PATH_OPEN : VRP_PATH_BLOCK;
		}

	for (size_t block_count = server->block_count, block_index = 0; block_index != block_count; ++block_index)
		if ((server->block_table[block_index].y <= VRP_MAX_MAP_HEIGHT) && (server->block_table[block_index].x <= VRP_MAX_MAP_WIDTH))
			server->map_state[(int)server->block_table[block_index].y * w + (int)server->block_table[block_index].x].path_finder_state = VRP_PATH_BLOCK;

	for (size_t device_count = server->device_count, device_index = 0; device_index != device_count; ++device_index)
		if ((server->device_table[device_index].type == VRP_DEVICE_TYPE_GOPIGO) && (server->device_table[device_index].y <= VRP_MAX_MAP_HEIGHT) && (server->device_table[device_index].x <= VRP_MAX_MAP_WIDTH))
		{
			server->map_state[(int)server->device_table[device_index].y * w + (int)server->device_table[device_index].x].path_finder_state = VRP_PATH_BLOCK;
			if ((server->device_table[device_index].move_to_y != VRP_COORDINATE_UNDEFINED) && (server->device_table[device_index].move_to_x != VRP_COORDINATE_UNDEFINED))
				server->map_state[(int)server->device_table[device_index].move_to_y * w + (int)server->device_table[device_index].move_to_x].path_finder_state = VRP_PATH_BLOCK;
		}

	if ((exclude_x < w) && (exclude_y < h))
		server->map_state[exclude_y * w + exclude_x].path_finder_state = VRP_PATH_OPEN;

	/*
	for (int y = h; y--;)
	{
		for (int x = 0; x != w; ++x)
			printf("%c", server->map_state[y * w + x].path_finder_state == VRP_PATH_OPEN ? '#' : ' ');
		printf("\n");
	}
	*/
}

int vrp_create_path_map(vrp_server_t* server, int target_x, int target_y)
{
	__assume(
		(server->map_height <= VRP_MAX_MAP_HEIGHT) &&
		(server->map_width <= VRP_MAX_MAP_WIDTH));

	int h = (int)server->map_height;
	int w = (int)server->map_width;

	if ((server->map_height <= target_y) ||
		(server->map_width <= target_x) ||
		(server->map_state[target_y * w + target_x].path_finder_state != VRP_PATH_OPEN))
		return 0;

	server->map_state[target_y * w + target_x].path_finder_state = VRP_PATH_TARGET;

	for (int i = 0, c = 1; c; ++i)
	{
		c = 0;
		for (int y = 0; y != h; ++y)
			for (int x = 0; x != w; ++x)
				if (server->map_state[y * w + x].path_finder_state == i)
				{
					if ((y - 1 >= 0) && (server->map_state[(y - 1) * w + x].path_finder_state == VRP_PATH_OPEN))
					{
						server->map_state[(y - 1) * w + x].path_finder_state = i + 1;
						++c;
					}
					if ((x - 1 >= 0) && (server->map_state[y * w + (x - 1)].path_finder_state == VRP_PATH_OPEN))
					{
						server->map_state[y * w + (x - 1)].path_finder_state = i + 1;
						++c;
					}
					if ((x + 1 < w) && (server->map_state[y * w + (x + 1)].path_finder_state == VRP_PATH_OPEN))
					{
						server->map_state[y * w + (x + 1)].path_finder_state = i + 1;
						++c;
					}
					if ((y + 1 < h) && (server->map_state[(y + 1) * w + x].path_finder_state == VRP_PATH_OPEN))
					{
						server->map_state[(y + 1) * w + x].path_finder_state = i + 1;
						++c;
					}
				}
	}

	for (int y = 0; y != h; ++y)
		for (int x = 0; x != w; ++x)
			if (server->map_state[y * w + x].path_finder_state == VRP_PATH_OPEN)
				server->map_state[y * w + x].path_finder_state = VRP_PATH_UNREACHABLE;

	/*
	for (int y = h; y--;)
	{
		for (int x = 0; x != w; ++x)
			printf("%c", server->map_state[y * w + x].path_finder_state > -1 ? ('0' + (char)server->map_state[y * w + x].path_finder_state) : ' ');
		printf("\n");
	}
	*/

	return 1;
}

int vrp_calculate_direction_to_target(vrp_server_t* server, int x, int y, int target_x, int target_y)
{
	__assume(
		(server->map_height <= VRP_MAX_MAP_HEIGHT) &&
		(server->map_width <= VRP_MAX_MAP_WIDTH));

	int h = (int)server->map_height;
	int w = (int)server->map_width;

	if (y >= h || x >= w)
		return VRP_DIRECTION_UNDEFINED;

	vrp_initialize_path_finder(server, x, y);
	if (!vrp_create_path_map(server, target_x, target_y))
		return VRP_DIRECTION_UNDEFINED;

	if (server->map_state[y * w + x].path_finder_state < 1)
		return VRP_DIRECTION_UNDEFINED;

	if ((y - 1 >= 0) && (server->map_state[(y - 1) * w + x].path_finder_state > -1) && (server->map_state[(y - 1) * w + x].path_finder_state < server->map_state[y * w + x].path_finder_state))
		return VRP_DIRECTION_DOWN;
	else if ((x - 1 >= 0) && (server->map_state[y * w + (x - 1)].path_finder_state > -1) && (server->map_state[y * w + (x - 1)].path_finder_state < server->map_state[y * w + x].path_finder_state))
		return VRP_DIRECTION_LEFT;
	else if ((x + 1 < w) && (server->map_state[y * w + (x + 1)].path_finder_state > -1) && (server->map_state[y * w + (x + 1)].path_finder_state < server->map_state[y * w + x].path_finder_state))
		return VRP_DIRECTION_RIGHT;
	else if ((y + 1 < h) && (server->map_state[(y + 1) * w + x].path_finder_state > -1) && (server->map_state[(y + 1) * w + x].path_finder_state < server->map_state[y * w + x].path_finder_state))
		return VRP_DIRECTION_UP;

	return VRP_DIRECTION_UNDEFINED;
}
