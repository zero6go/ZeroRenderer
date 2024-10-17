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
        QLabel *label0 = new QLabel("分辨率(0~1000)：");
        QLineEdit *line0 = new QLineEdit();
        line0->setText(QString::number(width() - 300));
        QLineEdit *line1 = new QLineEdit();
        line1->setText(QString::number(height() - 30));
        QIntValidator *validator = new QIntValidator(100, 1000, this);
        line0->setValidator(validator);
        line1->setValidator(validator);
        layout->addWidget(label0, 0, 0);
        layout->addWidget(line0, 0, 1);
        layout->addWidget(line1, 0, 2);

        //摄像机
        QLabel *label1 = new QLabel("摄像机位置：");
        QLineEdit *line2 = new QLineEdit();
        line2->setText(QString::number(camera.x()));
        QLineEdit *line3 = new QLineEdit();
        line3->setText(QString::number(camera.y()));
        QLineEdit *line4 = new QLineEdit();
        line4->setText(QString::number(camera.z()));
        layout->addWidget(label1, 1, 0);
        layout->addWidget(line2, 1, 1);
        layout->addWidget(line3, 1, 2);
        layout->addWidget(line4, 1, 3);

        //光照
        QLabel *label2 = new QLabel("光照方向：");
        QLineEdit *line5 = new QLineEdit();
        line5->setText(QString::number(lightDir.x()));
        QLineEdit *line6 = new QLineEdit();
        line6->setText(QString::number(lightDir.y()));
        QLineEdit *line7 = new QLineEdit();
        line7->setText(QString::number(lightDir.z()));
        layout->addWidget(label2, 2, 0);
        layout->addWidget(line5, 2, 1);
        layout->addWidget(line6, 2, 2);
        layout->addWidget(line7, 2, 3);

        QLabel *label3 = new QLabel("环境光强度：");
        QLineEdit *line8 = new QLineEdit();
        line8->setText(QString::number(ambient));
        layout->addWidget(label3, 3, 0);
        layout->addWidget(line8, 3, 1);

        //着色器
        QLabel *label4 = new QLabel("着色器：");
        QComboBox *comboBox = new QComboBox(this);
        comboBox->addItem("FlatShader");
        comboBox->addItem("GouraudShader");
        comboBox->addItem("ToonShader");
        comboBox->addItem("PhongShader");
        comboBox->addItem("BlinnPhongShader");
        comboBox->setCurrentIndex(4);
        layout->addWidget(label4, 4, 0);
        layout->addWidget(comboBox, 4, 1, 1, 2);

        //渲染按钮
        QPushButton *button = new QPushButton("渲染", this);
        connect(button, SIGNAL(clicked()), this, SLOT(reDraw()));
        layout->addWidget(button, 5, 0, 1, 4);

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
    if(dock){
        QList<QLineEdit*> list = dock->findChildren<QLineEdit*>();
        if(!list.isEmpty()){
            w = list[0]->text().toInt();
            h = list[1]->text().toInt();
            if(w != width() - 300 || h != height() - 30){
                f = true;
            }
            camera = Vec3f(list[2]->text().toFloat(), list[3]->text().toFloat(), list[4]->text().toFloat());
            lightDir = Vec3f(list[5]->text().toFloat(), list[6]->text().toFloat(), list[7]->text().toFloat());
            ambient = list[8]->text().toFloat();
        }
        QList<QComboBox*> list1 = dock->findChildren<QComboBox*>();
        if(!list1.empty()){
            shaderType = list1[0]->currentIndex();
        }
    }
    else{
        w = width() - 300; h = height() - 30;
    }

    image = TGAImage(w, h, TGAImage::RGB);
    float* zbuffer = new float[w * h];
    for (int i = 0; i < w * h; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }
    Matrix viewport = getViewport(w, h);
    Matrix projection = getProjection(w / h, fov, near, far);
    Matrix view = getView(camera, center, Vec3f(0, 1.0f, 0));

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
