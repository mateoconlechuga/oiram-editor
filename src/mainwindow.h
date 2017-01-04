#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QGraphicsScene>
#include <QModelIndexList>
#include <QLabel>
#include <QLineEdit>
#include <QDir>

#include "tileview.h"
#include "level.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *p = Q_NULLPTR);
    ~MainWindow();

public slots:
    void updatePos(int x, int y, int id);
    void setMode(int type);
    void setNewMode(int type);
    void noStatus();
    void setNeedSave();
    void setPipeExitMode();
    void setPipeDone();

protected:
    virtual void closeEvent(QCloseEvent *e) Q_DECL_OVERRIDE;

private:
    void setLevelColor();
    void setLevel(uint8_t level);

    void cancelPipe();

    void clrNeedSave();
    bool checkSave();
    void loadPack();
    bool savePack();

    void setBrushMode();
    void setFillMode();
    void setSelectMode();

    void addLevel();
    void deleteLevel();
    void changeLevel(int newLevel);
    void levelUp();
    void levelDown();
    void setResizeSpins();

    void setSelectionToolsVisible(bool state);

    void initScene();
    void recolor();
    void resize();

    void setEnemyMode();

    void addGoomba();
    void addRedKoopa();
    void addGreenKoopa();
    void addRedKoopaFly();
    void addGreenKoopaFly();
    void addFish();
    void addChomper();
    void addFireChomper();
    void addThwomp();
    void addFireball();
    void addBoo();
    void addBones();
    void addOiram();
    void addSpike();
    void addReswob();

    void createNew();

    void setPipeEnterMode();

    void addDownPipe();
    void addRightPipe();
    void addLeftPipe();
    void addUpPipe();

    void setOptions(bool state);

    void setupLevelComboBox();

    Ui::MainWindow *ui;
    QGraphicsScene mainScene;
    QGraphicsScene tileScene;
    QLabel statusLabel;
    QString modeStr;
    int mode;
    QColor backgroundColor;
    QLineEdit descriptionEdit;
    QLabel descriptionLabel;
    QLineEdit varEdit;
    QLabel varLabel;
    QLineEdit authorEdit;
    QLabel authorLabel;
    QDir currDir;
    QString currPath;

    bool placingPipe = false;
    bool placedEnterPipe = false;
};

#endif
