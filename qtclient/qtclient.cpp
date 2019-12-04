#include "qtclient.h"
#include "ui_qtclient.h"

QtClient::QtClient(QWidget *parent, quint16 tcp_port_to_master, quint16 udp_port_from_master) :
    QMainWindow(parent), ui(new Ui::QtClient), tcp_port_to_master(tcp_port_to_master), udp_port_from_master(udp_port_from_master)
{
    ui->setupUi(this);

    // rlm
    ui->spinBox_mode->setMinimum(0);
    ui->spinBox_mode->setMaximum(1);
    ui->spinBox_row_count->setMinimum(1);
    ui->spinBox_row_count->setMaximum(256);
    ui->spinBox_beginning_row->setMinimum(0);
    ui->spinBox_beginning_row->setMaximum(255);

    // pom
    ui->spinBox_product_id->setMinimum(0);
    ui->spinBox_product_id->setMaximum(255);

    // mcm (rcm)
    ui->spinBox_control_device_id->setMinimum(0); // lopullisessa versiossa 1
    ui->spinBox_control_device_id->setMaximum(254); // lopullisessa versiossa 9
    ui->spinBox_control_flag->setMinimum(0);
    ui->spinBox_control_flag->setMaximum(1);
    ui->spinBox_direction->setMinimum(0);
    ui->spinBox_direction->setMaximum(3);

    connect(&udp_socket,    SIGNAL(readyRead()),                   this, SLOT(slot_readyRead_udp()));
    connect(&tcp_socket,    SIGNAL(hostFound()),                   this, SLOT(slot_hostFound()));
    connect(&tcp_socket,    SIGNAL(connected()),                   this, SLOT(slot_connected()));
    connect(&tcp_socket,    SIGNAL(disconnected()),                this, SLOT(slot_disconnected()));
    connect(&tcp_socket,    SIGNAL(readyRead()),                   this, SLOT(slot_readyRead_tcp()));

    ui->terminal->append(QString("Qt Client port bindings:"));
    ui->terminal->append(QString("TCP port to master: ") + QString::number(tcp_port_to_master));
    ui->terminal->append(QString("UDP port from master: ") + QString::number(udp_port_from_master));


    // LOAD IMAGES FROM RESOURCES:

    // white background :)
    if(pixmap_white_background.load(":/images/white_background", "png"))
    {
        qDebug() << "image load successful (:/images/white_background.png)";
        graphics_item_white_background = scene.addPixmap(pixmap_white_background);
    }
    else
    {
        qDebug() << "image load failure (:/images/white_background.png)";
    }

    // graphics view grid
    if(pixmap_graphicsview_grid.load(":/images/graphicsview_grid", "png"))
    {
        qDebug() << "image load successful (:/images/graphicsview_grid.png)";
        graphics_item_graphicsview_grid = scene.addPixmap(pixmap_graphicsview_grid);
    }
    else
    {
        qDebug() << "image load failure (:/images/graphicsview_grid.png)";
    }

    // tables
    if(pixmap_tables.load(":/images/tables", "png"))
    {
        qDebug() << "image load successful (:/images/tables.png)";
        graphics_item_tables = scene.addPixmap(pixmap_tables);
    }
    else
    {
        qDebug() << "image load failure (:/images/tables.png)";
    }

    // gopigo_grid
    if(pixmap_gopigo_grid.load(":/images/gopigo_grid", "png"))
    {
        qDebug() << "image load successful (:/images/gopigo_grid.png)";
        graphics_item_gopigo_grid = scene.addPixmap(pixmap_gopigo_grid);
    }
    else
    {
        qDebug() << "image load failure (:/images/gopigo_grid.png)";
    }

    // ur5
    if(pixmap_ur5.load(":/images/ur5", "png"))
    {
        qDebug() << "image load successful (:/images/ur5.png)";
        graphics_item_ur5 = scene.addPixmap(pixmap_ur5);
    }
    else
    {
        qDebug() << "image load failure (:/images/ur5.png)";
    }

    // 4 instances of gopigo images
    for(quint8 i = 0; i < 4; ++i)
    {
        if(pixmap_gopigo_list[i].load(":/images/gopigo", "png"))
        {
            qDebug() << "image load successful (:/images/gopigo.png)";
            graphics_item_gopigo[i] = scene.addPixmap(pixmap_gopigo_list[i]);
            graphics_item_gopigo[i]->setPos(280-18, 490-i*70-18);
            qDebug() << "graphics_item_gopigo: " << graphics_item_gopigo[i]->pos();
        }
        else
        {
            qDebug() << "image load failure (:/images/gopigo.png)";
        }
    }

    // 42 istances of obstacle images
    quint8 i = 0;
    for(quint8 y = 0; y < 6; ++y)
    {
        for(quint8 x = 0; x < 9; ++x)
        {
            if( ((y == 5 || y == 4) && (x == 0 || x == 1 || x == 7 || x == 8)) || (x == 4 && !(y == 4 || y == 5)) )
                continue;

            if(pixmap_obstacle_list[i].load(":/images/obstacle", "png"))
            {
                graphics_item_obstacle[i] = scene.addPixmap(pixmap_obstacle_list[i]);
                graphics_item_obstacle[i]->hide();
                graphics_item_obstacle[i]->setPos(280-18 + 70*x, 490-18 - 70*y);
                qDebug() << "image load successful (:/images/obstacle.png)" << "x: " << x << "y:" << y << "graphics_item_obstacle:" << graphics_item_obstacle[i]->pos();
                i++;
            }
            else
            {
                qDebug() << "image load failure (:/images/obstacle.png)";
            }
        }
    }

    scene.setSceneRect(0, 0, 1120, 630);
    graphics_item_white_background->setPos(0, 0);    // occupies 1120x630, the whole graphicsview dimensions
    graphics_item_graphicsview_grid->setPos(0, 0);   // occupies 1120x630, the whole graphicsview dimensions
    graphics_item_tables->setPos(245-1, 105-1);      // (-1 offset due to added border in the image)
    graphics_item_gopigo_grid->setPos(280-1, 140-1); // (-1 offset due to added border in the image)
    graphics_item_ur5->setPos(525-1, 350-1);         // (-1 offset due to added border in the image)


//    graphics_item_obstacle[0]->setPos(280-18, 490-18);
//    graphics_item_obstacle[1]->setPos(280-18, 420-18);
//    graphics_item_obstacle[2]->setPos(280-18, 350-18);
//    graphics_item_obstacle[3]->setPos(280-18, 280-18);


    ui->graphicsView->setScene(&scene);
    ui->graphicsView->show();
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
    datagram_size = udp_socket.pendingDatagramSize();
    if(datagram_size != -1)
    {
        udp_socket.readDatagram(datagram, datagram_size, &qha, &master_udp_port);
        master_ip = qha.toString();
        master_ip.remove(0,7);
        sbm = SystemBroadcastMessage(datagram, datagram_size);
        sbm.parse_datagram();

        quint8 obstacle_index = 0;
        for(quint8 i = 0; i < sbm.obstacle_count; ++i)
        {
            quint8 x = sbm.obstacle_list[i].x;
            quint8 y = sbm.obstacle_list[i].y;

            if( !(((y == 5 || y == 4) && (x == 0 || x == 1 || x == 7 || x == 8)) || (x == 4 && !(y == 4 || y == 5))) )
            {
                if(y <= 3)
                {
                    obstacle_index = y*9 - (y + (x > 4)) + x;
                }
                else if(y == 4)
                {
                    obstacle_index = 32 + x - 2;
                }
                else if(y == 5)
                {
                    obstacle_index = 37 + x - 2;
                }
                graphics_item_obstacle[obstacle_index]->show();
            }
        }

        quint8 gopigo_index = 0;
        for(quint8 i = 0; i < sbm.device_count; ++i)
        {
            if(sbm.device_list[i].type == 2)
            {
                quint8 x = sbm.device_list[i].point.x;
                quint8 y = sbm.device_list[i].point.y;
                graphics_item_gopigo[gopigo_index]->setPos(280-18 + 70*x, 490-18 - 70*y);
                gopigo_index++;
            }
        }

        // get gopigo coords and draw them in the graphicsview
        /*
        quint8 obstacle_count;              // max 54
        quint8 device_count;                // 4x GoPiGo, UR5, Drone, QtClient = 7 (master not included here)
        Point obstacle_list[54];            // x, y // 1, 1 [54*2]
        VarastoRoboDevice device_list[32];  // type, id, x, y, ipv4 // 1, 1, 1, 1, 4 bytes [7*8]
        */
    }
}

