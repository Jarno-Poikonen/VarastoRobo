/*
	VarastoRobo master server version 0.4.3 2019-11-20 by Santtu Nyman.
*/

#include "vrp_master_server_base.h"
#include "vrp_path_logic.h"

int vrp_process_gopigo_idle(vrp_server_t* server, size_t i);

int vrp_process_ur5_idle(vrp_server_t* server, size_t i);

int vrp_process_wfm(vrp_server_t* server, size_t i, size_t total_message_size);

int vrp_process_client_command(vrp_server_t* server, size_t i, size_t total_message_size);

int vrp_process_ncm(vrp_server_t* server, size_t i, size_t total_message_size);

int vrp_process_device(vrp_server_t* server, size_t i);

void vrp_process_order(vrp_server_t* server, size_t i);

DWORD vrp_run_setup(vrp_server_t** server);

int vrp_process_gopigo_idle(vrp_server_t* server, size_t i)
{
	assert((i < VRP_MAX_DEVICE_COUNT) && (server->device_table[i].type == VRP_DEVICE_TYPE_GOPIGO) && (server->device_table[i].connection_state == VRP_CONNECTION_IDLE));

	size_t product_order_index = vrp_get_order_index_of_transport_device(server, i);
	size_t pickup_location_index = (size_t)~0;

	if ((server->device_table[i].x != VRP_COORDINATE_UNDEFINED) && (server->device_table[i].y != VRP_COORDINATE_UNDEFINED))
	{
		if (product_order_index != (size_t)~0)
		{
			if (server->product_order_table[product_order_index].order_status == VRP_ORDER_IN_STORAGE)
			{
				pickup_location_index = vrp_get_nearest_pickup_location_index_for_device(server, i);
				if (pickup_location_index != (size_t)~0)
				{
					server->device_table[i].destination_x = server->pickup_location_table[pickup_location_index].x;
					server->device_table[i].destination_y = server->pickup_location_table[pickup_location_index].y;
				}
				else
				{
					server->device_table[i].destination_x = VRP_COORDINATE_UNDEFINED;
					server->device_table[i].destination_y = VRP_COORDINATE_UNDEFINED;
				}
			}
			else if (server->product_order_table[product_order_index].order_status == VRP_ORDER_ON_MOVE)
			{
				server->device_table[i].destination_x = server->product_order_table[product_order_index].destination_x;
				server->device_table[i].destination_y = server->product_order_table[product_order_index].destination_y;
			}
			else if (server->product_order_table[product_order_index].order_status == VRP_ORDER_FINAL_WAITING)
			{
				server->device_table[i].destination_x = server->device_table[i].x;
				server->device_table[i].destination_y = server->device_table[i].y;
			}
			else
				vrp_get_idle_location_for_device(server, i, &server->device_table[i].destination_x, &server->device_table[i].destination_y);
		}

		if ((server->device_table[i].destination_x == VRP_COORDINATE_UNDEFINED) || (server->device_table[i].destination_y == VRP_COORDINATE_UNDEFINED))
			vrp_get_idle_location_for_device(server, i, &server->device_table[i].destination_x, &server->device_table[i].destination_y);
		
		if ((server->device_table[i].destination_x != VRP_COORDINATE_UNDEFINED) && (server->device_table[i].destination_y != VRP_COORDINATE_UNDEFINED))
		{
			uint8_t direction = (uint8_t)vrp_calculate_direction_to_target(server, server->device_table[i].x, server->device_table[i].y, server->device_table[i].destination_x, server->device_table[i].destination_y);
			if (direction != VRP_DIRECTION_UNDEFINED)
			{
				vrp_coordinate_from_direction(server->device_table[i].x, server->device_table[i].y, direction, &server->device_table[i].move_to_x, &server->device_table[i].move_to_y);

				server->device_table[i].command = VRP_MESSAGE_MCM;
				server->device_table[i].io_memory[0] = server->device_table[i].command;
				server->device_table[i].io_memory[1] = 1;
				server->device_table[i].io_memory[2] = 0;
				server->device_table[i].io_memory[3] = 0;
				server->device_table[i].io_memory[4] = 0;
				server->device_table[i].io_memory[5] = direction;

				if (!vrp_write(server, i, 0, 6))
					return 0;
				server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;
			}
			else if ((server->device_table[i].x == server->device_table[i].destination_x) && (server->device_table[i].y == server->device_table[i].destination_y))
			{
				const int debug_auto_finish_transport = 1;
				if (debug_auto_finish_transport &&
					(product_order_index != (size_t)~0) &&
					(server->device_table[i].x == server->product_order_table[product_order_index].destination_x) &&
					(server->device_table[i].y == server->product_order_table[product_order_index].destination_y))
				{

					vrp_remove_product_order(server, server->product_order_table[product_order_index].order_number);
					server->device_table[i].destination_x = VRP_COORDINATE_UNDEFINED;
					server->device_table[i].destination_y = VRP_COORDINATE_UNDEFINED;
				}

				if ((server->device_table[i].destination_x == VRP_COORDINATE_UNDEFINED) || (server->device_table[i].destination_y == VRP_COORDINATE_UNDEFINED))
				{
					if (!vrp_get_idle_location_for_device(server, i, &server->device_table[i].destination_x, &server->device_table[i].destination_y))
					{
						server->device_table[i].destination_x = VRP_COORDINATE_UNDEFINED;
						server->device_table[i].destination_y = VRP_COORDINATE_UNDEFINED;
					}
				}
			}
			else
			{
				//sprintf(server->log_entry_buffer, "Error routing gopigo %lu from %lu,%lu to %lu,%lu",
				//	server->device_table[i].id, server->device_table[i].x, server->device_table[i].y, server->device_table[i].destination_x, server->device_table[i].destination_y);
				//vrp_write_log_entry(&server->log, server->log_entry_buffer);

				server->device_table[i].destination_x = VRP_COORDINATE_UNDEFINED;
				server->device_table[i].destination_y = VRP_COORDINATE_UNDEFINED;
			}
		}
	}
	else if (server->time - server->device_table[i].last_uppdate_time > 60000)
	{
		server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;

		server->device_table[i].command = VRP_MESSAGE_SQM;
		server->device_table[i].io_memory[0] = server->device_table[i].command;
		server->device_table[i].io_memory[1] = 0;
		server->device_table[i].io_memory[2] = 0;
		server->device_table[i].io_memory[3] = 0;
		server->device_table[i].io_memory[4] = 0;

		if (!vrp_write(server, i, 0, 5))
			return 0;
	}
	else if (server->time - server->device_table[i].connection_begin_time > 120000)
	{
		server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;

		server->device_table[i].command = VRP_MESSAGE_CCM;
		server->device_table[i].io_memory[0] = server->device_table[i].command;
		server->device_table[i].io_memory[1] = 0;
		server->device_table[i].io_memory[2] = 0;
		server->device_table[i].io_memory[3] = 0;
		server->device_table[i].io_memory[4] = 0;

		if (!vrp_write(server, i, 0, 5))
			return 0;
	}

	return 1;
}

