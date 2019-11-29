#ifndef QTCLIENT_H
#define QTCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QtMath>
#include <QHostAddress>
#include <QByteArray>
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

    SystemBroadcastMessage smb;

    // ncm 2
    char const new_connection_message[11] = {
        static_cast<char>(2),   // message type
        static_cast<char>(6),   // message length byte1
        static_cast<char>(0),   // message length byte2
        static_cast<char>(0),   // message length byte3
        static_cast<char>(0),   // message length byte4
        static_cast<char>(1),   // device type
        static_cast<char>(255), // device id
        static_cast<char>(0),   // device x coordinate
        static_cast<char>(0),   // device y coordinate
        static_cast<char>(0),   // device orientation
        static_cast<char>(1)    // device state
    };

    // ccm 5
    char const disconnection_message[5] = {
        static_cast<char>(5),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0)
    };

    // sqm 6
    char const status_query_message[5] = {
        static_cast<char>(6),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0)
    };

    // ssm 7
    const char system_startup_message[5] = {
        static_cast<char>(7),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0)
    };

    // shm 8
    char const system_shutdown_message[5] = {
        static_cast<char>(8),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0)
    };

    // ufm 9
    char const unfreeze_message[5] = {
        static_cast<char>(9),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0)
    };

    // rlm 10
    char read_log_message[11] = {
        static_cast<char>(10), // message type               (known)
        static_cast<char>(6),  // message length (lsb)       (known)
        static_cast<char>(0),  // message length             (known)
        static_cast<char>(0),  // message length             (known)
        static_cast<char>(0),  // message length (msb)       (known)
        static_cast<char>(0),  // mode                       (to be determined)
        static_cast<char>(0),  // row count                  (to be determined)
        static_cast<char>(0),  // beginning row number (lsb) (to be determined)
        static_cast<char>(0),  // beginning row number       (to be determined)
        static_cast<char>(0),  // beginning row number       (to be determined)
        static_cast<char>(0)   // beginning row number (msb) (to be determined)
    };

    // pom 11
    char product_order_message[14] = {
      static_cast<char>(11), // message type               (known)
      static_cast<char>(0),  // message length (lsb)       (to be determined)
      static_cast<char>(0),  // message length             (to be determined)
      static_cast<char>(0),  // message length             (to be determined)
      static_cast<char>(0),  // message length (msb)       (to be determined)
      static_cast<char>(0), // product_id (to be determined)
      static_cast<char>(0), // destination 1 x (to be determined)
      static_cast<char>(0), // destination 1 y (to be determined)
      static_cast<char>(0), // destination 2 x (to be determined)
      static_cast<char>(0), // destination 2 y (to be determined)
      static_cast<char>(0), // destination 3 x (to be determined)
      static_cast<char>(0), // destination 3 y (to be determined)
      static_cast<char>(0), // destination 4 x (to be determined)
      static_cast<char>(0), // destination 4 y (to be determined)
    };

    // rcm 14
    char remote_control_message[13] = {
        static_cast<char>(14), // message type (known)
        static_cast<char>(8),  // message length (lsb)       (known, because as of now, MCM is the only RMC command)
        static_cast<char>(0),  // message length             (known, because as of now, MCM is the only RMC command)
        static_cast<char>(0),  // message length             (known, because as of now, MCM is the only RMC command)
        static_cast<char>(0),  // message length (msb)       (known, because as of now, MCM is the only RMC command)
        static_cast<char>(0),  // target_device_id
        static_cast<char>(0),  // control_flag
        static_cast<char>(13), // MCM (message_type)
        static_cast<char>(1),  // MCM (message_length lsb)
        static_cast<char>(0),  // MCM (message_length)
        static_cast<char>(0),  // MCM (message_length)
        static_cast<char>(0),  // MCM (message_length msb)
        static_cast<char>(0)   // MCM (direction)            (determined in the ui)
    };

    // SBM FREEZE
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

    // poista
    quint32 temp;

    signals:
        void finished();

    public slots:
        void slot_readyRead_udp();
        void slot_hostFound();
        void slot_connected();
        void slot_disconnected();
        void slot_readyRead_tcp();

    private slots:
        void on_pbSMBread_clicked();
        void on_pbConnect_clicked();
        void on_pbNCM_clicked();
        void on_pbCCM_clicked();
        void on_pbSQM_clicked();
        void on_pbSSM_clicked();
        void on_pbSHM_clicked();
        void on_pbUFM_clicked();
        void on_pbRLM_clicked();
        void on_pbPOM_clicked();
        void on_pbMCM_clicked();
        void on_pbSMBfreeze_clicked();
};
#endif // QTCLIENT_H
