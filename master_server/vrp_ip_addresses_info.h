/*
	VarastoRobo master server version 1.1.1 2019-12-16 by Santtu Nyman.
	github repository https://github.com/Jarno-Poikonen/VarastoRobo
*/

#ifndef VRP_IP_ADDRESS_INFO_H
#define VRP_IP_ADDRESS_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <ip2string.h>
#include <windows.h>

DWORD vrp_get_host_ip_address(uint32_t* host_ip_address, uint32_t* subnet_mask);

#ifdef __cplusplus
extern "C" {
#endif

#endif