/*
	VarastoRobo master server version 1.1.0 2019-12-12 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo

	This file contains code only for testing the master server.
*/

#ifndef VRP_TEST_CLIENT_H
#define VRP_TEST_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "vrp_constants.h"
#include "vrp_master_server_base.h"
#include <stdio.h>

DWORD vrp_create_test_clients(vrp_server_t* server);

#ifdef __cplusplus
}
#endif

#endif