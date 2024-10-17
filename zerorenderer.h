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

private:
    std::vector<Model*> models;
    Vec3f lightDir = Vec3f(0.3, -0.7, -1);
    Vec3f camera = Vec3f(0.3, 0.3, 2);
    Vec3f viewDir = Vec3f(0, 0, -1);
    float ambient = 0.1f;
    float near = -0.1f;
    float far = -100.0f;
    float fov = 30.0f;

    TGAImage image;

private slots:
    void openFile() {
        QStringList filePaths = QFileDialog::getOpenFileNames(this, "打开文件", "", "obj (*.obj)");
        if (!filePaths.isEmpty()) {
            addModel(filePaths);
            draw();
        }
    }
    void reDraw(){
        draw();
    }
};
#endif // ZERORENDERER_H
