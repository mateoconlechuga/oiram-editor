#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QMessageBox>

#include "tilemapview.h"
#include "utils.h"
#include "tile.h"
#include "element.h"
#include "level.h"

TilemapView::TilemapView(QWidget *parent) : QGraphicsView(parent) {
    setMouseTracking(true);
    installEventFilter(this);
    setAcceptDrops(true);
}

void TilemapView::setMode(int newMode) {
    mode = newMode;
    selector.removePixmap();
    clearSelection();
    setMouseTracking(newMode != SELECT_MODE);
}

void TilemapView::deleteTilemapData() {
    scene()->removeItem(&selector);
    scene()->clear();
}

void TilemapView::selectionUpdated(QPixmap *pixmap, int x, int y, int width, int height) {
    selector.setElement(x, y, width, height, pixmap);
}

void TilemapView::floodFill(int x, int y) {
    if ((x >= 0 && x < mWidth) && (y >= 0 && y < mHeight)) {
        uint8_t id = mTilemap[x][y]->getID();
        if (id == oldID) {
            setID(x, y, newID);
            floodFill(x+1,y);
            floodFill(x-1,y);
            floodFill(x,y+1);
            floodFill(x,y-1);
        }
    }
}

void TilemapView::setCurrentElement() {
    selector.setFlags(QGraphicsItem::ItemSendsGeometryChanges);
    selector.setOpacity(0.5);
    scene()->addItem(&selector);
}

void TilemapView::mouseDoubleClickEvent(QMouseEvent *e) {
    e->ignore();
}

bool TilemapView::eventFilter(QObject *obj, QEvent *e) {
    if (e->type() == QEvent::Leave) {
        mouseLeft();
        selector.setVisible(false);
    }
    return QGraphicsView::eventFilter(obj, e);
}

void TilemapView::removePipeEnter() {
    scene()->removeItem(overlay);
    delete overlay;
    curPipe->enterX = curPipe->enterY =
    curPipe->exitX = curPipe->exitY = 255;
    curPipe->enabled = false;
}

