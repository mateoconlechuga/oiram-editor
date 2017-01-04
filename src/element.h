#ifndef ELEMENT_H
#define ELEMENT_H

#include <QGraphicsItem>
#include <QString>
#include <QList>

#include "tile.h"

class Element : public QGraphicsItem {
    friend class TilemapView;

public:
    explicit Element();
    virtual int isElementOrTile() { return IS_ELEMENT; }
    void setElement(int x, int y, int width, int height, QPixmap *texture);
    void setElementOffsets(int x, int y);
    uint8_t getElementID(int x, int y);
    int getElementWidth();
    int getElementHeight();
    void setHighlight(bool);
    void setPipe();
    bool getPipe();

    int ID = 0;

private:
    void removePixmap();

    int mAnchorX;
    int mAnchorY;
    int mWidth;
    int mHeight;
    int mWidthM;
    int mHeightM;
    int xOffset;
    int yOffset;
    bool highlight = true;

    bool isPipe = false;

    QPixmap *mTexture = Q_NULLPTR;
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif
