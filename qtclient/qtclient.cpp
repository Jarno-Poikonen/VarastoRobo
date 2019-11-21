#include "qtclient.h"
#include "ui_qtclient.h"

QtClient::QtClient(QWidget *parent, quint16 master_tcp_port, quint16 local_udp_port) :
    QMainWindow(parent), ui(new Ui::QtClient), master_tcp_port(master_tcp_port), local_udp_port(local_udp_port)
{
    ui->setupUi(this);
    connect(&udp_socket,    SIGNAL(readyRead()),                   this, SLOT(slot_readyRead_udp()));
    connect(this,           SIGNAL(master_found()),                this, SLOT(slot_connect_master()));
    connect(&tcp_socket,    SIGNAL(hostFound()),                   this, SLOT(slot_hostFound()));
    connect(&tcp_socket,    SIGNAL(connected()),                   this, SLOT(slot_connected()));
    connect(&tcp_socket,    SIGNAL(disconnected()),                this, SLOT(slot_disconnected()));
    connect(&tcp_socket,    SIGNAL(readyRead()),                   this, SLOT(slot_readyRead_tcp()));

    qDebug() << "master_tcp_port number" << master_tcp_port;
    qDebug() << "local_udp_port number"  << local_udp_port;
    ui->textEdit->append(QString("master tcp port number: ") + QString::number(master_tcp_port));
    ui->textEdit->append(QString("local udp port number: ") + QString::number(local_udp_port));
}

QtClient::~QtClient()
{
    delete ui;
}

void QtClient::bind()
{
    bool udp_binding_result = udp_socket.bind(local_udp_port);

    qDebug() << "udp_binding_result:" << udp_binding_result;
    ui->textEdit->append(QString("udp_binding_result: ") + QString::number(udp_binding_result) + QString("\n"));
}

void QtClient::emit_finished()
{
    emit finished();
}

void QtClient::slot_readyRead_udp()
{
    datagram_size = udp_socket.pendingDatagramSize();
    if(datagram_size != -1)
    {
        udp_socket.readDatagram(datagram_string, datagram_size, &qha, &master_udp_port);
        master_ip = qha.toString();
        master_ip.remove(0,7);
        smb.parse_datagram_string(datagram_string, datagram_size);

        if(!is_master_known)
        {
            qDebug() << "Master IP:" << master_ip << ", Master Port:" << master_udp_port << "\n" << smb.str;
            qDebug() << "datagram size:" << datagram_size;
            ui->textEdit->append(QString("Master IP: ") + master_ip);
            ui->textEdit->append(QString("Master Port: ") + QString::number(master_udp_port));
            ui->textEdit->append(QString("System Broadcast Message (datagram): ") + QString::number(datagram_size));
            ui->textEdit->append(QString("System Broadcast Message (parsed): ") + smb.str + QString("\n"));
            is_master_known = true;
            emit master_found();
        }
    }
}

void QtClient::slot_connect_master()
{
    qDebug("Looking for host.");
    ui->textEdit->append(QString("Looking for host."));
    tcp_socket.connectToHost(master_ip, master_tcp_port, QTcpSocket::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
}

void QtClient::slot_hostFound()
{
    qDebug("Host lookup successful.");
    ui->textEdit->append(QString("Host lookup successful."));
}

void QtClient::slot_connected()
{
    qDebug("Connected.");
    ui->textEdit->append(QString("Connected.\n"));
    sent_bytes = tcp_socket.write(new_connection_message, 11);
    qDebug() << "NCM sent (" << sent_bytes << "bytes )";
    ui->textEdit->append(QString("NCM sent (") + QString::number(sent_bytes) + QString(" bytes)\n"));
}

void QtClient::slot_disconnected()
{
    is_master_known = false;
    qDebug() << "Disconnected.";
    ui->textEdit->append(QString("Disconnected."));
}

void QtClient::slot_readyRead_tcp()
{
    // if common header is not known
    // with 5 or more bytes we can check out the message type and message length
    bytes_received = tcp_socket.bytesAvailable();
    qDebug() << "bytes_received" << bytes_received;
    ui->textEdit->append(QString("bytes_received ") + QString::number(bytes_received));
    if (bytes_received >= 5 && !is_common_header_read)
    {
        tcp_socket.read(tcp_read_buffer, 5);
        message_type = static_cast<quint8>(tcp_read_buffer[0]);

        qDebug() << "message_type:"         << static_cast<qint8>(tcp_read_buffer[0]);
        qDebug() << "message_length_byte1:" << static_cast<qint8>(tcp_read_buffer[1]);
        qDebug() << "message_length_byte2:" << static_cast<qint8>(tcp_read_buffer[2]);
        qDebug() << "message_length_byte3:" << static_cast<qint8>(tcp_read_buffer[3]);
        qDebug() << "message_length_byte4:" << static_cast<qint8>(tcp_read_buffer[4]);
        ui->textEdit->append(QString("message_type: ")         + QString::number(static_cast<qint8>(tcp_read_buffer[0])));
        ui->textEdit->append(QString("message_length_byte1: ") + QString::number(static_cast<qint8>(tcp_read_buffer[1])));
        ui->textEdit->append(QString("message_length_byte2: ") + QString::number(static_cast<qint8>(tcp_read_buffer[2])));
        ui->textEdit->append(QString("message_length_byte3: ") + QString::number(static_cast<qint8>(tcp_read_buffer[3])));
        ui->textEdit->append(QString("message_length_byte4: ") + QString::number(static_cast<qint8>(tcp_read_buffer[4])));

        message_length = 0;
        for(quint8 i = 4; i > 0; --i)
        {
            message_length <<= 8;
            message_length |= static_cast<quint8>(tcp_read_buffer[i]);
        }

        qDebug() << "message_length" << message_length;
        ui->textEdit->append(QString("message_length: ") + QString::number(message_length) + QString("\n"));

        is_common_header_read = true;

    }

    // if common header is known we just wait for the data to arrive
    if(is_common_header_read && tcp_socket.bytesAvailable() >= message_length)
    {
        tcp_socket.read(tcp_read_buffer, message_length);
        if(message_type == 3) // SCM
        {
            system_control          = static_cast<quint8>(tcp_read_buffer[0]);
            device_id               = static_cast<quint8>(tcp_read_buffer[1]);

            is_common_header_read   = false; // done with a message
            qDebug() << "SCM: {";
            qDebug() << "  system_control:" << system_control << ",";
            qDebug() << "  device_id:" << device_id;
            qDebug() << "}\n";
            ui->textEdit->append(QString("SCM: {"));
            ui->textEdit->append(QString("  system_control: ") + QString::number(system_control) + QString(","));
            ui->textEdit->append(QString("  device_id: ")      + QString::number(device_id));
            ui->textEdit->append(QString("}\n"));
        }
    }
}