void TilemapView::setID(int x, int y, int id) {
    int prev_id = mTilemap[x][y]->getID();
    if (!id) { id = 27; }
    if (!loading && !resizing) { if (prev_id == id) { return; } }

    if (!needSave) {
        setSaveFlag();
        needSave = true;
    }

    if(!loading && prev_id >= 0xf0) {
        if (prev_id == OIRAM_ENEMY) {
            if (oiramElement) {
                scene()->removeItem(oiramElement);
                delete oiramElement;
                oiramElement = Q_NULLPTR;
            }
            pack.level[currLevel].oiramX = pack.level[currLevel].oiramY = 255;
        } else if (!resizing) {
            if(prev_id != 14 && prev_id != 16 && prev_id != 18 && prev_id != 20) {
                Element *overlay = static_cast<Element*>(scene()->collidingItems(mTilemap[x][y], Qt::IntersectsItemShape).last());
                scene()->removeItem(overlay);
                delete overlay;
            }
        }
    }

    // test pipes
    if (!loading) {
        if(prev_id == 14 || prev_id == 16 || prev_id == 18 || prev_id == 20 || prev_id == 231 || prev_id == 232) {
            QList<QGraphicsItem*> itemList = scene()->collidingItems(mTilemap[x][y], Qt::IntersectsItemShape);
            Element *currP;
            for (int i=0; i<itemList.count(); i++) {
                Element *curr = static_cast<Element*>(itemList.at(i));
                if (curr->isElementOrTile() == IS_ELEMENT) {
                    if (curr->getPipe()) {
                        pipe_t *thisPipe = &pack.level[curLevel].pipesDoors[curr->ID];
                        if (thisPipe->enabled) {
                            pack.level[curLevel].pipesDoorsCount--;
                            thisPipe->enabled = false;
                        }

                        Element testPos;
                        testPos.setPos(thisPipe->enterX * TILE_WIDTH, thisPipe->enterY * TILE_HEIGHT);
                        thisPipe->enterY = thisPipe->enterX = 255;

                        bool k = false;

                        do {
                            QList<QGraphicsItem*> pipeList = scene()->collidingItems(&testPos, Qt::IntersectsItemShape);
                            for (int j=0; j<pipeList.count(); j++) {
                                currP = static_cast<Element*>(pipeList.at(j));
                                if (currP->isElementOrTile() == IS_ELEMENT) {
                                    if (currP->getPipe()) {
                                        scene()->removeItem(currP);
                                        delete currP;
                                        break;
                                    }
                                }
                            }
                            testPos.setPos(thisPipe->exitX * TILE_WIDTH, thisPipe->exitY * TILE_HEIGHT);
                            thisPipe->exitX = thisPipe->exitY = 255;
                            k ^= true;
                        } while(k);

                        i = -1;
                        itemList = scene()->collidingItems(mTilemap[x][y], Qt::IntersectsItemShape);
                    }
                }
            }
        }
    }

    if (id >= 0xf0) {
        if (id == OIRAM_ENEMY) {
            level_t *l = &pack.level[curLevel];
            if (!(l->oiramX == 255 && l->oiramY == 255)) {
                if (oiramElement) {
                    scene()->removeItem(oiramElement);
                    delete oiramElement;
                    oiramElement = Q_NULLPTR;
                }
                mTilemap[l->oiramX][l->oiramY]->setID(27);
            }
            l->oiramX = x;
            l->oiramY = y;
        }
        Element *overlay = new Element();
        overlay->setPos(x * TILE_WIDTH, y * TILE_HEIGHT);
        overlay->ID = id;
        overlay->setHighlight(false);
        switch (id) {
            case GOOMBA_ENEMY:
                    overlay->setElement(0, 0, 1, 1, pixGoomba);
                    break;
            case RED_KOOPA_ENEMY:
                    overlay->setElement(0, 0, 1, 2, pixRedKoopa);
                    overlay->setElementOffsets(0, 5);
                    break;
            case GREEN_KOOPA_ENEMY:
                    overlay->setElement(0, 0, 1, 2, pixGreenKoopa);
                    overlay->setElementOffsets(0, 5);
                    break;
            case RED_KOOPA_FLY_ENEMY:
                    overlay->setElement(0, 0, 1, 2, pixRedKoopaFly);
                    overlay->setElementOffsets(0, 5);
                    break;
            case GREEN_KOOPA_FLY_ENEMY:
                    overlay->setElement(0, 0, 1, 2, pixGreenKoopaFly);
                    overlay->setElementOffsets(0, 5);
                    break;
            case FISH_ENEMY:
                    overlay->setElement(0, 0, 1, 1, pixFish);
                    break;
            case CHOMPER_ENEMY:
                    overlay->setElement(0, 0, 2, 1, pixChomper);
                    overlay->setElementOffsets(8, 0);
                    break;
            case CHOMPER_FIRE_ENEMY:
                    overlay->setElement(0, 0, 2, 1, pixFireChomper);
                    overlay->setElementOffsets(8, 0);
                    break;
            case THWOMP_ENEMY:
                    overlay->setElement(0, 0, 2, 2, pixThwomp);
                    overlay->setElementOffsets(4, 0);
                    break;
            case FIREBALL_ENEMY:
                    overlay->setElement(0, 0, 1, 1, pixFireball);
                    overlay->setElementOffsets(1, 0);
                    break;
            case BOO_ENEMY:
                    overlay->setElement(0, 0, 1, 1, pixBoo);
                    break;
            case BONES_ENEMY:
                    overlay->setElement(0, 0, 1, 2, pixBones);
                    overlay->setElementOffsets(0, 5);
                    break;
            case SPIKE_ENEMY:
                    overlay->setElement(0, 0, 1, 1, pixSpike);
                    break;
            case OIRAM_ENEMY:
                    overlay->setElement(0, 0, 1, 1, pixOiram);
                    break;
            case RESWOB_ENEMY:
                    overlay->setElement(0, 0, 2, 3, pixReswob);
                    overlay->setElementOffsets(0, 8);
                    break;
            default:
                    break;
        }
        overlay->setZValue(15000);
        if (id == OIRAM_ENEMY) {
            oiramElement = overlay;
        }
        scene()->addItem(overlay);
    }
    mTilemap[x][y]->setID(id);
}

