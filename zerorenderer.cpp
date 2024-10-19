#include "zerorenderer.h"

ZeroRenderer::ZeroRenderer(QWidget *parent)
    : QMainWindow(parent)
{
    this->resize(1100, 830);
    //菜单栏
    {
        QMenuBar *menuBar = new QMenuBar(this);
        QMenu *fileMenu = menuBar->addMenu(tr("文件"));

        QAction *openAction1 = new QAction(tr("打开"), this);
        fileMenu->addAction(openAction1);
        connect(openAction1, SIGNAL(triggered()), this, SLOT(openFile()));
        QAction *openAction2 = new QAction(tr("清空模型"), this);
        fileMenu->addAction(openAction2);
        connect(openAction2, SIGNAL(triggered()), this, SLOT(cleanModels()));

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
        Bline0->setText(QString::number(cameraPos.x()));
        QLineEdit *Bline1 = new QLineEdit();
        Bline1->setText(QString::number(cameraPos.y()));
        QLineEdit *Bline2 = new QLineEdit();
        Bline2->setText(QString::number(cameraPos.z()));
        layout->addWidget(Blabel0, 1, 0);
        layout->addWidget(Bline0, 1, 1);
        layout->addWidget(Bline1, 1, 2);
        layout->addWidget(Bline2, 1, 3);

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
        QDoubleValidator *dv = new QDoubleValidator(-360.0, 360.0, 2, this);  // 允许输入带2位小数的浮点数
        Bline0->setValidator(dv);
        Bline1->setValidator(dv);
        Bline2->setValidator(dv);
        Cline0->setValidator(dv);
        Cline1->setValidator(dv);
        Cline2->setValidator(dv);

        this->addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
        this->setFocus();
    }
}

ZeroRenderer::~ZeroRenderer() {
    for(int i = 0; i < models.size(); i++){
        delete models[i];
    }
}

void ZeroRenderer::draw()
{
    //读取GUI数据
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
        cameraPos = Vec3f(lineEditList[2]->text().toFloat(), lineEditList[3]->text().toFloat(), lineEditList[4]->text().toFloat());
        lightDir = Vec3f(lineEditList[5]->text().toFloat(), lineEditList[6]->text().toFloat(), lineEditList[7]->text().toFloat());
        ambient = lineEditList[8]->text().toFloat();
    }
    if(!comboBoxList.empty()){
        shaderType = comboBoxList[0]->currentIndex();
    }
    else{
        w = width() - 300; h = height() - 30;
    }

    //shadowbuffer
    TGAImage lightImage = TGAImage(2000, 2000, TGAImage::RGB);
    float *shadowbuffer = new float[2000 * 2000];
    for (int i = 0; i < 2000 * 2000; i++) {
        shadowbuffer[i] = -std::numeric_limits<float>::max();
    }
    Matrix lightViewport = getViewport(2000, 2000, 1000);
    Matrix lightProjection = getProjection(1, fov, near, far);
    Matrix lightView = getView(-lightDir * 2, Vec3f(0, 1, 0));
    Matrix shadowMVP = lightViewport * lightProjection * lightView;
    for(int idx = 0; idx < models.size(); idx++){
        Shader *shader = new ShadowShader(lightViewport, lightProjection, lightView);
        for (int i = 0; i < models[idx]->nfaces(); i++) {
            Vec3f screenCoords[3];
            for (int j = 0; j < 3; j++) {
                Vec3f v = models[idx]->vert(i, j);
                Vec2i uv = models[idx]->uv(i, j);
                Vec3f normal = models[idx]->normal(i, j);
                screenCoords[j] = shader->vertex(v, uv, normal, j);
            }
            triangleBoundingBox(screenCoords, shader, lightImage, shadowbuffer);
        }
        std::cout << "Shadow " << idx << " Completed!" << std::endl;
        delete shader;
    }

    //正式渲染
    image = TGAImage(w, h, TGAImage::RGB);
    float* zbuffer = new float[w * h];
    for (int i = 0; i < w * h; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }
    Matrix viewport = getViewport(w, h, 1000);
    Matrix projection = getProjection(w / h, fov, near, far);
    Matrix view = getView(cameraPos, Vec3f(0, 1, 0));
    viewDir = -cameraPos;
    viewDir.normalize();

    for(int idx = 0; idx < models.size(); idx++){
        TGAImage texture = models[idx]->getTexture();
        TGAImage specularMap = models[idx]->getSpecular();
        TGAImage normalMap = models[idx]->getNormal();

        Shader *shader;
        switch(shaderType){
            case 0:{
                shader = new FlatShader(viewport, projection, view, lightDir, texture, shadowbuffer, shadowMVP, w);
                break;
            }
            case 1:{
                shader = new GouraudShader(viewport, projection, view, lightDir, texture, shadowbuffer, shadowMVP, w);
                break;
            }
            case 2:{
                shader = new ToonShader(viewport, projection, view, lightDir, shadowbuffer, shadowMVP, w);
                break;
            }
            case 3:{
                shader = new PhongShader(viewport, projection, view, lightDir, texture, ambient, viewDir, specularMap,
                                         64.0f, normalMap, shadowbuffer, shadowMVP, w);
                break;
            }
            case 4:{
                shader = new BlinnPhongShader(viewport, projection, view, lightDir, texture, ambient, viewDir,
                                              specularMap, 64.0f, normalMap, shadowbuffer, shadowMVP, w);
                break;
            }
        }
        for (int i = 0; i < models[idx]->nfaces(); i++) {
            Vec3f screenCoords[3];
            for (int j = 0; j < 3; j++) {
                Vec3f v = models[idx]->vert(i, j);
                Vec2i uv = models[idx]->uv(i, j);
                Vec3f normal = models[idx]->normal(i, j);
                screenCoords[j] = shader->vertex(v, uv, normal, j);
            }
            triangleBoundingBox(screenCoords, shader, image, zbuffer);
        }
        std::cout << "Model " << idx << " Completed!" << std::endl;
        delete shader;
    }
    std::cout<<std::endl;

    image.flip_vertically();
    delete[] zbuffer;
    delete[] shadowbuffer;

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

