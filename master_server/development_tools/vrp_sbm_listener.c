/*
	SBM listener program for VarastoRobotti project by Santtu Nyman
*/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <Winsock2.h>
#include <windows.h>

int is_block_at_location(size_t block_count, uint8_t* block_table, uint8_t x, uint8_t y)
{
	for (size_t i = 0; i != block_count; ++i)
		if (block_table[i * 2 + 0] == x && block_table[i * 2 + 1] == y)
			return 1;
	return 0;
}

int is_gopigo_at_location(size_t device_count, uint8_t* device_table, uint8_t x, uint8_t y)
{
	for (size_t i = 0; i != device_count; ++i)
		if (device_table[(i * 8) + 0] == 2 && device_table[(i * 8) + 2] == x && device_table[(i * 8) + 3] == y)
			return 1;
	return 0;
}

void print_ascii_map(uint8_t map_height, uint8_t map_width, uint8_t* map, size_t block_count, uint8_t* block_table, size_t device_count, uint8_t* device_table)
{
	printf("     +");
	for (uint8_t x = 0; x != map_width; ++x)
		printf("-");
	printf("+\n");
	for (uint8_t y = 0; y != map_height; ++y)
	{
		printf("    %i", ((map_height - 1) - y) % 10);
		printf("|");
		for (uint8_t x = 0; x != map_width; ++x)
		{
			if (is_gopigo_at_location(device_count, device_table, x, ((map_height - 1) - y)))
				printf("G");
			else if (is_block_at_location(block_count, block_table, x, ((map_height - 1) - y)))
				printf("B");
			else
			{
				int in_memory_bit_index = ((int)((map_height - 1) - y) * (int)map_width) + (int)x;
				int byte_index = in_memory_bit_index / 8;
				int bit_index = in_memory_bit_index % 8;
				printf((map[byte_index] & (uint8_t)(1 << bit_index)) ? "#" : " ");
			}
		}
		printf("|\n");
	}
	printf("     +");
	for (uint8_t x = 0; x != map_width; ++x)
		printf("-");
	printf("+\n");
	printf("      ");
	for (uint8_t x = 0; x != map_width; ++x)
		printf("%i", x % 10);
	printf("\n");
}