void TilemapView::mousePressEvent(QMouseEvent *e) {
    QPointF loc = mapToScene(e->pos());
    int xpos = loc.x()/TILE_WIDTH;
    int ypos = loc.y()/TILE_HEIGHT;

    if(e->button() == Qt::LeftButton) {
        if (lockedDrawing) {
            if (drawDir == DRAW_VERTICAL) {
                xpos = forceX;
            } else
            if (drawDir == DRAW_HORIZONTAL) {
                ypos = forceY;
            }
        }
        if (mode == BRUSH_MODE) {
            hold = true;
            selector.setPos(xpos * TILE_WIDTH, ypos * TILE_HEIGHT);
            placeElement(xpos, ypos);
        }
        if ((xpos >= 0 && xpos < mWidth) && (ypos >= 0 && ypos < mHeight)) {
            if (mode == FILL_MODE) {
                oldID = mTilemap[xpos][ypos]->getID();
                newID = selector.getElementID(0,0);
                if (newID != oldID && newID != 13 && newID) {
                    floodFill(xpos,ypos);
                    scene()->update();
                }
            } else
            if (mode == SELECT_MODE) {
                clearSelection();
                endLoopX = startLoopX = xpos;
                endLoopY = startLoopY = ypos;
                mTilemap[xpos][ypos]->setSelected(true);
            } else
            if (mode == ENEMY_MODE) {
                setID(xpos, ypos, selector.ID);
            } else
            if (mode == PIPE_ENTER_MODE || (mode == PIPE_EXIT_MODE && selector.ID != 255)) {
                int id = mTilemap[xpos][ypos]->getID();
                overlay = new Element();

                do {
                    if (selector.ID & (MASK_DOOR_E | MASK_DOOR_X)) {
                        if (id != 231 && id != 232)  {
                            delete overlay;
                            break;
                        }
                    } else
                    if (selector.ID & (MASK_PIPE_LEFT | MASK_PIPE_RIGHT)) {
                        if (id != 18 && id != 20) {
                            delete overlay;
                            break;
                        }
                    } else
                    if (id != 14 && id != 16) {
                        delete overlay;
                        break;
                    }

                    if (selector.ID & MASK_DOOR_E) {
                        overlay->setElement(0, 0, 1, 2, pixDoorEnter);
                    } else
                    if (selector.ID & MASK_DOOR_X) {
                        overlay->setElement(0, 0, 1, 2, pixDoorExit);
                    } else
                    if (selector.ID & MASK_PIPE_LEFT) {
                        overlay->setElement(0, 0, 1, 2, pixPipeLeft);
                    } else
                    if (selector.ID & MASK_PIPE_RIGHT) {
                        overlay->setElement(0, 0, 1, 2, pixPipeRight);
                    } else
                    if (selector.ID & MASK_PIPE_UP) {
                        overlay->setElement(0, 0, 2, 1, pixPipeUp);
                    } else {
                        overlay->setElement(0, 0, 2, 1, pixPipeDown);
                    }

                    if (mode == PIPE_ENTER_MODE) {
                        pipeIndex = findAvailablePipe();
                        curPipe = &pack.level[curLevel].pipesDoors[pipeIndex];
                        curPipe->enterX = xpos;
                        curPipe->enterY = ypos;
                        curPipe->enterDir = selector.ID;
                        emit placedEnterPipeDoor();
                    } else {
                        curPipe->exitX = xpos;
                        curPipe->exitY = ypos;
                        curPipe->exitDir = selector.ID;
                        curPipe->enabled = true;
                        pack.level[curLevel].pipesDoorsCount++;
                        emit placedExitPipeDoor();
                    }

                    overlay->setPipe();
                    overlay->setPos(xpos * TILE_WIDTH, ypos * TILE_HEIGHT);
                    overlay->ID = pipeIndex;
                    overlay->setHighlight(false);
                    overlay->setZValue(15500);
                    scene()->addItem(overlay);
                } while(0);
            }
        }
    }
}

