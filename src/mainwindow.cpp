#include <QColorDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "utils.h"
#include "level.h"

MainWindow::MainWindow(QWidget *p) : QMainWindow(p), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    descriptionLabel.setText(QStringLiteral("Description: "));
    authorLabel.setText(QStringLiteral("Author:"));
    varLabel.setText(QStringLiteral("Name:"));
    descriptionEdit.setMaxLength(25);
    authorEdit.setMaxLength(15);
    varEdit.setMaxLength(8);

    descriptionEdit.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    varEdit.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    authorEdit.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    ui->statusBar->addWidget(&statusLabel);
    ui->toolBar->addWidget(&descriptionLabel);
    ui->toolBar->addWidget(&descriptionEdit);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(&varLabel);
    ui->toolBar->addWidget(&varEdit);
    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(&authorLabel);
    ui->toolBar->addWidget(&authorEdit);
    ui->toolBar->addSeparator();

    initScene();

    connect(ui->graphicsView, &TilemapView::updatePos, this, &MainWindow::updatePos);
    connect(ui->tileView, &TileView::selectionUpdated, ui->graphicsView, &TilemapView::selectionUpdated);

    connect(ui->actionChangePassword, &QAction::triggered, this, &MainWindow::setPassword);
    connect(ui->actionBrush, &QAction::triggered, this, &MainWindow::setBrushMode);
    connect(ui->actionFill, &QAction::triggered, this, &MainWindow::setFillMode);
    connect(ui->actionSelect, &QAction::triggered, this, &MainWindow::setSelectMode);

    connect(ui->graphicsView, &TilemapView::setSaveFlag, this, &MainWindow::setNeedSave);

    connect(ui->buttonChangeColor, &QPushButton::clicked, this, &MainWindow::recolor);
    connect(ui->buttonResize, &QPushButton::clicked, this, &MainWindow::resize);

    connect(ui->tileView, &TileView::changeToBrush, this, &MainWindow::setBrushMode);

    connect(ui->actionCopy, &QAction::triggered, ui->graphicsView, &TilemapView::doCopy);
    connect(ui->actionDelete, &QAction::triggered, ui->graphicsView, &TilemapView::doDelete);
    connect(ui->actionPaste, &QAction::triggered, ui->graphicsView, &TilemapView::doPaste);
    connect(ui->actionCut, &QAction::triggered, ui->graphicsView, &TilemapView::doCut);

    connect(ui->tileView, &TileView::triggerNewMode, this, &MainWindow::setNewMode);

    connect(ui->buttonAdd, &QToolButton::clicked, this, &MainWindow::addLevel);
    connect(ui->buttonRemove, &QToolButton::clicked, this, &MainWindow::deleteLevel);
    connect(ui->buttonLevelDown, &QToolButton::clicked, this, &MainWindow::levelDown);
    connect(ui->buttonLevelUp, &QToolButton::clicked, this, &MainWindow::levelUp);

    connect(ui->graphicsView, &TilemapView::mouseLeft, this, &MainWindow::noStatus);

    connect(ui->goomba, &QToolButton::clicked, this, &MainWindow::addGoomba);
    connect(ui->bones, &QToolButton::clicked, this, &MainWindow::addBones);
    connect(ui->fish, &QToolButton::clicked, this, &MainWindow::addFish);
    connect(ui->thwomp, &QToolButton::clicked, this, &MainWindow::addThwomp);
    connect(ui->red, &QToolButton::clicked, this, &MainWindow::addRedKoopa);
    connect(ui->redfly, &QToolButton::clicked, this, &MainWindow::addRedKoopaFly);
    connect(ui->green, &QToolButton::clicked, this, &MainWindow::addGreenKoopa);
    connect(ui->greenfly, &QToolButton::clicked, this, &MainWindow::addGreenKoopaFly);
    connect(ui->boo, &QToolButton::clicked, this, &MainWindow::addBoo);
    connect(ui->chomper, &QToolButton::clicked, this, &MainWindow::addChomper);
    connect(ui->fireball, &QToolButton::clicked, this, &MainWindow::addFireball);
    connect(ui->firechomper, &QToolButton::clicked, this, &MainWindow::addFireChomper);
    connect(ui->oiram, &QToolButton::clicked, this, &MainWindow::addOiram);
    connect(ui->reswob, &QToolButton::clicked, this, &MainWindow::addReswob);
    connect(ui->spike, &QToolButton::clicked, this, &MainWindow::addSpike);

    connect(ui->pipedown, &QToolButton::clicked, this, &MainWindow::addDownPipe);
    connect(ui->piperight, &QToolButton::clicked, this, &MainWindow::addRightPipe);
    connect(ui->pipeleft, &QToolButton::clicked, this, &MainWindow::addLeftPipe);
    connect(ui->pipeup, &QToolButton::clicked, this, &MainWindow::addUpPipe);
    connect(ui->doorEnter, &QToolButton::clicked, this, &MainWindow::addDoorEnter);

    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::savePack);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::loadPack);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::createNew);

    connect(ui->actionToggleGrid, &QAction::triggered, ui->graphicsView, &TilemapView::toggleGrid);
    connect(ui->actionSaveImage, &QAction::triggered, this, &MainWindow::saveImage);

    connect(ui->comboLevels, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::changeLevel);
    connect(ui->comboScroll, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::changeScroll);

    connect(&descriptionEdit, &QLineEdit::textChanged, this, &MainWindow::setNeedSave);
    connect(&authorEdit, &QLineEdit::textChanged, this, &MainWindow::setNeedSave);
    connect(&varEdit, &QLineEdit::textChanged, this, &MainWindow::setNeedSave);

    connect(ui->graphicsView, &TilemapView::placedEnterPipeDoor, this, &MainWindow::setPipeExitMode);
    connect(ui->graphicsView, &TilemapView::placedExitPipeDoor, this, &MainWindow::setPipeDone);

    connect(ui->buttonPipeCancel, &QPushButton::clicked, this, &MainWindow::cancelPipe);

    setBrushMode();
    clrNeedSave();

    setWindowTitle("Oiram Level Editor " VERSION_STRING);
}

