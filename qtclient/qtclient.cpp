#include "qtclient.h"
#include "ui_qtclient.h"

QtClient::QtClient(QWidget *parent, quint16 tcp_port_to_master, quint16 udp_port_from_master) :
    QMainWindow(parent), ui(new Ui::QtClient), tcp_port_to_master(tcp_port_to_master), udp_port_from_master(udp_port_from_master)
{
    ui->setupUi(this);

    // rlm
    ui->spinBox_mode->setMinimum(0);
    ui->spinBox_mode->setMaximum(1);
    ui->spinBox_mode->setValue(1);
    ui->spinBox_row_count->setMinimum(1);
    ui->spinBox_row_count->setMaximum(255);
    ui->spinBox_row_count->setValue(11);
    ui->spinBox_beginning_row->setMinimum(0);
    ui->spinBox_beginning_row->setMaximum(255);
    ui->spinBox_beginning_row->setValue(10);

    // pom
    ui->spinBox_product_id->setMinimum(0);
    ui->spinBox_product_id->setMaximum(255);

    // mcm (rcm)
    ui->spinBox_control_device_id->setMinimum(1);
    ui->spinBox_control_device_id->setMaximum(9);
    ui->spinBox_control_flag->setMinimum(0);
    ui->spinBox_control_flag->setMaximum(1);
    ui->spinBox_direction->setMinimum(0);
    ui->spinBox_direction->setMaximum(4);

    ui->graphicsView->setScene(&graphics.scene);
    ui->graphicsView->show();

    ui->terminal->append(QString("Qt Client port bindings:"));
    ui->terminal->append(QString("TCP port to master: ") + QString::number(tcp_port_to_master));
    ui->terminal->append(QString("UDP port from master: ") + QString::number(udp_port_from_master));

    connect(&udp_socket,    SIGNAL(readyRead()),                   this, SLOT(slot_readyRead_udp()));
    connect(&tcp_socket,    SIGNAL(hostFound()),                   this, SLOT(slot_hostFound()));
    connect(&tcp_socket,    SIGNAL(connected()),                   this, SLOT(slot_connected()));
    connect(&tcp_socket,    SIGNAL(disconnected()),                this, SLOT(slot_disconnected()));
    connect(&tcp_socket,    SIGNAL(readyRead()),                   this, SLOT(slot_readyRead_tcp()));
    connect(&timer,         SIGNAL(timeout()),                     this, SLOT(slot_timeout()));

    timer.setTimerType(Qt::VeryCoarseTimer);
    timer.setInterval(5000);
}

QtClient::~QtClient()
{
    delete ui;
}

void QtClient::bind()
{
    bool udp_binding_result = udp_socket.bind(udp_port_from_master, QAbstractSocket::ReuseAddressHint);
    ui->terminal->append(QString("UDP binding result: ") + QString::number(udp_binding_result) + QString("\n"));
}

void QtClient::slot_readyRead_udp() // 1
{
    timer.stop();

    datagram_size = udp_socket.pendingDatagramSize();
    udp_socket.readDatagram(datagram, datagram_size, &qha, &master_udp_port);

    master_ip = qha.toString().remove(0, 7);

    sbm = SystemBroadcastMessage(datagram, datagram_size);

    graphics.hide_all_obstacles();
    graphics.show_found_obstacles(sbm.get_obstacle_count(), sbm.get_obstacle_list());
    graphics.update_gopigo_locations(sbm.get_device_count(), sbm.get_device_list());
    graphics.update_system_state_indicator(sbm.get_state());

    timer.start();
}

void QtClient::slot_timeout()
{
    graphics.hide_all_gopigos(sbm.get_device_count(), sbm.get_device_list());
    graphics.hide_all_obstacles();

    // reset sbm and host info
    sbm.reset();
    master_ip = "";
    master_udp_port = 0;

    graphics.update_system_state_indicator(sbm.get_state());

    timer.stop();
}

void QtClient::on_read_sbm_triggered() // 1
{
    if(sbm.get_str() != "" && master_ip != "" && master_udp_port != 0)
    {
        ui->terminal->append(sbm.get_str());
        ui->terminal->append(QString("Master advertized at: ") + master_ip + QString(":") + QString::number(master_udp_port));
    }
    else
    {
        ui->terminal->append("no broadcast message");
    }
}