void TilemapView::saveLevel() {
    if (!pack.count) { return; }
    level_t level;
    level.data = static_cast<uint8_t*>(malloc(mHeight * mWidth));
    level.height = mHeight;
    level.width = mWidth;
    for(int i=0; i<mHeight; i++) {
        int offset = i*mWidth;
        for(int j=0; j<mWidth; j++) {
            level.data[j+offset] = mTilemap[j][i]->getID();
        }
    }
    setLevel(currLevel, &level);
    free(level.data);
}

void TilemapView::loadPipes() {
    for (int i=0; i<255; i++) {
        pipe_t *thisPipe = &pack.level[curLevel].pipesDoors[i];
        if (thisPipe->enabled) {
            if (thisPipe->enterX > mWidth || thisPipe->enterY > mHeight || thisPipe->exitX > mWidth || thisPipe->exitY > mHeight) {
                thisPipe->enabled = false;
                thisPipe->enterX = thisPipe->exitX = 255;
                thisPipe->enterY = thisPipe->exitY = 255;
                continue;
            }
            Element *overlayEnter = new Element();
            Element *overlayExit = new Element();

            if (thisPipe->enterDir & MASK_DOOR_E) {
                overlayEnter->setElement(0, 0, 1, 2, pixDoorEnter);
            } else
            if (thisPipe->enterDir & MASK_DOOR_X) {
                overlayEnter->setElement(0, 0, 1, 2, pixDoorExit);
            } else
            if (thisPipe->enterDir & MASK_PIPE_LEFT) {
                overlayEnter->setElement(0, 0, 1, 2, pixPipeLeft);
            } else
            if (thisPipe->enterDir & MASK_PIPE_RIGHT) {
                overlayEnter->setElement(0, 0, 1, 2, pixPipeRight);
            } else
            if (thisPipe->enterDir & MASK_PIPE_UP) {
                overlayEnter->setElement(0, 0, 2, 1, pixPipeUp);
            } else {
                overlayEnter->setElement(0, 0, 2, 1, pixPipeDown);
            }

            overlayEnter->setPipe();
            overlayEnter->setPos(thisPipe->enterX * TILE_WIDTH, thisPipe->enterY * TILE_HEIGHT);
            overlayEnter->ID = i;
            overlayEnter->setHighlight(false);
            overlayEnter->setZValue(15500);

            if (thisPipe->exitDir & MASK_DOOR_E) {
                overlayExit->setElement(0, 0, 1, 2, pixDoorEnter);
            } else
            if (thisPipe->exitDir & MASK_DOOR_X) {
                overlayExit->setElement(0, 0, 1, 2, pixDoorExit);
            } else
            if (thisPipe->exitDir & MASK_PIPE_LEFT) {
                overlayExit->setElement(0, 0, 1, 2, pixPipeLeft);
            } else
            if (thisPipe->exitDir & MASK_PIPE_RIGHT) {
                overlayExit->setElement(0, 0, 1, 2, pixPipeRight);
            } else
            if (thisPipe->exitDir & MASK_PIPE_UP) {
                overlayExit->setElement(0, 0, 2, 1, pixPipeUp);
            } else {
                overlayExit->setElement(0, 0, 2, 1, pixPipeDown);
            }

            overlayExit->setPipe();
            overlayExit->setPos(thisPipe->exitX * TILE_WIDTH, thisPipe->exitY * TILE_HEIGHT);
            overlayExit->ID = i;
            overlayExit->setHighlight(false);
            overlayExit->setZValue(15100);

            scene()->addItem(overlayExit);
            scene()->addItem(overlayEnter);
        }
    }
}

void TilemapView::loadLevel(uint8_t get) {
    uint8_t width, height;

    level_t *cur = getLevel(currLevel = get);

    loading = true;

    width = cur->width;
    height = cur->height;

    resize(width, height);

    for(int i=0; i<height; i++) {
        int offset = i*width;
        for(int j=0; j<width; j++) {
            setID(j, i, cur->data[j+offset]);
        }
    }

    level.width = mWidth;
    level.height = mHeight;
    scene()->update();

    loading = false;
}