void MainWindow::saveImage() {
    QFileDialog dialog(this, "Save Image");
    dialog.setNameFilter("PNG (*.png)");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(QCoreApplication::applicationDirPath());
    dialog.setDefaultSuffix("png");
    if (!dialog.exec()) {
        return;
    }
    QString fileName = dialog.selectedFiles().first();

    if (!fileName.isNull()) {
        QImage image(ui->graphicsView->scene()->sceneRect().size().toSize(), QImage::Format_ARGB32);
        QPainter painter(&image);
        ui->graphicsView->scene()->render(&painter);
        image.save(fileName);
    }
}

void MainWindow::setPipeEnterMode() {
    if(mode == PIPE_ENTER_MODE) { return; }
    placingPipeDoor = true;
    modeStr = "pipe/door enter";
    setMode(mode = PIPE_ENTER_MODE);
    setSelectionToolsVisible(false);
    if (placingDoor) {
        setOptions(false);
        ui->buttonPipeCancel->setEnabled(true);
        ui->pipedown->setEnabled(false);
        ui->piperight->setEnabled(false);
        ui->pipeleft->setEnabled(false);
        ui->pipeup->setEnabled(false);
        ui->doorEnter->setEnabled(false);
    }
}

void MainWindow::setPipeExitMode() {
    if(mode == PIPE_EXIT_MODE) { return; }
    placedEnterPipeDoor = true;
    modeStr = "pipe/door exit";
    setMode(mode = PIPE_EXIT_MODE);
    ui->graphicsView->selector.ID = 255;
    setSelectionToolsVisible(false);
    setOptions(false);
    if (placingDoor) {
        ui->buttonPipeCancel->setEnabled(true);
        ui->pipedown->setEnabled(false);
        ui->piperight->setEnabled(false);
        ui->pipeleft->setEnabled(false);
        ui->pipeup->setEnabled(false);
        ui->doorEnter->setEnabled(false);
        addDoorExit();
    } else {
        ui->doorEnter->setEnabled(false);
    }
}

void MainWindow::setOptions(bool state) {
    ui->tileView->setEnabled(state);
    ui->goomba->setEnabled(state);
    ui->bones->setEnabled(state);
    ui->fish->setEnabled(state);
    ui->thwomp->setEnabled(state);
    ui->red->setEnabled(state);
    ui->redfly->setEnabled(state);
    ui->green->setEnabled(state);
    ui->greenfly->setEnabled(state);
    ui->boo->setEnabled(state);
    ui->chomper->setEnabled(state);
    ui->fireball->setEnabled(state);
    ui->firechomper->setEnabled(state);
    ui->spinHeight->setEnabled(state);
    ui->spinWidth->setEnabled(state);
    ui->actionBrush->setEnabled(state);
    ui->actionFill->setEnabled(state);
    ui->actionSelect->setEnabled(state);
    ui->buttonChangeColor->setEnabled(state);
    ui->buttonLevelDown->setEnabled(state);
    ui->buttonLevelUp->setEnabled(state);
    ui->buttonAdd->setEnabled(state);
    ui->buttonRemove->setEnabled(state);
    ui->buttonRemove->setEnabled(state);
    ui->comboLevels->setEnabled(state);
    ui->buttonResize->setEnabled(state);
    ui->oiram->setEnabled(state);
    ui->reswob->setEnabled(state);
    ui->spike->setEnabled(state);
    ui->buttonPipeCancel->setEnabled(!state);
    if (state) {
        ui->pipedown->setEnabled(true);
        ui->piperight->setEnabled(true);
        ui->pipeleft->setEnabled(true);
        ui->pipeup->setEnabled(true);
        ui->doorEnter->setEnabled(true);
    }
}

void MainWindow::cancelPipe() {
    if (!placingDoor || (placingDoor && placedEnterPipeDoor)) {
        ui->graphicsView->removePipeEnter();
    }
    placingDoor = placingPipeDoor = placedEnterPipeDoor = false;
    setOptions(true);
    setBrushMode();
}

void MainWindow::setPipeDone() {
    placingDoor = placingPipeDoor = placedEnterPipeDoor = false;
    setNeedSave();
    setOptions(true);
    setBrushMode();
}

void MainWindow::setNeedSave() {
    QIcon enabled(":/resources/icons/save.png");
    ui->graphicsView->needSave = true;
    ui->actionSave->setIcon(enabled);
}

void MainWindow::clrNeedSave() {
    QIcon denabled(":/resources/icons/nosave.png");
    ui->graphicsView->needSave = false;
    ui->actionSave->setIcon(denabled);
}

void MainWindow::closeEvent(QCloseEvent *e) {
    if (!checkSave()) { e->ignore(); return; }
    QMainWindow::closeEvent(e);
}

void MainWindow::setupLevelComboBox() {
    ui->comboLevels->blockSignals(true);
    ui->comboLevels->clear();
    int max = static_cast<int>(pack.count);
    for(int i=1; i<=max; i++) {
        ui->comboLevels->addItem(QString::number(i));
    }
    ui->comboLevels->setCurrentIndex(0);
    ui->comboLevels->blockSignals(false);
}

void MainWindow::setLevel(uint8_t level) {
    level_t *e = &pack.level[level];
    ui->graphicsView->loadLevel(curLevel = level);
    ui->spinWidth->setValue(e->width);
    ui->spinHeight->setValue(e->height);
    ui->comboScroll->blockSignals(true);
    ui->comboScroll->setCurrentIndex(e->scroll);
    ui->comboScroll->blockSignals(false);
    setLevelColor();
}

void MainWindow::createNew() {
    if (!checkSave()) { return; }
    deletePack();
    initPack();
    insertLevel(0);
    setLevel(0);
    ui->spinHeight->setValue(10);
    ui->spinWidth->setValue(20);
    setupLevelComboBox();
    ui->graphicsView->clearSelection();
    ui->tileView->clearSelection();
    descriptionEdit.clear();
    varEdit.clear();
    passwordStr.clear();
    ui->actionSave->setText("Save");
    currPath.clear();
    setNeedSave();
}

