/*
	VarastoRobo master server version 0.3.0 2019-11-17 by Santtu Nyman.
*/

#include "vrp_configuration.h"

static jsonpl_value_t* find_child_by_name(jsonpl_value_t* value_tree, const char* child_name)
{
	if (value_tree->type == JSONPL_TYPE_OBJECT)
		for (size_t i = 0; i != value_tree->object.value_count; ++i)
			if (!strcmp(child_name, value_tree->object.table[i].name))
				return value_tree->object.table[i].value;
	return 0;
}

static uint8_t* read_static_map_from_json(const jsonpl_value_t* configuration, uint8_t* map_height, uint8_t* map_width)
{
	jsonpl_value_t* static_map = find_child_by_name(configuration, "static_map");
	if (!static_map || static_map->type != JSONPL_TYPE_ARRAY || !static_map->array.value_count || static_map->array.table[0]->type != JSONPL_TYPE_ARRAY || !static_map->array.table[0]->array.value_count)
		return 0;
	uint8_t height = (uint8_t)static_map->array.value_count;
	uint8_t width = (uint8_t)static_map->array.table[0]->array.value_count;
	for (uint8_t y = 1; y != height; ++y)
		if (static_map->array.table[y]->type != JSONPL_TYPE_ARRAY || static_map->array.table[y]->array.value_count != (size_t)width)
			return 0;
	uint8_t* map = (uint8_t*)malloc((((size_t)height * (size_t)width) + 7) / 8);
	if (!map)
		return 0;
	memset(map, 0, (((size_t)height * (size_t)width) + 7) / 8);
	for (uint8_t y = 0; y != height; ++y)
		for (uint8_t x = 0; x != width; ++x)
		{
			int in_memory_bit_index = ((int)y * (int)width) + (int)x;
			int byte_index = in_memory_bit_index / 8;
			int bit_index = in_memory_bit_index % 8;
			map[byte_index] |= (static_map->array.table[(height - 1) - y]->array.table[x]->type == JSONPL_TYPE_BOOLEAN && static_map->array.table[(height - 1) - y]->array.table[x]->boolean_value) ? (uint8_t)(1 << bit_index) : 0;
		}
	*map_height = height;
	*map_width = width;
	return map;
}

static uint32_t read_broadcast_delay_from_json(const jsonpl_value_t* configuration)
{
	uint32_t delay = 5000;
	jsonpl_value_t* broadcast_ms_delay = find_child_by_name(configuration, "broadcast_ms_delay");
	if (broadcast_ms_delay && broadcast_ms_delay->type == JSONPL_TYPE_NUMBER)
		delay = (uint32_t)broadcast_ms_delay->number_value;
	return delay;
}

static uint32_t read_io_timeout_from_json(const jsonpl_value_t* configuration)
{
	uint32_t time = 60000;
	jsonpl_value_t* system_io_ms_timeout = find_child_by_name(configuration, "system_io_ms_timeout");
	if (system_io_ms_timeout && system_io_ms_timeout->type == JSONPL_TYPE_NUMBER)
		time = (uint32_t)system_io_ms_timeout->number_value;
	return time;
}

static uint32_t read_command_timeout_from_json(const jsonpl_value_t* configuration)
{
	uint32_t time = 60000;
	jsonpl_value_t* command_execution_ms_timeout = find_child_by_name(configuration, "command_execution_ms_timeout");
	if (command_execution_ms_timeout && command_execution_ms_timeout->type == JSONPL_TYPE_NUMBER)
		time = (uint32_t)command_execution_ms_timeout->number_value;
	return time;
}

static uint8_t read_master_id_from_json(const jsonpl_value_t* configuration)
{
	uint8_t id = 42;
	jsonpl_value_t* master_id = find_child_by_name(configuration, "master_device_id");
	if (master_id && master_id->type == JSONPL_TYPE_NUMBER)
		id = (uint8_t)master_id->number_value;
	return id;
}

static uint8_t read_system_status_from_json(const jsonpl_value_t* configuration)
{
	uint8_t status = 1;
	jsonpl_value_t* initial_system_status = find_child_by_name(configuration, "initial_system_status");
	if (initial_system_status && initial_system_status->type == JSONPL_TYPE_NUMBER)
		status = (uint8_t)initial_system_status->number_value;
	return status;
}

static uint8_t read_min_temporal_id_from_json(const jsonpl_value_t* configuration)
{
	uint8_t id_number = 0xFF;
	jsonpl_value_t* json_id_number = find_child_by_name(configuration, "min_temporal_id");
	if (json_id_number && json_id_number->type == JSONPL_TYPE_NUMBER)
		id_number = (uint8_t)json_id_number->number_value;
	return id_number;
}

static uint8_t read_max_temporal_id_from_json(const jsonpl_value_t* configuration)
{
	uint8_t id_number = 0xFF;
	jsonpl_value_t* json_id_number = find_child_by_name(configuration, "max_temporal_id");
	if (json_id_number && json_id_number->type == JSONPL_TYPE_NUMBER)
		id_number = (uint8_t)json_id_number->number_value;
	return id_number;
}

uint8_t read_debug_no_emergency_listen_from_json(const jsonpl_value_t* configuration)
{
	uint8_t debug_no_emergency_listen = 0;
	jsonpl_value_t* json_debug_no_emergency_listen = find_child_by_name(configuration, "debug_no_emergency_listen");
	if (json_debug_no_emergency_listen && json_debug_no_emergency_listen->type == JSONPL_TYPE_BOOLEAN)
		debug_no_emergency_listen = (uint8_t)json_debug_no_emergency_listen->boolean_value;
	return debug_no_emergency_listen;
}

