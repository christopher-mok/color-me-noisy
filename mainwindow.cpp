#include "mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_display(new QLabel(this)),
    m_timer(new QTimer(this)),
    m_currentFrame(0)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->addWidget(m_display);
    setCentralWidget(central);

    resize(800, 600);
    setWindowTitle("Color Me Noisy");

    loadFrames(dirpath);

    connect(m_timer, &QTimer::timeout, this, &MainWindow::advanceFrame);
    m_timer->start(41); // ~24 fps
}

MainWindow::~MainWindow() {}



void MainWindow::loadFrames(const QString &dirPath)
{
    QDir dir(dirPath);
    QStringList filters = {"*.png", "*.jpg", "*.jpeg", "*.bmp"};
    m_framePaths = dir.entryList(filters, QDir::Files, QDir::Name); //sort by name and put into m_framePaths

    std::cout << "Found " << m_framePaths.size() << " frames in " << dirPath.toStdString() << std::endl;
    std::cout << "Looking in: " << dir.absolutePath().toStdString() << std::endl;

    for (QString &path : m_framePaths) {
        path = dir.absoluteFilePath(path);
    }
}

void MainWindow::advanceFrame()
{
    if (m_framePaths.isEmpty()) return;

    QPixmap pix(m_framePaths[m_currentFrame]);
    if (!pix.isNull()) {
        m_display->setPixmap(
            pix.scaled(m_display->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
    }

    m_currentFrame = (m_currentFrame + 1) % m_framePaths.size();
}

