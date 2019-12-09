/*
	VarastoRobo master server version 0.9.3 2019-12-09 by Santtu Nyman.
*/

#ifndef VRP_CONSTANTS_H
#define VRP_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

#define VRP_MESSAGE_SBM                0x01
#define VRP_MESSAGE_NCM                0x02
#define VRP_MESSAGE_SCM                0x03
#define VRP_MESSAGE_WFM                0x04
#define VRP_MESSAGE_CCM                0x05
#define VRP_MESSAGE_SQM                0x06
#define VRP_MESSAGE_SSM                0x07
#define VRP_MESSAGE_SHM                0x08
#define VRP_MESSAGE_UFM                0x09
#define VRP_MESSAGE_RLM                0x0A
#define VRP_MESSAGE_POM                0x0B
#define VRP_MESSAGE_MPM                0x0C
#define VRP_MESSAGE_MCM                0x0D
#define VRP_MESSAGE_RCM                0x0E
#define VRP_MESSAGE_FOM                0x0F
#define VRP_MESSAGE_UNDEFINED          0xFF

#define VRP_ERROR_SUCCESS              0x00
#define VRP_ERROR_INVALID_MESSAGE      0x01
#define VRP_ERROR_NOT_SUPPORTED        0x02
#define VRP_ERROR_INVALID_PARAMETER    0x03
#define VRP_ERROR_ITEM_NOT_FOUND       0x04
#define VRP_ERROR_ACCESS_DENIED        0x05
#define VRP_ERROR_DEVICE_MALFUNCTION   0x06
#define VRP_ERROR_OUT_OF_RESOURCES     0x07
#define VRP_ERROR_CANSELED             0x08
#define VRP_ERROR_PATH_NOT_FOUND       0x09
#define VRP_ERROR_UNABLE_TO_USE_PATH   0x0A

#define VRP_STATE_FROZEN               0x00
#define VRP_STATE_NORMAL               0x01
#define VRP_STATE_SLEEP                0x02
#define VRP_STATE_BATTERY_LOW          0x03
#define VRP_STATE_INVALID              0x04
#define VRP_STATE_UNDEFINED            0xFF

#define VRP_ID_ENERGENCY_BROADCAST     0x00
#define VRP_ID_GOPIGO_DEFAULT_LOW      0x01
#define VRP_ID_GOPIGO_DEFAULT_HIGH     0x09
#define VRP_ID_UR5_DEFAULT             0x37
#define VRP_ID_MASTER_DEFAULT          0x2A
#define VRP_ID_DRONE_DEFAULT           0x63
#define VRP_ID_TMP_DEFAULT_LOW         0x64
#define VRP_ID_TMP_DEFAULT_HIGH        0xC7
#define VRP_ID_UNDEFINED               0xFF

#define VRP_DEVICE_TYPE_MASTER         0x00
#define VRP_DEVICE_TYPE_CLIENT         0x01
#define VRP_DEVICE_TYPE_GOPIGO         0x02
#define VRP_DEVICE_TYPE_UR5            0x03
#define VRP_DEVICE_TYPE_DRONE          0x04
#define VRP_DEVICE_TYPE_UNDEFINED      0xFF

#define VRP_COORDINATE_UNDEFINED       0xFF

#define VRP_DIRECTION_RIGHT            0x00
#define VRP_DIRECTION_UP               0x01
#define VRP_DIRECTION_LEFT             0x02
#define VRP_DIRECTION_DOWN             0x03
#define VRP_DIRECTION_UNDEFINED        0xFF

#define VRP_PRODUCT_TYPE_DEXTER_PACKET 0x00
#define VRP_PRODUCT_TYPE_GOPIGO_WHEEL  0x01
#define VRP_PRODUCT_TYPE_UNDEFINED     0xFF

// do not try to deallocate or write to the strings returned by these functions. they are part of the program image. also never give negative inputs to these functions.

const char* vrp_get_message_type_string(int message_type_number, int short_name);

const char* vrp_get_error_string(int error_number);

const char* vrp_get_status_string(int status_number);

const char* vrp_get_device_type_string(int device_type_number);

const char* vrp_get_direction_string(int direction_number);

const char* vrp_get_product_string(int product_number, int product_marker_shape);

#ifdef __cplusplus
}
#endif

#endif