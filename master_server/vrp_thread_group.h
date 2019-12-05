/*
	VarastoRobo master server version 0.9.2 2019-12-05 by Santtu Nyman.
*/

#ifndef VRP_THREAD_GROUP_H
#define VRP_THREAD_GROUP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <errno.h>

#define VRP_FLAG_BLOCK_UNTIL_FINISHED 0x00000001

int vrp_create_thread_group(uint32_t flags, size_t thread_count, void* thread_group_parameter, void(*thread_procedure)(void* thread_group_parameter, size_t thread_count, size_t thread_index), void(*thread_group_exit_procedure)(uint32_t flags, void* thread_group_parameter, size_t thread_count));

#ifdef __cplusplus
}
#endif

#endif