void TilemapView::setNewLevel(uint8_t newLevel) {
    currLevel = newLevel;
}

void TilemapView::placeElement(int xpos, int ypos) {
    for(int x=0; x<selector.getElementWidth(); x++) {
        for(int y=0; y<selector.getElementHeight(); y++) {
            int stox = xpos+x;
            int stoy = ypos+y;
            if (stox >= 0 && stox < mWidth && stoy >= 0 && stoy < mHeight) {
                int thisID = selector.getElementID(x,y);
                if (thisID != 13) {
                    setID(stox, stoy, thisID);
                }
            }
        }
    }
}

void TilemapView::clearSelection() {
    foreach(QGraphicsItem* item, scene()->selectedItems()) {
        Tile* element = static_cast<Tile*>(item);

        element->setSelected(false);
    }
}

void TilemapView::mouseMoveEvent(QMouseEvent *e) {
    QPointF loc = mapToScene(e->pos());
    int xpos = loc.x()/TILE_WIDTH;
    int ypos = loc.y()/TILE_HEIGHT;
    selector.setVisible(mode != SELECT_MODE);

    if (mode != SELECT_MODE) {
        int id;
        if (lockedDrawing) {
            if (drawDir == DRAW_NOT_SET) {
                forceX = xpos;
                forceY = ypos;
                drawDir = DRAW_SET;
            } else
            if (drawDir == DRAW_SET) {
                if (forceX != xpos) {
                    drawDir = DRAW_HORIZONTAL;
                } else
                if (forceY != ypos) {
                    drawDir = DRAW_VERTICAL;
                }
            } else
            if (drawDir == DRAW_VERTICAL) {
                xpos = forceX;
            } else
            if (drawDir == DRAW_HORIZONTAL) {
                ypos = forceY;
            }
        }
        selector.setPos(xpos * TILE_WIDTH, ypos * TILE_HEIGHT);
        if (mode == BRUSH_MODE) {
            if (hold) {
                placeElement(xpos, ypos);
            }
        }
        if (xpos >= 0 && xpos < mWidth && ypos >= 0 && ypos < mHeight) {
            id = mTilemap[xpos][ypos]->getID();
        } else { id = 27; }
        updatePos(xpos, ypos, id);
    } else {
        if ((xpos >= 0 && xpos < mWidth) && (ypos >= 0 && ypos < mHeight)) {
            if (!hold) {
                startx = xpos;
                starty = ypos;
                hold = true;
            }

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
                    mTilemap[i][j]->setSelected(true);
                }
            }

            updatePos(endLoopX - startLoopX + 1, endLoopY - startLoopY + 1, 0);
        }
    }
    QGraphicsView::mouseMoveEvent(e);
}

void TilemapView::mouseReleaseEvent(QMouseEvent *e) {
    hold = false;
    QGraphicsView::mouseReleaseEvent(e);
}

void TilemapView::paintEvent(QPaintEvent *e) {
    QGraphicsView::paintEvent(e);
}

void TilemapView::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Shift) {
        drawDir = DRAW_NOT_SET;
        lockedDrawing = true;
    }
    QGraphicsView::keyPressEvent(e);
}

void TilemapView::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Shift) {
        drawDir = DRAW_NOT_SET;
        lockedDrawing = false;
    }
    QGraphicsView::keyReleaseEvent(e);
}

void TilemapView::drawForeground(QPainter *painter, const QRectF &rect) {
    QGraphicsView::drawForeground(painter, rect);
}

void TilemapView::toggleGrid() {
    gridEnabled = !gridEnabled;
    for(int i=0;i<mWidth;i++) {
        for(int j=0; j<mHeight; j++) {
            mTilemap[i][j]->setGrid(gridEnabled);
        }
    }
    repaint();
    update();
    viewport()->update();
}