bool MainWindow::checkSave() {
    if (ui->graphicsView->needSave) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, QStringLiteral("Save Pack?"),
                                        QStringLiteral("Would you like to save the current pack first?\nAny unsaved information will be lost."),
                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (reply == QMessageBox::Yes) {
            return savePack();
        } else if (reply == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::setResizeSpins() {
    int current = ui->comboLevels->currentIndex();
    uint8_t i = static_cast<uint8_t>(current);

    ui->graphicsView->saveLevel();

    ui->spinHeight->setValue(pack.level[i].height);
    ui->spinWidth->setValue(pack.level[i].width);
}

void MainWindow::loadPackData(const QString &path) {
    char *new_desc, *new_var, *new_author;
    if (loadFilePack(path.toStdString().c_str(), &new_desc, &new_var, &new_author)) {
        varEdit.setText(QString::fromLatin1(new_var));
        descriptionEdit.setText(QString::fromLatin1(new_desc));
        authorEdit.setText(QString::fromLatin1(new_author));
        setupLevelComboBox();
        setLevel(0);
        clrNeedSave();
    }
}

void MainWindow::loadPack() {
    QFileDialog dialog(this);

    if (!checkSave()) { return; }

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setDirectory(currDir);
    dialog.setNameFilter(QStringLiteral("Oiram Level Pack (*.8xv)"));
    dialog.setWindowTitle("Load");
    dialog.setDefaultSuffix(QStringLiteral("8xv"));

    if (dialog.exec()) {
        uint32_t hash;
        currPath = dialog.selectedFiles().first();
        ui->actionSave->setText(currPath);
        if (getPackMetadata(currPath.toStdString().c_str(), &hash)) {
            if (hash) {
                bool ok;
                QString passwd = QInputDialog::getText(this, QStringLiteral("Pack Password"), QStringLiteral("Pack is password protected. Please input password to unlock."), QLineEdit::Normal, QString(), &ok);
                if (ok && !passwd.isEmpty()) {
                    uint32_t check = computeHash(reinterpret_cast<const uint8_t*>(passwd.toStdString().c_str()), passwd.length());
                    if (check == hash) {
                        passwordStr = passwd;
                        loadPackData(currPath);
                    } else {
                        QMessageBox::warning(this, "Incorrect Password", "Incorrect Password specified.");
                        setNeedSave();
                    }
                }
            } else {
                loadPackData(currPath);
            }
        }
    }
    currDir = dialog.directory();
}

bool MainWindow::savePack() {
    QFileDialog dialog(this);
    ui->graphicsView->saveLevel();
    uint32_t hash;

    if (varEdit.text().isEmpty() || authorEdit.text().isEmpty() || descriptionEdit.text().isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Cannot save"),
                                       QStringLiteral("Please set the description, variable name, and author name."));
        return false;
    }

    if (ui->graphicsView->needSave == false) { return false; }

    if (passwordStr.isEmpty()) {
        hash = 0;
    } else {
        hash = computeHash(reinterpret_cast<const uint8_t*>(passwordStr.toStdString().c_str()), passwordStr.length());
    }

    if (currPath.isEmpty()) {

        dialog.setAcceptMode(QFileDialog::AcceptSave);
        dialog.setFileMode(QFileDialog::AnyFile);
        dialog.setDirectory(currDir);
        dialog.selectFile(varEdit.text());
        dialog.setNameFilter(QStringLiteral("Oiram Level Pack (*.8xv)"));
        dialog.setWindowTitle("Save");
        dialog.setDefaultSuffix(QStringLiteral("8xv"));

        if (dialog.exec()) {
            currPath = dialog.selectedFiles().first();
            ui->actionSave->setText(currPath);
            if (saveFilePack(currPath.toStdString().c_str(), descriptionEdit.text().toStdString().c_str(), varEdit.text().toStdString().c_str(), authorEdit.text().toStdString().c_str(), hash)) {
                clrNeedSave();
            }
        }
        currDir = dialog.directory();
    } else {
        if (saveFilePack(currPath.toStdString().c_str(), descriptionEdit.text().toStdString().c_str(), varEdit.text().toStdString().c_str(), authorEdit.text().toStdString().c_str(), hash)) {
            clrNeedSave();
        }
    }
    return true;
}

void MainWindow::addDownPipe() {
    if (placedEnterPipeDoor) {
        setPipeExitMode();
    } else {
        setPipeEnterMode();
    }
    ui->graphicsView->selector.setElement(0, 0, 2, 1, pixPipeDown);
    ui->graphicsView->selector.ID = MASK_PIPE_DOWN;
}
void MainWindow::addRightPipe() {
    if (placedEnterPipeDoor) {
        setPipeExitMode();
    } else {
        setPipeEnterMode();
    }
    ui->graphicsView->selector.setElement(0, 0, 1, 2, pixPipeRight);
    ui->graphicsView->selector.ID = MASK_PIPE_RIGHT;

}
void MainWindow::addLeftPipe() {
    if (placedEnterPipeDoor) {
        setPipeExitMode();
    } else {
        setPipeEnterMode();
    }
    ui->graphicsView->selector.setElement(0, 0, 1, 2, pixPipeLeft);
    ui->graphicsView->selector.ID = MASK_PIPE_LEFT;
}
void MainWindow::addUpPipe() {
    if (placedEnterPipeDoor) {
        setPipeExitMode();
    } else {
        setPipeEnterMode();
    }
    ui->graphicsView->selector.setElement(0, 0, 2, 1, pixPipeUp);
    ui->graphicsView->selector.ID = MASK_PIPE_UP;
}

void MainWindow::addDoorEnter() {
    if(mode == PIPE_EXIT_MODE || mode == PIPE_ENTER_MODE) { return; }
    placingDoor = true;
    setPipeEnterMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 2, pixDoorEnter);
    ui->graphicsView->selector.ID = MASK_DOOR_E;
}
void MainWindow::addDoorExit() {
    placingDoor = true;
    setPipeExitMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 2, pixDoorExit);
    ui->graphicsView->selector.ID = MASK_DOOR_X;
}