int main(int argc, char** argv)
{
	printf("Starting VarastoRobotti SBM listener process...\n");

	struct sockaddr_in any_address = { 0 };
	any_address.sin_family = AF_INET;
	any_address.sin_port = htons(1732);
	any_address.sin_addr.s_addr = INADDR_ANY;

	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data))
	{
		printf("Error initializing Winsock DLL\n");
		return EXIT_FAILURE;
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("Error in creating socket\n");
		WSACleanup();
		return EXIT_FAILURE;
	}

	const BOOL reuse_address_enable = TRUE;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_address_enable, sizeof(BOOL));

	const BOOL broadcast_enable = TRUE;
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast_enable, sizeof(BOOL)) == -1)
	{
		printf("Error enabling broadcast on socket\n");
		closesocket(sock);
		WSACleanup();
		return EXIT_FAILURE;
	}

	if (bind(sock, (struct sockaddr*)&any_address, sizeof(struct sockaddr_in)) == -1)
	{
		printf("Error binding socket\n");
		closesocket(sock);
		WSACleanup();
		return EXIT_FAILURE;
	}

	struct sockaddr_in source_address;
	int source_address_size = sizeof(struct sockaddr_in);
	uint8_t buffer[512];
	size_t message_size;

	uint32_t source_ip_address;
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

	for (;;)
	{
		for (int wait_for_message = 1; wait_for_message;)
		{
			printf("Waiting for next SBM...\n");
			message_size = (size_t)recvfrom(sock, (char*)buffer, sizeof(buffer), 0, (struct sockaddr*) & source_address, &source_address_size);
			if (message_size == (size_t)-1)
			{
				printf("Error in receiving IO operation from socket\n");
				closesocket(sock);
				WSACleanup();
				return EXIT_FAILURE;
			}

			if (message_size < 8)
				printf("Received invalid packet with size %zu from address %lu.%lu.%lu.%lu\n", message_size, (source_ip_address >> 24) & 0xFF, (source_ip_address >> 16) & 0xFF, (source_ip_address >> 8) & 0xFF, (source_ip_address >> 0) & 0xFF);
			else
			{
				source_ip_address = ntohl(source_address.sin_addr.s_addr);
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

				if (message_canstant != 0x0701 || (8 + map_size + block_size + device_size) != message_size)
					printf("Received invalid packet with size %zu from address %lu.%lu.%lu.%lu\n", message_size, (source_ip_address >> 24) & 0xFF, (source_ip_address >> 16) & 0xFF, (source_ip_address >> 8) & 0xFF, (source_ip_address >> 0) & 0xFF);
				else
					wait_for_message = 0;
			}
		}

		if (master_id)
		{
			if (system_status == 1)
				printf("Received normal SBM from master\n");
			else if (!system_status)
				printf("Received emergency stop SBM from master\n");
			else if (system_status == 2)
				printf("Received sleep SBM from master\n");
			else
				printf("Received invalid SBM from master\n");
		}
		else
		{
			if (!system_status)
				printf("Received emergency stop SBM from QT-Client\n");
			else
				printf("Received invalid SBM from QT-Client\n");
		}

		const char* status_text_table[3] = { "emergency stop", "normal", "sleep" };
		printf("Master %lu%s device in state %lu(%s) at address %lu.%lu.%lu.%lu\n",
			master_id, (master_id ? "" : "(QT-Client)"), system_status, ((system_status < 3) ? status_text_table[system_status] : "Invalid"), (source_ip_address >> 24) & 0xFF, (source_ip_address >> 16) & 0xFF, (source_ip_address >> 8) & 0xFF, (source_ip_address >> 0) & 0xFF);

		printf("map\n");
		print_ascii_map(map_height, map_width, buffer + 8, block_count, buffer + 8 + map_size, device_count, buffer + 8 + map_size + block_size);

		printf("%lu blocks(B)\n", block_count);
		for (size_t i = 0; i != (size_t)block_count; ++i)
		{
			uint8_t x = buffer[8 + map_size + (i * 2) + 0];
			uint8_t y = buffer[8 + map_size + (i * 2) + 1];
			printf("    Block found at map x=%lu,y=%lu\n", x, y);
		}

		printf("%lu other devices(GoPiGo=G)\n", device_count);
		for (size_t i = 0; i != (size_t)device_count; ++i)
		{
			uint8_t type = buffer[8 + map_size + block_size + (i * 8) + 0];
			uint8_t id = buffer[8 + map_size + block_size + (i * 8) + 1];
			uint8_t x = buffer[8 + map_size + block_size + (i * 8) + 2];
			uint8_t y = buffer[8 + map_size + block_size + (i * 8) + 3];
			uint32_t ip =
				((uint32_t)buffer[8 + map_size + block_size + (i * 8) + 4] << 0) |
				((uint32_t)buffer[8 + map_size + block_size + (i * 8) + 5] << 8) |
				((uint32_t)buffer[8 + map_size + block_size + (i * 8) + 6] << 16) |
				((uint32_t)buffer[8 + map_size + block_size + (i * 8) + 7] << 24);
			const char* status_text_table[5] = { "master", "QT-client", "GoPiGo", "UR5-gateway", "drone" };
			printf("    Device %lu of type %lu(%s) found at address %lu.%lu.%lu.%lu and map x=%lu,y=%lu%s\n",
				id, type, (type != 0xFF) ? ((type < 5) ? status_text_table[type] : "invalid") : "undefined", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, (ip >> 0) & 0xFF, x, y, (x == 0xFF && y == 0xFF) ? "(undefined coordinate)" : "");
		}
	}

	closesocket(sock);
	WSACleanup();
	return EXIT_FAILURE;
}