#ifndef QTCLIENT_H
#define QTCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QtMath>
#include <QHostAddress>
#include "systembroadcastmessage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QtClient; }
QT_END_NAMESPACE

class QtClient : public QMainWindow
{
    Q_OBJECT

public:
    QtClient(QWidget *parent = nullptr,
             quint16 master_tcp_port = 0,
             quint16 local_udp_port = 0);
    ~QtClient();

    void emit_finished();
    void bind();

private:
    Ui::QtClient *ui;
    quint16 master_tcp_port;
    quint16 local_udp_port;

    QTcpSocket tcp_socket;
    QUdpSocket udp_socket;

    QHostAddress qha;
    QString master_ip;
    quint16 master_udp_port;

    qint64 datagram_size;
    char datagram_string[512];

    SystemBroadcastMessage smb;

    bool is_master_known = false;

    char const new_connection_message[11] = {
        static_cast<char>(2),
        static_cast<char>(6),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(1),
        static_cast<char>(255),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(1)
    };

    qint64 sent_bytes;
    char tcp_read_buffer[512];

    bool is_common_header_read = false;

    // common header
    quint8 message_type;
    quint32 message_length;

    // scm
    quint8 system_control;
    quint8 device_id;

    qint64 bytes_received;

    QString gui_console;

    signals:
        void finished();
        void master_found();
        void tcp_message_received();

    public slots:
        void slot_readyRead_udp();
        void slot_connect_master();
        void slot_hostFound();
        void slot_connected();
        void slot_disconnected();
        void slot_readyRead_tcp();
};
#endif // QTCLIENT_H
