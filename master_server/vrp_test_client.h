/*
	VarastoRobo master server version 0.4.2 2019-11-20 by Santtu Nyman.
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
#include <stdio.h>

DWORD vrp_create_test_clients();

#ifdef __cplusplus
}
#endif

#endif