#include "zerorenderer.h"

ZeroRenderer::ZeroRenderer(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(1050, 780);
    //菜单栏
    {
        QMenuBar *menuBar = new QMenuBar(this);
        QMenu *fileMenu = menuBar->addMenu(tr("文件"));

        QAction *openAction1 = new QAction(tr("打开"), this);
        fileMenu->addAction(openAction1);
        connect(openAction1, SIGNAL(triggered()), this, SLOT(openFile()));

        this->setMenuBar(menuBar);
    }

    //属性窗口
    {
        QDockWidget *dockWidget = new QDockWidget("属性窗口", this);
        dockWidget->setFixedSize(QSize(300, 500));
        QWidget *container = new QWidget();
        dockWidget->setWidget(container);
        QGridLayout *layout = new QGridLayout();
        container->setLayout(layout);

        //分辨率
        QLabel *Alabel0 = new QLabel("分辨率(0~1000)：");
        QLineEdit *Aline0 = new QLineEdit();
        Aline0->setText(QString::number(width() - 300));
        QLineEdit *Aline1 = new QLineEdit();
        Aline1->setText(QString::number(height() - 30));
        QIntValidator *validator = new QIntValidator(100, 1000, this);
        Aline0->setValidator(validator);
        Aline1->setValidator(validator);
        layout->addWidget(Alabel0, 0, 0);
        layout->addWidget(Aline0, 0, 1);
        layout->addWidget(Aline1, 0, 2);

        //摄像机
        QLabel *Blabel0 = new QLabel("摄像机位置：");
        QLineEdit *Bline0 = new QLineEdit();
        Bline0->setText(QString::number(camera.x()));
        QLineEdit *Bline1 = new QLineEdit();
        Bline1->setText(QString::number(camera.y()));
        QLineEdit *Bline2 = new QLineEdit();
        Bline2->setText(QString::number(camera.z()));
        layout->addWidget(Blabel0, 1, 0);
        layout->addWidget(Bline0, 1, 1);
        layout->addWidget(Bline1, 1, 2);
        layout->addWidget(Bline2, 1, 3);

        QLabel *Blabel1 = new QLabel("摄像机方向：");
        QLineEdit *Bline3 = new QLineEdit();
        Bline3->setText(QString::number(viewDir.x()));
        QLineEdit *Bline4 = new QLineEdit();
        Bline4->setText(QString::number(viewDir.y()));
        QLineEdit *Bline5 = new QLineEdit();
        Bline5->setText(QString::number(viewDir.z()));
        layout->addWidget(Blabel1, 2, 0);
        layout->addWidget(Bline3, 2, 1);
        layout->addWidget(Bline4, 2, 2);
        layout->addWidget(Bline5, 2, 3);

        //光照
        QLabel *Clabel0 = new QLabel("光照方向：");
        QLineEdit *Cline0 = new QLineEdit();
        Cline0->setText(QString::number(lightDir.x()));
        QLineEdit *Cline1 = new QLineEdit();
        Cline1->setText(QString::number(lightDir.y()));
        QLineEdit *Cline2 = new QLineEdit();
        Cline2->setText(QString::number(lightDir.z()));
        layout->addWidget(Clabel0, 3, 0);
        layout->addWidget(Cline0, 3, 1);
        layout->addWidget(Cline1, 3, 2);
        layout->addWidget(Cline2, 3, 3);

        QLabel *Clabel1 = new QLabel("环境光强度：");
        QLineEdit *Cline3 = new QLineEdit();
        Cline3->setText(QString::number(ambient));
        layout->addWidget(Clabel1, 4, 0);
        layout->addWidget(Cline3, 4, 1);

        //着色器
        QLabel *Dlabel0 = new QLabel("着色器：");
        QComboBox *comboBox = new QComboBox(this);
        comboBox->addItem("FlatShader");
        comboBox->addItem("GouraudShader");
        comboBox->addItem("ToonShader");
        comboBox->addItem("PhongShader");
        comboBox->addItem("BlinnPhongShader");
        comboBox->setCurrentIndex(4);
        layout->addWidget(Dlabel0, 5, 0);
        layout->addWidget(comboBox, 5, 1, 1, 2);

        //渲染按钮
        QPushButton *button = new QPushButton("渲染", this);
        connect(button, SIGNAL(clicked()), this, SLOT(reDraw()));
        layout->addWidget(button, 6, 0, 1, 4);

        //设置小数限制
        QDoubleValidator *dv = new QDoubleValidator(-100.0, 100.0, 2, this);  // 允许输入带2位小数的浮点数
        Bline0->setValidator(dv);
        Bline1->setValidator(dv);
        Bline2->setValidator(dv);
        Bline3->setValidator(dv);
        Bline4->setValidator(dv);
        Bline5->setValidator(dv);
        Cline0->setValidator(dv);
        Cline1->setValidator(dv);
        Cline2->setValidator(dv);

        this->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    }
}

