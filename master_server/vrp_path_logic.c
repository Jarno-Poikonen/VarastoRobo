/*
	VarastoRobo master server version 0.8.0 2019-12-03 by Santtu Nyman.
*/

#include "vrp_path_logic.h"

#define VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE 3

#define VRP_PATH_BLOCK -3
#define VRP_PATH_UNREACHABLE -2
#define VRP_PATH_OPEN -1
#define VRP_PATH_TARGET 0

void vrp_coordinate_from_direction(uint8_t base_x, uint8_t base_y, int direction, uint8_t* x, uint8_t* y)
{
	switch (direction)
	{
		case VRP_DIRECTION_RIGHT :
			*x = base_x + 1;
			*y = base_y;
			return;
		case VRP_DIRECTION_UP:
			*x = base_x;
			*y = base_y + 1;
			return;
		case VRP_DIRECTION_LEFT:
			*x = base_x - 1;
			*y = base_y;
			return;
		case VRP_DIRECTION_DOWN:
			*x = base_x;
			*y = base_y - 1;
			return;
		default:
			assert(0);
			return;
	}

	__assume(0);
}

int vrp_is_cell_open(const vrp_server_t* server, int x, int y)
{
	__assume(
		(server->map_height <= VRP_MAX_MAP_HEIGHT) &&
		(server->map_width <= VRP_MAX_MAP_WIDTH));

	if ((x >= VRP_MAX_MAP_WIDTH) || (y >= VRP_MAX_MAP_HEIGHT))
		return 0;

	int cell_index = y * server->map_width + x;
	if (!((server->map_bitmap[cell_index / 8] >> (cell_index % 8)) & 1))
		return 0;

	for (size_t block_count = server->block_count, block_index = 0; block_index != block_count; ++block_index)
		if ((server->block_table[block_index].y <= VRP_MAX_MAP_HEIGHT) &&
			(server->block_table[block_index].x <= VRP_MAX_MAP_WIDTH) &&
			(server->block_table[block_index].y == y) &&
			(server->block_table[block_index].x == x))
				return 0;

	for (size_t device_count = server->device_count, device_index = 0; device_index != device_count; ++device_index)
		if ((server->device_table[device_index].type == VRP_DEVICE_TYPE_GOPIGO) &&
			(server->device_table[device_index].y <= VRP_MAX_MAP_HEIGHT) &&
			(server->device_table[device_index].x <= VRP_MAX_MAP_WIDTH) &&
			(((server->device_table[device_index].y == y) && (server->device_table[device_index].x == x)) ||
			((server->device_table[device_index].move_to_y == y) && (server->device_table[device_index].move_to_x == x))))
				return 0;

	return 1;
}

