/*
	VarastoRobo master server version 1.1.1 2019-12-16 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#include <Winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
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

static uint32_t read_idle_status_query_delay_from_json(const jsonpl_value_t* configuration)
{
	uint32_t delay = 60000;
	jsonpl_value_t* idle_status_query_ms_delay = find_child_by_name(configuration, "idle_status_query_ms_delay");
	if (idle_status_query_ms_delay && idle_status_query_ms_delay->type == JSONPL_TYPE_NUMBER)
		delay = (uint32_t)idle_status_query_ms_delay->number_value;
	return delay;
}

static uint32_t read_acceptable_product_mask_from_json(const jsonpl_value_t* configuration)
{
	uint32_t mask = 0x0000001F;
	jsonpl_value_t* acceptable_product_mask = find_child_by_name(configuration, "acceptable_product_mask");
	if (acceptable_product_mask && acceptable_product_mask->type == JSONPL_TYPE_NUMBER)
		mask = (uint32_t)acceptable_product_mask->number_value;
	return mask;
}

static uint32_t read_product_pickup_status_query_delay_from_json(const jsonpl_value_t* configuration)
{
	uint32_t delay = 1000;
	jsonpl_value_t* product_pickup_status_query_ms_delay = find_child_by_name(configuration, "product_pickup_status_query_ms_delay");
	if (product_pickup_status_query_ms_delay && product_pickup_status_query_ms_delay->type == JSONPL_TYPE_NUMBER)
		delay = (uint32_t)product_pickup_status_query_ms_delay->number_value;
	return delay;
}

static uint32_t read_command_timeout_from_json(const jsonpl_value_t* configuration)
{
	uint32_t time = 60000;
	jsonpl_value_t* command_execution_ms_timeout = find_child_by_name(configuration, "command_execution_ms_timeout");
	if (command_execution_ms_timeout && command_execution_ms_timeout->type == JSONPL_TYPE_NUMBER)
		time = (uint32_t)command_execution_ms_timeout->number_value;
	return time;
}

static uint32_t read_broadcast_ip_address_from_json(const jsonpl_value_t* configuration)
{
	uint32_t ip = INADDR_BROADCAST;
	jsonpl_value_t* broadcast_ip_address = find_child_by_name(configuration, "broadcast_ip_address");
	if (broadcast_ip_address && broadcast_ip_address->type == JSONPL_TYPE_STRING)
		ip = (uint32_t)inet_addr(broadcast_ip_address->string.value);
	return ip;
}

static uint32_t read_server_ip_address_from_json(const jsonpl_value_t* configuration)
{
	uint32_t ip = INADDR_ANY;
	jsonpl_value_t* server_ip_address = find_child_by_name(configuration, "server_ip_address");
	if (server_ip_address && server_ip_address->type == JSONPL_TYPE_STRING)
		ip = (uint32_t)inet_addr(server_ip_address->string.value);
	return ip;
}

static uint32_t read_network_prefix_length_from_json(const jsonpl_value_t* configuration)
{
	uint32_t network_prefix_length = 0xFFFFFFFF;
	jsonpl_value_t* json_network_prefix_length = find_child_by_name(configuration, "network_prefix_length");
	if (json_network_prefix_length && json_network_prefix_length->type == JSONPL_TYPE_NUMBER)
		network_prefix_length = (uint32_t)json_network_prefix_length->number_value;
	return network_prefix_length;
}

static uint32_t read_carried_product_confidence_max_from_json(const jsonpl_value_t* configuration)
{
	uint32_t max = 15;
	jsonpl_value_t* json_max = find_child_by_name(configuration, "carried_product_confidence_max");
	if (json_max && json_max->type == JSONPL_TYPE_NUMBER)
		max = (uint32_t)json_max->number_value;
	return max;
}

static uint32_t read_carried_product_confidence_pickup_limit_from_json(const jsonpl_value_t* configuration)
{
	uint32_t limit = 7;
	jsonpl_value_t* json_limit = find_child_by_name(configuration, "carried_product_confidence_pickup_limit");
	if (json_limit && json_limit->type == JSONPL_TYPE_NUMBER)
		limit = (uint32_t)json_limit->number_value;
	return limit;
}

static uint32_t read_block_expiration_time_from_json(const jsonpl_value_t* configuration)
{
	uint32_t time = 300000;
	jsonpl_value_t* json_time = find_child_by_name(configuration, "block_expiration_ms_time");
	if (json_time && json_time->type == JSONPL_TYPE_NUMBER)
		time = (uint32_t)json_time->number_value;
	return time;
}

static uint32_t read_wait_for_path_ms_timeout_from_json(const jsonpl_value_t* configuration)
{
	uint32_t time = 60000;
	jsonpl_value_t* json_time = find_child_by_name(configuration, "wait_for_path_ms_timeout");
	if (json_time && json_time->type == JSONPL_TYPE_NUMBER)
		time = (uint32_t)json_time->number_value;
	return time;
}

static uint32_t read_product_not_available_ms_timeout_from_json(const jsonpl_value_t* configuration)
{
	uint32_t time = 60000;
	jsonpl_value_t* json_time = find_child_by_name(configuration, "product_not_available_ms_timeout");
	if (json_time && json_time->type == JSONPL_TYPE_NUMBER)
		time = (uint32_t)json_time->number_value;
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

static uint8_t read_trust_lost_device_from_json(const jsonpl_value_t* configuration)
{
	uint8_t value = 1;
	jsonpl_value_t* json_value = find_child_by_name(configuration, "trust_lost_devices");
	if (json_value && json_value->type == JSONPL_TYPE_BOOLEAN)
		value = (uint8_t)json_value->boolean_value;
	return value;
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

uint8_t read_debug_no_broadcast_from_json(const jsonpl_value_t* configuration)
{
	uint8_t debug_no_broadcast = 0;
	jsonpl_value_t* json_debug_no_broadcast = find_child_by_name(configuration, "debug_no_broadcast");
	if (json_debug_no_broadcast && json_debug_no_broadcast->type == JSONPL_TYPE_BOOLEAN)
		debug_no_broadcast = (uint8_t)json_debug_no_broadcast->boolean_value;
	return debug_no_broadcast;
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

static size_t read_idle_location_list_from_json(const jsonpl_value_t* configuration, uint8_t** location_list)
{
	jsonpl_value_t* list = find_child_by_name(configuration, "idle_location_list");
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
	*location_list = table;
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
	uint8_t* table = (uint8_t*)malloc(n ? (n * 4) : 1);
	if (!table)
		return 0;
	for (size_t c = 0, i = 0; c != n; ++i)
		if (list->array.table[i]->type == JSONPL_TYPE_OBJECT)
		{
			jsonpl_value_t* component = find_child_by_name(list->array.table[i], "id");
			if (component && component->type == JSONPL_TYPE_NUMBER)
			{
				table[c * 4 + 0] = (uint8_t)component->number_value;
				component = find_child_by_name(list->array.table[i], "x");
				if (component && component->type == JSONPL_TYPE_NUMBER)
					table[c * 4 + 1] = (uint8_t)component->number_value;
				else
					table[c * 4 + 1] = 0xFF;
				component = find_child_by_name(list->array.table[i], "y");
				if (component && component->type == JSONPL_TYPE_NUMBER)
					table[c * 4 + 2] = (uint8_t)component->number_value;
				else
					table[c * 4 + 2] = 0xFF;
				component = find_child_by_name(list->array.table[i], "direction");
				if (component && component->type == JSONPL_TYPE_NUMBER)
					table[c * 4 + 3] = (uint8_t)component->number_value;
				else
					table[c * 4 + 3] = 0xFF;
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
	uint32_t on_wire_broadcast_ip_address = read_broadcast_ip_address_from_json(json);
	uint32_t on_wire_server_ip_address = read_server_ip_address_from_json(json);
	uint32_t network_prefix_length = read_network_prefix_length_from_json(json);
	uint32_t idle_status_query_delay = read_idle_status_query_delay_from_json(json);
	uint32_t product_pickup_status_query_delay = read_product_pickup_status_query_delay_from_json(json);
	uint32_t acceptable_product_mask = read_acceptable_product_mask_from_json(json);
	uint32_t carried_product_confidence_max = read_carried_product_confidence_max_from_json(json);
	uint32_t carried_product_confidence_pickup_limit = read_carried_product_confidence_pickup_limit_from_json(json);
	uint32_t block_expiration_time = read_block_expiration_time_from_json(json);
	uint32_t wait_for_path_ms_timeout = read_wait_for_path_ms_timeout_from_json(json);
	uint32_t product_not_available_ms_timeout = read_product_not_available_ms_timeout_from_json(json);
	uint8_t trust_lost_device = read_trust_lost_device_from_json(json);
	uint8_t system_status = read_system_status_from_json(json);
	uint8_t master_id = read_master_id_from_json(json);
	uint8_t min_temporal_id = read_min_temporal_id_from_json(json);
	uint8_t max_temporal_id = read_max_temporal_id_from_json(json);
	uint8_t debug_no_emergency_listen = read_debug_no_emergency_listen_from_json(json);
	uint8_t debug_no_broadcast = read_debug_no_broadcast_from_json(json);


	uint8_t map_height;
	uint8_t map_width;
	uint8_t* map = read_static_map_from_json(json, &map_height, &map_width);
	if (!map)
	{
		vrp_free_file_data(json);
		return ERROR_INVALID_DATA;
	}

	
	uint8_t* idle_location_table = 0;
	size_t idle_location_count = read_idle_location_list_from_json(json, &idle_location_table);

	uint8_t* block_table = 0;
	size_t block_count = read_block_list_from_json(json, &block_table);

	uint8_t* pickup_location_table = 0;
	size_t pickup_location_count = read_pickup_location_list_from_json(json, &pickup_location_table);

	vrp_free_file_data(json);

	vrp_configuration_t* configuration = (vrp_configuration_t*)malloc(sizeof(vrp_configuration_t));
	if (!configuration)
	{
		if (idle_location_table)
			free(idle_location_table);
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
	configuration->on_wire_broadcast_ip_address = on_wire_broadcast_ip_address;
	configuration->on_wire_server_ip_address = on_wire_server_ip_address;
	configuration->network_prefix_length = network_prefix_length;
	configuration->idle_status_query_delay = idle_status_query_delay;
	configuration->product_pickup_status_query_delay = product_pickup_status_query_delay;
	configuration->acceptable_product_mask = acceptable_product_mask;
	configuration->carried_product_confidence_max = carried_product_confidence_max;
	configuration->carried_product_confidence_pickup_limit = carried_product_confidence_pickup_limit;
	configuration->block_expiration_time = block_expiration_time;
	configuration->wait_for_path_ms_timeout = wait_for_path_ms_timeout;
	configuration->product_not_available_ms_timeout = product_not_available_ms_timeout;
	configuration->master_id = master_id;
	configuration->trust_lost_device = trust_lost_device;
	configuration->system_status = system_status;
	configuration->min_temporal_id = min_temporal_id;
	configuration->max_temporal_id = max_temporal_id;
	configuration->map_height = map_height;
	configuration->map_width = map_width;
	configuration->debug_no_emergency_listen = debug_no_emergency_listen;
	configuration->debug_no_broadcast = debug_no_broadcast;
	configuration->map = map;
	configuration->block_count = block_count;
	configuration->block_table = block_table;
	configuration->pickup_location_count = pickup_location_count;
	configuration->pickup_location_table = pickup_location_table;
	configuration->idle_location_table = idle_location_table;
	configuration->idle_location_count = idle_location_count;

	*master_configuration = configuration;
	return 0;
}

void vrp_free_master_configuration(vrp_configuration_t* master_configuration)
{
	if (master_configuration->idle_location_table)
		free(master_configuration->idle_location_table);
	if (master_configuration->pickup_location_table)
		free(master_configuration->pickup_location_table);
	if (master_configuration->block_table)
		free(master_configuration->block_table);
	free(master_configuration->map);
	free(master_configuration);
}