void QtClient::on_read_sbm_triggered() // 1
{
    ui->terminal->append(sbm.str);
    ui->terminal->append(QString("Master advertized at: ") + master_ip + QString(":") + QString::number(master_udp_port));
}

void QtClient::on_pbSBMfreeze_clicked() // 1
{
    sent_bytes = udp_socket.writeDatagram(sbm_freeze_message, 8,  QHostAddress::Broadcast, 1732);
    ui->terminal->append(QString("SENT (")                        + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("SBM(FREEZE): {"));
    ui->terminal->append(QString("  constant(lsb): ")             + QString::number(static_cast<quint8>(sbm_freeze_message[0])));
    ui->terminal->append(QString("  constant(msb): ")             + QString::number(static_cast<quint8>(sbm_freeze_message[1])));
    ui->terminal->append(QString("  state: ")                     + QString::number(static_cast<quint8>(sbm_freeze_message[2])));
    ui->terminal->append(QString("  master id: ")                 + QString::number(static_cast<quint8>(sbm_freeze_message[3])));
    ui->terminal->append(QString("  map height: ")                + QString::number(static_cast<quint8>(sbm_freeze_message[4])));
    ui->terminal->append(QString("  map width: ")                 + QString::number(static_cast<quint8>(sbm_freeze_message[5])));
    ui->terminal->append(QString("  obstacle count: ")            + QString::number(static_cast<quint8>(sbm_freeze_message[6])));
    ui->terminal->append(QString("  device count: ")              + QString::number(static_cast<quint8>(sbm_freeze_message[7])));
    ui->terminal->append(QString("}\n"));
}