void vrp_initialize_path_finder(vrp_server_t* server, int exclude_x, int exclude_y, int device_ignore_distance, int ignore_all_devices_below_priority)
{
	__assume(
		(server->map_height <= VRP_MAX_MAP_HEIGHT) &&
		(server->map_width <= VRP_MAX_MAP_WIDTH));

	assert(device_ignore_distance > -1);

	if (!device_ignore_distance)
		device_ignore_distance = INT_MAX;
	else if (device_ignore_distance != INT_MAX)
		device_ignore_distance = device_ignore_distance * device_ignore_distance;

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
		if ((server->device_table[device_index].type == VRP_DEVICE_TYPE_GOPIGO) &&
			(server->device_table[device_index].y <= VRP_MAX_MAP_HEIGHT) &&
			(server->device_table[device_index].x <= VRP_MAX_MAP_WIDTH) &&
			((((exclude_x - server->device_table[device_index].x) * (exclude_x - server->device_table[device_index].x)) + ((exclude_y - server->device_table[device_index].y) * (exclude_y - server->device_table[device_index].y))) < device_ignore_distance) &&
			(vrp_calculate_device_movement_priority(server, server->device_table[device_index].id) >= ignore_all_devices_below_priority))
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

int vrp_create_path_map(vrp_server_t* server, int target_x, int target_y, int do_not_care_if_target_cell_is_not_open)
{
	__assume(
		(server->map_height <= VRP_MAX_MAP_HEIGHT) &&
		(server->map_width <= VRP_MAX_MAP_WIDTH));

	int h = (int)server->map_height;
	int w = (int)server->map_width;

	if ((server->map_height <= target_y) || (server->map_width <= target_x))
		return 0;

	if (do_not_care_if_target_cell_is_not_open)
	{
		server->map_state[target_y * w + target_x].path_finder_state = VRP_PATH_OPEN;
	}
	else
	{
		if (server->map_state[target_y * w + target_x].path_finder_state != VRP_PATH_OPEN)
			return 0;
	}

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

int vrp_calculate_direction_to_coordinate(int base_x, int base_y, int target_x, int target_y)
{
	if (base_x < target_x)
		return VRP_DIRECTION_RIGHT;
	else if (base_x > target_x)
		return VRP_DIRECTION_LEFT;
	else if (base_y < target_y)
		return VRP_DIRECTION_UP;
	else if (base_y > target_y)
		return VRP_DIRECTION_DOWN;
	else
		return VRP_DIRECTION_UNDEFINED;
}

void vrp_calculate_coordinate_form_direction(int base_x, int base_y, int direction, uint8_t* x, uint8_t* y)
{
	switch (direction)
	{
		case VRP_DIRECTION_RIGHT:
			*x = (uint8_t)base_x + 1;
			*y = (uint8_t)base_y;
			return;
		case VRP_DIRECTION_LEFT:
			*x = (uint8_t)base_x - 1;
			*y = (uint8_t)base_y;
			return;
		case VRP_DIRECTION_UP:
			*x = (uint8_t)base_x;
			*y = (uint8_t)base_y + 1;
			return;
		case VRP_DIRECTION_DOWN:
			*x = (uint8_t)base_x;
			*y = (uint8_t)base_y - 1;
			return;
		default:
			*x = (uint8_t)base_x;
			*y = (uint8_t)base_y;
			return;
	}
}

int vrp_calculate_immediate_path_to_target(vrp_server_t* server, size_t device_index)
{
	__assume(
		(server->map_height <= VRP_MAX_MAP_HEIGHT) &&
		(server->map_width <= VRP_MAX_MAP_WIDTH));

	int h = (int)server->map_height;
	int w = (int)server->map_width;
	int x = server->device_table[device_index].x;
	int y = server->device_table[device_index].y;
	int target_x = server->device_table[device_index].destination_x;
	int target_y = server->device_table[device_index].destination_y;

	if (y >= h || x >= w)
	{
		server->device_table[device_index].immediate_path_length = 0;
		return VRP_DIRECTION_UNDEFINED;
	}

	vrp_initialize_path_finder(server, x, y, VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE, vrp_calculate_device_movement_priority(server, server->device_table[device_index].id));
	if (!vrp_create_path_map(server, target_x, target_y, 0))
	{
		server->device_table[device_index].immediate_path_length = 0;
		return VRP_DIRECTION_UNDEFINED;
	}

	if (server->map_state[y * w + x].path_finder_state < 1)
	{
		server->device_table[device_index].immediate_path_length = 0;
		return VRP_DIRECTION_UNDEFINED;
	}

	server->device_table[device_index].immediate_path_length = 0;
	while (server->device_table[device_index].immediate_path_length != VRP_MAX_IMMEDIATE_PATH_LENGTH &&
		!(server->device_table[device_index].immediate_path_length && (server->device_table[device_index].immediate_path[server->device_table[device_index].immediate_path_length - 1].x == target_x && server->device_table[device_index].immediate_path[server->device_table[device_index].immediate_path_length - 1].y == target_y)))
	{
		if ((y - 1 >= 0) && (server->map_state[(y - 1) * w + x].path_finder_state > -1) && (server->map_state[(y - 1) * w + x].path_finder_state < server->map_state[y * w + x].path_finder_state))
			--y;
		else if ((x - 1 >= 0) && (server->map_state[y * w + (x - 1)].path_finder_state > -1) && (server->map_state[y * w + (x - 1)].path_finder_state < server->map_state[y * w + x].path_finder_state))
			--x;
		else if ((x + 1 < w) && (server->map_state[y * w + (x + 1)].path_finder_state > -1) && (server->map_state[y * w + (x + 1)].path_finder_state < server->map_state[y * w + x].path_finder_state))
			++x;
		else if ((y + 1 < h) && (server->map_state[(y + 1) * w + x].path_finder_state > -1) && (server->map_state[(y + 1) * w + x].path_finder_state < server->map_state[y * w + x].path_finder_state))
			++y;
		else
			break;

		server->device_table[device_index].immediate_path[server->device_table[device_index].immediate_path_length].x = x;
		server->device_table[device_index].immediate_path[server->device_table[device_index].immediate_path_length].y = y;
		server->device_table[device_index].immediate_path_length++;
	}

	if (server->device_table[device_index].immediate_path_length)
	{
		int first_direction = vrp_calculate_direction_to_coordinate(server->device_table[device_index].x, server->device_table[device_index].y, server->device_table[device_index].immediate_path[0].x, server->device_table[device_index].immediate_path[0].y);
		assert(first_direction != VRP_DIRECTION_UNDEFINED);
		return first_direction;
	}

	return VRP_DIRECTION_UNDEFINED;
}

int vrp_is_coordinate_idle_location(const vrp_server_t* server, int x, int y)
{
	for (size_t i = 0; i != server->idle_location_count; ++i)
		if ((x == server->idle_location_table[i].x) && (y == server->idle_location_table[i].y))
			return 1;
	return 0;
}

int vrp_get_idle_location_for_device(const vrp_server_t* server, size_t device_index, uint8_t* x, uint8_t* y)
{
	assert(device_index < VRP_MAX_DEVICE_COUNT);

	if (vrp_is_coordinate_idle_location(server, server->device_table[device_index].x, server->device_table[device_index].y))
		return 0;

	for (size_t i = 0; i != server->idle_location_count; ++i)
		if (vrp_is_cell_open(server, server->idle_location_table[i].x, server->idle_location_table[i].y))
		{
			*x = server->idle_location_table[i].x;
			*y = server->idle_location_table[i].y;
			return 1;
		}
		
	if (vrp_is_cell_open(server, 0, 0))
	{
		*x = 0;
		*y = 0;
		return 1;
	}

	return 0;
}

int vrp_get_device_distance_to_pickup_location(vrp_server_t* server, size_t device_index, size_t* pickup_location_index)
{
	assert(device_index < VRP_MAX_DEVICE_COUNT);

	if ((server->device_table[device_index].type != VRP_DEVICE_TYPE_GOPIGO) ||
		(server->device_table[device_index].x == VRP_COORDINATE_UNDEFINED) ||
		(server->device_table[device_index].y == VRP_COORDINATE_UNDEFINED))
		return INT_MAX;

	size_t nearest_pickup_location = (size_t)~0;
	int distance_to_nearest_pickup_location = INT_MAX;

	for (size_t i = 0; i != server->pickup_location_count; ++i)
	{
		vrp_initialize_path_finder(server, server->device_table[device_index].x, server->device_table[device_index].y, VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE, vrp_calculate_device_movement_priority(server, server->device_table[device_index].id));
		if (vrp_create_path_map(server, server->pickup_location_table[i].x, server->pickup_location_table[i].y, 0) &&
			(server->map_state[(int)server->device_table[device_index].y * (int)server->map_width + (int)server->device_table[device_index].x].path_finder_state > -1) &&
			(server->map_state[(int)server->device_table[device_index].y * (int)server->map_width + (int)server->device_table[device_index].x].path_finder_state < distance_to_nearest_pickup_location))
		{
			nearest_pickup_location = i;
			distance_to_nearest_pickup_location = server->map_state[(int)server->device_table[device_index].y * (int)server->map_width + (int)server->device_table[device_index].x].path_finder_state;
			assert(distance_to_nearest_pickup_location > -1 && distance_to_nearest_pickup_location != INT_MAX);
		}
	}

	if (pickup_location_index)
		*pickup_location_index = nearest_pickup_location;
	return distance_to_nearest_pickup_location;
}

size_t vrp_get_device_index_for_free_transport(vrp_server_t* server)
{
	size_t nearest_device_index = (size_t)~0;
	int nearest_device_distance = INT_MAX;

	for (size_t i = 0; i != server->device_count; ++i)
	{
		if (vrp_get_order_index_of_transport_device(server, i) == (size_t)~0)
		{
			int distance = vrp_get_device_distance_to_pickup_location(server, i, 0);
			if (distance < nearest_device_distance)
			{
				nearest_device_distance = distance;
				nearest_device_index = i;
			}
		}
	}

	return nearest_device_index;
}

size_t vrp_get_nearest_pickup_location_index_for_device(vrp_server_t* server, size_t device_index)
{
	if ((server->device_table[device_index].type != VRP_DEVICE_TYPE_GOPIGO) ||
		(server->device_table[device_index].x == VRP_COORDINATE_UNDEFINED) ||
		(server->device_table[device_index].y == VRP_COORDINATE_UNDEFINED))
		return (size_t)~0;

	size_t pickup_location_index;
	size_t nearest_pickup_location_index = (size_t)~0;
	int nearest_pickup_location_distance = INT_MAX;

	for (size_t i = 0; i != server->pickup_location_count; ++i)
		if (vrp_get_device_distance_to_pickup_location(server, device_index, &pickup_location_index) < nearest_pickup_location_distance)
			nearest_pickup_location_index = pickup_location_index;

	return nearest_pickup_location_index;
}

size_t vrp_is_device_on_pickup_location(vrp_server_t* server, size_t device_index)
{
	if ((server->device_table[device_index].type != VRP_DEVICE_TYPE_GOPIGO) ||
		(server->device_table[device_index].x == VRP_COORDINATE_UNDEFINED) ||
		(server->device_table[device_index].y == VRP_COORDINATE_UNDEFINED))
		return (size_t)~0;

	for (size_t i = 0; i != server->pickup_location_count; ++i)
		if ((server->pickup_location_table[i].x == server->device_table[device_index].x) && (server->pickup_location_table[i].y == server->device_table[device_index].y))
			return i;

	return (size_t)~0;
}

size_t vrp_get_pickup_location_index_by_coordinate(const vrp_server_t * server, int x, int y)
{
	__assume((x < VRP_MAX_MAP_WIDTH || x == VRP_COORDINATE_UNDEFINED) && (y < VRP_MAX_MAP_WIDTH || y == VRP_COORDINATE_UNDEFINED));

	for (size_t i = 0; i != server->pickup_location_count; ++i)
		if ((server->pickup_location_table[i].x == (uint8_t)x) && (server->pickup_location_table[i].y == (uint8_t)y))
			return i;

	return (size_t)~0;
}

size_t vrp_get_transport_device_index_by_coordinate(const vrp_server_t* server, int x, int y)
{
	__assume((x < VRP_MAX_MAP_WIDTH || x == VRP_COORDINATE_UNDEFINED) && (y < VRP_MAX_MAP_WIDTH || y == VRP_COORDINATE_UNDEFINED));

	for (size_t i = 0; i != server->device_count; ++i)
		if ((server->device_table[i].type == VRP_DEVICE_TYPE_GOPIGO) && (server->device_table[i].x == (uint8_t)x) && (server->device_table[i].y == (uint8_t)y))
			return i;

	return (size_t)~0;
}

int vrp_get_any_transport_device_from_pickup_location(vrp_server_t* server, size_t* device_index, size_t* pickup_location_index)
{
	for (size_t i = 0; i != server->device_count; ++i)
		if (server->device_table[i].type == VRP_DEVICE_TYPE_GOPIGO)
		{
			size_t pickup_location = vrp_is_device_on_pickup_location(server, i);
			if ((pickup_location != (size_t)~0))
			{
				*device_index = i;
				*pickup_location_index = pickup_location;
				return 1;
			}
		}
	return 0;
}

int vrp_is_device_likely_to_colide(const vrp_server_t* server, size_t device_this_index, size_t device_other_index, size_t time_step_count, int do_not_care_if_this_device_has_higher_priority, uint8_t* x, uint8_t* y)
{
	if (!server->device_table[device_this_index].immediate_path_length || (do_not_care_if_this_device_has_higher_priority && (vrp_calculate_device_movement_priority(server, server->device_table[device_this_index].id) > vrp_calculate_device_movement_priority(server, server->device_table[device_other_index].id))))
		return 0;

	for (size_t n = (time_step_count < server->device_table[device_this_index].immediate_path_length) ? time_step_count : server->device_table[device_this_index].immediate_path_length, i = 0; i != n; ++i)
	{
		if ((server->device_table[device_this_index].immediate_path[i].x == server->device_table[device_other_index].x) && (server->device_table[device_this_index].immediate_path[i].y == server->device_table[device_other_index].y))
		{
			if (x && y)
			{
				*x = server->device_table[device_other_index].x;
				*y = server->device_table[device_other_index].y;
			}
			return 1;
		}
		for (size_t l = (n < server->device_table[device_other_index].immediate_path_length) ? n : server->device_table[device_other_index].immediate_path_length, j = 0; j != l; ++j)
			if ((server->device_table[device_this_index].immediate_path[i].x == server->device_table[device_other_index].immediate_path[j].x) || (server->device_table[device_this_index].immediate_path[i].y == server->device_table[device_other_index].immediate_path[j].y))
			{
				if (x && y)
				{
					*x = server->device_table[device_other_index].immediate_path[j].x;
					*y = server->device_table[device_other_index].immediate_path[j].y;
				}
				return 1;
			}
	}

	return 0;
}

size_t vrp_get_likely_to_colide_device_index(const vrp_server_t* server, size_t device_this_index, int do_not_care_if_this_device_has_higher_priority, uint8_t* x, uint8_t* y)
{
	for (size_t i = 0; i != server->device_count; ++i)
		if ((i != device_this_index) && vrp_is_device_likely_to_colide(server, device_this_index, i, VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE, do_not_care_if_this_device_has_higher_priority, x, y))
			return i;

	return (size_t)~0;
}

int vrp_is_coordinate_in_any_immediate_path(const vrp_server_t* server, size_t excluded_device_index, size_t time_steps, uint8_t x, uint8_t y)
{
	for (size_t i = 0; i != server->device_count; ++i)
		if (i != excluded_device_index)
		{
			if ((server->device_table[i].x == x) && (server->device_table[i].y == y))
				return 1;
			for (size_t n = (time_steps < server->device_table[i].immediate_path_length) ? time_steps : server->device_table[i].immediate_path_length, j = 0; j != n; ++j)
				if ((server->device_table[i].immediate_path[j].x == x) && (server->device_table[i].immediate_path[j].y == y))
					return 1;
		}
	return 0;
}

int vrp_calculate_direction_to_make_way_for_other_devices(vrp_server_t* server, size_t device_index)
{
	__assume((server->map_width < VRP_MAX_MAP_WIDTH || server->map_height <= VRP_MAX_MAP_HEIGHT));

	int w = server->map_width;
	int h = server->map_height;
	int x = server->device_table[device_index].x;
	int y = server->device_table[device_index].y;

	if (x >= w || y >= h)
		return VRP_DIRECTION_UNDEFINED;

	vrp_initialize_path_finder(server, x, y, VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE, INT_MAX);

	if ((x + 1 < w) && (server->map_state[y * w + (x + 1)].path_finder_state == VRP_PATH_OPEN) && !vrp_is_coordinate_in_any_immediate_path(server, device_index, VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE, x + 1, y))
	{
		server->device_table[device_index].immediate_path[0].x = x + 1;
		server->device_table[device_index].immediate_path[0].y = y;
		server->device_table[device_index].immediate_path_length = 1;
		return VRP_DIRECTION_RIGHT;
	}
	else if (x && (server->map_state[y * w + (x - 1)].path_finder_state == VRP_PATH_OPEN) && !vrp_is_coordinate_in_any_immediate_path(server, device_index, VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE, x - 1, y))
	{
		server->device_table[device_index].immediate_path[0].x = x - 1;
		server->device_table[device_index].immediate_path[0].y = y;
		server->device_table[device_index].immediate_path_length = 1;
		return VRP_DIRECTION_LEFT;
	}
	else if (y && (server->map_state[(y - 1) * w + x].path_finder_state == VRP_PATH_OPEN) && !vrp_is_coordinate_in_any_immediate_path(server, device_index, VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE, x, y - 1))
	{
		server->device_table[device_index].immediate_path[0].x = x;
		server->device_table[device_index].immediate_path[0].y = y - 1;
		server->device_table[device_index].immediate_path_length = 1;
		return VRP_DIRECTION_DOWN;
	}
	else if ((y + 1 < h) && (server->map_state[(y + 1) * w + x].path_finder_state == VRP_PATH_OPEN) && !vrp_is_coordinate_in_any_immediate_path(server, device_index, VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE, x, y + 1))
	{
		server->device_table[device_index].immediate_path[0].x = x;
		server->device_table[device_index].immediate_path[0].y = y + 1;
		server->device_table[device_index].immediate_path_length = 1;
		return VRP_DIRECTION_UP;
	}

	size_t nearest_device_index = (size_t)~0;
	int nearest_device_distance2 = INT_MAX;
	int this_device_priority = vrp_calculate_device_movement_priority(server, server->device_table[device_index].id);
	for (size_t i = 0; i != server->device_count; ++i)
		if ((i != device_index) && (this_device_priority < vrp_calculate_device_movement_priority(server, server->device_table[i].id)))
		{
			int i_device_distance2 =
				((server->device_table[i].x - server->device_table[device_index].x) * (server->device_table[i].x - server->device_table[device_index].x)) +
				((server->device_table[i].y - server->device_table[device_index].y) * (server->device_table[i].y - server->device_table[device_index].y));
			if (i_device_distance2 < nearest_device_distance2)
			{
				nearest_device_distance2 = i_device_distance2;
				nearest_device_index = i;
			}
		}
	if ((nearest_device_index != (size_t)~0))
	{
		int left_open = (int)(server->device_table[device_index].x && (server->map_state[y * w + (x - 1)].path_finder_state == VRP_PATH_OPEN));
		int right_open = (int)((server->device_table[device_index].x + 1 < w) && (server->map_state[y * w + (x + 1)].path_finder_state == VRP_PATH_OPEN));
		int down_open = (int)(server->device_table[device_index].y && (server->map_state[(y - 1) * w + x].path_finder_state == VRP_PATH_OPEN));
		int up_open = (int)((server->device_table[device_index].y + 1 < h) && (server->map_state[(y + 1) * w + x].path_finder_state == VRP_PATH_OPEN));
		
		if (vrp_create_path_map(server, server->device_table[nearest_device_index].x, server->device_table[nearest_device_index].y, 1) &&
			(server->map_state[(int)server->device_table[device_index].y * w + (int)server->device_table[device_index].x].path_finder_state < VRP_DEFAULT_DEVICE_ON_PAHT_IGNORE_DISTANCE))
		{
			if (server->device_table[device_index].x < server->device_table[nearest_device_index].x)
			{
				if (left_open)
				{
					server->device_table[device_index].immediate_path[0].x = x - 1;
					server->device_table[device_index].immediate_path[0].y = y;
					server->device_table[device_index].immediate_path_length = 1;
					return VRP_DIRECTION_LEFT;
				}
			}
			else if (server->device_table[device_index].x > server->device_table[nearest_device_index].x)
			{
				if (right_open)
				{
					server->device_table[device_index].immediate_path[0].x = x + 1;
					server->device_table[device_index].immediate_path[0].y = y;
					server->device_table[device_index].immediate_path_length = 1;
					return VRP_DIRECTION_RIGHT;
				}
			}
			if (server->device_table[device_index].y < server->device_table[nearest_device_index].y)
			{
				if (down_open)
				{
					server->device_table[device_index].immediate_path[0].x = x;
					server->device_table[device_index].immediate_path[0].y = y - 1;
					server->device_table[device_index].immediate_path_length = 1;
					return VRP_DIRECTION_DOWN;
				}
			}
			else if (server->device_table[device_index].y < server->device_table[nearest_device_index].y)
			{
				if (up_open)
				{
					server->device_table[device_index].immediate_path[0].x = x;
					server->device_table[device_index].immediate_path[0].y = y + 1;
					server->device_table[device_index].immediate_path_length = 1;
					return VRP_DIRECTION_UP;
				}
			}
		}

		int center_x = w / 2;
		int center_y = h / 2;
		if (server->device_table[device_index].x <= center_x && left_open)
		{
			server->device_table[device_index].immediate_path[0].x = x - 1;
			server->device_table[device_index].immediate_path[0].y = y;
			server->device_table[device_index].immediate_path_length = 1;
			return VRP_DIRECTION_LEFT;
		}
		else if (server->device_table[device_index].x > center_x && right_open)
		{
			server->device_table[device_index].immediate_path[0].x = x + 1;
			server->device_table[device_index].immediate_path[0].y = y;
			server->device_table[device_index].immediate_path_length = 1;
			return VRP_DIRECTION_RIGHT;
		}
		else if (server->device_table[device_index].y <= center_y && down_open)
		{
			server->device_table[device_index].immediate_path[0].x = x;
			server->device_table[device_index].immediate_path[0].y = y - 1;
			server->device_table[device_index].immediate_path_length = 1;
			return VRP_DIRECTION_DOWN;
		}
		else if (server->device_table[device_index].y > center_y && up_open)
		{
			server->device_table[device_index].immediate_path[0].x = x;
			server->device_table[device_index].immediate_path[0].y = y + 1;
			server->device_table[device_index].immediate_path_length = 1;
			return VRP_DIRECTION_UP;
		}
	}
	
	return VRP_DIRECTION_UNDEFINED;
}
