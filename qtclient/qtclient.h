#ifndef QTCLIENT_H
#define QTCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QtMath>
#include <QHostAddress>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QTimer>
#include "systembroadcastmessage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QtClient; }
QT_END_NAMESPACE

class QtClient : public QMainWindow
{
    Q_OBJECT

public:
    QtClient(QWidget *parent = nullptr,
             quint16 tcp_port_to_master = 0,
             quint16 udp_port_from_master = 0);
    ~QtClient();

    void emit_finished();
    void bind();

private:
    Ui::QtClient *ui;
    quint16 tcp_port_to_master;
    quint16 udp_port_from_master;

    QTcpSocket tcp_socket;
    QUdpSocket udp_socket;

    QHostAddress qha;
    QString master_ip;
    quint16 master_udp_port;

    qint64 datagram_size;
    char datagram[512];

    SystemBroadcastMessage sbm;

    QTimer timer;

    // Message #1 System Broadcast Message FREEZE
    char sbm_freeze_message[8] = {
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
    char const new_connection_message[11] = {
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
    char const disconnection_message[5] = {
        static_cast<char>(0x05), // message type (CCM)
        static_cast<char>(0x00), // message length byte1 (LSB)
        static_cast<char>(0x00), // message length byte2
        static_cast<char>(0x00), // message length byte3
        static_cast<char>(0x00)  // message length byte4 (MSB)
    };

    // Message #6: Status Query Message (SQM)
    char const status_query_message[5] = {
        static_cast<char>(0x06), // message type (SQM) 
        static_cast<char>(0x00), // message length byte1 (LSB)
        static_cast<char>(0x00), // message length byte2
        static_cast<char>(0x00), // message length byte3
        static_cast<char>(0x00)  // message length byte4 (MSB)
    };

    // Message #7: System Startup Message (SSM)
    const char system_startup_message[5] = {
        static_cast<char>(0x07), // message type (SSM) 
        static_cast<char>(0x00), // message length byte1 (LSB)
        static_cast<char>(0x00), // message length byte2
        static_cast<char>(0x00), // message length byte3
        static_cast<char>(0x00)  // message length byte4 (MSB)
    };

    // Message #8: System Shutdown Message (SHM)
    char const system_shutdown_message[5] = {
        static_cast<char>(0x08), // message type (SHM) 
        static_cast<char>(0x00), // message length byte1 (LSB)
        static_cast<char>(0x00), // message length byte2
        static_cast<char>(0x00), // message length byte3
        static_cast<char>(0x00)  // message length byte4 (MSB)
    };

    // Message #9: UnFreeze Message (UFM)
    char const unfreeze_message[5] = {
        static_cast<char>(0x09), // message type (UFM)
        static_cast<char>(0x00), // message length byte 1 (LSB)
        static_cast<char>(0x00), // message length byte 2
        static_cast<char>(0x00), // message length byte 3
        static_cast<char>(0x00)  // message length byte 4 (MSB)
    };

    // Message #10 Read Log Message (RLM)
    char read_log_message[11] = {
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
    char product_order_message[14] = {
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
    char remote_control_message[13] = {
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

    qint64 sent_bytes;
    char tcp_read_buffer[4096];

    bool is_common_header_read = false;

    // common header
    quint8 message_type;
    quint32 message_length;

    // scm
    quint8 system_control;
    quint8 device_id;

    qint64 bytes_received;

    // wfm
    quint8 command_number;
    quint8 error_code;
    quint8 atomic_execution;
    quint8 device_x_coord;
    quint8 device_y_coord;
    quint8 device_orientation;
    quint8 device_state;
    quint8 extra_data[4096];

    // for inner wfm
    QString extra_data_str;

    // pom
    quint32 product_order_number;

    QGraphicsScene scene;
    QPixmap pixmap_white_background;
    QPixmap pixmap_graphicsview_grid;
    QPixmap pixmap_tables;
    QPixmap pixmap_gopigo_grid;
    QPixmap pixmap_ur5;
    QPixmap pixmap_gopigo_list[4];
    QPixmap pixmap_obstacle_list[42];
    QPixmap pixmap_system_state_red;
    QPixmap pixmap_system_state_green;
    QPixmap pixmap_system_state_blue;
    QPixmap pixmap_system_state_grey;

    QGraphicsPixmapItem* graphics_item_white_background;
    QGraphicsPixmapItem* graphics_item_graphicsview_grid;
    QGraphicsPixmapItem* graphics_item_tables;
    QGraphicsPixmapItem* graphics_item_gopigo_grid;
    QGraphicsPixmapItem* graphics_item_ur5;
    QGraphicsPixmapItem* graphics_item_gopigo[4];
    QGraphicsTextItem* graphics_item_gopigo_id[4];
    QGraphicsPixmapItem* graphics_item_obstacle[42];
    QGraphicsItem* graphics_item_system_state_red;
    QGraphicsItem* graphics_item_system_state_green;
    QGraphicsItem* graphics_item_system_state_blue;
    QGraphicsItem* graphics_item_system_state_grey;

    QFont gpg_id_font;

    void parse_common_header();
    void parse_message();
    void parse_and_view_command();
    void parse_scm();
    void parse_wfm();
    void parse_rlm();
    void parse_pom();
    void parse_rcm();

    void terminal_view_scm();
    void terminal_view_wfm();
    void terminal_view_rlm();
    void server_logs_view_rlm();
    void terminal_view_pom();
    void terminal_view_rcm();
    void update_cursors();

    signals:
        void finished();

    public slots:
        void slot_readyRead_udp();
        void slot_hostFound();
        void slot_connected();
        void slot_disconnected();
        void slot_readyRead_tcp();
        void slot_timeout();

    private slots:
        void on_pbSSM_clicked();
        void on_pbSHM_clicked();
        void on_pbUFM_clicked();
        void on_pbRLM_clicked();
        void on_pbPOM_clicked();
        void on_pbMCM_clicked();
        void on_pbSBMfreeze_clicked();
        void on_read_sbm_triggered();
        void on_connect_to_host_triggered();
        void on_send_ncm_triggered();
        void on_send_ccm_triggered();
        void on_send_sqm_triggered();
};
#endif // QTCLIENT_H
