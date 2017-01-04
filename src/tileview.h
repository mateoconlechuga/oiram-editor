#ifndef TILEVIEW_H
#define TILEVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QString>
#include <QMenu>

#include "tile.h"
#include <stdint.h>

class TileView : public QGraphicsView {
    Q_OBJECT

public:
    explicit TileView(QWidget *p = Q_NULLPTR);
    void loadTiles(QString tileset);
    void clearSelection();
    void setMode(int newMode);

signals:
    void updateTilePos(int id);
    void selectionUpdated(QPixmap*, int x, int y, int width, int height);
    void changeToBrush();
    void triggerNewMode(int);

protected:
    virtual void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    virtual void drawForeground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;

private:
    Tile ***mTiles = Q_NULLPTR;
    bool hold = false;
    int startx;
    int starty;
    int endLoopX;
    int startLoopX;
    int startLoopY;
    int endLoopY;
    int mode;
    QPixmap *selection = Q_NULLPTR;
    QPixmap pixTileset;
    int mHeight, mWidth;
    QImage *grayTiles;
    QGraphicsPixmapItem *grayPixmap;
};

#endif
