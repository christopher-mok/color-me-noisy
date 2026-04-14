#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QDir>
#include <QPixmap>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString dirpath = "../../../frames";

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void advanceFrame();

    void loadFrames(const QString &dirPath);

    QLabel *m_display;
    QTimer *m_timer;
    QStringList m_framePaths; // sorted list of image file paths
    int m_currentFrame;
};

#endif // MAINWINDOW_H