void QtClient::on_pbSBMfreeze_clicked() // 1
{
    sent_bytes = udp_socket.writeDatagram(message.sbm_freeze, 8,  QHostAddress::Broadcast, 1732);
    ui->terminal->append(QString("SENT (")                        + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("SBM(FREEZE): {"));
    ui->terminal->append(QString("  constant(lsb): ")             + QString::number(static_cast<quint8>(message.sbm_freeze[0])));
    ui->terminal->append(QString("  constant(msb): ")             + QString::number(static_cast<quint8>(message.sbm_freeze[1])));
    ui->terminal->append(QString("  state: ")                     + QString::number(static_cast<quint8>(message.sbm_freeze[2])));
    ui->terminal->append(QString("  master id: ")                 + QString::number(static_cast<quint8>(message.sbm_freeze[3])));
    ui->terminal->append(QString("  map height: ")                + QString::number(static_cast<quint8>(message.sbm_freeze[4])));
    ui->terminal->append(QString("  map width: ")                 + QString::number(static_cast<quint8>(message.sbm_freeze[5])));
    ui->terminal->append(QString("  obstacle count: ")            + QString::number(static_cast<quint8>(message.sbm_freeze[6])));
    ui->terminal->append(QString("  device count: ")              + QString::number(static_cast<quint8>(message.sbm_freeze[7])));
    ui->terminal->append(QString("}\n"));
}

void QtClient::on_connect_to_host_triggered()
{
    if(master_ip != "")
    {
        ui->terminal->append(QString("Performing host lookup on: ") + master_ip + QString(":") + QString::number(tcp_port_to_master));
        tcp_socket.connectToHost(master_ip, tcp_port_to_master, QTcpSocket::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
    }
    else
    {
        ui->terminal->append(QString("no known host"));
    }
}

void QtClient::on_send_ncm_triggered() // 2
{
    sent_bytes = tcp_socket.write(message.new_connection, 11);

    if(sent_bytes == -1)
    {
        ui->terminal->append(QString("connect to host first"));
    }
    else
    {
        ui->terminal->append(QString("SENT (")                        + QString::number(sent_bytes) + QString(" bytes)"));
        ui->terminal->append(QString("NCM: {"));
        ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(message.new_connection[0])));
        ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(message.new_connection[1])));
        ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(message.new_connection[2])));
        ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(message.new_connection[3])));
        ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(message.new_connection[4])));
        ui->terminal->append(QString("  device_type: ")               + QString::number(static_cast<quint8>(message.new_connection[5])));
        ui->terminal->append(QString("  device_id: ")                 + QString::number(static_cast<quint8>(message.new_connection[6])));
        ui->terminal->append(QString("  device_x_coordinate: ")       + QString::number(static_cast<quint8>(message.new_connection[7])));
        ui->terminal->append(QString("  device_y_coordinate: ")       + QString::number(static_cast<quint8>(message.new_connection[8])));
        ui->terminal->append(QString("  device_orientation: ")        + QString::number(static_cast<quint8>(message.new_connection[9])));
        ui->terminal->append(QString("  device_state: ")              + QString::number(static_cast<quint8>(message.new_connection[10])));
        ui->terminal->append(QString("}\n"));
        is_connected = true;
    }
}

void QtClient::on_send_ccm_triggered()  // 5
{
    sent_bytes = tcp_socket.write(message.disconnection, 5);

    if(sent_bytes == -1)
    {
        ui->terminal->append(QString("connect to master first"));
    }
    else
    {
        ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
        ui->terminal->append(QString("CCM: {"));
        ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(message.disconnection[0])));
        ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(message.disconnection[1])));
        ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(message.disconnection[2])));
        ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(message.disconnection[3])));
        ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(message.disconnection[4])));
        ui->terminal->append(QString("}\n"));
    }
}

void QtClient::on_send_sqm_triggered() // 6
{
    sent_bytes = tcp_socket.write(message.status_query, 5);

    if(sent_bytes == -1)
    {
        ui->terminal->append(QString("connect to master first"));
    }
    else
    {
        ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
        ui->terminal->append(QString("SQM: {"));
        ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(message.status_query[0])));
        ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(message.status_query[1])));
        ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(message.status_query[2])));
        ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(message.status_query[3])));
        ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(message.status_query[4])));
        ui->terminal->append(QString("}\n"));
    }
}