MainWindow::~MainWindow() {
    ui->graphicsView->deleteTilemapData();
    deletePixmaps();
    deletePack();
    delete ui;
}

void MainWindow::recolor() {
    setNeedSave();
    QColorDialog dialog(this);
    int c = ui->comboLevels->currentIndex();
    dialog.setCurrentColor(QColor(176, 224, 248));
    dialog.exec();
    backgroundColor = dialog.currentColor();
    recolorTilesetPixmap(backgroundColor);
    pack.level[c].r = backgroundColor.red();
    pack.level[c].g = backgroundColor.green();
    pack.level[c].b = backgroundColor.blue();
    ui->graphicsView->repaint();
    ui->graphicsView->viewport()->update();
}

void MainWindow::resize() {
    if(!ui->graphicsView->resize(ui->spinWidth->value(), ui->spinHeight->value())) {
        setResizeSpins();
        QMessageBox::warning(this, QStringLiteral("Invalid dimensions"), QStringLiteral("Cannot resize, dimensions too large"));
    }
}

void MainWindow::setBrushMode() {
    modeStr = "brush";
    setMode(BRUSH_MODE);
    setSelectionToolsVisible(false);
}
void MainWindow::setFillMode() {
    modeStr = "fill";
    setMode(FILL_MODE);
    setSelectionToolsVisible(false);
}
void MainWindow::setSelectMode() {
    modeStr = "select";
    mode = SELECT_MODE;
    setMode(SELECT_MODE);
    setSelectionToolsVisible(true);
}
void MainWindow::setPassword() {
    bool ok;
    QString passwd = QInputDialog::getText(this, QStringLiteral("Pack Password"), QStringLiteral("Input Pack editing password:"), QLineEdit::Normal, passwordStr, &ok);
    if (ok && !passwd.isEmpty()) {
        passwordStr = passwd;
        setNeedSave();
    }
}

void MainWindow::setSelectionToolsVisible(bool state) {
    ui->actionCopy->setVisible(state);
    ui->actionDelete->setVisible(state);
    ui->actionPaste->setVisible(state);
    ui->actionCut->setVisible(state);
}

