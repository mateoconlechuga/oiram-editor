#include "element.h"
#include <QGraphicsScene>
#include <QPixmapCache>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QStyleOption>
#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "tile.h"
#include "tileview.h"
#include "level.h"
#include "utils.h"

Element::Element() {
    setElement(15,12,1,1,Q_NULLPTR);
}

QRectF Element::boundingRect() const {
    return QRectF(0, 0, mWidth, mHeight);
}

QPainterPath Element::shape() const {
    QPainterPath p;
    p.addRect(0, 0, TILE_WIDTH, TILE_HEIGHT);
    return p;
}

void Element::setPipe() {
    isPipe = true;
}

bool Element::getPipe() {
    return isPipe;
}

void Element::setElement(int x, int y, int width, int height, QPixmap *texture) {
    mAnchorX = x;
    mAnchorY = y;
    mWidthM = width;
    mHeightM = height;
    mWidth = width * TILE_WIDTH;
    mHeight = height * TILE_HEIGHT;
    mTexture = texture;
    setElementOffsets(0,0);
}

void Element::setElementOffsets(int x, int y) {
    xOffset = x;
    yOffset = y;
}

uint8_t Element::getElementID(int x, int y) {
    if (ID) {
        return ID;
    }
    return tilemap_ids[mAnchorX+x][mAnchorY+y];
}

void Element::setHighlight(bool state) {
    highlight = state;
}

void Element::removePixmap() {
    setElement(15,12,1,1,Q_NULLPTR);
}

int Element::getElementWidth() {
    return mWidthM;
}

int Element::getElementHeight() {
    return mHeightM;
}

void Element::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(widget);
    Q_UNUSED(option);

    if (mTexture) {
        painter->drawPixmap(xOffset, yOffset, mTexture->width(), mTexture->height(), *mTexture);
    }
    if (highlight) {
        painter->fillRect(0, 0, mWidth, mHeight, QColor(50,0,200,90));
    }
}