ZeroRenderer::~ZeroRenderer() {
    for(int i = 0; i < models.size(); i++){
        delete models[i];
    }
}

void ZeroRenderer::draw()
{
    bool f = false;
    int w = 100, h = 100, shaderType = 4;
    QDockWidget *dock = this->findChild<QDockWidget*>();
    if(dock && !isActive){
        lineEditList = dock->findChildren<QLineEdit*>();
        comboBoxList = dock->findChildren<QComboBox*>();
        isActive = true;
    }
    if(!lineEditList.isEmpty()){
        w = lineEditList[0]->text().toInt();
        h = lineEditList[1]->text().toInt();
        if(w != width() - 300 || h != height() - 30){
            f = true;
        }
        camera = Vec3f(lineEditList[2]->text().toFloat(), lineEditList[3]->text().toFloat(), lineEditList[4]->text().toFloat());
        viewDir = Vec3f(lineEditList[5]->text().toFloat(), lineEditList[6]->text().toFloat(), lineEditList[7]->text().toFloat());
        lightDir = Vec3f(lineEditList[8]->text().toFloat(), lineEditList[9]->text().toFloat(), lineEditList[10]->text().toFloat());
        ambient = lineEditList[11]->text().toFloat();
    }
    if(!comboBoxList.empty()){
        shaderType = comboBoxList[0]->currentIndex();
    }
    else{
        w = width() - 300; h = height() - 30;
    }

    image = TGAImage(w, h, TGAImage::RGB);
    float* zbuffer = new float[w * h];
    for (int i = 0; i < w * h; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }
    viewDir.normalize();
    Matrix viewport = getViewport(w, h);
    Matrix projection = getProjection(w / h, fov, near, far);
    Matrix view = getView(camera, viewDir, Vec3f(0, 1.0f, 0));

    for(int idx = 0; idx < models.size(); idx++){
        TGAImage texture = models[idx]->getTexture();
        TGAImage specularMap = models[idx]->getSpecular();
        TGAImage normalMap = models[idx]->getNormal();

        switch(shaderType){
            case 0:{
                FlatShader shader(viewport, projection, view, lightDir, texture);
                for (int i = 0; i < models[idx]->nfaces(); i++) {
                    Vec3f screenCoords[3];
                    for (int j = 0; j < 3; j++) {
                        Vec3f v = models[idx]->vert(i, j);
                        Vec2i uv = models[idx]->uv(i, j);
                        Vec3f normal = models[idx]->normal(i, j);
                        screenCoords[j] = shader.vertex(v, uv, normal, j);
                    }
                    triangleBoundingBox(screenCoords, shader, image, zbuffer);
                }
                std::cout << "Completed!" << std::endl;
                break;
            }
            case 1:{
                GouraudShader shader(viewport, projection, view, lightDir, texture);
                for (int i = 0; i < models[idx]->nfaces(); i++) {
                    Vec3f screenCoords[3];
                    for (int j = 0; j < 3; j++) {
                        Vec3f v = models[idx]->vert(i, j);
                        Vec2i uv = models[idx]->uv(i, j);
                        Vec3f normal = models[idx]->normal(i, j);
                        screenCoords[j] = shader.vertex(v, uv, normal, j);
                    }
                    triangleBoundingBox(screenCoords, shader, image, zbuffer);
                }
                std::cout << "Completed!" << std::endl;
                break;
            }
            case 2:{
                ToonShader shader(viewport, projection, view, lightDir);
                for (int i = 0; i < models[idx]->nfaces(); i++) {
                    Vec3f screenCoords[3];
                    for (int j = 0; j < 3; j++) {
                        Vec3f v = models[idx]->vert(i, j);
                        Vec2i uv = models[idx]->uv(i, j);
                        Vec3f normal = models[idx]->normal(i, j);
                        screenCoords[j] = shader.vertex(v, uv, normal, j);
                    }
                    triangleBoundingBox(screenCoords, shader, image, zbuffer);
                }
                std::cout << "Completed!" << std::endl;
                break;
            }
            case 3:{
                PhongShader shader(viewport, projection, view, lightDir, texture, ambient, viewDir, specularMap, 64.0f, normalMap);
                for (int i = 0; i < models[idx]->nfaces(); i++) {
                    Vec3f screenCoords[3];
                    for (int j = 0; j < 3; j++) {
                        Vec3f v = models[idx]->vert(i, j);
                        Vec2i uv = models[idx]->uv(i, j);
                        Vec3f normal = models[idx]->normal(i, j);
                        screenCoords[j] = shader.vertex(v, uv, normal, j);
                    }
                    triangleBoundingBox(screenCoords, shader, image, zbuffer);
                }
                std::cout << "Completed!" << std::endl;
                break;
            }
            case 4:{
                BlinnPhongShader shader(viewport, projection, view, lightDir, texture, ambient, viewDir, specularMap, 64.0f, normalMap);
                for (int i = 0; i < models[idx]->nfaces(); i++) {
                    Vec3f screenCoords[3];
                    for (int j = 0; j < 3; j++) {
                        Vec3f v = models[idx]->vert(i, j);
                        Vec2i uv = models[idx]->uv(i, j);
                        Vec3f normal = models[idx]->normal(i, j);
                        screenCoords[j] = shader.vertex(v, uv, normal, j);
                    }
                    triangleBoundingBox(screenCoords, shader, image, zbuffer);
                }
                std::cout << "Completed!" << std::endl;
                break;
            }
        }
    }

    image.flip_vertically();
    delete[] zbuffer;

    if(f) this->resize(w + 300, h + 30);
    else repaint();
}

