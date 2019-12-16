#include "graphics.h"

Graphics::Graphics()
{
    // LOAD IMAGES FROM RESOURCES:
    if(pixmap_system_state_red.load(":/images/system_state_red", "png"))
    {
        item_system_state_red = scene.addPixmap(pixmap_system_state_red);
        item_system_state_red->hide();
    }

    if(pixmap_system_state_green.load(":/images/system_state_green", "png"))
    {
        item_system_state_green = scene.addPixmap(pixmap_system_state_green);
        item_system_state_green->hide();
    }

    if(pixmap_system_state_blue.load(":/images/system_state_blue", "png"))
    {
        item_system_state_blue = scene.addPixmap(pixmap_system_state_blue);
        item_system_state_blue->hide();
    }

    if(pixmap_system_state_grey.load(":/images/system_state_grey", "png"))
        item_system_state_grey = scene.addPixmap(pixmap_system_state_grey);

    if(pixmap_graphicsview_grid.load(":/images/graphicsview_grid", "png"))
        pixmap_item_graphicsview_grid = scene.addPixmap(pixmap_graphicsview_grid);

    if(pixmap_gopigo_grid.load(":/images/gopigo_grid", "png"))
        pixmap_item_gopigo_grid = scene.addPixmap(pixmap_gopigo_grid);

    if(pixmap_ur5.load(":/images/ur5", "png"))
        pixmap_item_ur5 = scene.addPixmap(pixmap_ur5);


    // 4 instances of gopigo images
    for(quint8 i = 0; i < 4; ++i)
    {
        if(pixmap_gopigo_list[i].load(":/images/gopigo", "png"))
        {
            pixmap_item_gopigo[i] = scene.addPixmap(pixmap_gopigo_list[i]);
            pixmap_item_gopigo[i]->setPos(70-18, 420-i*70-18);
            pixmap_item_gopigo[i]->hide();
            gpg_id_font = QFont("MS Shell Dlg 2", 10, 400);
            text_item_gopigo_id[i] = scene.addText("");
            text_item_gopigo_id[i]->setFont(gpg_id_font);
            text_item_gopigo_id[i]->hide();
            text_item_gopigo_id[i]->setDefaultTextColor(QColor(Qt::black));
        }
        if(pixmap_remote_controlled_list[i].load(":/images/remote_controlled", "png"))
        {
            pixmap_item_remote_controlled[i] = scene.addPixmap(pixmap_remote_controlled_list[i]);
            pixmap_item_remote_controlled[i]->setPos(70-18, 420-i*70-18);
            pixmap_item_remote_controlled[i]->hide();
            gpg_id_font_remote = QFont("MS Shell Dlg 2", 10, 400);
            text_item_gopigo_id_remote[i] = scene.addText("");
            text_item_gopigo_id_remote[i]->setFont(gpg_id_font_remote);
            text_item_gopigo_id_remote[i]->hide();
            text_item_gopigo_id_remote[i]->setDefaultTextColor(QColor(Qt::white));
        }
    }

    // 44 istances of obstacle images
    quint8 i = 0;
    for(quint8 y = 0; y < 6; ++y)
    {
        for(quint8 x = 0; x < 10; ++x)
        {
            if( ((y == 5 || y == 4) && (x == 0 || x == 1 || x == 8 || x == 9)) || ( (x == 4 || x == 5) && !(y == 4 || y == 5)) )
                continue;

            if(pixmap_obstacle_list[i].load(":/images/obstacle", "png"))
            {
                pixmap_item_obstacle[i] = scene.addPixmap(pixmap_obstacle_list[i]);
                pixmap_item_obstacle[i]->hide();
                pixmap_item_obstacle[i]->setPos(70-18 + 70*x, 420-18 - 70*y);
                i++;
            }
        }
    }

    scene.setSceneRect(0, 0, 770, 490);
    pixmap_item_graphicsview_grid->setPos(0, 0); // occupies the whole graphicsview area
    pixmap_item_gopigo_grid->setPos(70-1, 70-1); // (-1 offset due to added border in the image)
    pixmap_item_ur5->setPos(315-1, 175-1);       // (-1 offset due to added border in the image)
}


void Graphics::hide_all_obstacles()
{
    for(quint8 i = 0; i < 44; ++i)
        pixmap_item_obstacle[i]->hide();
}

void Graphics::show_found_obstacles(quint8 obstacle_count, Point* obstacle_list)
{
    quint8 obstacle_index = 0;
    for(quint8 i = 0, j = obstacle_count; i < j; ++i)
    {
        quint8 x = obstacle_list[i].x;
        quint8 y = obstacle_list[i].y;

        if( !(((y == 5 || y == 4) && (x == 0 || x == 1 || x == 8 || x == 9)) || ( (x == 4 || x == 5) && !(y == 4 || y == 5))) )
        {
            if(y <= 3)
            {
                obstacle_index = y*8 + (x > 5 ? x-2 : x);
            }
            else if(y == 4)
            {
                obstacle_index = 32 + x - 2;
            }
            else if(y == 5)
            {
                obstacle_index = 38 + x - 2;
            }
            pixmap_item_obstacle[obstacle_index]->show();
        }
    }
}

void Graphics::update_gopigo_locations(quint8 device_count, VarastoRoboDevice* device_list)
{
    // update gopigo locations
    quint8 gopigo_index = 0;
    for(quint8 i = 0; i < device_count; ++i)
    {
        if(device_list[i].type == 2)
        {
            quint8 x = device_list[i].point.x;
            quint8 y = device_list[i].point.y;
            pixmap_item_gopigo[gopigo_index]->setPos(70-18 + 70*x, 420-18 - 70*y);
            pixmap_item_gopigo[gopigo_index]->show();

            // gopigo id text, fixed length of 2 characters (gopigo id range = 1-9) => from "G1" to "G9"
            QString gpg_text = "G" + QString::number(device_list[i].id);

            text_item_gopigo_id[gopigo_index]->setPlainText(gpg_text);
            QFontMetrics qfm(gpg_id_font);

            text_item_gopigo_id[gopigo_index]->setPos(70-18 + 70*x + qfm.horizontalAdvance(gpg_text) / 5, 420-18 - 70*y + qfm.height() / 5);
            text_item_gopigo_id[gopigo_index]->show();
            gopigo_index++;
        }
    }
}

void Graphics::update_system_state_indicator(quint8 state)
{
    // update system state indicator
    switch(state)
    {
        case 0:
            item_system_state_red->show();
            item_system_state_green->hide();
            item_system_state_blue->hide();
            item_system_state_grey->hide();
        break;
        case 1:
            item_system_state_red->hide();
            item_system_state_green->show();
            item_system_state_blue->hide();
            item_system_state_grey->hide();
        break;
        case 2:
            item_system_state_red->hide();
            item_system_state_green->hide();
            item_system_state_blue->show();
            item_system_state_grey->hide();
        break;
        case 4:
            item_system_state_red->hide();
            item_system_state_green->hide();
            item_system_state_blue->hide();
            item_system_state_grey->show();
        break;
    }
}

void Graphics::hide_all_gopigos(quint8 device_count, VarastoRoboDevice* device_list)
{
    quint8 gopigo_index = 0;
    for(quint8 i = 0; i < device_count; ++i)
    {
        if(device_list[i].type == 2)
        {
            pixmap_item_gopigo[gopigo_index]->hide();
            text_item_gopigo_id[gopigo_index]->hide();
            gopigo_index++;
        }
    }
}