void QtClient::on_connect_to_host_triggered()
{
    ui->terminal->append(QString("Performing host lookup on: ") + master_ip + QString(":") + QString::number(tcp_port_to_master));
    tcp_socket.connectToHost(master_ip, tcp_port_to_master, QTcpSocket::ReadWrite, QAbstractSocket::NetworkLayerProtocol::IPv4Protocol);
}

void QtClient::on_send_ncm_triggered() // 2
{
    sent_bytes = tcp_socket.write(new_connection_message, 11);

    ui->terminal->append(QString("SENT (")                        + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("NCM: {"));
    ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(new_connection_message[0])));
    ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(new_connection_message[1])));
    ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(new_connection_message[2])));
    ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(new_connection_message[3])));
    ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(new_connection_message[4])));
    ui->terminal->append(QString("  device_type: ")               + QString::number(static_cast<quint8>(new_connection_message[5])));
    ui->terminal->append(QString("  device_id: ")                 + QString::number(static_cast<quint8>(new_connection_message[6])));
    ui->terminal->append(QString("  device_x_coordinate: ")       + QString::number(static_cast<quint8>(new_connection_message[7])));
    ui->terminal->append(QString("  device_y_coordinate: ")       + QString::number(static_cast<quint8>(new_connection_message[8])));
    ui->terminal->append(QString("  device_orientation: ")        + QString::number(static_cast<quint8>(new_connection_message[9])));
    ui->terminal->append(QString("  device_state: ")              + QString::number(static_cast<quint8>(new_connection_message[10])));
    ui->terminal->append(QString("}\n"));
}

void QtClient::on_send_ccm_triggered()  // 5
{
    sent_bytes = tcp_socket.write(disconnection_message, 5);

    ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("CCM: {"));
    ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(disconnection_message[0])));
    ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(disconnection_message[1])));
    ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(disconnection_message[2])));
    ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(disconnection_message[3])));
    ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(disconnection_message[4])));
    ui->terminal->append(QString("}\n"));
}

