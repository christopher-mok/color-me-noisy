#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QDir>
#include <QPixmap>

#include "cult.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QString dirpath = "../color-me-noisy/frames";
    QString m_texturePath = "../color-me-noisy/textures/Checkerboard_pattern.jpeg"; //Path to image texture
    QStringList m_framePaths; // sorted list of image file paths

    QLabel *m_display;
    QTimer *m_timer;
    int m_currentFrame;


    void advanceFrame();

    void loadFrames(const QString &dirPath);

};

#endif // MAINWINDOW_H