bool TilemapView::resize(int width, int height) {
    if(!loading) { if (width == mWidth && height == mHeight) { return true; } }
    // don't allow maps greater than 10K in size
    if (width * height > 10000) {
        return false;
    }

    if (!needSave) {
        setSaveFlag();
        needSave = true;
    }

    resizing = true;

    if(mTilemap == Q_NULLPTR) {
        mTilemap = new Tile**[width];
        for(int i=0;i<width;i++) {
            mTilemap[i] = new Tile*[height];
            for(int j=0; j<height; j++) {
                mTilemap[i][j] = new Tile(gridEnabled);
                scene()->addItem(mTilemap[i][j]);
                mTilemap[i][j]->setPos(i * TILE_WIDTH, j * TILE_HEIGHT);
            }
        }
        selector.setFlags(QGraphicsItem::ItemSendsGeometryChanges);
        selector.setOpacity(0.5);
        selector.setZValue(0x10000);
        scene()->addItem(&selector);
    } else {

        scene()->removeItem(&selector);

        Tile*** pTilemap = new Tile**[width];
        for(int i=0; i<width; i++) {
            pTilemap[i] = new Tile*[height];
            for(int j=0; j<height; j++) {
                pTilemap[i][j] = new Tile(gridEnabled);
                if(i < mWidth && j < mHeight) {
                    pTilemap[i][j]->setID(mTilemap[i][j]->getID());
                }
                pTilemap[i][j]->setPos(i * TILE_WIDTH, j * TILE_HEIGHT);
            }
        }

        scene()->clear();
        oiramElement = Q_NULLPTR;

        for(int i=0;i<width;i++) {
            for(int j=0; j<height; j++) {
                scene()->addItem(pTilemap[i][j]);
            }
        }

        for(int i=0; i<mWidth; i++) {
            delete[] mTilemap[i];
        }

        delete[] mTilemap;

        mTilemap = pTilemap;

        if (!loading) {
            for(int i=0;i<width;i++) {
                for(int j=0; j<height; j++) {
                    setID(i, j, mTilemap[i][j]->getID());
                }
            }
        }

        scene()->addItem(&selector);
    }

    mWidth = width;
    mHeight = height;

    loadPipes();

    scene()->setSceneRect(0, 0, width * TILE_WIDTH, height * TILE_WIDTH);

    resizing = false;

    return true;
}


void TilemapView::doCopy() {
    if (mode == SELECT_MODE) {
        clipboard.clear();
        for(int i=startLoopX; i<=endLoopX; i++) {
            for(int j=startLoopY; j<=endLoopY; j++) {
                clipboard.append(mTilemap[i][j]->getID());
            }
        }
        clipboardWidth = endLoopX - startLoopX + 1;
        clipboardHeight = endLoopY - startLoopY + 1;
    }
}

void TilemapView::doCut() {
    if (mode == SELECT_MODE) {
        doCopy();
        doDelete();
    }
}

void TilemapView::doPaste() {
    if (mode == SELECT_MODE) {
        if(!clipboard.count()) return;
        int o = 0;
        int height = startLoopY+clipboardHeight;
        int width = startLoopX+clipboardWidth;
        for(int i=startLoopX; i<width; i++) {
            for(int j=startLoopY; j<height; j++) {
                if (i<mWidth && j<mHeight) {
                    setID(i, j, clipboard.at(o));
                }
                o++;
            }
        }
        clearSelection();
        scene()->update();
    }
}

void TilemapView::doDelete() {
    if (mode == SELECT_MODE) {
        if (!needSave) {
            setSaveFlag();
            needSave = true;
        }

        foreach(QGraphicsItem* item, scene()->selectedItems()) {
            Tile* thisTile = static_cast<Tile*>(item);
            setID(((int)thisTile->pos().x())/TILE_WIDTH, ((int)thisTile->pos().y())/TILE_HEIGHT, 27);
        }
        clearSelection();
        scene()->update();
    }
}

void TilemapView::dropEvent(QDropEvent *e) {
    e->accept();
    inDrag = false;
}

void TilemapView::dragEnterEvent(QDragEnterEvent *e) {
    e->accept();
    inDrag = true;
}

void TilemapView::dragLeaveEvent(QDragLeaveEvent *e) {
    e->accept();
    inDrag = false;
}