void QtClient::on_send_sqm_triggered() // 6
{
    sent_bytes = tcp_socket.write(status_query_message, 5);

    ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("SQM: {"));
    ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(status_query_message[0])));
    ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(status_query_message[1])));
    ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(status_query_message[2])));
    ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(status_query_message[3])));
    ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(status_query_message[4])));
    ui->terminal->append(QString("}\n"));
}

void QtClient::on_pbSSM_clicked() // 7
{
    sent_bytes = tcp_socket.write(system_startup_message, 5);
    ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("SSM: {"));
    ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(system_startup_message[0])));
    ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(system_startup_message[1])));
    ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(system_startup_message[2])));
    ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(system_startup_message[3])));
    ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(system_startup_message[4])));
    ui->terminal->append(QString("}\n"));
}

void QtClient::on_pbSHM_clicked() // 8
{
    sent_bytes = tcp_socket.write(system_shutdown_message, 5);
    ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("SHM: {"));
    ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(system_shutdown_message[0])));
    ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(system_shutdown_message[1])));
    ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(system_shutdown_message[2])));
    ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(system_shutdown_message[3])));
    ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(system_shutdown_message[4])));
    ui->terminal->append(QString("}\n"));
}

void QtClient::on_pbUFM_clicked() // 9
{
    sent_bytes = tcp_socket.write(unfreeze_message, 5);
    ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("SHM: {"));
    ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(unfreeze_message[0])));
    ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(unfreeze_message[1])));
    ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(unfreeze_message[2])));
    ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(unfreeze_message[3])));
    ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(unfreeze_message[4])));
    ui->terminal->append(QString("}\n"));
}

void QtClient::on_pbRLM_clicked() // 10
{
    int beginning_row_number = ui->spinBox_beginning_row->value();
    read_log_message[5] = static_cast<char>(ui->spinBox_mode->value());
    read_log_message[6] = static_cast<char>(ui->spinBox_row_count->value());
    read_log_message[7] = static_cast<char>(beginning_row_number & 0xff);
    read_log_message[8] = static_cast<char>(beginning_row_number >> 8);
    read_log_message[9] = static_cast<char>(beginning_row_number >> 16);
    read_log_message[10] = static_cast<char>(beginning_row_number >> 24);
    sent_bytes = tcp_socket.write(read_log_message, 11);
    ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("RLM: {"));
    ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(read_log_message[0])));
    ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(read_log_message[1])));
    ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(read_log_message[2])));
    ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(read_log_message[3])));
    ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(read_log_message[4])));
    ui->terminal->append(QString("  mode: ")                      + QString::number(static_cast<quint8>(read_log_message[5])));
    ui->terminal->append(QString("  row_count: ")                 + QString::number(static_cast<quint8>(read_log_message[6])));
    ui->terminal->append(QString("  beginning_row_number(lsb): ") + QString::number(static_cast<quint8>(read_log_message[7])));
    ui->terminal->append(QString("  beginning_row_number: ")      + QString::number(static_cast<quint8>(read_log_message[8])));
    ui->terminal->append(QString("  beginning_row_number: ")      + QString::number(static_cast<quint8>(read_log_message[9])));
    ui->terminal->append(QString("  beginning_row_number(msb): ") + QString::number(static_cast<quint8>(read_log_message[10])));
    ui->terminal->append(QString("}\n"));
}

void QtClient::on_pbPOM_clicked() // 11
{
    quint8 i = 6;
    bool send = false;

    product_order_message[5] = static_cast<char>(ui->spinBox_product_id->value());
    if(ui->checkBox_order_destination_8_0->isChecked()){
        product_order_message[i++] = static_cast<char>(8);
        product_order_message[i++] = static_cast<char>(0);
        send = true;
    }
    if(ui->checkBox_order_destination_8_1->isChecked()){
        product_order_message[i++] = static_cast<char>(8);
        product_order_message[i++] = static_cast<char>(1);
        send = true;
    }
    if(ui->checkBox_order_destination_8_2->isChecked()){
        product_order_message[i++] = static_cast<char>(8);
        product_order_message[i++] = static_cast<char>(2);
        send = true;
    }
    if(ui->checkBox_order_destination_8_3->isChecked()){
        product_order_message[i++] = static_cast<char>(8);
        product_order_message[i++] = static_cast<char>(3);
        send = true;
    }
    if(send)
    {
        product_order_message[1] = static_cast<char>(i - 5);
        sent_bytes = tcp_socket.write(product_order_message, i);
        ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
        ui->terminal->append(QString("POM: {"));
        ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(product_order_message[0])));
        ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(product_order_message[1])));
        ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(product_order_message[2])));
        ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(product_order_message[3])));
        ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(product_order_message[4])));
        ui->terminal->append(QString("  product_id: ")                + QString::number(static_cast<quint8>(product_order_message[5])));
        ui->terminal->append(QString("  destination_coords: {"));
        for(quint8 j = 6; j < i; j += 2)
        {
            ui->terminal->append(QString("    x: ")                   + QString::number(static_cast<quint8>(product_order_message[j])));
            ui->terminal->append(QString("    y: ")                   + QString::number(static_cast<quint8>(product_order_message[j+1])));
        }
        ui->terminal->append(QString("  }"));
        ui->terminal->append(QString("}\n"));
    }
}

void QtClient::on_pbMCM_clicked() // 13, 14
{
    remote_control_message[5] = static_cast<char>(ui->spinBox_control_device_id->value());
    remote_control_message[6] = static_cast<char>(ui->spinBox_control_flag->value());
    remote_control_message[12] = static_cast<char>(ui->spinBox_direction->value());
    sent_bytes = tcp_socket.write(remote_control_message, 13);
    ui->terminal->append(QString("SENT (") + QString::number(sent_bytes) + QString(" bytes)"));
    ui->terminal->append(QString("RCM: {"));
    ui->terminal->append(QString("  message_type: ")              + QString::number(static_cast<quint8>(remote_control_message[0])));
    ui->terminal->append(QString("  message_length_byte1(lsb): ") + QString::number(static_cast<quint8>(remote_control_message[1])));
    ui->terminal->append(QString("  message_length_byte2: ")      + QString::number(static_cast<quint8>(remote_control_message[2])));
    ui->terminal->append(QString("  message_length_byte3: ")      + QString::number(static_cast<quint8>(remote_control_message[3])));
    ui->terminal->append(QString("  message_length_byte4(msb): ") + QString::number(static_cast<quint8>(remote_control_message[4])));
    ui->terminal->append(QString("  control_device_id: ")         + QString::number(static_cast<quint8>(remote_control_message[5])));
    ui->terminal->append(QString("  control_flag: ")              + QString::number(static_cast<quint8>(remote_control_message[6])));
    ui->terminal->append(QString("  MCM: {"));
    ui->terminal->append(QString("   message_type: ")               + QString::number(static_cast<quint8>(remote_control_message[7])));
    ui->terminal->append(QString("   message_length_byte1(lsb): ")  + QString::number(static_cast<quint8>(remote_control_message[8])));
    ui->terminal->append(QString("   message_length_byte2: ")       + QString::number(static_cast<quint8>(remote_control_message[9])));
    ui->terminal->append(QString("   message_length_byte3: ")       + QString::number(static_cast<quint8>(remote_control_message[10])));
    ui->terminal->append(QString("   message_length_byte4(msb): ")  + QString::number(static_cast<quint8>(remote_control_message[11])));
    ui->terminal->append(QString("   direction: ")                  + QString::number(static_cast<quint8>(remote_control_message[12])));
    ui->terminal->append(QString("  }"));
    ui->terminal->append(QString("}\n"));
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
}