static size_t read_block_list_from_json(const jsonpl_value_t* configuration, uint8_t** block_list)
{
	jsonpl_value_t* list = find_child_by_name(configuration, "initial_block_list");
	if (!list || list->type != JSONPL_TYPE_ARRAY)
		return 0;
	size_t n = 0;
	for (size_t i = 0; i != list->array.value_count; ++i)
		if (list->array.table[i]->type == JSONPL_TYPE_OBJECT)
			++n;
	uint8_t* table = (uint8_t*)malloc(n ? (n * 2) : 1);
	if (!table)
		return 0;
	for (size_t c = 0, i = 0; c != n; ++i)
		if (list->array.table[i]->type == JSONPL_TYPE_OBJECT)
		{
			jsonpl_value_t* component = find_child_by_name(list->array.table[i], "x");
			if (component && component->type == JSONPL_TYPE_NUMBER)
				table[c * 2 + 0] = (uint8_t)component->number_value;
			else
				table[c * 2 + 0] = 0xFF;
			component = find_child_by_name(list->array.table[i], "y");
			if (component && component->type == JSONPL_TYPE_NUMBER)
				table[c * 2 + 1] = (uint8_t)component->number_value;
			else
				table[c * 2 + 1] = 0xFF;
			++c;
		}
	*block_list = table;
	return n;
}

static size_t read_pickup_location_list_from_json(const jsonpl_value_t* configuration, uint8_t** pickup_location_list)
{
	jsonpl_value_t* list = find_child_by_name(configuration, "pickup_location_list");
	if (!list || list->type != JSONPL_TYPE_ARRAY)
		return 0;
	size_t n = 0;
	for (size_t i = 0; i != list->array.value_count; ++i)
		if (list->array.table[i]->type == JSONPL_TYPE_OBJECT)
		{
			jsonpl_value_t* component = find_child_by_name(list->array.table[i], "id");
			if (component && component->type == JSONPL_TYPE_NUMBER)
				++n;
		}
	uint8_t* table = (uint8_t*)malloc(n ? (n * 3) : 1);
	if (!table)
		return 0;
	for (size_t c = 0, i = 0; c != n; ++i)
		if (list->array.table[i]->type == JSONPL_TYPE_OBJECT)
		{
			jsonpl_value_t* component = find_child_by_name(list->array.table[i], "id");
			if (component && component->type == JSONPL_TYPE_NUMBER)
			{
				table[c * 3 + 0] = (uint8_t)component->number_value;
				component = find_child_by_name(list->array.table[i], "x");
				if (component && component->type == JSONPL_TYPE_NUMBER)
					table[c * 3 + 1] = (uint8_t)component->number_value;
				else
					table[c * 3 + 1] = 0xFF;
				component = find_child_by_name(list->array.table[i], "y");
				if (component && component->type == JSONPL_TYPE_NUMBER)
					table[c * 3 + 2] = (uint8_t)component->number_value;
				else
					table[c * 3 + 2] = 0xFF;
				++c;
			}
		}
	*pickup_location_list = table;
	return n;
}

DWORD vrp_load_master_configuration(vrp_configuration_t** master_configuration)
{
	jsonpl_value_t* json;
	DWORD error = vrp_load_json_from_program_directory_file(L"master_server.json", &json);
	if (error)
		return error;

	uint32_t broadcast_delay = read_broadcast_delay_from_json(json);
	uint32_t io_timeout = read_io_timeout_from_json(json);
	uint32_t command_timeout = read_command_timeout_from_json(json);
	uint8_t system_status = read_system_status_from_json(json);
	uint8_t master_id = read_master_id_from_json(json);
	uint8_t min_temporal_id = read_min_temporal_id_from_json(json);
	uint8_t max_temporal_id = read_max_temporal_id_from_json(json);
	uint8_t debug_no_emergency_listen = read_debug_no_emergency_listen_from_json(json);

	uint8_t map_height;
	uint8_t map_width;
	uint8_t* map = read_static_map_from_json(json, &map_height, &map_width);
	if (!map)
	{
		vrp_free_file_data(json);
		return ERROR_INVALID_DATA;
	}

	uint8_t* block_table = 0;
	size_t block_count = read_block_list_from_json(json, &block_table);

	uint8_t* pickup_location_table = 0;
	size_t pickup_location_count = read_pickup_location_list_from_json(json, &pickup_location_table);

	vrp_free_file_data(json);

	vrp_configuration_t* configuration = (vrp_configuration_t*)malloc(sizeof(vrp_configuration_t));
	if (!configuration)
	{
		if (pickup_location_table)
			free(pickup_location_table);
		if (block_table)
			free(block_table);
		free(map);
		return ERROR_OUTOFMEMORY;
	}
	
	configuration->broadcast_ms_delay = broadcast_delay;
	configuration->io_ms_timeout = io_timeout;
	configuration->command_ms_timeout = command_timeout;
	configuration->master_id = master_id;
	configuration->system_status = system_status;
	configuration->min_temporal_id = min_temporal_id;
	configuration->max_temporal_id = max_temporal_id;
	configuration->map_height = map_height;
	configuration->map_width = map_width;
	configuration->debug_no_emergency_listen = debug_no_emergency_listen;
	configuration->map = map;
	configuration->block_count = block_count;
	configuration->block_table = block_table;
	configuration->pickup_location_count = pickup_location_count;
	configuration->pickup_location_table = pickup_location_table;

	*master_configuration = configuration;
	return 0;
}

void vrp_free_master_configuration(vrp_configuration_t* master_configuration)
{
	if (master_configuration->pickup_location_table)
		free(master_configuration->pickup_location_table);
	if (master_configuration->block_table)
		free(master_configuration->block_table);
	free(master_configuration->map);
	free(master_configuration);
}