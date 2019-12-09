/*
	VarastoRobo master server version 0.9.3 2019-12-09 by Santtu Nyman.
*/

#include "vrp_constants.h"
#include <assert.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 6384)
#endif

const char* vrp_get_message_type_string(int message_type_number, int short_name)
{
	assert(message_type_number >= 0 && short_name >= 0);

	static const char* unknown = "unknown message";
	static const char* short_unknown = "UM";
	static const char* string_table[] = {
		"SBM", "system broadcast message",
		"NCM", "new connection message",
		"SCM", "setup connection message",
		"WFM", "work finished message",
		"CCM", "closed connection message",
		"SQM", "status query message",
		"SSM", "system startup message",
		"SHM", "system shutdown message",
		"UFM", "unfreeze message",
		"RLM", "read log message",
		"POM", "product order message",
		"MPM", "move product message",
		"MCM", "move cell message",
		"RCM", "remote control message",
		"FOM", "finish order message" };
	static const int string_count = (sizeof(string_table) / sizeof(*string_table)) / 2;

	if (message_type_number && (message_type_number < string_count))
		return string_table[(2 * (message_type_number - 1)) + (short_name ? 0 : 1)];
	else
		return short_name ? short_unknown : unknown;
}

const char* vrp_get_error_string(int error_number)
{
	assert(error_number >= 0);

	static const char* unknown = "unknown or invalid error";
	static const char* string_table[] = {
		"no error",
		"invalid message",
		"not supported",
		"invalid parameter",
		"item not found",
		"access denied",
		"device malfunction",
		"out of resources",
		"operation canseled",
		"path not found",
		"unable to use path" };
	static const int string_count = sizeof(string_table) / sizeof(*string_table);

	if (error_number < string_count)
		return string_table[error_number];
	else
		return unknown;
}

const char* vrp_get_status_string(int status_number)
{
	assert(status_number >= 0);

	static const char* unknown = "unknown";
	static const char* string_table[] = {
		"frozen",
		"normal",
		"sleep",
		"battery low (normal)",
		"invalid" };
	static const int string_count = sizeof(string_table) / sizeof(*string_table);

	if (status_number < string_count)
		return string_table[status_number];
	else
		return unknown;
}

const char* vrp_get_device_type_string(int device_type_number)
{
	assert(device_type_number >= 0);

	static const char* unknown = "unknown";
	static const char* string_table[] = {
		"master",
		"QT-client",
		"GoPiGo",
		"UR5",
		"drone" };
	static const int string_count = sizeof(string_table) / sizeof(*string_table);

	if (device_type_number < string_count)
		return string_table[device_type_number];
	else
		return unknown;
}

const char* vrp_get_direction_string(int direction_number)
{
	assert(direction_number >= 0);

	static const char* unknown = "unknown";
	static const char* undefined = "undefined";
	static const char* string_table[] = {
		"right",
		"up",
		"left",
		"down" };
	static const int string_count = sizeof(string_table) / sizeof(*string_table);

	if (direction_number < string_count)
		return string_table[direction_number];
	else if (direction_number == 0xFF)
		return undefined;
	else
		return unknown;
}

const char* vrp_get_product_string(int product_number, int product_marker_shape)
{
	assert(product_number >= 0 && product_marker_shape >= 0);

	static const char* unknown = "unknown";
	static const char* undefined = "undefined";
	static const char* string_table[] = {
		"Dexter packet", "square",
		"GoPiGo wheel", "circle",
		"reserved", "triangle",
		"reserved", "pentagon"};
	const int string_count = (sizeof(string_table) / sizeof(*string_table)) / 2;

	if (product_number < string_count)
		return string_table[(2 * product_number) + (product_marker_shape ? 1 : 0)];
	else if (product_number == 0xFF)
		return undefined;
	else
		return unknown;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif