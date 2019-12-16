/********************************************************************************
** Form generated from reading UI file 'qtclient.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTCLIENT_H
#define UI_QTCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtClient
{
public:
    QAction *read_sbm;
    QAction *connect_to_host;
    QAction *send_ncm;
    QAction *send_ccm;
    QAction *send_sqm;
    QWidget *centralwidget;
    QTextEdit *terminal;
    QPushButton *pbRLM;
    QSpinBox *spinBox_mode;
    QSpinBox *spinBox_row_count;
    QSpinBox *spinBox_beginning_row;
    QLabel *label_mode;
    QLabel *label_row_count;
    QLabel *label_beginning_row;
    QPushButton *pbSBMfreeze;
    QTextEdit *server_logs;
    QPushButton *pbSSM;
    QPushButton *pbSHM;
    QPushButton *pbUFM;
    QGraphicsView *graphicsView;
    QLabel *label_product_id;
    QPushButton *pbMCM;
    QLabel *label_send_order_to_2;
    QLabel *label_send_order_to_3;
    QCheckBox *checkBox_order_destination_9_1;
    QSpinBox *spinBox_direction;
    QPushButton *pbPOM;
    QCheckBox *checkBox_order_destination_9_0;
    QLabel *label_direction;
    QLabel *label_control_flag;
    QLabel *label_send_order_to_1;
    QLabel *label_send_order_to_0;
    QCheckBox *checkBox_order_destination_9_2;
    QLabel *label_control_device_id;
    QSpinBox *spinBox_control_device_id;
    QSpinBox *spinBox_product_id;
    QSpinBox *spinBox_control_flag;
    QCheckBox *checkBox_order_destination_9_3;
    QMenuBar *menubar;
    QMenu *Broadcast;
    QMenu *menuConnection;

    void setupUi(QMainWindow *QtClient)
    {
        if (QtClient->objectName().isEmpty())
            QtClient->setObjectName(QString::fromUtf8("QtClient"));
        QtClient->resize(1280, 720);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(QtClient->sizePolicy().hasHeightForWidth());
        QtClient->setSizePolicy(sizePolicy);
        QtClient->setMinimumSize(QSize(1280, 720));
        QtClient->setMaximumSize(QSize(1280, 750));
        QtClient->setStyleSheet(QString::fromUtf8("QWidget {\n"
"	background-color: black;\n"
"	color: white;\n"
"	font-size: 16px;\n"
"	border: 1px solid white;\n"
"}\n"
"\n"
"QMenu, QAction {\n"
"	background-color: Orange;\n"
"	color: black;\n"
"}\n"
"\n"
"QCheckBox {\n"
"	padding-left: 6px;\n"
"}"));
        read_sbm = new QAction(QtClient);
        read_sbm->setObjectName(QString::fromUtf8("read_sbm"));
        connect_to_host = new QAction(QtClient);
        connect_to_host->setObjectName(QString::fromUtf8("connect_to_host"));
        send_ncm = new QAction(QtClient);
        send_ncm->setObjectName(QString::fromUtf8("send_ncm"));
        send_ccm = new QAction(QtClient);
        send_ccm->setObjectName(QString::fromUtf8("send_ccm"));
        send_sqm = new QAction(QtClient);
        send_sqm->setObjectName(QString::fromUtf8("send_sqm"));
        centralwidget = new QWidget(QtClient);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        centralwidget->setMinimumSize(QSize(1280, 720));
        centralwidget->setMaximumSize(QSize(1280, 720));
        terminal = new QTextEdit(centralwidget);
        terminal->setObjectName(QString::fromUtf8("terminal"));
        terminal->setGeometry(QRect(970, 0, 310, 490));
        sizePolicy.setHeightForWidth(terminal->sizePolicy().hasHeightForWidth());
        terminal->setSizePolicy(sizePolicy);
        terminal->setMinimumSize(QSize(200, 490));
        terminal->setMaximumSize(QSize(310, 490));
        terminal->setStyleSheet(QString::fromUtf8("background-color: black;\n"
"font-size: 14px;"));
        pbRLM = new QPushButton(centralwidget);
        pbRLM->setObjectName(QString::fromUtf8("pbRLM"));
        pbRLM->setGeometry(QRect(0, 640, 200, 50));
        sizePolicy.setHeightForWidth(pbRLM->sizePolicy().hasHeightForWidth());
        pbRLM->setSizePolicy(sizePolicy);
        pbRLM->setMinimumSize(QSize(200, 50));
        pbRLM->setMaximumSize(QSize(200, 50));
        pbRLM->setStyleSheet(QString::fromUtf8("QPushButton::hover {\n"
"	background-color: white;\n"
"	color: brown;\n"
"}\n"
"QPushButton {\n"
"	background-color: brown;\n"
"}\n"
""));
        spinBox_mode = new QSpinBox(centralwidget);
        spinBox_mode->setObjectName(QString::fromUtf8("spinBox_mode"));
        spinBox_mode->setGeometry(QRect(120, 490, 80, 50));
        sizePolicy.setHeightForWidth(spinBox_mode->sizePolicy().hasHeightForWidth());
        spinBox_mode->setSizePolicy(sizePolicy);
        spinBox_mode->setMinimumSize(QSize(80, 50));
        spinBox_mode->setMaximumSize(QSize(80, 50));
        spinBox_mode->setStyleSheet(QString::fromUtf8("QSpinBox::hover {\n"
"	background-color: white;\n"
"	color: brown;\n"
"}\n"
"\n"
"QSpinBox {\n"
"	background-color: brown;\n"
"}"));
        spinBox_mode->setAlignment(Qt::AlignCenter);
        spinBox_row_count = new QSpinBox(centralwidget);
        spinBox_row_count->setObjectName(QString::fromUtf8("spinBox_row_count"));
        spinBox_row_count->setGeometry(QRect(120, 540, 80, 50));
        sizePolicy.setHeightForWidth(spinBox_row_count->sizePolicy().hasHeightForWidth());
        spinBox_row_count->setSizePolicy(sizePolicy);
        spinBox_row_count->setMinimumSize(QSize(80, 50));
        spinBox_row_count->setMaximumSize(QSize(80, 50));
        spinBox_row_count->setStyleSheet(QString::fromUtf8("QSpinBox::hover {\n"
"	background-color: white;\n"
"	color: brown;\n"
"}\n"
"\n"
"QSpinBox {\n"
"	background-color: brown;\n"
"}"));
        spinBox_row_count->setAlignment(Qt::AlignCenter);
        spinBox_beginning_row = new QSpinBox(centralwidget);
        spinBox_beginning_row->setObjectName(QString::fromUtf8("spinBox_beginning_row"));
        spinBox_beginning_row->setGeometry(QRect(120, 590, 80, 50));
        sizePolicy.setHeightForWidth(spinBox_beginning_row->sizePolicy().hasHeightForWidth());
        spinBox_beginning_row->setSizePolicy(sizePolicy);
        spinBox_beginning_row->setMinimumSize(QSize(80, 50));
        spinBox_beginning_row->setMaximumSize(QSize(80, 50));
        spinBox_beginning_row->setStyleSheet(QString::fromUtf8("QSpinBox::hover {\n"
"	background-color: white;\n"
"	color: brown;\n"
"}\n"
"\n"
"QSpinBox {\n"
"	background-color: brown;\n"
"}"));
        spinBox_beginning_row->setAlignment(Qt::AlignCenter);
        label_mode = new QLabel(centralwidget);
        label_mode->setObjectName(QString::fromUtf8("label_mode"));
        label_mode->setGeometry(QRect(0, 490, 120, 50));
        sizePolicy.setHeightForWidth(label_mode->sizePolicy().hasHeightForWidth());
        label_mode->setSizePolicy(sizePolicy);
        label_mode->setMinimumSize(QSize(120, 50));
        label_mode->setMaximumSize(QSize(120, 50));
        label_mode->setStyleSheet(QString::fromUtf8("background-color:  brown;"));
        label_mode->setAlignment(Qt::AlignCenter);
        label_row_count = new QLabel(centralwidget);
        label_row_count->setObjectName(QString::fromUtf8("label_row_count"));
        label_row_count->setGeometry(QRect(0, 540, 120, 50));
        sizePolicy.setHeightForWidth(label_row_count->sizePolicy().hasHeightForWidth());
        label_row_count->setSizePolicy(sizePolicy);
        label_row_count->setMinimumSize(QSize(120, 50));
        label_row_count->setMaximumSize(QSize(120, 50));
        label_row_count->setStyleSheet(QString::fromUtf8("background-color:  brown;"));
        label_row_count->setAlignment(Qt::AlignCenter);
        label_beginning_row = new QLabel(centralwidget);
        label_beginning_row->setObjectName(QString::fromUtf8("label_beginning_row"));
        label_beginning_row->setGeometry(QRect(0, 590, 120, 50));
        sizePolicy.setHeightForWidth(label_beginning_row->sizePolicy().hasHeightForWidth());
        label_beginning_row->setSizePolicy(sizePolicy);
        label_beginning_row->setMinimumSize(QSize(120, 50));
        label_beginning_row->setMaximumSize(QSize(120, 50));
        label_beginning_row->setStyleSheet(QString::fromUtf8("background-color:  brown;"));
        label_beginning_row->setAlignment(Qt::AlignCenter);
        pbSBMfreeze = new QPushButton(centralwidget);
        pbSBMfreeze->setObjectName(QString::fromUtf8("pbSBMfreeze"));
        pbSBMfreeze->setGeometry(QRect(970, 640, 310, 50));
        sizePolicy.setHeightForWidth(pbSBMfreeze->sizePolicy().hasHeightForWidth());
        pbSBMfreeze->setSizePolicy(sizePolicy);
        pbSBMfreeze->setMinimumSize(QSize(200, 50));
        pbSBMfreeze->setMaximumSize(QSize(310, 50));
        pbSBMfreeze->setStyleSheet(QString::fromUtf8("::hover {\n"
"	background-color: white;\n"
"	color: darkred;\n"
"}\n"
"QPushButton {\n"
"	background-color: darkred;\n"
"}"));
        server_logs = new QTextEdit(centralwidget);
        server_logs->setObjectName(QString::fromUtf8("server_logs"));
        server_logs->setGeometry(QRect(200, 490, 770, 200));
        sizePolicy.setHeightForWidth(server_logs->sizePolicy().hasHeightForWidth());
        server_logs->setSizePolicy(sizePolicy);
        server_logs->setMinimumSize(QSize(770, 200));
        server_logs->setMaximumSize(QSize(770, 200));
        server_logs->setStyleSheet(QString::fromUtf8("background-color: black;\n"
"font-size: 14px;"));
        pbSSM = new QPushButton(centralwidget);
        pbSSM->setObjectName(QString::fromUtf8("pbSSM"));
        pbSSM->setGeometry(QRect(970, 540, 310, 50));
        sizePolicy.setHeightForWidth(pbSSM->sizePolicy().hasHeightForWidth());
        pbSSM->setSizePolicy(sizePolicy);
        pbSSM->setMinimumSize(QSize(200, 50));
        pbSSM->setMaximumSize(QSize(310, 50));
        pbSSM->setStyleSheet(QString::fromUtf8("::hover {\n"
"	background-color: white;\n"
"	color: darkgreen;\n"
"}\n"
"QPushButton {\n"
"	background-color: darkgreen;\n"
"}\n"
"\n"
""));
        pbSHM = new QPushButton(centralwidget);
        pbSHM->setObjectName(QString::fromUtf8("pbSHM"));
        pbSHM->setGeometry(QRect(970, 590, 310, 50));
        sizePolicy.setHeightForWidth(pbSHM->sizePolicy().hasHeightForWidth());
        pbSHM->setSizePolicy(sizePolicy);
        pbSHM->setMinimumSize(QSize(200, 50));
        pbSHM->setMaximumSize(QSize(310, 50));
        pbSHM->setStyleSheet(QString::fromUtf8("::hover {\n"
"	background-color: white;\n"
"	color: darkblue;\n"
"}\n"
"QPushButton {\n"
"	background-color: darkblue;\n"
"}"));
        pbUFM = new QPushButton(centralwidget);
        pbUFM->setObjectName(QString::fromUtf8("pbUFM"));
        pbUFM->setGeometry(QRect(970, 490, 310, 50));
        sizePolicy.setHeightForWidth(pbUFM->sizePolicy().hasHeightForWidth());
        pbUFM->setSizePolicy(sizePolicy);
        pbUFM->setMinimumSize(QSize(200, 50));
        pbUFM->setMaximumSize(QSize(310, 50));
        pbUFM->setStyleSheet(QString::fromUtf8("QPushButton::hover {\n"
"	background-color: white;\n"
"	color: darkorange;\n"
"}\n"
"QPushButton {\n"
"	background-color: darkorange;\n"
"}\n"
"\n"
""));
        graphicsView = new QGraphicsView(centralwidget);
        graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
        graphicsView->setGeometry(QRect(200, 0, 770, 490));
        sizePolicy.setHeightForWidth(graphicsView->sizePolicy().hasHeightForWidth());
        graphicsView->setSizePolicy(sizePolicy);
        graphicsView->setMinimumSize(QSize(770, 490));
        graphicsView->setMaximumSize(QSize(770, 490));
        graphicsView->setStyleSheet(QString::fromUtf8("QGraphicsView {\n"
"	background-color: darkgrey;\n"
"border-width:0;\n"
"}"));
        graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        label_product_id = new QLabel(centralwidget);
        label_product_id->setObjectName(QString::fromUtf8("label_product_id"));
        label_product_id->setGeometry(QRect(0, 194, 120, 48));
        sizePolicy.setHeightForWidth(label_product_id->sizePolicy().hasHeightForWidth());
        label_product_id->setSizePolicy(sizePolicy);
        label_product_id->setMinimumSize(QSize(120, 48));
        label_product_id->setMaximumSize(QSize(120, 48));
        label_product_id->setStyleSheet(QString::fromUtf8("background-color: teal;"));
        label_product_id->setAlignment(Qt::AlignCenter);
        pbMCM = new QPushButton(centralwidget);
        pbMCM->setObjectName(QString::fromUtf8("pbMCM"));
        pbMCM->setGeometry(QRect(0, 144, 200, 50));
        sizePolicy.setHeightForWidth(pbMCM->sizePolicy().hasHeightForWidth());
        pbMCM->setSizePolicy(sizePolicy);
        pbMCM->setMinimumSize(QSize(200, 50));
        pbMCM->setMaximumSize(QSize(200, 50));
        pbMCM->setStyleSheet(QString::fromUtf8("QPushButton:hover {\n"
"	background-color: white;\n"
"	color: purple;\n"
"}\n"
"\n"
"QPushButton {\n"
"	background-color: purple;\n"
"}"));
        label_send_order_to_2 = new QLabel(centralwidget);
        label_send_order_to_2->setObjectName(QString::fromUtf8("label_send_order_to_2"));
        label_send_order_to_2->setGeometry(QRect(0, 338, 120, 48));
        sizePolicy.setHeightForWidth(label_send_order_to_2->sizePolicy().hasHeightForWidth());
        label_send_order_to_2->setSizePolicy(sizePolicy);
        label_send_order_to_2->setMinimumSize(QSize(120, 48));
        label_send_order_to_2->setMaximumSize(QSize(120, 48));
        label_send_order_to_2->setStyleSheet(QString::fromUtf8("background-color: teal;"));
        label_send_order_to_2->setAlignment(Qt::AlignCenter);
        label_send_order_to_3 = new QLabel(centralwidget);
        label_send_order_to_3->setObjectName(QString::fromUtf8("label_send_order_to_3"));
        label_send_order_to_3->setGeometry(QRect(0, 386, 120, 48));
        sizePolicy.setHeightForWidth(label_send_order_to_3->sizePolicy().hasHeightForWidth());
        label_send_order_to_3->setSizePolicy(sizePolicy);
        label_send_order_to_3->setMinimumSize(QSize(120, 48));
        label_send_order_to_3->setMaximumSize(QSize(120, 48));
        label_send_order_to_3->setStyleSheet(QString::fromUtf8("background-color: teal;"));
        label_send_order_to_3->setAlignment(Qt::AlignCenter);
        checkBox_order_destination_9_1 = new QCheckBox(centralwidget);
        checkBox_order_destination_9_1->setObjectName(QString::fromUtf8("checkBox_order_destination_9_1"));
        checkBox_order_destination_9_1->setGeometry(QRect(120, 338, 80, 48));
        sizePolicy.setHeightForWidth(checkBox_order_destination_9_1->sizePolicy().hasHeightForWidth());
        checkBox_order_destination_9_1->setSizePolicy(sizePolicy);
        checkBox_order_destination_9_1->setMinimumSize(QSize(80, 48));
        checkBox_order_destination_9_1->setMaximumSize(QSize(80, 48));
        checkBox_order_destination_9_1->setStyleSheet(QString::fromUtf8("QCheckBox::hover {\n"
"	background-color: white;\n"
"	color: teal;\n"
"}\n"
"\n"
"QCheckBox {\n"
"	background-color: teal;\n"
"}\n"
""));
        spinBox_direction = new QSpinBox(centralwidget);
        spinBox_direction->setObjectName(QString::fromUtf8("spinBox_direction"));
        spinBox_direction->setGeometry(QRect(120, 96, 80, 48));
        sizePolicy.setHeightForWidth(spinBox_direction->sizePolicy().hasHeightForWidth());
        spinBox_direction->setSizePolicy(sizePolicy);
        spinBox_direction->setMinimumSize(QSize(80, 48));
        spinBox_direction->setMaximumSize(QSize(80, 48));
        spinBox_direction->setStyleSheet(QString::fromUtf8("QSpinBox:hover {\n"
"	background-color: white;\n"
"	color:  purple;\n"
"}\n"
"\n"
"QSpinBox {\n"
"	background-color:  purple;\n"
"}\n"
"\n"
""));
        spinBox_direction->setAlignment(Qt::AlignCenter);
        pbPOM = new QPushButton(centralwidget);
        pbPOM->setObjectName(QString::fromUtf8("pbPOM"));
        pbPOM->setGeometry(QRect(0, 434, 200, 56));
        sizePolicy.setHeightForWidth(pbPOM->sizePolicy().hasHeightForWidth());
        pbPOM->setSizePolicy(sizePolicy);
        pbPOM->setMinimumSize(QSize(200, 56));
        pbPOM->setMaximumSize(QSize(200, 56));
        pbPOM->setStyleSheet(QString::fromUtf8("QPushButton::hover {\n"
"	background-color: white;\n"
"	color: teal;\n"
"}\n"
"QPushButton {\n"
"	background-color: teal;\n"
"}\n"
""));
        checkBox_order_destination_9_0 = new QCheckBox(centralwidget);
        checkBox_order_destination_9_0->setObjectName(QString::fromUtf8("checkBox_order_destination_9_0"));
        checkBox_order_destination_9_0->setGeometry(QRect(120, 386, 80, 48));
        sizePolicy.setHeightForWidth(checkBox_order_destination_9_0->sizePolicy().hasHeightForWidth());
        checkBox_order_destination_9_0->setSizePolicy(sizePolicy);
        checkBox_order_destination_9_0->setMinimumSize(QSize(80, 48));
        checkBox_order_destination_9_0->setMaximumSize(QSize(80, 48));
        checkBox_order_destination_9_0->setStyleSheet(QString::fromUtf8("QCheckBox::hover {\n"
"	background-color: white;\n"
"	color: teal;\n"
"}\n"
"\n"
"QCheckBox {\n"
"	background-color: teal;\n"
"}\n"
""));
        checkBox_order_destination_9_0->setIconSize(QSize(20, 20));
        label_direction = new QLabel(centralwidget);
        label_direction->setObjectName(QString::fromUtf8("label_direction"));
        label_direction->setGeometry(QRect(0, 96, 120, 48));
        sizePolicy.setHeightForWidth(label_direction->sizePolicy().hasHeightForWidth());
        label_direction->setSizePolicy(sizePolicy);
        label_direction->setMinimumSize(QSize(120, 48));
        label_direction->setMaximumSize(QSize(120, 48));
        label_direction->setStyleSheet(QString::fromUtf8("background-color:  purple;"));
        label_direction->setAlignment(Qt::AlignCenter);
        label_control_flag = new QLabel(centralwidget);
        label_control_flag->setObjectName(QString::fromUtf8("label_control_flag"));
        label_control_flag->setGeometry(QRect(0, 48, 120, 48));
        sizePolicy.setHeightForWidth(label_control_flag->sizePolicy().hasHeightForWidth());
        label_control_flag->setSizePolicy(sizePolicy);
        label_control_flag->setMinimumSize(QSize(120, 48));
        label_control_flag->setMaximumSize(QSize(120, 48));
        label_control_flag->setStyleSheet(QString::fromUtf8("background-color:  purple;"));
        label_control_flag->setAlignment(Qt::AlignCenter);
        label_send_order_to_1 = new QLabel(centralwidget);
        label_send_order_to_1->setObjectName(QString::fromUtf8("label_send_order_to_1"));
        label_send_order_to_1->setGeometry(QRect(0, 290, 120, 48));
        sizePolicy.setHeightForWidth(label_send_order_to_1->sizePolicy().hasHeightForWidth());
        label_send_order_to_1->setSizePolicy(sizePolicy);
        label_send_order_to_1->setMinimumSize(QSize(120, 48));
        label_send_order_to_1->setMaximumSize(QSize(120, 48));
        label_send_order_to_1->setStyleSheet(QString::fromUtf8("background-color: teal;"));
        label_send_order_to_1->setAlignment(Qt::AlignCenter);
        label_send_order_to_0 = new QLabel(centralwidget);
        label_send_order_to_0->setObjectName(QString::fromUtf8("label_send_order_to_0"));
        label_send_order_to_0->setGeometry(QRect(0, 242, 120, 48));
        sizePolicy.setHeightForWidth(label_send_order_to_0->sizePolicy().hasHeightForWidth());
        label_send_order_to_0->setSizePolicy(sizePolicy);
        label_send_order_to_0->setMinimumSize(QSize(120, 48));
        label_send_order_to_0->setMaximumSize(QSize(120, 48));
        label_send_order_to_0->setStyleSheet(QString::fromUtf8("background-color: teal;"));
        label_send_order_to_0->setAlignment(Qt::AlignCenter);
        checkBox_order_destination_9_2 = new QCheckBox(centralwidget);
        checkBox_order_destination_9_2->setObjectName(QString::fromUtf8("checkBox_order_destination_9_2"));
        checkBox_order_destination_9_2->setGeometry(QRect(120, 290, 80, 48));
        sizePolicy.setHeightForWidth(checkBox_order_destination_9_2->sizePolicy().hasHeightForWidth());
        checkBox_order_destination_9_2->setSizePolicy(sizePolicy);
        checkBox_order_destination_9_2->setMinimumSize(QSize(80, 48));
        checkBox_order_destination_9_2->setMaximumSize(QSize(80, 48));
        checkBox_order_destination_9_2->setStyleSheet(QString::fromUtf8("QCheckBox::hover {\n"
"	background-color: white;\n"
"	color: teal;\n"
"}\n"
"\n"
"QCheckBox {\n"
"	background-color: teal;\n"
"}\n"
""));
        label_control_device_id = new QLabel(centralwidget);
        label_control_device_id->setObjectName(QString::fromUtf8("label_control_device_id"));
        label_control_device_id->setGeometry(QRect(0, 0, 120, 48));
        sizePolicy.setHeightForWidth(label_control_device_id->sizePolicy().hasHeightForWidth());
        label_control_device_id->setSizePolicy(sizePolicy);
        label_control_device_id->setMinimumSize(QSize(120, 48));
        label_control_device_id->setMaximumSize(QSize(120, 48));
        label_control_device_id->setStyleSheet(QString::fromUtf8("background-color:  purple;"));
        label_control_device_id->setAlignment(Qt::AlignCenter);
        spinBox_control_device_id = new QSpinBox(centralwidget);
        spinBox_control_device_id->setObjectName(QString::fromUtf8("spinBox_control_device_id"));
        spinBox_control_device_id->setGeometry(QRect(120, 0, 80, 48));
        sizePolicy.setHeightForWidth(spinBox_control_device_id->sizePolicy().hasHeightForWidth());
        spinBox_control_device_id->setSizePolicy(sizePolicy);
        spinBox_control_device_id->setMinimumSize(QSize(80, 48));
        spinBox_control_device_id->setMaximumSize(QSize(80, 48));
        spinBox_control_device_id->setStyleSheet(QString::fromUtf8("QSpinBox:hover {\n"
"	background-color: white;\n"
"	color:  purple;\n"
"}\n"
"\n"
"QSpinBox {\n"
"	background-color:  purple;\n"
"}\n"
"\n"
"\n"
""));
        spinBox_control_device_id->setAlignment(Qt::AlignCenter);
        spinBox_product_id = new QSpinBox(centralwidget);
        spinBox_product_id->setObjectName(QString::fromUtf8("spinBox_product_id"));
        spinBox_product_id->setGeometry(QRect(120, 194, 80, 48));
        sizePolicy.setHeightForWidth(spinBox_product_id->sizePolicy().hasHeightForWidth());
        spinBox_product_id->setSizePolicy(sizePolicy);
        spinBox_product_id->setMinimumSize(QSize(80, 48));
        spinBox_product_id->setMaximumSize(QSize(80, 48));
        spinBox_product_id->setStyleSheet(QString::fromUtf8("QSpinBox::hover {\n"
"	background-color: white;\n"
"	color: teal;\n"
"}\n"
"QSpinBox {\n"
"	background-color: teal;\n"
"}\n"
""));
        spinBox_product_id->setAlignment(Qt::AlignCenter);
        spinBox_control_flag = new QSpinBox(centralwidget);
        spinBox_control_flag->setObjectName(QString::fromUtf8("spinBox_control_flag"));
        spinBox_control_flag->setGeometry(QRect(120, 48, 80, 48));
        sizePolicy.setHeightForWidth(spinBox_control_flag->sizePolicy().hasHeightForWidth());
        spinBox_control_flag->setSizePolicy(sizePolicy);
        spinBox_control_flag->setMinimumSize(QSize(80, 48));
        spinBox_control_flag->setMaximumSize(QSize(80, 48));
        spinBox_control_flag->setStyleSheet(QString::fromUtf8("QSpinBox:hover {\n"
"	background-color: white;\n"
"	color:  purple;\n"
"}\n"
"\n"
"QSpinBox {\n"
"	background-color:  purple;\n"
"}\n"
"\n"
""));
        spinBox_control_flag->setAlignment(Qt::AlignCenter);
        checkBox_order_destination_9_3 = new QCheckBox(centralwidget);
        checkBox_order_destination_9_3->setObjectName(QString::fromUtf8("checkBox_order_destination_9_3"));
        checkBox_order_destination_9_3->setGeometry(QRect(120, 242, 80, 48));
        sizePolicy.setHeightForWidth(checkBox_order_destination_9_3->sizePolicy().hasHeightForWidth());
        checkBox_order_destination_9_3->setSizePolicy(sizePolicy);
        checkBox_order_destination_9_3->setMinimumSize(QSize(80, 48));
        checkBox_order_destination_9_3->setMaximumSize(QSize(80, 48));
        checkBox_order_destination_9_3->setStyleSheet(QString::fromUtf8("QCheckBox::hover {\n"
"	background-color: white;\n"
"	color: teal;\n"
"}\n"
"\n"
"QCheckBox {\n"
"	background-color: teal;\n"
"}\n"
""));
        QtClient->setCentralWidget(centralwidget);
        graphicsView->raise();
        terminal->raise();
        pbRLM->raise();
        spinBox_mode->raise();
        spinBox_row_count->raise();
        spinBox_beginning_row->raise();
        label_mode->raise();
        label_row_count->raise();
        label_beginning_row->raise();
        pbSBMfreeze->raise();
        pbSSM->raise();
        pbSHM->raise();
        label_product_id->raise();
        label_send_order_to_2->raise();
        label_send_order_to_3->raise();
        checkBox_order_destination_9_1->raise();
        spinBox_direction->raise();
        checkBox_order_destination_9_0->raise();
        label_direction->raise();
        label_send_order_to_1->raise();
        label_send_order_to_0->raise();
        checkBox_order_destination_9_2->raise();
        spinBox_product_id->raise();
        spinBox_control_flag->raise();
        pbPOM->raise();
        label_control_flag->raise();
        label_control_device_id->raise();
        pbMCM->raise();
        spinBox_control_device_id->raise();
        server_logs->raise();
        pbUFM->raise();
        checkBox_order_destination_9_3->raise();
        menubar = new QMenuBar(QtClient);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setEnabled(true);
        menubar->setGeometry(QRect(0, 0, 1280, 30));
        menubar->setMinimumSize(QSize(1280, 30));
        menubar->setMaximumSize(QSize(1280, 30));
        Broadcast = new QMenu(menubar);
        Broadcast->setObjectName(QString::fromUtf8("Broadcast"));
        menuConnection = new QMenu(menubar);
        menuConnection->setObjectName(QString::fromUtf8("menuConnection"));
        QtClient->setMenuBar(menubar);
        QWidget::setTabOrder(pbRLM, spinBox_mode);
        QWidget::setTabOrder(spinBox_mode, spinBox_row_count);
        QWidget::setTabOrder(spinBox_row_count, spinBox_beginning_row);
        QWidget::setTabOrder(spinBox_beginning_row, pbSBMfreeze);
        QWidget::setTabOrder(pbSBMfreeze, terminal);

        menubar->addAction(Broadcast->menuAction());
        menubar->addAction(menuConnection->menuAction());
        Broadcast->addAction(read_sbm);
        menuConnection->addAction(connect_to_host);
        menuConnection->addAction(send_ncm);
        menuConnection->addAction(send_ccm);
        menuConnection->addAction(send_sqm);

        retranslateUi(QtClient);

        QMetaObject::connectSlotsByName(QtClient);
    } // setupUi

    void retranslateUi(QMainWindow *QtClient)
    {
        QtClient->setWindowTitle(QCoreApplication::translate("QtClient", "QtClient", nullptr));
        read_sbm->setText(QCoreApplication::translate("QtClient", "Read SBM", nullptr));
        connect_to_host->setText(QCoreApplication::translate("QtClient", "Connect To Host", nullptr));
        send_ncm->setText(QCoreApplication::translate("QtClient", "Send NCM", nullptr));
        send_ccm->setText(QCoreApplication::translate("QtClient", "Send CCM", nullptr));
        send_sqm->setText(QCoreApplication::translate("QtClient", "Send SQM", nullptr));
        pbRLM->setText(QCoreApplication::translate("QtClient", "Send (RLM)\n"
"Read Log Message", nullptr));
        label_mode->setText(QCoreApplication::translate("QtClient", "mode", nullptr));
        label_row_count->setText(QCoreApplication::translate("QtClient", "row count", nullptr));
        label_beginning_row->setText(QCoreApplication::translate("QtClient", "beginning row", nullptr));
        pbSBMfreeze->setText(QCoreApplication::translate("QtClient", "EMERGENCY\n"
"STOP", nullptr));
        pbSSM->setText(QCoreApplication::translate("QtClient", "Send (SSM)\n"
"System Startup Message", nullptr));
        pbSHM->setText(QCoreApplication::translate("QtClient", "Send (SHM)\n"
"System Shutdown Message", nullptr));
        pbUFM->setText(QCoreApplication::translate("QtClient", "Send (UFM)\n"
"UnFreeze Message", nullptr));
        label_product_id->setText(QCoreApplication::translate("QtClient", "product id", nullptr));
        pbMCM->setText(QCoreApplication::translate("QtClient", "Send (MCM)\n"
"Move Cell Message", nullptr));
        label_send_order_to_2->setText(QCoreApplication::translate("QtClient", "send to", nullptr));
        label_send_order_to_3->setText(QCoreApplication::translate("QtClient", "send to", nullptr));
        checkBox_order_destination_9_1->setText(QCoreApplication::translate("QtClient", "(9, 1)", nullptr));
        pbPOM->setText(QCoreApplication::translate("QtClient", "Send (POM)\n"
"Product Order Message", nullptr));
        checkBox_order_destination_9_0->setText(QCoreApplication::translate("QtClient", "(9, 0)", nullptr));
        label_direction->setText(QCoreApplication::translate("QtClient", "direction", nullptr));
        label_control_flag->setText(QCoreApplication::translate("QtClient", "control flag", nullptr));
        label_send_order_to_1->setText(QCoreApplication::translate("QtClient", "send to", nullptr));
        label_send_order_to_0->setText(QCoreApplication::translate("QtClient", "send to", nullptr));
        checkBox_order_destination_9_2->setText(QCoreApplication::translate("QtClient", "(9, 2)", nullptr));
        label_control_device_id->setText(QCoreApplication::translate("QtClient", "device id", nullptr));
        checkBox_order_destination_9_3->setText(QCoreApplication::translate("QtClient", "(9, 3)", nullptr));
        Broadcast->setTitle(QCoreApplication::translate("QtClient", "Broadcast", nullptr));
        menuConnection->setTitle(QCoreApplication::translate("QtClient", "Connection", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtClient: public Ui_QtClient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTCLIENT_H