void QtClient::on_pbSSM_clicked() // 7
{
    sent_bytes = tcp_socket.write(message.system_startup, 5);
    if(sent_bytes == -1)
    {
        ui->terminal->append(QString("connect to master first"));
    }
    else
    {
        ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
        ui->terminal->append(QString("SSM: {"));
        ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(message.system_startup[0])));
        ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(message.system_startup[1])));
        ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(message.system_startup[2])));
        ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(message.system_startup[3])));
        ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(message.system_startup[4])));
        ui->terminal->append(QString("}\n"));
    }
}

void QtClient::on_pbSHM_clicked() // 8
{
    sent_bytes = tcp_socket.write(message.system_shutdown, 5);
    if(sent_bytes == -1)
    {
        ui->terminal->append(QString("connect to master first"));
    }
    else
    {
        ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
        ui->terminal->append(QString("SHM: {"));
        ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(message.system_shutdown[0])));
        ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(message.system_shutdown[1])));
        ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(message.system_shutdown[2])));
        ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(message.system_shutdown[3])));
        ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(message.system_shutdown[4])));
        ui->terminal->append(QString("}\n"));
    }
}

void QtClient::on_pbUFM_clicked() // 9
{
    sent_bytes = tcp_socket.write(message.unfreeze, 5);
    if(sent_bytes == -1)
    {
        ui->terminal->append(QString("connect to master first"));
    }
    else
    {
        ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
        ui->terminal->append(QString("SHM: {"));
        ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(message.unfreeze[0])));
        ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(message.unfreeze[1])));
        ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(message.unfreeze[2])));
        ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(message.unfreeze[3])));
        ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(message.unfreeze[4])));
        ui->terminal->append(QString("}\n"));
    }
}

void QtClient::on_pbRLM_clicked() // 10
{
    int beginning_row_number = ui->spinBox_beginning_row->value();
    message.read_log[5] = static_cast<char>(ui->spinBox_mode->value());
    message.read_log[6] = static_cast<char>(ui->spinBox_row_count->value());
    message.read_log[7] = static_cast<char>(beginning_row_number & 0xff);
    message.read_log[8] = static_cast<char>(beginning_row_number >> 8);
    message.read_log[9] = static_cast<char>(beginning_row_number >> 16);
    message.read_log[10] = static_cast<char>(beginning_row_number >> 24);
    sent_bytes = tcp_socket.write(message.read_log, 11);

    if(sent_bytes == -1)
    {
        ui->terminal->append(QString("connect to master first"));
    }
    else
    {
        ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
        ui->terminal->append(QString("RLM: {"));
        ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(message.read_log[0])));
        ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(message.read_log[1])));
        ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(message.read_log[2])));
        ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(message.read_log[3])));
        ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(message.read_log[4])));
        ui->terminal->append(QString("  mode: ")                      + QString::number(static_cast<quint8>(message.read_log[5])));
        ui->terminal->append(QString("  row_count: ")                 + QString::number(static_cast<quint8>(message.read_log[6])));
        ui->terminal->append(QString("  beginning_row_number(lsb): ") + QString::number(static_cast<quint8>(message.read_log[7])));
        ui->terminal->append(QString("  beginning_row_number: ")      + QString::number(static_cast<quint8>(message.read_log[8])));
        ui->terminal->append(QString("  beginning_row_number: ")      + QString::number(static_cast<quint8>(message.read_log[9])));
        ui->terminal->append(QString("  beginning_row_number(msb): ") + QString::number(static_cast<quint8>(message.read_log[10])));
        ui->terminal->append(QString("}\n"));
    }
}

