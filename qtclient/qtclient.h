#ifndef QTCLIENT_H
#define QTCLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QtMath>
#include <QHostAddress>
#include <QTimer>
#include "message.h"
#include "graphics.h"
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
    Message message;

    QTimer timer;

    bool is_connected = false;
    bool is_common_header_read = false;
    char tcp_read_buffer[32768];

    qint64 sent_bytes;
    qint64 bytes_received;

    // common header
    quint8 message_type;
    quint32 message_length;

    // scm
    quint8 system_control;
    quint8 device_id;

    // wfm
    quint8 command_number;
    quint8 error_code;
    quint8 atomic_execution;
    quint8 device_x_coord;
    quint8 device_y_coord;
    quint8 device_orientation;
    quint8 device_state;
    quint8 extra_data[32768];

    // for inner wfm
    QString extra_data_str;

    // pom
    quint32 product_order_number;

    Graphics graphics;

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