void MainWindow::setMode(int type) {
    if (mode == PIPE_ENTER_MODE) {
        placingPipeDoor = placedEnterPipeDoor = false;
    }
    ui->graphicsView->selector.ID = 0;
    mode = type;
    ui->graphicsView->setMode(type);
    ui->tileView->setMode(type);
    updatePos(0, 0, 27);
}

void MainWindow::setNewMode(int type) {
    if (type == SELECT_MODE) {
        setSelectMode();
    } else
    if (type == FILL_MODE) {
        setFillMode();
    } else
    if (type == BRUSH_MODE) {
        setBrushMode();
    }
    if (type == ENEMY_MODE) {
        setEnemyMode();
    }
}

void MainWindow::updatePos(int x, int y, int id) {
    if (mode != SELECT_MODE) {
        statusLabel.setText(modeStr + QStringLiteral(": ") + QString::number(x) + QStringLiteral(",") + QString::number(y) + QStringLiteral(" [") + ((id == 27) ?  QStringLiteral("empty") : QString::number(id)) + QStringLiteral("]"));
    } else {
        statusLabel.setText(modeStr + QStringLiteral(": Rectangle (") + QString::number(x) + QStringLiteral(",") + QString::number(y) + QStringLiteral(")"));
    }
}

void MainWindow::noStatus() {
    statusLabel.setText(modeStr);
}

void MainWindow::changeLevel(int newLevel) {
    if (pack.count) {
        ui->graphicsView->saveLevel();
        setLevel(static_cast<uint8_t>(newLevel));
        setResizeSpins();
    }
}

void MainWindow::changeScroll(int scroll) {
    int c = ui->comboLevels->currentIndex();
    pack.level[c].scroll = scroll;
    setNeedSave();
}

void MainWindow::addLevel() {
    if (pack.count > 50) {
        QMessageBox::information(this, QStringLiteral("Error"), QStringLiteral("Cannot add another level; pack full."));
        return;
    }
    int c = ui->comboLevels->currentIndex()+1;
    insertLevel(static_cast<uint8_t>(c));

    pack.level[c].r = 176;
    pack.level[c].g = 224;
    pack.level[c].b = 248;

    ui->comboLevels->blockSignals(true);
    ui->comboScroll->blockSignals(true);
    ui->comboLevels->clear();
    ui->comboScroll->setCurrentIndex(0);

    int max = static_cast<int>(pack.count);
    for(int i=1; i<=max; i++) {
        ui->comboLevels->addItem(QString::number(i));
    }

    ui->comboLevels->blockSignals(false);
    ui->comboScroll->blockSignals(false);

    if (pack.count != 1) {
        ui->comboLevels->setCurrentIndex(c);
    } else {
        ui->comboLevels->setCurrentIndex(0);
    }
    setNeedSave();
    setResizeSpins();
}

void MainWindow::setLevelColor() {
    uint8_t c = curLevel;
    recolorTilesetPixmap(QColor(pack.level[c].r, pack.level[c].g, pack.level[c].b));
    ui->tileView->scene()->update();
}

void MainWindow::deleteLevel() {
    if (pack.count == 1) { return; }
    int current = ui->comboLevels->currentIndex();
    removeLevel(static_cast<uint8_t>(current));

    setupLevelComboBox();

    ui->comboLevels->blockSignals(true);
    if (current) { ui->comboLevels->setCurrentIndex(current-1); }
    ui->comboLevels->blockSignals(false);

    if (current) {
        setLevel(static_cast<uint8_t>(current-1));
    } else {
        setLevel(0);
    }
    setNeedSave();
}

void MainWindow::levelUp() {
    int current = ui->comboLevels->currentIndex();
    uint8_t i = static_cast<uint8_t>(current);
    if (moveLevelUp(i)) {
        ui->comboLevels->blockSignals(true);
        ui->comboLevels->setCurrentIndex(current-1);
        ui->graphicsView->setNewLevel(i-1);
        ui->comboLevels->blockSignals(false);
        setNeedSave();
    }
}

