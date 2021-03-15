#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "videodecoder.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "videoplayer.h"
#include "streamopen.h"

class QHBoxLayout;
class QVBoxLayout;


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void startDecode();

public slots:
    void freshPixmap(QPixmap pixmap);

private:
    QGraphicsView        *m_pNormalView;
    QGraphicsScene       *m_pNormalScene;
    static QGraphicsPixmapItem *m_pNormalItem;
    VideoDecoder *m_pVideoDecoder;

//    QHBoxLayout *m_pHBoxLayout;
    QVBoxLayout *m_pVBoxLayout;
    VideoPlayer *m_pVideoPlayer;
    StreamOpen *m_pStreamOpen;
};

#endif // MAINWINDOW_H
