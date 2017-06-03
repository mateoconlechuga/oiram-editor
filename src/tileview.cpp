#include "tileview.h"
#include "utils.h"
#include "level.h"

TileView::TileView(QWidget *parent) : QGraphicsView(parent) {

}

void TileView::loadTiles(QString path) {
    pixTileset.load(path);

    mWidth = pixTileset.width()/TILE_WIDTH;
    mHeight = pixTileset.height()/TILE_HEIGHT;

    scene()->items().clear();
    scene()->setSceneRect(0, 0, pixTileset.width(), pixTileset.height());

    QImage gray(pixTileset.toImage());

    int pixels = gray.width() * gray.height();
    QRgb *data = (unsigned int*)gray.bits();
    for (int i = 0; i < pixels; ++i) {
        int val = qGray(data[i]);
        data[i] = qRgba(val, val, val, qAlpha(data[i]));
    }

    QPixmap px = QPixmap::fromImage(gray);
    QPainter painter(&px);
    painter.setPen(QPen(QBrush(QColor(0,0,0,70)), 1, Qt::DashLine));
    for (int i=0; i<gray.width(); i+= TILE_WIDTH) {
        painter.drawLine(i, 0, i, gray.height());
    }
    for (int i=0; i<gray.height(); i+= TILE_HEIGHT) {
        painter.drawLine(0, i, gray.width(), i);
    }
    grayPixmap = new QGraphicsPixmapItem(px);
    grayPixmap->setZValue(0);
    scene()->addItem(grayPixmap);

    mTiles = new Tile**[mWidth];
    for(int i=0; i<mWidth; i++) {
        mTiles[i] = new Tile*[mHeight];
        for(int j=0; j<mHeight; j++) {
            mTiles[i][j] = new Tile(true, tilemap_ids[i][j]);
            scene()->addItem(mTiles[i][j]);
            mTiles[i][j]->setPos(i * TILE_WIDTH, j * TILE_HEIGHT);
        }
    }
}

void TileView::setMode(int newMode) {
    clearSelection();
    if ((mode == BRUSH_MODE && newMode == FILL_MODE) || (mode == FILL_MODE && newMode == BRUSH_MODE)) {
        if (selection) delete selection;
        selection = new QPixmap(pixTileset.copy(startLoopX * TILE_WIDTH, startLoopY * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT));
        mTiles[startLoopX][startLoopY]->setSelected(true);
        selectionUpdated(selection, startLoopX, startLoopY, 1, 1);
    }
    mode = newMode;
}

void TileView::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::LeftButton) {
        if (mode == PIPE_EXIT_MODE) {
            e->ignore();
            return;
        }
        if (mode == SELECT_MODE || mode == ENEMY_MODE || mode == PIPE_ENTER_MODE) {
            changeToBrush();
            mode = BRUSH_MODE;
        }
        QPointF pos = mapToScene(e->pos());
        int xpos = pos.x()/TILE_WIDTH;
        int ypos = pos.y()/TILE_WIDTH;
        if ((xpos >= 0 && xpos < mWidth) && (ypos >= 0 && ypos < mHeight)) {
            endLoopX = startLoopX = xpos;
            endLoopY = startLoopY = ypos;
        }
    }
    QGraphicsView::mousePressEvent(e);
}

void TileView::clearSelection() {
    for(int i=0; i<mWidth; i++) {
        for(int j=0; j<mHeight; j++) {
            mTiles[i][j]->setSelected(false);
        }
    }

    if (selection) { delete selection; selection = Q_NULLPTR; }
}

void TileView::mouseMoveEvent(QMouseEvent *e) {
    QPointF pos = mapToScene(e->pos());
    int xpos = pos.x()/TILE_WIDTH;
    int ypos = pos.y()/TILE_WIDTH;

    if ((xpos >= 0 && xpos < mWidth) && (ypos >= 0 && ypos < mHeight)) {
        if (mode == FILL_MODE && hold) {
            if (startx != xpos || starty != ypos) {
                triggerNewMode(BRUSH_MODE);
                mode = BRUSH_MODE;
            }

        }
        if (!hold) {
            startx = xpos;
            starty = ypos;
            hold = true;
        }
        if (mode == BRUSH_MODE) {

            if (xpos < startx) {
                endLoopX = startx;
                startLoopX = xpos;
            } else {
                endLoopX = xpos;
                startLoopX = startx;
            }

            if (ypos < starty) {
                endLoopY = starty;
                startLoopY = ypos;
            } else {
                endLoopY = ypos;
                startLoopY = starty;
            }

            clearSelection();

            for(int i=startLoopX; i<=endLoopX; i++) {
                for(int j=startLoopY; j<=endLoopY; j++) {
                    mTiles[i][j]->setSelected(true);
                }
            }
        }
    }
    QGraphicsView::mouseMoveEvent(e);
}

void TileView::mouseReleaseEvent(QMouseEvent *e) {
    int width = endLoopX - startLoopX + 1;
    int height = endLoopY - startLoopY + 1;

    if (selection) delete selection;
    selection = new QPixmap(pixTileset.copy(startLoopX * TILE_WIDTH, startLoopY * TILE_HEIGHT, width * TILE_WIDTH, height * TILE_HEIGHT));

    selectionUpdated(selection, startLoopX, startLoopY, width, height);

    hold = false;
    QGraphicsView::mouseReleaseEvent(e);
}

void TileView::paintEvent(QPaintEvent *e) {
    if (!isEnabled()) {
        grayPixmap->setZValue(10000);
    } else {
        grayPixmap->setZValue(0);
    }
    QGraphicsView::paintEvent(e);
}

void TileView::drawForeground(QPainter *painter, const QRectF &rect) {
    QGraphicsView::drawForeground(painter, rect);
}
