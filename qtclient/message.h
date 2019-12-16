#ifndef MESSAGE_H
#define MESSAGE_H


class Message
{
public:
    // Message #1 System Broadcast Message FREEZE
    char sbm_freeze[8] = {
        static_cast<char>(0x01), // constant lsb
        static_cast<char>(0x07), // constant msb
        static_cast<char>(0x00), // state
        static_cast<char>(0x00), // master id
        static_cast<char>(0x00), // map height
        static_cast<char>(0x00), // map width
        static_cast<char>(0x00), // obstacle count
        static_cast<char>(0x00), // device count
    };

    // Message #2: New Connection Message (NCM)
    char const new_connection[11] = {
        static_cast<char>(0x02), // message type (NCM)
        static_cast<char>(0x06), // message length byte1 (LSB)
        static_cast<char>(0x00), // message length byte2
        static_cast<char>(0x00), // message length byte3
        static_cast<char>(0x00), // message length byte4 (MSB)
        static_cast<char>(0x01), // device type (0x01 == QtClint)
        static_cast<char>(0xff), // device id (0xff == tells master to QtClient a real one in a returning SCM message)
        static_cast<char>(0x00), // device x coordinate (N/A for QtClient, may ignore)
        static_cast<char>(0x00), // device y coordinate (N/A for QtClient, may ignore)
        static_cast<char>(0x00), // device orientation  (N/A for QtClient, may ignore)
        static_cast<char>(0x01)  // device state (0x01 == normal)
    };

    // Message #5: Close Connection Message (CCM)
    char const disconnection[5] = {
        static_cast<char>(0x05), // message type (CCM)
        static_cast<char>(0x00), // message length byte1 (LSB)
        static_cast<char>(0x00), // message length byte2
        static_cast<char>(0x00), // message length byte3
        static_cast<char>(0x00)  // message length byte4 (MSB)
    };

    // Message #6: Status Query Message (SQM)
    char const status_query[5] = {
        static_cast<char>(0x06), // message type (SQM)
        static_cast<char>(0x00), // message length byte1 (LSB)
        static_cast<char>(0x00), // message length byte2
        static_cast<char>(0x00), // message length byte3
        static_cast<char>(0x00)  // message length byte4 (MSB)
    };

    // Message #7: System Startup Message (SSM)
    const char system_startup[5] = {
        static_cast<char>(0x07), // message type (SSM)
        static_cast<char>(0x00), // message length byte1 (LSB)
        static_cast<char>(0x00), // message length byte2
        static_cast<char>(0x00), // message length byte3
        static_cast<char>(0x00)  // message length byte4 (MSB)
    };

    // Message #8: System Shutdown Message (SHM)
    char const system_shutdown[5] = {
        static_cast<char>(0x08), // message type (SHM)
        static_cast<char>(0x00), // message length byte1 (LSB)
        static_cast<char>(0x00), // message length byte2
        static_cast<char>(0x00), // message length byte3
        static_cast<char>(0x00)  // message length byte4 (MSB)
    };

    // Message #9: UnFreeze Message (UFM)
    char const unfreeze[5] = {
        static_cast<char>(0x09), // message type (UFM)
        static_cast<char>(0x00), // message length byte 1 (LSB)
        static_cast<char>(0x00), // message length byte 2
        static_cast<char>(0x00), // message length byte 3
        static_cast<char>(0x00)  // message length byte 4 (MSB)
    };

    // Message #10 Read Log Message (RLM)
    char read_log[11] = {
        static_cast<char>(0x0A), // message type (RLM)
        static_cast<char>(0x06), // message length byte 1 (LSB)
        static_cast<char>(0x00), // message length byte 2
        static_cast<char>(0x00), // message length byte 3
        static_cast<char>(0x00), // message length byte 4 (MSB)
        static_cast<char>(0x00), // mode
        static_cast<char>(0x00), // row count
        static_cast<char>(0x00), // beginning row number byte 1 (LSB)
        static_cast<char>(0x00), // beginning row number byte 2
        static_cast<char>(0x00), // beginning row number byte 3
        static_cast<char>(0x00)  // beginning row number byte 4 (MSB)
    };

    // Message #11 Product Order Message (POM)
    char product_order[14] = {
        static_cast<char>(0x0B), // message type
        static_cast<char>(0x00), // message length (LSB)
        static_cast<char>(0x00), // message length
        static_cast<char>(0x00), // message length
        static_cast<char>(0x00), // message length (MSB)
        static_cast<char>(0x00), // product_id
        static_cast<char>(0x00), // destination 1 x
        static_cast<char>(0x00), // destination 1 y
        static_cast<char>(0x00), // destination 2 x
        static_cast<char>(0x00), // destination 2 y
        static_cast<char>(0x00), // destination 3 x
        static_cast<char>(0x00), // destination 3 y
        static_cast<char>(0x00), // destination 4 x
        static_cast<char>(0x00), // destination 4 y
    };

    // Message #14 Remote Control Message (RCM)
    char remote_control[13] = {
        static_cast<char>(0x0E), // message type (known)
        static_cast<char>(0x08), // message length (lsb)       (known, because as of now, MCM is the only RMC command)
        static_cast<char>(0x00), // message length             (known, because as of now, MCM is the only RMC command)
        static_cast<char>(0x00), // message length             (known, because as of now, MCM is the only RMC command)
        static_cast<char>(0x00), // message length (msb)       (known, because as of now, MCM is the only RMC command)
        static_cast<char>(0x00), // target_device_id
        static_cast<char>(0x00), // control_flag
        static_cast<char>(0x0D), // MCM (message_type)
        static_cast<char>(0x01), // MCM (message_length lsb)
        static_cast<char>(0x00), // MCM (message_length)
        static_cast<char>(0x00), // MCM (message_length)
        static_cast<char>(0x00), // MCM (message_length msb)
        static_cast<char>(0x00)  // MCM (direction)            (determined in the ui)
    };
};

#endif // MESSAGE_H