int vrp_process_ur5_idle(vrp_server_t* server, size_t i)
{
	assert((i < VRP_MAX_DEVICE_COUNT) && (server->device_table[i].type == VRP_DEVICE_TYPE_UR5) && (server->device_table[i].connection_state == VRP_CONNECTION_IDLE));

	size_t product_order_index = (size_t)~0;
	size_t transport_device_index = (size_t)~0;
	size_t pickup_location_index = (size_t)~0;

	const int debug_send_stupid_mpm = 0;
	if (debug_send_stupid_mpm && (server->device_table[i].connection_state == VRP_CONNECTION_IDLE))
	{
		product_order_index = vrp_get_nonstarted_product_order_index(server);

		if (product_order_index != (size_t)~0)
		{
			transport_device_index = (size_t)~0;
			pickup_location_index = rand() % server->pickup_location_count;

			server->device_table[i].command = VRP_MESSAGE_MPM;
			server->device_table[i].io_memory[0] = server->device_table[i].command;
			server->device_table[i].io_memory[1] = 2;
			server->device_table[i].io_memory[2] = 0;
			server->device_table[i].io_memory[3] = 0;
			server->device_table[i].io_memory[4] = 0;
			server->device_table[i].io_memory[5] = server->product_order_table[product_order_index].product_id;
			server->device_table[i].io_memory[6] = server->pickup_location_table[pickup_location_index].id;

			if (!vrp_write(server, i, 0, 7))
				return 0;
			server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;

			server->device_table[i].move_to_x = server->pickup_location_table[pickup_location_index].x;
			server->device_table[i].move_to_y = server->pickup_location_table[pickup_location_index].y;
		}
	}

	if (server->device_table[i].connection_state == VRP_CONNECTION_IDLE)
		for (size_t j = 0; pickup_location_index == (size_t)~0 && j != server->product_order_count; ++j)
			if ((server->product_order_table[j].order_status == VRP_ORDER_IN_STORAGE) && (server->product_order_table[j].transport_device_id != VRP_ID_UNDEFINED))
			{
				transport_device_index = vrp_get_device_index_by_id(server, server->product_order_table[j].transport_device_id);
				if (transport_device_index != (size_t)~0)
				{
					pickup_location_index = vrp_is_device_on_pickup_location(server, transport_device_index);
					if (pickup_location_index != (size_t)~0)
						product_order_index = j;
				}
			}

	if ((server->device_table[i].connection_state == VRP_CONNECTION_IDLE) && (pickup_location_index != (size_t)~0))
	{
		server->device_table[i].command = VRP_MESSAGE_MPM;
		server->device_table[i].io_memory[0] = server->device_table[i].command;
		server->device_table[i].io_memory[1] = 2;
		server->device_table[i].io_memory[2] = 0;
		server->device_table[i].io_memory[3] = 0;
		server->device_table[i].io_memory[4] = 0;
		server->device_table[i].io_memory[5] = server->product_order_table[product_order_index].product_id;
		server->device_table[i].io_memory[6] = server->pickup_location_table[pickup_location_index].id;

		if (!vrp_write(server, i, 0, 7))
			return 0;
		server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;

		server->device_table[i].move_to_x = server->pickup_location_table[pickup_location_index].x;
		server->device_table[i].move_to_y = server->pickup_location_table[pickup_location_index].y;
	}

	if ((server->device_table[i].connection_state == VRP_CONNECTION_IDLE) && (server->time - server->device_table[i].last_uppdate_time > 60000))
	{
		server->device_table[i].command = VRP_MESSAGE_SQM;
		server->device_table[i].io_memory[0] = server->device_table[i].command;
		server->device_table[i].io_memory[1] = 0;
		server->device_table[i].io_memory[2] = 0;
		server->device_table[i].io_memory[3] = 0;
		server->device_table[i].io_memory[4] = 0;

		if (!vrp_write(server, i, 0, 5))
			return 0;
		server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;
	}

	return 1;
}