void ZeroRenderer::addModel(QStringList filePaths)
{
    for(int i = 0; i < filePaths.size(); i++){
        std::string s = filePaths[i].toStdString();
        Model *m = new Model(s);
        models.push_back(m);
    }
}

void ZeroRenderer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    for(int i = 300; i < width(); i++){
        for(int j = 30; j < height(); j++){
            TGAColor c = image.get(i - 300, j - 30);
            painter.setPen(QColor(c[2], c[1], c[0]));
            painter.drawPoint(QPoint(i, j));
        }
    }
}

void ZeroRenderer::keyPressEvent(QKeyEvent *event) {
    if(isActive){
        if(event->key() == Qt::Key_W) {
            lineEditList[2]->setText(QString::number(camera.x() + viewDir.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(camera.y() + viewDir.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(camera.z() + viewDir.z() / 10, 'f', 2));
            draw();
        }
        if(event->key() == Qt::Key_S) {
            lineEditList[2]->setText(QString::number(camera.x() - viewDir.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(camera.y() - viewDir.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(camera.z() - viewDir.z() / 10, 'f', 2));
            draw();
        }
        if(event->key() == Qt::Key_A) {
            Vec3f up(0, 1, 0);
            Vec3f left = up.cross(viewDir);
            lineEditList[2]->setText(QString::number(camera.x() + left.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(camera.y() + left.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(camera.z() + left.z() / 10, 'f', 2));
            draw();
        }
        if(event->key() == Qt::Key_D) {
            Vec3f up(0, 1, 0);
            Vec3f left = up.cross(viewDir);
            lineEditList[2]->setText(QString::number(camera.x() - left.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(camera.y() - left.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(camera.z() - left.z() / 10, 'f', 2));
            draw();
        }

        if(event->key() == Qt::Key_Up) {
            viewDir.y() += 0.1f;
            viewDir.normalize();
            lineEditList[5]->setText(QString::number(viewDir.x(), 'f', 2));
            lineEditList[6]->setText(QString::number(viewDir.y(), 'f', 2));
            lineEditList[7]->setText(QString::number(viewDir.z(), 'f', 2));
            draw();
        }
        if(event->key() == Qt::Key_Down) {
            viewDir.y() -= 0.1f;
            viewDir.normalize();
            lineEditList[5]->setText(QString::number(viewDir.x(), 'f', 2));
            lineEditList[6]->setText(QString::number(viewDir.y(), 'f', 2));
            lineEditList[7]->setText(QString::number(viewDir.z(), 'f', 2));
            draw();
        }
        if(event->key() == Qt::Key_Left) {
            viewDir.x() -= 0.1f;
            viewDir.normalize();
            lineEditList[5]->setText(QString::number(viewDir.x(), 'f', 2));
            lineEditList[6]->setText(QString::number(viewDir.y(), 'f', 2));
            lineEditList[7]->setText(QString::number(viewDir.z(), 'f', 2));
            draw();
        }
        if(event->key() == Qt::Key_Right) {
            viewDir.x() += 0.1f;
            viewDir.normalize();
            lineEditList[5]->setText(QString::number(viewDir.x(), 'f', 2));
            lineEditList[6]->setText(QString::number(viewDir.y(), 'f', 2));
            lineEditList[7]->setText(QString::number(viewDir.z(), 'f', 2));
            draw();
        }
    }
}