// tähän
void QtClient::on_pbPOM_clicked() // 11
{
    quint8 i = 6;
    bool send = false;

    message.product_order[5] = static_cast<char>(ui->spinBox_product_id->value());
    if(ui->checkBox_order_destination_9_3->isChecked()){
        message.product_order[i++] = static_cast<char>(9);
        message.product_order[i++] = static_cast<char>(3);
        send = true;
    }
    if(ui->checkBox_order_destination_9_2->isChecked()){
        message.product_order[i++] = static_cast<char>(9);
        message.product_order[i++] = static_cast<char>(2);
        send = true;
    }
    if(ui->checkBox_order_destination_9_1->isChecked()){
        message.product_order[i++] = static_cast<char>(9);
        message.product_order[i++] = static_cast<char>(1);
        send = true;
    }
    if(ui->checkBox_order_destination_9_0->isChecked()){
        message.product_order[i++] = static_cast<char>(9);
        message.product_order[i++] = static_cast<char>(0);
        send = true;
    }
    if(send)
    {
        message.product_order[1] = static_cast<char>(i - 5);
        sent_bytes = tcp_socket.write(message.product_order, i);
        if(sent_bytes == -1)
        {
            ui->terminal->append(QString("connect to master first"));
        }
        else
        {
            ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
            ui->terminal->append(QString("POM: {"));
            ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(message.product_order[0])));
            ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(message.product_order[1])));
            ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(message.product_order[2])));
            ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(message.product_order[3])));
            ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(message.product_order[4])));
            ui->terminal->append(QString("  product_id: ")                + QString::number(static_cast<quint8>(message.product_order[5])));
            ui->terminal->append(QString("  destination_coords: {"));
            for(quint8 j = 6; j < i; j += 2)
            {
                ui->terminal->append(QString("    x: ")                   + QString::number(static_cast<quint8>(message.product_order[j])));
                ui->terminal->append(QString("    y: ")                   + QString::number(static_cast<quint8>(message.product_order[j+1])));
            }
            ui->terminal->append(QString("  }"));
            ui->terminal->append(QString("}\n"));
        }
    }
}

void QtClient::on_pbMCM_clicked() // 13, 14
{
    message.remote_control[5] = static_cast<char>(ui->spinBox_control_device_id->value());
    message.remote_control[6] = static_cast<char>(ui->spinBox_control_flag->value());
    message.remote_control[12] = static_cast<char>(ui->spinBox_direction->value());
    sent_bytes = tcp_socket.write(message.remote_control, 13);

    if(sent_bytes == -1)
    {
        ui->terminal->append(QString("connect to master first"));
    }
    else
    {
        ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
        ui->terminal->append(QString("RCM: {"));
        ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(message.remote_control[0])));
        ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(message.remote_control[1])));
        ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(message.remote_control[2])));
        ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(message.remote_control[3])));
        ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(message.remote_control[4])));
        ui->terminal->append(QString("  control_device_id: ")         + QString::number(static_cast<quint8>(message.remote_control[5])));
        ui->terminal->append(QString("  control_flag: ")              + QString::number(static_cast<quint8>(message.remote_control[6])));
        ui->terminal->append(QString("  MCM: {"));
        ui->terminal->append(QString("   message_type: ")               + QString::number(static_cast<quint8>(message.remote_control[7])));
        ui->terminal->append(QString("   message_length_byte1(lsb): ")  + QString::number(static_cast<quint8>(message.remote_control[8])));
        ui->terminal->append(QString("   message_length_byte2: ")       + QString::number(static_cast<quint8>(message.remote_control[9])));
        ui->terminal->append(QString("   message_length_byte3: ")       + QString::number(static_cast<quint8>(message.remote_control[10])));
        ui->terminal->append(QString("   message_length_byte4(msb): ")  + QString::number(static_cast<quint8>(message.remote_control[11])));
        ui->terminal->append(QString("   direction: ")                  + QString::number(static_cast<quint8>(message.remote_control[12])));
        ui->terminal->append(QString("  }"));
        ui->terminal->append(QString("}\n"));
    }
}


void QtClient::slot_hostFound()
{
    ui->terminal->append(QString("Host lookup successful."));
}

void QtClient::slot_connected()
{
    ui->terminal->append(QString("Connected.\n"));
}

void QtClient::slot_disconnected()
{
    ui->terminal->append(QString("Disconnected.\n"));
    is_connected = false;
}

void QtClient::slot_readyRead_tcp()
{
    // if common header is not known
    // with 5 or more bytes we can check out the message type and message length
    bytes_received = tcp_socket.bytesAvailable();
    if (bytes_received >= 5 && !is_common_header_read)
    {
        // finds message_type and message_length from common header
        parse_common_header();
    }

    // if common header is known we just wait for the data to arrive
    if(is_common_header_read)
    {
        bytes_received = tcp_socket.bytesAvailable();
        if(bytes_received >= message_length)
        {
            parse_message();
        }
    }
}