void ZeroRenderer::keyPressEvent(QKeyEvent *event)
{
    if(isActive){
        if(event->key() == Qt::Key_W) {
            Vec3f up(0, 1, 0);
            Vec3f left = up.cross(viewDir);
            Vec3f down = left.cross(viewDir);
            lineEditList[2]->setText(QString::number(cameraPos.x() - down.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(cameraPos.y() - down.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(cameraPos.z() - down.z() / 10, 'f', 2));
            draw();
        }
        if(event->key() == Qt::Key_S) {
            Vec3f up(0, 1, 0);
            Vec3f left = up.cross(viewDir);
            Vec3f down = left.cross(viewDir);
            lineEditList[2]->setText(QString::number(cameraPos.x() + down.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(cameraPos.y() + down.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(cameraPos.z() + down.z() / 10, 'f', 2));
            draw();
        }
        if(event->key() == Qt::Key_A) {
            Vec3f up(0, 1, 0);
            Vec3f left = up.cross(viewDir);
            lineEditList[2]->setText(QString::number(cameraPos.x() + left.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(cameraPos.y() + left.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(cameraPos.z() + left.z() / 10, 'f', 2));
            draw();
        }
        if(event->key() == Qt::Key_D) {
            Vec3f up(0, 1, 0);
            Vec3f left = up.cross(viewDir);
            lineEditList[2]->setText(QString::number(cameraPos.x() - left.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(cameraPos.y() - left.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(cameraPos.z() - left.z() / 10, 'f', 2));
            draw();
        }
    }
}

void ZeroRenderer::mousePressEvent(QMouseEvent *event)
{
    int x = event->pos().x(), y = event->pos().y();
    if(x >= 300 && x < width() && y >= 30 && y < height()){
        this->setFocus();
    }
}

void ZeroRenderer::wheelEvent(QWheelEvent *event)
{
    if(isActive){
        if(event->angleDelta().y() > 0){
            lineEditList[2]->setText(QString::number(cameraPos.x() + viewDir.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(cameraPos.y() + viewDir.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(cameraPos.z() + viewDir.z() / 10, 'f', 2));
            draw();
        }
        else{
            lineEditList[2]->setText(QString::number(cameraPos.x() - viewDir.x() / 10, 'f', 2));
            lineEditList[3]->setText(QString::number(cameraPos.y() - viewDir.y() / 10, 'f', 2));
            lineEditList[4]->setText(QString::number(cameraPos.z() - viewDir.z() / 10, 'f', 2));
            draw();
        }
    }
}
