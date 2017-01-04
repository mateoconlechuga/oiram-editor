#include <QPixmapCache>
#include <QPainter>

#include "tile.h"
#include "utils.h"
#include "level.h"

Tile::Tile(int newID) {
    id = newID;
    setFlags(ItemIsSelectable);
}

uint8_t Tile::getID() {
    return id;
}

void Tile::setID(int newID) {
    id = newID;
}

QRectF Tile::boundingRect() const {
    return QRectF(0, 0, TILE_WIDTH, TILE_HEIGHT);
}

QPainterPath Tile::shape() const {
    QPainterPath mPath;
    mPath.addRect(0, 0, TILE_WIDTH, TILE_HEIGHT);
    return mPath;
}

void Tile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(widget);
    Q_UNUSED(option);

    if (id != 13) {
        painter->drawPixmap(0, 0, TILE_WIDTH, TILE_HEIGHT, idToPixmap(id));
    }

    if (isSelected()) {
        painter->fillRect(0, 0, TILE_WIDTH, TILE_HEIGHT, QColor(50,0,200,90));
    }

    painter->setPen(QPen(QBrush(QColor(0,0,0,70)), 1, Qt::DashLine));
    painter->drawRect(0, 0, TILE_WIDTH, TILE_HEIGHT);
}