void QtClient::parse_common_header()
{
    // read common header
    tcp_socket.read(tcp_read_buffer, 5);
    message_type = static_cast<quint8>(tcp_read_buffer[0]);
    message_length = 0;
    for(quint8 i = 4; i > 0; --i)
    {
        message_length <<= 8;
        message_length |= static_cast<quint8>(tcp_read_buffer[i]);
    }
    is_common_header_read = true;
}

void QtClient::parse_message()
{
    tcp_socket.read(tcp_read_buffer, message_length);
    if(message_type == 3) // SCM
    {
        parse_scm();
        terminal_view_scm();
    }
    else if(message_type == 4) // WFM
    {
        extra_data_str = ""; // inner wfm storage
        parse_wfm(); // finds command number
        parse_and_view_command();
        terminal_view_wfm();
    }
    update_cursors();
    is_common_header_read = false; // done with the message
}

void QtClient::parse_and_view_command()
{
    if(command_number == 10) // RLM
    {
        parse_rlm();
        server_logs_view_rlm();
    }
    else if(command_number == 11) // POM
    {
        parse_pom();
        terminal_view_pom();
    }
    else if(command_number == 14) // RCM
    {
        parse_rcm();
        terminal_view_rcm();
    }
}

void QtClient::parse_scm()
{
    system_control          = static_cast<quint8>(tcp_read_buffer[0]);
    device_id               = static_cast<quint8>(tcp_read_buffer[1]);
}

void QtClient::parse_wfm()
{
    command_number          = static_cast<quint8>(tcp_read_buffer[0]);
    error_code              = static_cast<quint8>(tcp_read_buffer[1]);
    atomic_execution        = static_cast<quint8>(tcp_read_buffer[2]);
    device_x_coord          = static_cast<quint8>(tcp_read_buffer[3]);
    device_y_coord          = static_cast<quint8>(tcp_read_buffer[4]);
    device_orientation      = static_cast<quint8>(tcp_read_buffer[5]);
    device_state            = static_cast<quint8>(tcp_read_buffer[6]);
}

void QtClient::parse_rlm()
{
    for(quint16 i = 7; i < message_length; ++i)
    {
        extra_data[i-7] = static_cast<quint8>(tcp_read_buffer[i]);
        extra_data_str += QString(tcp_read_buffer[i]);
    }
}

void QtClient::parse_pom()
{
    extra_data[0] = static_cast<quint8>(tcp_read_buffer[10]);
    extra_data[1] = static_cast<quint8>(tcp_read_buffer[9]);
    extra_data[2] = static_cast<quint8>(tcp_read_buffer[8]);
    extra_data[3] = static_cast<quint8>(tcp_read_buffer[7]);
    product_order_number = extra_data[0];
    product_order_number <<= 8;
    product_order_number |= extra_data[1];
    product_order_number <<= 8;
    product_order_number |= extra_data[2];
    product_order_number <<= 8;
    product_order_number |= extra_data[3];
}

void QtClient::parse_rcm()
{
    extra_data[0] = static_cast<quint8>(tcp_read_buffer[7]);    // messag type
    extra_data[1] = static_cast<quint8>(tcp_read_buffer[8]);    // message_length_byte_1 (lsb)
    extra_data[2] = static_cast<quint8>(tcp_read_buffer[9]);    // message_length_byte_2
    extra_data[3] = static_cast<quint8>(tcp_read_buffer[10]);   // message_length_byte_3
    extra_data[4] = static_cast<quint8>(tcp_read_buffer[11]);   // message_length_byte_4 (msb)
    extra_data[5] = static_cast<quint8>(tcp_read_buffer[12]);   // command number
    extra_data[6] = static_cast<quint8>(tcp_read_buffer[13]);   // error_code
    extra_data[7] = static_cast<quint8>(tcp_read_buffer[14]);   // atomic_execution
    extra_data[8] = static_cast<quint8>(tcp_read_buffer[15]);   // device_x_coord
    extra_data[9] = static_cast<quint8>(tcp_read_buffer[16]);   // device_y_coord
    extra_data[10] = static_cast<quint8>(tcp_read_buffer[17]);  // device_orientation
    extra_data[11] = static_cast<quint8>(tcp_read_buffer[18]);  // device_state
    extra_data[12] = static_cast<quint8>(tcp_read_buffer[19]);  // extra_data (1 byte)
}

