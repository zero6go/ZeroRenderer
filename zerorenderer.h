#ifndef ZERORENDERER_H
#define ZERORENDERER_H

#include <QMainWindow>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QMenuBar>
#include <QFileDialog>
#include <QDockWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QComboBox>
#include <QKeyEvent>
#include <QDoubleValidator>
#include <QMouseEvent>
#include <QWheelEvent>

#include "tgaimage.h"
#include "model.h"
#include "gl.h"
#include "shader.h"
#include <iostream>

class ZeroRenderer : public QMainWindow
{
    Q_OBJECT

public:
    ZeroRenderer(QWidget *parent = nullptr);
    ~ZeroRenderer();
protected:
    void draw();
    void addModel(QStringList filePaths);
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    std::vector<Model*> models;
    Vec3f lightDir = Vec3f(0.5, -0.5, -1);
    Vec3f cameraPos = Vec3f(0.5, 0.5, 3);
    Vec3f viewDir = Vec3f(0, 0, -1);
    float ambient = 0.1f;
    float near = -0.1f;
    float far = -100.0f;
    float fov = 30.0f;

    TGAImage image;

    bool isActive = false;
    QList<QLineEdit*> lineEditList;
    QList<QComboBox*> comboBoxList;

private slots:
    void openFile() {
        QStringList filePaths = QFileDialog::getOpenFileNames(this, "打开模型文件", "", "obj (*.obj)");
        if (!filePaths.isEmpty()) {
            addModel(filePaths);
            draw();
        }
    }
    void reDraw(){
        draw();
    }
    void cleanModels(){
        for(auto &m : models){
            delete m;
        }
        models = std::vector<Model*>();
        draw();
        isActive = false;
    }
};
#endif // ZERORENDERER_H