void QtClient::slot_readyRead_tcp()
{
    // if common header is not known
    // with 5 or more bytes we can check out the message type and message length
    bytes_received = tcp_socket.bytesAvailable();
    if (bytes_received >= 5 && !is_common_header_read)
    {
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

    // if common header is known we just wait for the data to arrive
    if(is_common_header_read)
    {
        bytes_received = tcp_socket.bytesAvailable();
        if(bytes_received >= message_length)
        {
            tcp_socket.read(tcp_read_buffer, message_length);
            if(message_type == 3) // SCM
            {
                system_control          = static_cast<quint8>(tcp_read_buffer[0]);
                device_id               = static_cast<quint8>(tcp_read_buffer[1]);

                is_common_header_read   = false; // done with a message

                ui->terminal->append(QString("RECEIVED (")         + QString::number(bytes_received) + QString(" bytes)"));
                ui->terminal->append(QString("SCM: {"));
                ui->terminal->append(QString("  system_control: ") + QString::number(system_control) + QString(","));
                ui->terminal->append(QString("  device_id: ")      + QString::number(device_id));
                ui->terminal->append(QString("}\n"));
            }
            else if(message_type == 4) // WFM
            {
                command_number          = static_cast<quint8>(tcp_read_buffer[0]);
                error_code              = static_cast<quint8>(tcp_read_buffer[1]);
                atomic_execution        = static_cast<quint8>(tcp_read_buffer[2]);
                device_x_coord          = static_cast<quint8>(tcp_read_buffer[3]);
                device_y_coord          = static_cast<quint8>(tcp_read_buffer[4]);
                device_orientation      = static_cast<quint8>(tcp_read_buffer[5]);
                device_state            = static_cast<quint8>(tcp_read_buffer[6]);

                QString extra_data_str;
                QString server_log_text;
                if(command_number == 10) // RLM
                {
                    extra_data_str += "    log: ";
                    ui->server_logs->setText("");
                    for(quint16 i = 7; i < message_length; ++i)
                    {
                        extra_data[i-7] = static_cast<quint8>(tcp_read_buffer[i]);
                        extra_data_str += QString(tcp_read_buffer[i]);
                        server_log_text += QString(tcp_read_buffer[i]);
                    }
                    ui->server_logs->setText(server_log_text);
                }
                else if(command_number == 11) // POM
                {
                    quint32 product_order_number = 0;
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
                    extra_data_str += "    product_order_number: " + QString::number(product_order_number, 16);
                }
                else if(command_number == 14) // extra_data == gopigo wfm
                {
                   extra_data[0] = static_cast<quint8>(tcp_read_buffer[7]);  // messag type
                   extra_data[1] = static_cast<quint8>(tcp_read_buffer[8]);  // message_length_byte_1 (lsb)
                   extra_data[2] = static_cast<quint8>(tcp_read_buffer[9]);  // message_length_byte_2
                   extra_data[3] = static_cast<quint8>(tcp_read_buffer[10]); // message_length_byte_3
                   extra_data[4] = static_cast<quint8>(tcp_read_buffer[11]); // message_length_byte_4 (msb)

                   extra_data[5] = static_cast<quint8>(tcp_read_buffer[12]);   // command number
                   extra_data[6] = static_cast<quint8>(tcp_read_buffer[13]);   // error_code
                   extra_data[7] = static_cast<quint8>(tcp_read_buffer[14]);   // atomic_execution
                   extra_data[8] = static_cast<quint8>(tcp_read_buffer[15]);  // device_x_coord
                   extra_data[9] = static_cast<quint8>(tcp_read_buffer[16]);  // device_y_coord
                   extra_data[10] = static_cast<quint8>(tcp_read_buffer[17]);  // device_orientation
                   extra_data[11] = static_cast<quint8>(tcp_read_buffer[18]);  // device_state
                   extra_data[12] = static_cast<quint8>(tcp_read_buffer[19]);  // extra_data (1 byte)
                   qDebug() << "message_type[0]: " << extra_data[0];
                   qDebug() << "message_length_byte_1(lsb)[1]: " << extra_data[1];
                   qDebug() << "message_length_byte_2[2]: " << extra_data[2];
                   qDebug() << "message_length_byte_3[3]: " << extra_data[3];
                   qDebug() << "message_length_byte_4(msb)[4]: " << extra_data[4];

                   if(extra_data[6] == 0) // 0 == "no error" == gopigo wfm is included
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
                   }
                }

                extra_data_str += "\n  }";

                is_common_header_read   = false; // done with a message

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
        }
    }
}

void QtClient::emit_finished()
{
    emit finished();
}