void MainWindow::levelDown() {
    int current = ui->comboLevels->currentIndex();
    if (current == ui->comboLevels->count()-1) { return; }
    uint8_t i = static_cast<uint8_t>(current);
    if (moveLevelDown(i)) {
        ui->comboLevels->blockSignals(true);
        ui->comboLevels->setCurrentIndex(current+1);
        ui->graphicsView->setNewLevel(i+1);
        ui->comboLevels->blockSignals(false);
        setNeedSave();
    }
}

void MainWindow::initScene() {
    initPack();
    initTilesetPixmap(":/resources/tiles/orig_tileset.png");
    ui->graphicsView->setScene(&mainScene);
    ui->graphicsView->show();
    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);

    ui->tileView->setScene(&tileScene);
    ui->tileView->setDragMode(QGraphicsView::NoDrag);
    ui->tileView->show();
    ui->tileView->loadTiles(":/resources/tiles/orig_tileset.png");

    insertLevel(0);
    setLevel(0);
}

void MainWindow::setEnemyMode() {
    if(mode == ENEMY_MODE) { return; }
    modeStr = "enemy";
    setMode(mode = ENEMY_MODE);
    setSelectionToolsVisible(false);
}

void MainWindow::addGoomba() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 1, pixGoomba);
    ui->graphicsView->selector.ID = GOOMBA_ENEMY;
}
void MainWindow::addRedKoopa() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 2, pixRedKoopa);
    ui->graphicsView->selector.setElementOffsets(0, 5);
    ui->graphicsView->selector.ID = RED_KOOPA_ENEMY;
}
void MainWindow::addGreenKoopa() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 2, pixGreenKoopa);
    ui->graphicsView->selector.setElementOffsets(0, 5);
    ui->graphicsView->selector.ID = GREEN_KOOPA_ENEMY;
}
void MainWindow::addRedKoopaFly() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 2, pixRedKoopaFly);
    ui->graphicsView->selector.setElementOffsets(0, 5);
    ui->graphicsView->selector.ID = RED_KOOPA_FLY_ENEMY;
}
void MainWindow::addGreenKoopaFly() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 2, pixGreenKoopaFly);
    ui->graphicsView->selector.setElementOffsets(0, 5);
    ui->graphicsView->selector.ID = GREEN_KOOPA_FLY_ENEMY;
}
void MainWindow::addFish() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 1, pixFish);
    ui->graphicsView->selector.ID = FISH_ENEMY;
}
void MainWindow::addChomper() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 2, 1, pixChomper);
    ui->graphicsView->selector.setElementOffsets(8, 0);
    ui->graphicsView->selector.ID = CHOMPER_ENEMY;
}
void MainWindow::addFireChomper() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 2, 1, pixFireChomper);
    ui->graphicsView->selector.setElementOffsets(8, 0);
    ui->graphicsView->selector.ID = CHOMPER_FIRE_ENEMY;
}
void MainWindow::addThwomp() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 2, 2, pixThwomp);
    ui->graphicsView->selector.setElementOffsets(4, 0);
    ui->graphicsView->selector.ID = THWOMP_ENEMY;
}
void MainWindow::addFireball() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 1, pixFireball);
    ui->graphicsView->selector.setElementOffsets(1, 0);
    ui->graphicsView->selector.ID = FIREBALL_ENEMY;
}
void MainWindow::addBoo() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 1, pixBoo);
    ui->graphicsView->selector.ID = BOO_ENEMY;
}
void MainWindow::addBones() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 2, pixBones);
    ui->graphicsView->selector.setElementOffsets(0, 5);
    ui->graphicsView->selector.ID = BONES_ENEMY;
}
void MainWindow::addOiram() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 1, pixOiram);
    ui->graphicsView->selector.ID = OIRAM_ENEMY;
}
void MainWindow::addSpike() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 1, 1, pixSpike);
    ui->graphicsView->selector.ID = SPIKE_ENEMY;
}
void MainWindow::addReswob() {
    setEnemyMode();
    ui->graphicsView->selector.setElement(0, 0, 2, 3, pixReswob);
    ui->graphicsView->selector.setElementOffsets(0, 8);
    ui->graphicsView->selector.ID = RESWOB_ENEMY;
}
