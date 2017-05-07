#ifndef TILEMAPVIEW_H
#define TILEMAPVIEW_H

#include <QWidget>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QString>
#include <QMenu>

#include "tile.h"
#include "element.h"
#include "level.h"

class TilemapView : public QGraphicsView {
    Q_OBJECT

public slots:
    void selectionUpdated(QPixmap*, int x, int y, int width, int height);
    void doDelete();
    void doCopy();
    void doCut();
    void doPaste();

public:
    explicit TilemapView(QWidget *p = Q_NULLPTR);
    bool resize(int width, int height);
    void setCurrentElement();
    void setMode(int mode);
    void clearSelection();
    void saveLevel();
    void loadLevel(uint8_t);
    void setNewLevel(uint8_t level);
    void setID(int x, int y, int id);
    void deleteTilemapData();
    void removePipeEnter();

    Element selector;
    Element *overlay;
    bool needSave = false;

signals:
    void updatePos(int x, int y, int id);
    void placedEnterPipeDoor();
    void placedExitPipeDoor();
    void mouseLeft();
    void setSaveFlag();

protected:
    virtual void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    virtual void drawForeground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    virtual void keyReleaseEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *obj, QEvent *e) Q_DECL_OVERRIDE;
    virtual void dropEvent(QDropEvent*) Q_DECL_OVERRIDE;
    virtual void dragEnterEvent(QDragEnterEvent*) Q_DECL_OVERRIDE;
    virtual void dragLeaveEvent(QDragLeaveEvent*) Q_DECL_OVERRIDE;

private:
    void placeElement(int xpos, int ypos);
    void floodFill(int x, int y);
    void setFlood();
    void loadPipes();

    int mode;

    int mWidth;
    int mHeight;
    bool hold = false;
    int posX = 0, posY = 0;

    int startx;
    int starty;
    int endLoopX;
    int startLoopX;
    int startLoopY;
    int endLoopY;

    Tile ***mTilemap = Q_NULLPTR;
    uint8_t *mData = Q_NULLPTR;
    QList<uint8_t> clipboard;
    int clipboardWidth;
    int clipboardHeight;

    uint8_t oldID;
    uint8_t newID;

    uint8_t currLevel = 0;
    level_t level;

    bool inDrag = false;
    uint8_t pipeIndex;
    pipe_t *curPipe;
    QList<Element*> pipeElements;
    Element *oiramElement = Q_NULLPTR;
    bool resizing = false;
    bool loading = false;

    enum DRAW_DIR {
        DRAW_NOT_SET=0,
        DRAW_SET,
        DRAW_VERTICAL,
        DRAW_HORIZONTAL
    };

    bool lockedDrawing = false;
    int drawDir = DRAW_SET;
    int forceX = 0;
    int forceY = 0;
};

#endif
