#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include "systembroadcastmessage.h"
#include "point.h"

class Graphics
{
public:
    Graphics();
    QGraphicsScene scene;
    void hide_all_obstacles();
    void show_found_obstacles(quint8 obstacle_count, Point* obstacle_list);
    void update_gopigo_locations(quint8 device_count, VarastoRoboDevice* device_list);
    void update_system_state_indicator(quint8 state);
    void hide_all_gopigos(quint8 device_count, VarastoRoboDevice* device_list);

private:
    QPixmap pixmap_graphicsview_grid;
    QPixmap pixmap_gopigo_grid;
    QPixmap pixmap_ur5;
    QPixmap pixmap_gopigo_list[4];
    QPixmap pixmap_remote_controlled_list[4];
    QPixmap pixmap_obstacle_list[44];

    QFont gpg_id_font;
    QFont gpg_id_font_remote;

    QPixmap pixmap_system_state_red;
    QPixmap pixmap_system_state_green;
    QPixmap pixmap_system_state_blue;
    QPixmap pixmap_system_state_grey;

    QGraphicsPixmapItem* pixmap_item_graphicsview_grid;
    QGraphicsPixmapItem* pixmap_item_gopigo_grid;
    QGraphicsPixmapItem* pixmap_item_ur5;
    QGraphicsPixmapItem* pixmap_item_gopigo[4];
    QGraphicsPixmapItem* pixmap_item_remote_controlled[4];
    QGraphicsPixmapItem* pixmap_item_obstacle[44];

    QGraphicsTextItem* text_item_gopigo_id[4];
    QGraphicsTextItem* text_item_gopigo_id_remote[4];

    QGraphicsItem* item_system_state_red;
    QGraphicsItem* item_system_state_green;
    QGraphicsItem* item_system_state_blue;
    QGraphicsItem* item_system_state_grey;
};

#endif // GRAPHICS_H
