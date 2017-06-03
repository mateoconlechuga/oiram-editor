#ifndef TILE_H
#define TILE_H

#include <QGraphicsItem>
#include <stdint.h>
#include "utils.h"

class Tile : public QGraphicsItem {

public:
    explicit Tile(bool onGrid, int newID = 27);
    uint8_t getID();
    void setID(int newID);
    virtual int isElementOrTile() { return IS_TILE; }

    QPainterPath shape() const;
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setGrid(bool state);

private:
    int id;
    bool gridOn;
};

#endif