int vrp_process_wfm(vrp_server_t* server, size_t i, size_t total_message_size)
{
	if ((total_message_size < 12) || (server->device_table[i].io_memory[0] != VRP_MESSAGE_WFM) || (server->device_table[i].io_memory[5] != server->device_table[i].command))
		return 0;

	uint8_t error_code = server->device_table[i].io_memory[6];
	uint8_t atomic_flag = server->device_table[i].io_memory[7];

	server->device_table[i].last_uppdate_time = server->time;

	if (server->device_table[i].type == VRP_DEVICE_TYPE_GOPIGO)
	{
		server->device_table[i].x = server->device_table[i].io_memory[8];
		server->device_table[i].y = server->device_table[i].io_memory[9];
		server->device_table[i].direction = server->device_table[i].io_memory[10];
		server->device_table[i].move_to_x = VRP_COORDINATE_UNDEFINED;
		server->device_table[i].move_to_y = VRP_COORDINATE_UNDEFINED;

		if (total_message_size >= 13)
		{
			if (server->device_table[i].io_memory[12] != 0xFF)
				server->device_table[i].carried_product_id = server->device_table[i].io_memory[12];

			if (server->device_table[i].carried_product_confidence < 4)
				server->device_table[i].carried_product_confidence++;
		}
		else
		{
			if (server->device_table[i].carried_product_confidence)
				server->device_table[i].carried_product_confidence--;
		}
	}
	else
	{
		server->device_table[i].x = VRP_COORDINATE_UNDEFINED;
		server->device_table[i].y = VRP_COORDINATE_UNDEFINED;
		server->device_table[i].direction = VRP_DIRECTION_UNDEFINED;
	}

	server->device_table[i].state = server->device_table[i].io_memory[11];

	if ((server->device_table[i].command == VRP_MESSAGE_MCM) && !atomic_flag)
	{
		server->status = VRP_STATE_FROZEN;

		sprintf(server->log_entry_buffer, "Device %lu nonatomic failure on move cell message. System can not recover. Emergency stop activated.", server->device_table[i].id);
		vrp_write_log_entry(&server->log, server->log_entry_buffer);
	}

	if (server->device_table[i].command == VRP_MESSAGE_CCM)
	{
		shutdown(server->device_table[i].sock, SD_BOTH);
		sprintf(server->log_entry_buffer, "Device %lu disconnected from address %lu.%lu.%lu.%lu", server->device_table[i].id, ((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
		vrp_write_log_entry(&server->log, server->log_entry_buffer);
		server->device_table[i].connection_state = VRP_CONNECTION_DISCONNECT;
		return 0;
	}
	else if (server->device_table[i].command == VRP_MESSAGE_SCM)
	{
		sprintf(server->log_entry_buffer, "Device %lu connected with type %lu and status %lu from address %lu.%lu.%lu.%lu", server->device_table[i].id, server->device_table[i].type, server->device_table[i].state,
			((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
		vrp_write_log_entry(&server->log, server->log_entry_buffer);

		server->device_table[i].move_to_x = VRP_COORDINATE_UNDEFINED;
		server->device_table[i].move_to_y = VRP_COORDINATE_UNDEFINED;
		/*
		const uint8_t debug_gopigo_destination_x = 7;
		const uint8_t debug_gopigo_destination_y = 1;
		if (server->device_table[i].type == VRP_DEVICE_TYPE_GOPIGO)
		{
			server->device_table[i].destination_x = debug_gopigo_destination_x;
			server->device_table[i].destination_y = debug_gopigo_destination_y;
		}
		*/
	}

	if ((server->device_table[i].type == VRP_DEVICE_TYPE_UR5) && (server->device_table[i].command == VRP_MESSAGE_MPM))
	{
		size_t transport_device_index = (size_t)~0;
		size_t product_order_index = (size_t)~0;
		//size_t pickup_location_index = (size_t)~0;

		if ((server->device_table[i].command == VRP_MESSAGE_MPM) && !atomic_flag)
		{
			server->status = VRP_STATE_FROZEN;

			sprintf(server->log_entry_buffer, "Device %lu nonatomic failure on move product message. System can not recover. Emergency stop activated.", server->device_table[i].id);
			vrp_write_log_entry(&server->log, server->log_entry_buffer);
		}
		else if (server->device_table[i].command == VRP_MESSAGE_MPM)
		{
			transport_device_index = vrp_get_transport_device_index_by_coordinate(server, server->device_table[i].move_to_x, server->device_table[i].move_to_y);
			
			if (transport_device_index != (size_t)~0)
				product_order_index = vrp_get_order_index_of_transport_device(server, transport_device_index);

			if (!error_code)
			{
				server->product_order_table[product_order_index].order_status = VRP_ORDER_ON_MOVE;
				server->product_order_table[product_order_index].transport_device_id = server->device_table[transport_device_index].id;

				sprintf(server->log_entry_buffer, "UR5 %lu failed to execute MPM. error %lu.", server->device_table[i].id, error_code);
				vrp_write_log_entry(&server->log, server->log_entry_buffer);
			}
			else
			{
				sprintf(server->log_entry_buffer, "UR5 %lu moved product %lu to device %lu for order %08X.",
					server->device_table[i].id, server->product_order_table[product_order_index].product_id, server->device_table[transport_device_index].id, server->product_order_table[product_order_index].order_number);
				vrp_write_log_entry(&server->log, server->log_entry_buffer);

				if (error_code == VRP_ERROR_ITEM_NOT_FOUND)
				{
					if (product_order_index != (size_t)~0)
					{
						server->product_order_table[product_order_index].order_status = VRP_ORDER_NOT_AVAILABLE;
						server->product_order_table[product_order_index].transport_device_id = VRP_ID_UNDEFINED;

						sprintf(server->log_entry_buffer, "Warning UR5 %lu responded product %lu not available for order %08X.",
							server->device_table[i].id, server->product_order_table[product_order_index].product_id, server->product_order_table[product_order_index].order_number);
						vrp_write_log_entry(&server->log, server->log_entry_buffer);
					}
					else
					{
						sprintf(server->log_entry_buffer, "Warning UR5 %lu moved a product for unknown order.", server->device_table[i].id);
						vrp_write_log_entry(&server->log, server->log_entry_buffer);
					}
				}
			}
		}

		server->device_table[i].move_to_x = VRP_COORDINATE_UNDEFINED;
		server->device_table[i].move_to_y = VRP_COORDINATE_UNDEFINED;
	}
	
	server->device_table[i].command = VRP_MESSAGE_UNDEFINED;
	server->device_table[i].connection_state = VRP_CONNECTION_IDLE;

	size_t controller_index = vrp_get_controlling_device_index(server, server->device_table[i].id);
	if (controller_index != (size_t)~0 && server->device_table[i].executing_remote_command)
		server->device_table[i].remote_command_finished = 1;

	return 1;
}

int vrp_process_client_command(vrp_server_t* server, size_t i, size_t total_message_size)
{
	if (total_message_size < 5)
		return 0;

	server->device_table[i].command = server->device_table[i].io_memory[0];

	sprintf(server->log_entry_buffer, "Received command %lu from device %lu at address %lu.%lu.%lu.%lu", server->device_table[i].command, server->device_table[i].id,
		((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
	vrp_write_log_entry(&server->log, server->log_entry_buffer);

	size_t wfm_size = 12;
	uint8_t wfm_error = VRP_ERROR_NOT_SUPPORTED;
	uint8_t wfm_atomic = 1;
	uint8_t lines_read = VRP_DIRECTION_UNDEFINED;

	switch (server->device_table[i].command)
	{
		case VRP_MESSAGE_CCM:
		{
			wfm_error = VRP_ERROR_SUCCESS;
			break;
		}
		case VRP_MESSAGE_SQM:
		{
			wfm_error = VRP_ERROR_SUCCESS;
			break;
		}
		case VRP_MESSAGE_RLM:
		{
			if (total_message_size < 11)
				wfm_error = VRP_ERROR_INVALID_MESSAGE;
			else
			{
				uint32_t log_line_offset = ((uint32_t)server->device_table[i].io_memory[7] << 0) | ((uint32_t)server->device_table[i].io_memory[8] << 8) | ((uint32_t)server->device_table[i].io_memory[9] << 16) | ((uint32_t)server->device_table[i].io_memory[10] << 24);
				uint32_t log_line_count = (uint32_t)server->device_table[i].io_memory[6];
				if (server->device_table[i].io_memory[5])
				{
					if ((server->log.line_count - 1) < log_line_offset)
						log_line_offset = 0;
					else
						log_line_offset = (server->log.line_count - 1) - log_line_offset;
				}
				uint32_t log_lines_read;
				size_t log_size_read;
				DWORD log_error = vrp_read_log(&server->log, log_line_offset, log_line_count, server->device_io_buffer_size - 12, server->device_table[i].io_memory + 12, &log_lines_read, &log_size_read);
				if (!log_error)
				{
					wfm_error = VRP_ERROR_SUCCESS;
					if (log_lines_read < (uint8_t)log_line_count)
						wfm_atomic = 0;
					lines_read = (uint8_t)log_lines_read;
					wfm_size = 12 + log_size_read;
				}
				else if (log_error == ERROR_FILE_NOT_FOUND)
					wfm_error = VRP_ERROR_INVALID_PARAMETER;
				else
					wfm_error = VRP_ERROR_DEVICE_MALFUNCTION;
			}
			break;
		}
		case VRP_MESSAGE_RCM:
		{
			if (total_message_size < 7)
				wfm_error = VRP_ERROR_INVALID_MESSAGE;
			else
			{
				uint8_t remote_command_target = server->device_table[i].io_memory[5];
				uint8_t remote_control_flag = server->device_table[i].io_memory[6];
				uint32_t remote_command_size = (uint32_t)(total_message_size - 7);
				if (remote_control_flag)
				{
					if (!remote_command_size)
						wfm_error = VRP_ERROR_INVALID_MESSAGE;
					else
					{
						size_t target_controller_index = vrp_get_controlling_device_index(server, remote_command_target);
						size_t target_index = vrp_get_device_index_by_id(server, remote_command_target);
						if ((target_controller_index == (size_t)~0 || target_controller_index == server->device_table[i].id) &&
							(remote_command_target != VRP_ID_UNDEFINED) &&
							(remote_command_target != VRP_ID_ENERGENCY_BROADCAST) &&
							(remote_command_target != server->id) &&
							(target_index != (size_t)~0) &&
							(server->device_table[target_index].connection_state != VRP_CONNECTION_DISCONNECT) &&
							(server->device_table[target_index].type != VRP_DEVICE_TYPE_CLIENT))
						{
							server->device_table[i].control_target_id = remote_command_target;
							server->device_table[target_index].executing_remote_command = 0;
							server->device_table[target_index].remote_command_finished = 0;
							server->device_table[i].connection_state = VRP_CONNECTION_REMOTE_COMMAND;
							server->device_table[i].unprocessed_io = VRP_IO_IDLE;
							wfm_error = VRP_ERROR_SUCCESS;
							return 1;
						}
						wfm_error = VRP_ERROR_ITEM_NOT_FOUND;
					}
				}
				else
					server->device_table[i].control_target_id = VRP_ID_UNDEFINED;
			}
			break;
		}
		case VRP_MESSAGE_POM:
		{
			uint8_t product_id = server->device_table[i].io_memory[5];
			uint8_t product_destination_x = VRP_COORDINATE_UNDEFINED;
			uint8_t product_destination_y = VRP_COORDINATE_UNDEFINED;

			if (vrp_choose_product_order_destination(server, (total_message_size - 6) / 2, server->device_table[i].io_memory + 6, &product_destination_x, &product_destination_y))
			{
				size_t order_index = vrp_create_product_order(server, product_id, product_destination_x, product_destination_y, server->device_table[i].id);
				if (order_index != (size_t)~0)
				{
					sprintf(server->log_entry_buffer,
						"Received product order command for product %lu from device %lu at address %lu.%lu.%lu.%lu. Order %08X", 0, server->device_table[i].id,
						((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF,
						server->product_order_table[order_index].order_number);
					vrp_write_log_entry(&server->log, server->log_entry_buffer);

					server->device_table[i].io_memory[12] = ((uint32_t)(server->product_order_table[order_index].order_number) >> 0) & 0xFF;
					server->device_table[i].io_memory[13] = ((uint32_t)(server->product_order_table[order_index].order_number) >> 8) & 0xFF;
					server->device_table[i].io_memory[14] = ((uint32_t)(server->product_order_table[order_index].order_number) >> 16) & 0xFF;
					server->device_table[i].io_memory[15] = ((uint32_t)(server->product_order_table[order_index].order_number) >> 24) & 0xFF;

					wfm_size = 16;
					wfm_error = VRP_ERROR_SUCCESS;
				}
				else
					wfm_error = VRP_ERROR_OUT_OF_RESOURCES;
			}
			else
				wfm_error = VRP_ERROR_INVALID_PARAMETER;
			break;
		}
		case VRP_MESSAGE_SSM:
		{
			if (server->status == 2)
			{
				server->status = 1;
				wfm_error = VRP_ERROR_SUCCESS;
			}
			else
				wfm_error = VRP_ERROR_INVALID_PARAMETER;
			sprintf(server->log_entry_buffer, "Received startup command from device %lu at address %lu.%lu.%lu.%lu", server->device_table[i].id,
				((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
			vrp_write_log_entry(&server->log, server->log_entry_buffer);
			break;
		}
		case VRP_MESSAGE_UFM:
		{
			if (!server->status)
			{
				server->status = 1;
				wfm_error = VRP_ERROR_SUCCESS;
			}
			else
				wfm_error = VRP_ERROR_INVALID_PARAMETER;
			sprintf(server->log_entry_buffer, "Received unfreeze command from device %lu at address %lu.%lu.%lu.%lu", server->device_table[i].id,
				((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
			vrp_write_log_entry(&server->log, server->log_entry_buffer);
			break;
		}
		case VRP_MESSAGE_SHM:
		{
			if (server->status == 1)
			{
				server->status = 2;
				wfm_error = VRP_ERROR_SUCCESS;
			}
			else
				wfm_error = VRP_ERROR_INVALID_PARAMETER;
			sprintf(server->log_entry_buffer, "Received shutdown command from device %lu at address %lu.%lu.%lu.%lu", server->device_table[i].id,
				((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
			vrp_write_log_entry(&server->log, server->log_entry_buffer);
			break;
		}
		case VRP_MESSAGE_FOM:
		{
			break;
		}
		default:
			break;
	}

	server->device_table[i].io_memory[0] = VRP_MESSAGE_WFM;
	server->device_table[i].io_memory[1] = ((uint32_t)(wfm_size - 5) >> 0) & 0xFF;
	server->device_table[i].io_memory[2] = ((uint32_t)(wfm_size - 5) >> 8) & 0xFF;
	server->device_table[i].io_memory[3] = ((uint32_t)(wfm_size - 5) >> 16) & 0xFF;
	server->device_table[i].io_memory[4] = ((uint32_t)(wfm_size - 5) >> 24) & 0xFF;
	server->device_table[i].io_memory[5] = server->device_table[i].command;
	server->device_table[i].io_memory[6] = wfm_error;
	server->device_table[i].io_memory[7] = wfm_atomic;
	server->device_table[i].io_memory[8] = VRP_COORDINATE_UNDEFINED;
	server->device_table[i].io_memory[9] = VRP_COORDINATE_UNDEFINED;
	server->device_table[i].io_memory[10] = lines_read;
	server->device_table[i].io_memory[11] = server->status;

	if (!vrp_write(server, i, 0, wfm_size))
		return 0;
	server->device_table[i].connection_state = VRP_CONNECTION_RESPONDING_TO_COMMAND;

	return 1;
}

int vrp_process_ncm(vrp_server_t* server, size_t i, size_t total_message_size)
{
	if ((total_message_size < 11) ||
		(server->device_table[i].io_memory[0] != VRP_MESSAGE_NCM) ||
		(server->device_table[i].io_memory[5] == VRP_DEVICE_TYPE_MASTER) ||
		(server->device_table[i].io_memory[5] == VRP_DEVICE_TYPE_UNDEFINED) ||
		(server->device_table[i].io_memory[6] == server->id) ||
		(vrp_get_device_index_by_id(server, server->device_table[i].io_memory[6]) != (size_t)~0))
		return 0;

	server->device_table[i].last_uppdate_time = server->time;

	if (server->device_table[i].io_memory[6] != VRP_ID_UNDEFINED)
		server->device_table[i].id = server->device_table[i].io_memory[6];
	else
	{
		server->device_table[i].id = vrp_get_temporal_device_id(server);
		if (server->device_table[i].id == VRP_ID_UNDEFINED)
			return 0;
	}

	server->device_table[i].type = server->device_table[i].io_memory[5];
	if (server->device_table[i].type == VRP_DEVICE_TYPE_GOPIGO)
	{
		server->device_table[i].x = server->device_table[i].io_memory[7];
		server->device_table[i].y = server->device_table[i].io_memory[8];
		server->device_table[i].direction = server->device_table[i].io_memory[9];
	}
	else
	{
		server->device_table[i].x = VRP_COORDINATE_UNDEFINED;
		server->device_table[i].y = VRP_COORDINATE_UNDEFINED;
		server->device_table[i].direction = VRP_DIRECTION_UNDEFINED;
	}
	server->device_table[i].state = server->device_table[i].io_memory[10];

	server->device_table[i].command = VRP_MESSAGE_SCM;
	server->device_table[i].io_memory[0] = VRP_MESSAGE_SCM;
	server->device_table[i].io_memory[1] = 2;
	server->device_table[i].io_memory[2] = 0;
	server->device_table[i].io_memory[3] = 0;
	server->device_table[i].io_memory[4] = 0;
	server->device_table[i].io_memory[5] = 0x00;// do not change the controlling device
	server->device_table[i].io_memory[6] = server->device_table[i].id;

	if (server->device_table[i].type == VRP_DEVICE_TYPE_CLIENT)
	{
		sprintf(server->log_entry_buffer, "Device %lu(QT-client) connected with type %lu and status %lu from address %lu.%lu.%lu.%lu", server->device_table[i].id, server->device_table[i].type, server->device_table[i].state,
			((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
		vrp_write_log_entry(&server->log, server->log_entry_buffer);

		server->device_table[i].command = VRP_MESSAGE_SCM;
		server->device_table[i].connection_state = VRP_CONNECTION_RESPONDING_TO_COMMAND;
		server->device_table[i].io_memory[5] = 0x01;// do change the controlling device
	}
	else
		server->device_table[i].connection_state = VRP_CONNECTION_SETUP;

	if (!vrp_write(server, i, 0, 7))
		return 0;

	return 1;
}

int vrp_process_device(vrp_server_t* server, size_t i)
{
	if (vrp_is_device_timeout_reached(server, i))
		return 0;
	
	if (server->device_table[i].unprocessed_io != VRP_IO_IDLE)
	{
		size_t io_data_size = (size_t)((uintptr_t)server->device_table[i].io_buffer.buf - (uintptr_t)server->device_table[i].io_memory);
		size_t total_message_size = (io_data_size >= 5) ? (5 + vrp_get_message_size(server->device_table[i].io_memory)) : 0;// total messge size is invalid if io_data_size < 5
		if (server->device_table[i].unprocessed_io == VRP_IO_WRITE)
		{
			if (server->device_table[i].connection_state == VRP_CONNECTION_SENDING_COMMAND)
			{
				server->device_table[i].connection_state = VRP_CONNECTION_WAITING_FOR_RESPONSE;
				if (!vrp_read(server, i, 0, server->device_io_buffer_size))
					return 0;
			}
			else if (server->device_table[i].connection_state == VRP_CONNECTION_SETUP)
			{
				server->device_table[i].connection_state = VRP_CONNECTION_WAITING_FOR_RESPONSE;
				if (!vrp_read(server, i, 0, server->device_io_buffer_size))
					return 0;
			}
			else if (server->device_table[i].connection_state == VRP_CONNECTION_RESPONDING_TO_COMMAND)
			{
				if (server->device_table[i].command == VRP_MESSAGE_CCM)
				{
					sprintf(server->log_entry_buffer, "Device %lu(QT-client) disconnected from address %lu.%lu.%lu.%lu", server->device_table[i].id, ((server->device_table[i].ip_address) >> 24) & 0xFF, ((server->device_table[i].ip_address) >> 16) & 0xFF, ((server->device_table[i].ip_address) >> 8) & 0xFF, ((server->device_table[i].ip_address) >> 0) & 0xFF);
					vrp_write_log_entry(&server->log, server->log_entry_buffer);
					vrp_shutdown_device_connection(server, i);
					return 0;
				}
				else
				{
					server->device_table[i].command = VRP_MESSAGE_UNDEFINED;
					server->device_table[i].connection_state = VRP_CONNECTION_WAITING_FOR_COMMAND;
					if (!vrp_read(server, i, 0, server->device_io_buffer_size))
						return 0;
				}
			}
			else
				return 0;
		}
		else
		{
			if (server->device_table[i].connection_state == VRP_CONNECTION_WAITING_FOR_RESPONSE)
			{
				if (!vrp_process_wfm(server, i, total_message_size))
					return 0;
			}
			else if (server->device_table[i].connection_state == VRP_CONNECTION_NEW)
			{
				if (!vrp_process_ncm(server, i, total_message_size))
					return 0;
			}
			else if (server->device_table[i].connection_state == VRP_CONNECTION_WAITING_FOR_COMMAND)
			{
				if (!vrp_process_client_command(server, i, total_message_size))
					return 0;
			}
			else
				return 0;
		}
		server->device_table[i].unprocessed_io = VRP_IO_IDLE;
	}
	
	if (server->device_table[i].connection_state == VRP_CONNECTION_REMOTE_COMMAND)
	{
		assert(server->device_table[i].type == VRP_DEVICE_TYPE_CLIENT);

		size_t target_index = vrp_get_device_index_by_id(server, server->device_table[i].control_target_id);
		if (target_index != (size_t)~0)
		{
			assert(server->device_table[target_index].type != VRP_DEVICE_TYPE_CLIENT);

			if (server->device_table[target_index].remote_command_finished)
			{
				size_t remote_command_wfm_size = 5 + vrp_get_message_size(server->device_table[target_index].io_memory);
				server->device_table[i].io_memory[0] = VRP_MESSAGE_WFM;
				server->device_table[i].io_memory[1] = ((uint32_t)(7 + remote_command_wfm_size) >> 0) & 0xFF;
				server->device_table[i].io_memory[2] = ((uint32_t)(7 + remote_command_wfm_size) >> 8) & 0xFF;
				server->device_table[i].io_memory[3] = ((uint32_t)(7 + remote_command_wfm_size) >> 16) & 0xFF;
				server->device_table[i].io_memory[4] = ((uint32_t)(7 + remote_command_wfm_size) >> 24) & 0xFF;
				server->device_table[i].io_memory[5] = VRP_MESSAGE_RCM;
				server->device_table[i].io_memory[6] = VRP_ERROR_SUCCESS;
				server->device_table[i].io_memory[7] = 1;
				server->device_table[i].io_memory[8] = VRP_COORDINATE_UNDEFINED;
				server->device_table[i].io_memory[9] = VRP_COORDINATE_UNDEFINED;
				server->device_table[i].io_memory[10] = VRP_DIRECTION_UNDEFINED;
				server->device_table[i].io_memory[11] = server->status;
				memcpy(server->device_table[i].io_memory + 12, server->device_table[target_index].io_memory, remote_command_wfm_size);
				if (!vrp_write(server, i, 0, 12 + remote_command_wfm_size))
					return 0;

				server->device_table[i].connection_state = VRP_CONNECTION_RESPONDING_TO_COMMAND;
			}
		}
		else
		{
			server->device_table[i].control_target_id = VRP_ID_UNDEFINED;
			server->device_table[i].io_memory[0] = VRP_MESSAGE_WFM;
			server->device_table[i].io_memory[1] = 7;
			server->device_table[i].io_memory[2] = 0;
			server->device_table[i].io_memory[3] = 0;
			server->device_table[i].io_memory[4] = 0;
			server->device_table[i].io_memory[5] = VRP_MESSAGE_RCM;
			server->device_table[i].io_memory[6] = VRP_ERROR_ITEM_NOT_FOUND;
			server->device_table[i].io_memory[7] = 0;
			server->device_table[i].io_memory[8] = VRP_COORDINATE_UNDEFINED;
			server->device_table[i].io_memory[9] = VRP_COORDINATE_UNDEFINED;
			server->device_table[i].io_memory[10] = VRP_DIRECTION_UNDEFINED;
			server->device_table[i].io_memory[11] = server->status;
			if (!vrp_write(server, i, 0, 12))
				return 0;

			server->device_table[i].connection_state = VRP_CONNECTION_RESPONDING_TO_COMMAND;
		}
	}

	if (server->device_table[i].connection_state == VRP_CONNECTION_IDLE)
	{
		size_t controller_index = vrp_get_controlling_device_index(server, server->device_table[i].id);
		if (controller_index != (size_t)~0)
		{
			assert((server->device_table[i].type != VRP_DEVICE_TYPE_CLIENT) && (server->device_table[controller_index].type == VRP_DEVICE_TYPE_CLIENT));

			if (!server->device_table[i].executing_remote_command && !server->device_table[i].remote_command_finished)
			{
				server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;

				server->device_table[i].command = server->device_table[controller_index].io_memory[7];
				size_t remote_command_size = 5 + vrp_get_message_size(server->device_table[controller_index].io_memory + 7);
				memcpy(server->device_table[i].io_memory, server->device_table[controller_index].io_memory + 7, remote_command_size);

				if (!vrp_write(server, i, 0, remote_command_size))
					return 0;

				server->device_table[i].executing_remote_command = 1;
			}
		}
		else
		{
			switch (server->device_table[i].type)
			{
				case VRP_DEVICE_TYPE_GOPIGO:
				{
					if (!vrp_process_gopigo_idle(server, i))
						return 0;
					break;
				}
				case VRP_DEVICE_TYPE_UR5:
				{
					if (!vrp_process_ur5_idle(server, i))
						return 0;
					break;
				}
				case VRP_DEVICE_TYPE_CLIENT:
				{
					// qt-client is not processed here
					assert(0);
					break;
				}
				default:
				{
					// disconnect any devices of unknown types
					server->device_table[i].command = VRP_MESSAGE_CCM;
					server->device_table[i].io_memory[0] = server->device_table[i].command;
					server->device_table[i].io_memory[1] = 0;
					server->device_table[i].io_memory[2] = 0;
					server->device_table[i].io_memory[3] = 0;
					server->device_table[i].io_memory[4] = 0;

					if (!vrp_write(server, i, 0, 5))
						return 0;

					server->device_table[i].connection_state = VRP_CONNECTION_SENDING_COMMAND;
					break;
				}
			}
		}
	}

	return 1;
}

void vrp_process_order(vrp_server_t* server, size_t i)
{
	size_t transport_device_index = (size_t)~0;

	if (server->product_order_table[i].transport_device_id != VRP_ID_UNDEFINED)
		transport_device_index = vrp_get_device_index_by_id(server, server->product_order_table[i].transport_device_id);

	if (transport_device_index == (size_t)~0)
		server->product_order_table[i].transport_device_id = VRP_ID_UNDEFINED;

	if (server->product_order_table[i].transport_device_id == VRP_ID_UNDEFINED)
	{
		if (server->product_order_table[i].order_status == VRP_ORDER_IN_STORAGE)
		{
			transport_device_index = vrp_get_device_index_for_free_transport(server);
			if (transport_device_index != (size_t)~0)
				server->product_order_table[i].transport_device_id = server->device_table[transport_device_index].id;
		}
		else
		{
			assert(0);
			// very bad. the order item is lost now
		}
	}
}

DWORD vrp_run_setup(vrp_server_t** server)
{
	printf("Starting VarastoRobo master 0.4.3...\n");
	
	const char* server_setup_error_info;
	DWORD error = vrp_create_server_instance(server, &server_setup_error_info);
	
	if (error)
	{
		printf("Failed to create server instance. Error code %lu error info \"%s\"\n", error, server_setup_error_info);
		return error;
	}

	assert(*server);

	char server_ip_address[16];
	char broadcast_ip_address[16];
	RtlIpv4AddressToStringA((const struct in_addr*)&(*server)->server_address.sin_addr.s_addr, server_ip_address);
	RtlIpv4AddressToStringA((const struct in_addr*)&(*server)->broadcast_address.sin_addr.s_addr, broadcast_ip_address);
	printf("server running at address %s broadcast address %s\n", server_ip_address, broadcast_ip_address);

	vrp_create_test_clients();

	return 0;
}

int main(int argc, char* argv)
{
	vrp_server_t* server;
	if (vrp_run_setup(&server))
		return EXIT_FAILURE;

	for (;;)
	{
#ifndef _NDEBUG
		// validate servers internal device list
		vrp_get_valid_device_entries(server);
#endif
		size_t i = vrp_wait_for_io(server);
		vrp_accept_incoming_connection(server);
		if (i == VRP_WAIT_EMERGENCY)
			vrp_process_possible_emergency(server);
		else if (i == VRP_WAIT_BROADCAST)
			vrp_finish_send_system_broadcast_message(server);
		else if (i < VRP_MAX_DEVICE_COUNT)
		{
			int io_type;
			size_t total_transfered = vrp_finish_io(server, i, &io_type);
			size_t continue_size = total_transfered != (size_t)~0 ? continue_size = vrp_message_transfer_incomplete(server, i, io_type) : (size_t)~0;
			if (continue_size && continue_size < (size_t)~0)
			{
				if (io_type == VRP_IO_READ)
					if (!vrp_read(server, i, total_transfered, continue_size))
						continue_size = (size_t)~0;
				else
					if (!vrp_write(server, i, total_transfered, continue_size))
						continue_size = (size_t)~0;
			}
			if (!continue_size)
				server->device_table[i].unprocessed_io = io_type;
			else if (continue_size == (size_t)~0)
				vrp_remove_device(server, i);
		}
		for (size_t j = 0; j != server->product_order_count; ++j)
			vrp_process_order(server, j);
		for (size_t j = 0; j != VRP_MAX_DEVICE_COUNT; ++j)
			if (server->device_table[j].sock != INVALID_SOCKET)
				if(!vrp_process_device(server, j))
					vrp_remove_device(server, j);
		if (!server->debug_no_broadcast && server->broadcast_io_state == VRP_IO_IDLE && (GetTickCount64() - server->last_broadcast_time) > server->broadcast_delay)
			vrp_send_system_broadcast_message(server);
	}

	__assume(0);
	return 0;
}