void QtClient::terminal_view_scm()
{
    ui->terminal->append(QString("RECEIVED (")         + QString::number(bytes_received) + QString(" bytes)"));
    ui->terminal->append(QString("SCM: {"));
    ui->terminal->append(QString("  system_control: ") + QString::number(system_control) + QString(","));
    ui->terminal->append(QString("  device_id: ")      + QString::number(device_id));
    ui->terminal->append(QString("}\n"));
}

void QtClient::terminal_view_wfm()
{
    extra_data_str += "\n  }";
    ui->terminal->append(QString("RECEIVED (") + QString::number(bytes_received) + QString(" bytes)"));
    ui->terminal->append(QString("WFM: {"));
    ui->terminal->append(QString("  command_number: ")      + QString::number(command_number)      + QString(","));
    ui->terminal->append(QString("  error_code: ")          + QString::number(error_code)          + QString(","));
    ui->terminal->append(QString("  atomic_execution: ")    + QString::number(atomic_execution)    + QString(","));
    ui->terminal->append(QString("  device_x_coord: ")      + QString::number(device_x_coord)      + QString(","));
    ui->terminal->append(QString("  device_y_coord: ")      + QString::number(device_y_coord)      + QString(","));
    ui->terminal->append(QString("  device_orientation: ")  + QString::number(device_orientation)  + QString(","));
    ui->terminal->append(QString("  device_state: ")        + QString::number(device_state)        + QString(","));
    ui->terminal->append(QString("  extra_data: {\n")       + extra_data_str);
    ui->terminal->append(QString("}\n"));
}

void QtClient::server_logs_view_rlm()
{
    qint16 rlm_length = static_cast<qint16>(message_length-7);
    QString rlm;
    for(qint16 i = 0; i < rlm_length; ++i)
    {
        rlm += static_cast<char>(extra_data[i]);
    }

    ui->server_logs->setText(QString("Server Logs:\n") + rlm);
}

void QtClient::terminal_view_pom()
{
    extra_data_str += "    product_order_number: " + QString::number(product_order_number, 16);
}

void QtClient::terminal_view_rcm()
{
    extra_data_str += "    WFM: {";
    extra_data_str += "    message_type: "                + QString::number(extra_data[0]) + ",\n";
    extra_data_str += "    message_length_byte_1(lsb): "  + QString::number(extra_data[1]) + ",\n";
    extra_data_str += "    message_length_byte_2: "       + QString::number(extra_data[2]) + ",\n";
    extra_data_str += "    message_length_byte_3: "       + QString::number(extra_data[3]) + ",\n";
    extra_data_str += "    message_length_byte_4(msb): "  + QString::number(extra_data[4]) + ",\n";
    extra_data_str += "    command_number: "     + QString::number(extra_data[5]) + ",\n";
    extra_data_str += "    error_code: "         + QString::number(extra_data[6]) + ",\n";
    extra_data_str += "    atomic_execution: "   + QString::number(extra_data[7]) + ",\n";
    extra_data_str += "    device_x_coord: "     + QString::number(extra_data[8]) + ",\n";
    extra_data_str += "    device_y_coord: "     + QString::number(extra_data[9]) + ",\n";
    extra_data_str += "    device_orientation: " + QString::number(extra_data[10]) + ",\n";
    extra_data_str += "    device_state: "       + QString::number(extra_data[11]) + ",\n";
    extra_data_str += "    extra_data {: "       + QString::number(extra_data[12]);
    // call to terminal_view_wfm() in parse_message() completes the view
}

void QtClient::update_cursors()
{
    // move cursor to the bottom of the terminal
    QTextCursor terminal_cursor = ui->terminal->textCursor();
    QString terminal_text = ui->terminal->toPlainText();
    terminal_cursor.setPosition(terminal_text.length());
    ui->terminal->setTextCursor(terminal_cursor);

    // move cursor to the bottom of the server_logs
    QTextCursor server_logs_cursor = ui->server_logs->textCursor();
    QString server_logs_text = ui->server_logs->toPlainText();
    server_logs_cursor.setPosition(server_logs_text.length());
    ui->server_logs->setTextCursor(server_logs_cursor);
}

void QtClient::emit_finished()
{
    emit finished();
}
