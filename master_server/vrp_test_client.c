/*
	VarastoRobo master server version 1.0.0 2019-12-10 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo

	This file contains code only for testing the master server.
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "vrp_test_client.h"
#include "ntdll_time.h"
#include <stdlib.h>
#include "vrp_thread_group.h"
#include "vrp_ip_addresses_info.h"
#include "vrp_master_server_base.h"
#include <assert.h>

static struct
{
	int w;
	int h;
	uint8_t map[9 * 6];
	volatile uint32_t blocks[9 * 6];
	volatile uint32_t items[9 * 6];
} vrp_test_map;

typedef struct vrp_test_client_configuration_t
{
	uint32_t on_wire_server_address;
	int random_delays;
} vrp_test_client_configuration_t;

uint32_t vrp_get_master_address_from_sbm()
{
	struct sockaddr_in any_address = { 0 };
	any_address.sin_family = AF_INET;
	any_address.sin_port = htons(1732);
	any_address.sin_addr.s_addr = INADDR_ANY;

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
		return INADDR_ANY;

	const BOOL reuse_address_enable = TRUE;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_address_enable, sizeof(BOOL));

	const BOOL broadcast_enable = TRUE;
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast_enable, sizeof(BOOL)) == -1)
	{
		closesocket(sock);
		return INADDR_ANY;
	}

	for (int wait_for_bind = 1; wait_for_bind;)
	{
		if (!bind(sock, (struct sockaddr*)&any_address, sizeof(struct sockaddr_in)))
			wait_for_bind = 0;
		else if (WSAGetLastError() == WSAEADDRINUSE)
		{
			ULONG min_time_resulution_100ns;
			ULONG max_time_resulution_100ns;
			ULONG current_time_resulution_100ns;
			if (NtQueryTimerResolution(&min_time_resulution_100ns, &max_time_resulution_100ns, &current_time_resulution_100ns) >> 31)
				Sleep((current_time_resulution_100ns + 9999) / 10000);
			else
				Sleep(0);
		}
		else
		{
			closesocket(sock);
			return INADDR_ANY;
		}
	}

	struct sockaddr_in source_address;
	int source_address_size = sizeof(struct sockaddr_in);
	uint8_t buffer[512];
	size_t message_size;

	uint32_t master_on_wire_ip_address;
	uint16_t message_canstant;
	uint8_t system_status;
	uint8_t master_id;
	uint8_t map_height;
	uint8_t map_width;
	uint8_t block_count;
	uint8_t device_count;

	size_t map_size;
	size_t block_size;
	size_t device_size;

	for (int wait_for_message = 1; wait_for_message;)
	{
		message_size = (size_t)recvfrom(sock, (char*)buffer, sizeof(buffer), 0, (struct sockaddr*)&source_address, &source_address_size);
		if (message_size == (size_t)-1)
		{
			closesocket(sock);
			return INADDR_ANY;
		}

		if (message_size > 7)
		{
			master_on_wire_ip_address = source_address.sin_addr.s_addr;
			message_canstant = (uint16_t)buffer[0] | (uint16_t)(buffer[1] << 8);
			system_status = buffer[2];
			master_id = buffer[3];
			map_height = buffer[4];
			map_width = buffer[5];
			block_count = buffer[6];
			device_count = buffer[7];

			map_size = (((size_t)map_height * (size_t)map_width) + 7) / 8;
			block_size = (size_t)block_count * 2;
			device_size = (size_t)device_count * 8;

			if (message_canstant == 0x0701 && (8 + map_size + block_size + device_size) == message_size && master_id)
				wait_for_message = 0;
		}
	}

	closesocket(sock);
	return master_on_wire_ip_address;
}

size_t vrp_send_message(SOCKET sock, size_t message_size, const void* message_buffer)
{
	for (size_t i = 0; i != message_size;)
	{
		int bytes_to_send = ((message_size - i) < (size_t)INT_MAX ? (int)(message_size - i) : INT_MAX);
		int bytes_send = send(sock, (const char*)((uintptr_t)message_buffer + i), bytes_to_send, 0);
		if (bytes_send < 1)
			return 0;
		i += (size_t)bytes_send;
	}
	return message_size;
}

size_t vrp_receive_message(SOCKET sock, size_t buffer_size, void* message_buffer)
{
	if (buffer_size < 5)
		return 0;
	size_t i = 0;
	while (i < 5)
	{
		int bytes_to_receive = ((buffer_size - i) < (size_t)INT_MAX ? (int)(buffer_size - i) : INT_MAX);
		int bytes_received = recv(sock, (char*)((uintptr_t)message_buffer + i), bytes_to_receive, 0);
		if (bytes_received < 1)
			return 0;
		i += (size_t)bytes_received;
	}
	size_t total_message_size = 5 +
		(size_t)(((uint32_t) * (uint8_t*)((uintptr_t)message_buffer + 1) << 0) |
		((uint32_t)*(uint8_t*)((uintptr_t)message_buffer + 2) << 8) |
		((uint32_t)*(uint8_t*)((uintptr_t)message_buffer + 3) << 16) |
		((uint32_t)*(uint8_t*)((uintptr_t)message_buffer + 4) << 24));
	if (buffer_size < total_message_size)
		return 0;
	while (i != total_message_size)
	{
		int bytes_to_receive = ((total_message_size - i) < (size_t)INT_MAX ? (int)(total_message_size - i) : INT_MAX);
		int bytes_received = recv(sock, (char*)((uintptr_t)message_buffer + i), bytes_to_receive, 0);
		if (bytes_received < 1)
			return 0;
		i += (size_t)bytes_received;
	}
	return total_message_size;
}

SOCKET vrp_test_connect(uint8_t* id, uint8_t type, uint8_t x, uint8_t y, uint8_t direction, uint8_t state, int random_delays, uint32_t on_wire_ip)
{
	if (random_delays && !(rand() & 3))
		Sleep(rand() % 1000);

	struct sockaddr_in server_address = { 0 };
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(1739);
	server_address.sin_addr.s_addr = (on_wire_ip != INADDR_ANY) ? on_wire_ip : vrp_get_master_address_from_sbm();

	if (server_address.sin_addr.s_addr == INADDR_ANY)
		return INVALID_SOCKET;

	if (random_delays && !(rand() & 3))
		Sleep(rand() % 1000);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		return INVALID_SOCKET;

	if (random_delays && !(rand() & 3))
		Sleep(rand() % 1000);

	if (connect(sock, (const struct sockaddr*)&server_address, sizeof(struct sockaddr_in)))
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	if (random_delays && !(rand() & 3))
		Sleep(rand() % 1000);

	uint8_t message_buffer[128];
	
	message_buffer[0] = VRP_MESSAGE_NCM;
	message_buffer[1] = 6;// msg size 6
	message_buffer[2] = 0;
	message_buffer[3] = 0;
	message_buffer[4] = 0;
	message_buffer[5] = type;
	message_buffer[6] = *id;
	message_buffer[7] = x;
	message_buffer[8] = y;
	message_buffer[9] = direction;
	message_buffer[10] = state;
	if (!vrp_send_message(sock, 11, message_buffer))
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	if (random_delays && !(rand() & 3))
		Sleep(rand() % 1000);

	size_t message_size = vrp_receive_message(sock, 128, message_buffer);
	if ((message_size != 7) || (message_buffer[0] != VRP_MESSAGE_SCM) || (message_buffer[5] != (uint8_t)(type == VRP_DEVICE_TYPE_CLIENT)) || ((*id != 0xFF) && (message_buffer[6] != *id)) || (message_buffer[6] == 0xFF))
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}
	*id = message_buffer[6];

	if (random_delays && !(rand() & 3))
		Sleep(rand() % 1000);

	if (type == VRP_DEVICE_TYPE_CLIENT)
		return sock;

	message_buffer[0] = VRP_MESSAGE_WFM;
	message_buffer[1] = 7;// msg size 6
	message_buffer[2] = 0;
	message_buffer[3] = 0;
	message_buffer[4] = 0;
	message_buffer[5] = VRP_MESSAGE_SCM;
	message_buffer[6] = VRP_ERROR_SUCCESS;
	message_buffer[7] = 1;
	message_buffer[8] = x;
	message_buffer[9] = y;
	message_buffer[10] = direction;
	message_buffer[11] = state;
	if (!vrp_send_message(sock, 12, message_buffer))
	{
		closesocket(sock);
		return INVALID_SOCKET;
	}

	if (random_delays && !(rand() & 3))
		Sleep(rand() % 1000);

	return sock;
}

void vrp_test_gopigo(vrp_test_client_configuration_t* configuration, size_t thread_count, size_t thread_index)
{
	const uint8_t destination_x = 8;
	const uint8_t destination_y = 2;

	int random_delay = configuration->random_delays;

	srand((int)(GetCurrentThreadId() >> 2));

	printf("gopigo %ul: starting in 3\n", GetCurrentThreadId());
	Sleep(1000);
	printf("gopigo %ul: starting in 2\n", GetCurrentThreadId());
	Sleep(1000);
	printf("gopigo %ul: starting in 1\n", GetCurrentThreadId());
	Sleep(1000);
	printf("gopigo %ul: starting in 0\n", GetCurrentThreadId());

	uint8_t id = 1 + (uint8_t)thread_index;
	uint8_t type = VRP_DEVICE_TYPE_GOPIGO;
	uint8_t x = 0;
	uint8_t y = (uint8_t)thread_index;
	uint8_t direction = VRP_DIRECTION_UP;
	uint8_t state = VRP_STATE_NORMAL;
	uint8_t message_buffer[256] = { 0 };

	if (random_delay && !(rand() & 3))
		Sleep(rand() % 1000);

	printf("gopigo %ul: Connecting...\n", GetCurrentThreadId());
	SOCKET sock = vrp_test_connect(&id, type, x, y, direction, state, 0, configuration->on_wire_server_address);
	if (sock == INVALID_SOCKET)
	{
		printf("gopigo %ul error: Failed to connect\n", GetCurrentThreadId());
		return;
	}

	if (random_delay && !(rand() & 3))
		Sleep(rand() % 1000);

	for (;;)
	{
		if (random_delay && !(rand() & 3))
			Sleep(rand() % 1000);

		printf("gopigo %ul: Waiting for command...\n", GetCurrentThreadId());
		size_t message_size = vrp_receive_message(sock, sizeof(message_buffer), message_buffer);
		if (!message_size)
		{


			printf("gopigo %ul error: Receiving command failed\n", GetCurrentThreadId());
			closesocket(sock);
			return;
		}

		MemoryBarrier();
		uint32_t item = (uint32_t)vrp_test_map.items[((int)y * (int)vrp_test_map.w) + (int)x];

		uint8_t command = message_buffer[0];
		switch (command)
		{
			case VRP_MESSAGE_SCM:
			{
				if ((message_size != 7) || message_buffer[5] || (message_buffer[6] == 0xFF))
				{
					message_buffer[0] = VRP_MESSAGE_WFM;
					message_buffer[1] = 7 + ((item != 0xFFFFFFFF) ? 1 : 0);
					message_buffer[2] = 0;
					message_buffer[3] = 0;
					message_buffer[4] = 0;
					message_buffer[5] = VRP_MESSAGE_SCM;
					message_buffer[6] = VRP_ERROR_INVALID_PARAMETER;
					message_buffer[7] = 1;
					message_buffer[8] = x;
					message_buffer[9] = y;
					message_buffer[10] = direction;
					message_buffer[11] = state;
					if (item != 0xFFFFFFFF)
						message_buffer[12] = (uint8_t)item;

					if (random_delay && !(rand() & 3))
						Sleep(rand() % 1000);

					if (!vrp_send_message(sock, 12 + ((item != 0xFFFFFFFF) ? 1 : 0), message_buffer))
					{
						printf("gopigo %ul error: Sending response failed\n", GetCurrentThreadId());
						closesocket(sock);
						return;
					}
					printf("gopigo %ul: Setup executed\n", GetCurrentThreadId());
				}
				else
				{
					message_buffer[0] = VRP_MESSAGE_WFM;
					message_buffer[1] = 7 + ((item != 0xFFFFFFFF) ? 1 : 0);
					message_buffer[2] = 0;
					message_buffer[3] = 0;
					message_buffer[4] = 0;
					message_buffer[5] = VRP_MESSAGE_SCM;
					message_buffer[6] = VRP_ERROR_SUCCESS;
					message_buffer[7] = 1;
					message_buffer[8] = x;
					message_buffer[9] = y;
					message_buffer[10] = direction;
					message_buffer[11] = state;
					if (item != 0xFFFFFFFF)
						message_buffer[12] = (uint8_t)item;

					if (random_delay && !(rand() & 3))
						Sleep(rand() % 1000);

					if (!vrp_send_message(sock, 12 + ((item != 0xFFFFFFFF) ? 1 : 0), message_buffer))
					{
						printf("gopigo %ul error: sending response failed\n", GetCurrentThreadId());
						closesocket(sock);
						return;
					}
					printf("gopigo %ul: Setup again? This seems to be wrong\n", GetCurrentThreadId());
				}
				break;
			}
			case VRP_MESSAGE_CCM:
			{
				message_buffer[0] = VRP_MESSAGE_WFM;
				message_buffer[1] = 7 + ((item != 0xFFFFFFFF) ? 1 : 0);
				message_buffer[2] = 0;
				message_buffer[3] = 0;
				message_buffer[4] = 0;
				message_buffer[5] = VRP_MESSAGE_CCM;
				message_buffer[6] = VRP_ERROR_SUCCESS;
				message_buffer[7] = 1;
				message_buffer[8] = x;
				message_buffer[9] = y;
				message_buffer[10] = direction;
				message_buffer[11] = state;
				if (item != 0xFFFFFFFF)
					message_buffer[12] = (uint8_t)item;

				if (random_delay && !(rand() & 3))
					Sleep(rand() % 1000);

				vrp_send_message(sock, 12 + ((item != 0xFFFFFFFF) ? 1 : 0), message_buffer);
				printf("gopigo %ul: Connection closed\n", GetCurrentThreadId());

				shutdown(sock, SD_BOTH);
				closesocket(sock);
				return;
			}
			case VRP_MESSAGE_SQM:
			{
				if ((x == destination_x) && (y == destination_y) && (item != 0xFFFFFFFF))
				{
					item = 0xFFFFFFFF;
					InterlockedExchange((volatile LONG*)&vrp_test_map.items[((int)y * (int)vrp_test_map.w) + (int)x], item);
				}

				message_buffer[0] = VRP_MESSAGE_WFM;
				message_buffer[1] = 7 + ((item != 0xFFFFFFFF) ? 1 : 0);
				message_buffer[2] = 0;
				message_buffer[3] = 0;
				message_buffer[4] = 0;
				message_buffer[5] = VRP_MESSAGE_SQM;
				message_buffer[6] = VRP_ERROR_SUCCESS;
				message_buffer[7] = 1;
				message_buffer[8] = x;
				message_buffer[9] = y;
				message_buffer[10] = direction;
				message_buffer[11] = state;
				if (item != 0xFFFFFFFF)
					message_buffer[12] = (uint8_t)item;

				if (random_delay && !(rand() & 3))
					Sleep(rand() % 1000);

				if (!vrp_send_message(sock, 12 + ((item != 0xFFFFFFFF) ? 1 : 0), message_buffer))
				{
					printf("gopigo %ul error: Sending response failed\n", GetCurrentThreadId());
					closesocket(sock);
					return;
				}
				printf("gopigo %ul: Executed SQM\n", GetCurrentThreadId());
				break;
			}
			case VRP_MESSAGE_MCM:
			{
				int cant_move = 0;
				uint8_t new_x = x;
				uint8_t new_y = y;
				switch (message_buffer[5])
				{
					case VRP_DIRECTION_LEFT:
						if (x)
						{
							direction = VRP_DIRECTION_LEFT;
							new_x -= 1;
						}
						else
							cant_move = 1;
						break;
					case VRP_DIRECTION_UP:
						if (y < 5)
						{
							direction = VRP_DIRECTION_UP;
							new_y += 1;
						}
						else
							cant_move = 1;
						break;
					case VRP_DIRECTION_RIGHT:
						if (x < 8)
						{
							direction = VRP_DIRECTION_RIGHT;
							new_x += 1;
						}
						else
							cant_move = 1;
						break;
					case VRP_DIRECTION_DOWN:
						if (y)
						{
							direction = VRP_DIRECTION_DOWN;
							new_y -= 1;
						}
						else
							cant_move = 1;
						break;
					default:
						cant_move = 1;
						break;
				}

				if (item != 0xFFFFFFFF)
				{
					InterlockedExchange((volatile LONG*)&vrp_test_map.items[((int)y * (int)vrp_test_map.w) + (int)x], 0xFFFFFFFF);
					InterlockedExchange((volatile LONG*)&vrp_test_map.items[((int)new_y * (int)vrp_test_map.w) + (int)new_x], item);
				}

				x = new_x;
				y = new_y;

				message_buffer[0] = VRP_MESSAGE_WFM;
				message_buffer[1] = 7 + ((item != 0xFFFFFFFF) ? 1 : 0);
				message_buffer[2] = 0;
				message_buffer[3] = 0;
				message_buffer[4] = 0;
				message_buffer[5] = VRP_MESSAGE_MCM;
				message_buffer[6] = !cant_move ? VRP_ERROR_SUCCESS : VRP_ERROR_UNABLE_TO_USE_PATH;
				message_buffer[7] = 1;
				message_buffer[8] = x;
				message_buffer[9] = y;
				message_buffer[10] = direction;
				message_buffer[11] = state;
				if (item != 0xFFFFFFFF)
					message_buffer[12] = (uint8_t)item;

				Sleep(2000 + (rand() % 500));
				if (random_delay && !(rand() & 3))
					Sleep(rand() % 1000);

				if (!vrp_send_message(sock, 12 + ((item != 0xFFFFFFFF) ? 1 : 0), message_buffer))
				{
					printf("gopigo %ul error: Sending response failed\n", GetCurrentThreadId());
					closesocket(sock);
					return;
				}
				printf("gopigo %ul: Executed MCM\n", GetCurrentThreadId());
				break;
			}
			default:
			{
				message_buffer[0] = VRP_MESSAGE_WFM;
				message_buffer[1] = 7 + ((item != 0xFFFFFFFF) ? 1 : 0);
				message_buffer[2] = 0;
				message_buffer[3] = 0;
				message_buffer[4] = 0;
				message_buffer[5] = command;
				message_buffer[6] = VRP_ERROR_NOT_SUPPORTED;
				message_buffer[7] = 1;
				message_buffer[8] = x;
				message_buffer[9] = y;
				message_buffer[10] = direction;
				message_buffer[11] = state;
				if (item != 0xFFFFFFFF)
					message_buffer[12] = (uint8_t)item;

				if (random_delay && !(rand() & 3))
					Sleep(rand() % 1000);

				if (!vrp_send_message(sock, 12 + ((item != 0xFFFFFFFF) ? 1 : 0), message_buffer))
				{
					printf("gopigo %ul error: Sending response failed\n", GetCurrentThreadId());
					closesocket(sock);
					return;
				}
				printf("gopigo %ul: Received unsupported command\n", GetCurrentThreadId());
				break;
			}
		}
	}
	return;
}

void vrp_test_ur5(vrp_test_client_configuration_t* configuration, size_t thread_count, size_t thread_index)
{
	struct { uint8_t x; uint8_t y; } pickeup_table[3] = { { 3, 0 }, { 3, 1 }, { 3, 2 } };

	int random_delay = configuration->random_delays;

	srand((int)(GetCurrentThreadId() >> 2));

	printf("ur5 %ul: starting in 3\n", GetCurrentThreadId());
	Sleep(1000);
	printf("ur5 %ul: starting in 2\n", GetCurrentThreadId());
	Sleep(1000);
	printf("ur5 %ul: starting in 1\n", GetCurrentThreadId());
	Sleep(1000);
	printf("ur5 %ul: starting in 0\n", GetCurrentThreadId());

	uint8_t id = 55;
	uint8_t type = VRP_DEVICE_TYPE_UR5;
	uint8_t x = VRP_COORDINATE_UNDEFINED;
	uint8_t y = VRP_COORDINATE_UNDEFINED;
	uint8_t direction = VRP_DIRECTION_UNDEFINED;
	uint8_t state = VRP_STATE_NORMAL;
	uint8_t message_buffer[256] = { 0 };

	if (random_delay && !(rand() & 3))
		Sleep(rand() % 1000);

	printf("ur5 %ul: Connecting...\n", GetCurrentThreadId());
	SOCKET sock = vrp_test_connect(&id, type, x, y, direction, state, 0, configuration->on_wire_server_address);
	if (sock == INVALID_SOCKET)
	{
		printf("ur5 %ul error: Failed to connect\n", GetCurrentThreadId());
		return;
	}

	if (random_delay && !(rand() & 3))
		Sleep(rand() % 1000);

	for (;;)
	{
		if (random_delay && !(rand() & 3))
			Sleep(rand() % 1000);

		printf("ur5 %ul: Waiting for command...\n", GetCurrentThreadId());
		size_t message_size = vrp_receive_message(sock, sizeof(message_buffer), message_buffer);
		if (!message_size)
		{
			printf("ur5 %ul error: Receiving command failed\n", GetCurrentThreadId());
			closesocket(sock);
			return;
		}
		uint8_t command = message_buffer[0];
		switch (command)
		{
			case VRP_MESSAGE_SCM:
			{
				if ((message_size != 7) || message_buffer[5] || (message_buffer[6] == 0xFF))
				{
					message_buffer[0] = VRP_MESSAGE_WFM;
					message_buffer[1] = 7;
					message_buffer[2] = 0;
					message_buffer[3] = 0;
					message_buffer[4] = 0;
					message_buffer[5] = VRP_MESSAGE_SCM;
					message_buffer[6] = VRP_ERROR_INVALID_PARAMETER;
					message_buffer[7] = 1;
					message_buffer[8] = x;
					message_buffer[9] = y;
					message_buffer[10] = direction;
					message_buffer[11] = state;

					if (random_delay && !(rand() & 3))
						Sleep(rand() % 1000);

					if (!vrp_send_message(sock, 12, message_buffer))
					{
						printf("ur5 %ul error: Sending response failed\n", GetCurrentThreadId());
						closesocket(sock);
						return;
					}
					printf("ur5 %ul: Setup executed\n", GetCurrentThreadId());
				}
				else
				{
					message_buffer[0] = VRP_MESSAGE_WFM;
					message_buffer[1] = 7;
					message_buffer[2] = 0;
					message_buffer[3] = 0;
					message_buffer[4] = 0;
					message_buffer[5] = VRP_MESSAGE_SCM;
					message_buffer[6] = VRP_ERROR_SUCCESS;
					message_buffer[7] = 1;
					message_buffer[8] = x;
					message_buffer[9] = y;
					message_buffer[10] = direction;
					message_buffer[11] = state;

					if (random_delay && !(rand() & 3))
						Sleep(rand() % 1000);

					if (!vrp_send_message(sock, 12, message_buffer))
					{
						printf("ur5 %ul error: sending response failed\n", GetCurrentThreadId());
						closesocket(sock);
						return;
					}
					printf("ur5 %ul: Setup again? This seems to be wrong\n", GetCurrentThreadId());
				}
				break;
			}
			case VRP_MESSAGE_CCM:
			{
				message_buffer[0] = VRP_MESSAGE_WFM;
				message_buffer[1] = 7;
				message_buffer[2] = 0;
				message_buffer[3] = 0;
				message_buffer[4] = 0;
				message_buffer[5] = VRP_MESSAGE_CCM;
				message_buffer[6] = VRP_ERROR_SUCCESS;
				message_buffer[7] = 1;
				message_buffer[8] = x;
				message_buffer[9] = y;
				message_buffer[10] = direction;
				message_buffer[11] = state;

				if (random_delay && !(rand() & 3))
					Sleep(rand() % 1000);

				vrp_send_message(sock, 12, message_buffer);
				printf("ur5 %ul: Connection closed\n", GetCurrentThreadId());

				shutdown(sock, SD_BOTH);
				closesocket(sock);
				return;
			}
			case VRP_MESSAGE_SQM:
			{
				message_buffer[0] = VRP_MESSAGE_WFM;
				message_buffer[1] = 7;
				message_buffer[2] = 0;
				message_buffer[3] = 0;
				message_buffer[4] = 0;
				message_buffer[5] = VRP_MESSAGE_SQM;
				message_buffer[6] = VRP_ERROR_SUCCESS;
				message_buffer[7] = 1;
				message_buffer[8] = x;
				message_buffer[9] = y;
				message_buffer[10] = direction;
				message_buffer[11] = state;

				if (random_delay && !(rand() & 3))
					Sleep(rand() % 1000);

				if (!vrp_send_message(sock, 12, message_buffer))
				{
					printf("ur5 %ul error: Sending response failed\n", GetCurrentThreadId());
					closesocket(sock);
					return;
				}
				printf("ur5 %ul: Executed SQM\n", GetCurrentThreadId());
				break;
			}
			case VRP_MESSAGE_MPM:
			{
				if (message_size >= 7)
				{
					uint8_t product_id = message_buffer[5];
					uint8_t destination = message_buffer[6];

					assert(product_id < 3);

					message_buffer[0] = VRP_MESSAGE_WFM;
					message_buffer[1] = 7;
					message_buffer[2] = 0;
					message_buffer[3] = 0;
					message_buffer[4] = 0;
					message_buffer[5] = VRP_MESSAGE_MPM;
					message_buffer[6] = VRP_ERROR_SUCCESS;
					message_buffer[7] = 1;
					message_buffer[8] = x;
					message_buffer[9] = y;
					message_buffer[10] = direction;
					message_buffer[11] = state;

					Sleep(1000 + (rand() % 1000));

					//assert(vrp_test_map.items[((int)pickeup_table[destination].y * vrp_test_map.w) + (int)pickeup_table[destination].x] == 0xFFFFFFFF);
					//InterlockedExchange((volatile LONG*)&vrp_test_map.items[((int)pickeup_table[destination].y * vrp_test_map.w) + (int)pickeup_table[destination].x], (LONG)product_id);

					if (!vrp_send_message(sock, 12, message_buffer))
					{
						printf("ur5 %ul error: Sending response failed\n", GetCurrentThreadId());
						closesocket(sock);
						return;
					}

					printf("ur5 %ul: Executed MPM. product=%lu destination=%lu\n", GetCurrentThreadId(), (unsigned long)product_id, (unsigned long)destination);
				}
				else
				{
					message_buffer[0] = VRP_MESSAGE_WFM;
					message_buffer[1] = 7;
					message_buffer[2] = 0;
					message_buffer[3] = 0;
					message_buffer[4] = 0;
					message_buffer[5] = VRP_MESSAGE_MPM;
					message_buffer[6] = VRP_ERROR_INVALID_MESSAGE;
					message_buffer[7] = 1;
					message_buffer[8] = x;
					message_buffer[9] = y;
					message_buffer[10] = direction;
					message_buffer[11] = state;

					if (!vrp_send_message(sock, 12, message_buffer))
					{
						printf("ur5 %ul error: Sending response failed\n", GetCurrentThreadId());
						closesocket(sock);
						return;
					}

					printf("ur5 %ul: Invalid MPM received\n", GetCurrentThreadId());
				}
				break;
			}
			default:
			{
				message_buffer[0] = VRP_MESSAGE_WFM;
				message_buffer[1] = 7;
				message_buffer[2] = 0;
				message_buffer[3] = 0;
				message_buffer[4] = 0;
				message_buffer[5] = command;
				message_buffer[6] = VRP_ERROR_NOT_SUPPORTED;
				message_buffer[7] = 1;
				message_buffer[8] = x;
				message_buffer[9] = y;
				message_buffer[10] = direction;
				message_buffer[11] = state;

				if (random_delay && !(rand() & 3))
					Sleep(rand() % 1000);

				if (!vrp_send_message(sock, 12, message_buffer))
				{
					printf("ur5 %ul error: Sending response failed\n", GetCurrentThreadId());
					closesocket(sock);
					return;
				}
				printf("ur5 %ul: Received unsupported command\n", GetCurrentThreadId());
				break;
			}
		}
	}
	return;
}

void vrp_test_client_order(vrp_test_client_configuration_t* configuration, size_t thread_count, size_t thread_index)
{
	int random_delay = configuration->random_delays;

	srand((int)(GetCurrentThreadId() >> 2));

	uint8_t id = VRP_ID_UNDEFINED;
	uint8_t type = VRP_DEVICE_TYPE_CLIENT;
	uint8_t x = VRP_COORDINATE_UNDEFINED;
	uint8_t y = VRP_COORDINATE_UNDEFINED;
	uint8_t direction = VRP_DIRECTION_UNDEFINED;
	uint8_t state = VRP_STATE_NORMAL;
	uint8_t message_buffer[256] = { 0 };
	
	printf("client %ul: starting in 10 seconds\n", GetCurrentThreadId());
	Sleep(10000);
	printf("client %ul: starting now\n", GetCurrentThreadId());

	if (random_delay && !(rand() & 3))
		Sleep(rand() % 1000);

	printf("client %ul: Connecting...\n", GetCurrentThreadId());
	SOCKET sock = vrp_test_connect(&id, type, x, y, direction, state, 0, configuration->on_wire_server_address);
	if (sock == INVALID_SOCKET)
	{
		printf("client %ul error: Failed to connect\n", GetCurrentThreadId());
		return;
	}

	if (random_delay && !(rand() & 3))
		Sleep(rand() % 1000);

	uint8_t product_id = 1;
	int8_t product_destination_x = 8;
	int8_t product_destination_y = 2;

	message_buffer[0] = VRP_MESSAGE_POM;
	message_buffer[1] = 3;
	message_buffer[2] = 0;
	message_buffer[3] = 0;
	message_buffer[4] = 0;
	message_buffer[5] = product_id;
	message_buffer[6] = product_destination_x;
	message_buffer[7] = product_destination_y;

	Sleep(1000 + (rand() % 1000));

	if (!vrp_send_message(sock, 8, message_buffer))
	{
		printf("client %ul error: Sending command failed\n", GetCurrentThreadId());
		closesocket(sock);
		return;
	}
	printf("client %ul: pom sent\n", GetCurrentThreadId());

	printf("client %ul: Waiting for response...\n", GetCurrentThreadId());
	size_t message_size = vrp_receive_message(sock, sizeof(message_buffer), message_buffer);
	if (message_size < 12)
	{
		printf("client %ul error: Receiving response failed\n", GetCurrentThreadId());
		closesocket(sock);
		return;
	}

	if (message_buffer[0] != VRP_MESSAGE_WFM ||
		message_buffer[5] != VRP_MESSAGE_POM ||
		message_size < 12)
	{
		printf("client %ul error: Received invalid wfm failed\n", GetCurrentThreadId());
		closesocket(sock);
		return;
	}

	printf("client %ul: product order error %lu atomic flag %lu order number %08X response from server\n", GetCurrentThreadId(), message_buffer[6], message_buffer[7], *(unsigned long*)(message_buffer + 12));

	message_buffer[0] = VRP_MESSAGE_CCM;
	message_buffer[1] = 0;
	message_buffer[2] = 0;
	message_buffer[3] = 0;
	message_buffer[4] = 0;

	Sleep(1000 + (rand() % 1000));

	if (!vrp_send_message(sock, 5, message_buffer))
	{
		printf("client %ul error: Sending ccm failed\n", GetCurrentThreadId());
		closesocket(sock);
		return;
	}
	printf("client %ul: ccm sent\n", GetCurrentThreadId());

	printf("client %ul: Waiting for response...\n", GetCurrentThreadId());
	message_size = vrp_receive_message(sock, sizeof(message_buffer), message_buffer);
	if (message_size < 12)
	{
		printf("client %ul error: Receiving response failed\n", GetCurrentThreadId());
		closesocket(sock);
		return;
	}
	printf("client %ul: disconnected\n", GetCurrentThreadId());

	closesocket(sock);
	return;
}

void vrp_test_client_order2(vrp_test_client_configuration_t* configuration, size_t thread_count, size_t thread_index)
{
	printf("client %ul: starting in 30 seconds\n", GetCurrentThreadId());
	Sleep(20000);
	vrp_test_client_order(configuration, thread_count, thread_index);
}

void vrp_test_client_order3(vrp_test_client_configuration_t* configuration, size_t thread_count, size_t thread_index)
{
	printf("client %ul: starting in 40 seconds\n", GetCurrentThreadId());
	Sleep(30000);
	vrp_test_client_order(configuration, thread_count, thread_index);
}

void vrp_free_test_client_configuraton(uint32_t flags, void* thread_group_parameter, size_t thread_count)
{
	VirtualFree(thread_group_parameter, 0, MEM_RELEASE);
}

DWORD vrp_create_test_clients(vrp_server_t* server)
{
	assert(server->map_height == 6 && server->map_width == 9);

	vrp_test_map.w = server->map_width;
	vrp_test_map.h = server->map_height;
	for (int y = 0; y != vrp_test_map.h; ++y)
		for (int x = 0; x != vrp_test_map.w; ++x)
			vrp_test_map.map[y * vrp_test_map.w + x] = (server->map_bitmap[(y * vrp_test_map.w + x) / 8] >> ((y * vrp_test_map.w + x) % 8)) & 1;

	for (int y = 0; y != vrp_test_map.h; ++y)
		for (int x = 0; x != vrp_test_map.w; ++x)
			vrp_test_map.items[y * vrp_test_map.w + x] = 0xFFFFFFFF;

	vrp_test_client_configuration_t* gopigo_configuration = (vrp_test_client_configuration_t*)VirtualAlloc(0, sizeof(vrp_test_client_configuration_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	vrp_test_client_configuration_t* ur5_configuration = (vrp_test_client_configuration_t*)VirtualAlloc(0, sizeof(vrp_test_client_configuration_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	vrp_test_client_configuration_t* client_order_configuration = (vrp_test_client_configuration_t*)VirtualAlloc(0, sizeof(vrp_test_client_configuration_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	vrp_test_client_configuration_t* client2_order_configuration = (vrp_test_client_configuration_t*)VirtualAlloc(0, sizeof(vrp_test_client_configuration_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	vrp_test_client_configuration_t* client3_order_configuration = (vrp_test_client_configuration_t*)VirtualAlloc(0, sizeof(vrp_test_client_configuration_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	// BuT COAMputterS HAv InfinET MeMori so Tsis CAn NOT be zEro right?
	assert(gopigo_configuration);
	assert(ur5_configuration);
	assert(client_order_configuration);
	assert(client2_order_configuration);
	assert(client3_order_configuration);

	uint32_t server_ip;
	uint32_t subnet_mask;
	vrp_get_host_ip_address(&server_ip, &subnet_mask);

	gopigo_configuration->on_wire_server_address = INADDR_ANY;
	gopigo_configuration->random_delays = 1;

	ur5_configuration->on_wire_server_address = INADDR_ANY;
	ur5_configuration->random_delays = 1;

	client_order_configuration->on_wire_server_address = INADDR_ANY;
	client_order_configuration->random_delays = 1;

	client2_order_configuration->on_wire_server_address = INADDR_ANY;
	client2_order_configuration->random_delays = 1;

	client3_order_configuration->on_wire_server_address = INADDR_ANY;
	client3_order_configuration->random_delays = 1;

	//vrp_create_thread_group(0, 3, gopigo_configuration, (void(*)(void*, size_t, size_t))vrp_test_gopigo, vrp_free_test_client_configuraton);
	vrp_create_thread_group(0, 1, ur5_configuration, (void(*)(void*, size_t, size_t))vrp_test_ur5, vrp_free_test_client_configuraton);
	//vrp_create_thread_group(0, 1, client_order_configuration, (void(*)(void*, size_t, size_t))vrp_test_client_order, vrp_free_test_client_configuraton);
	//vrp_create_thread_group(0, 1, client2_order_configuration, (void(*)(void*, size_t, size_t))vrp_test_client_order2, vrp_free_test_client_configuraton);
	//vrp_create_thread_group(0, 1, client3_order_configuration, (void(*)(void*, size_t, size_t))vrp_test_client_order3, vrp_free_test_client_configuraton);
	
	printf("server: client threads created\n");
	return 0;// also there were no errors, because no errors were checked
}

#ifdef __cplusplus
}
#endif
