#include "mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

QGraphicsPixmapItem *MainWindow::m_pNormalItem = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    m_pVBoxLayout = new QVBoxLayout(this);

    this->setLayout(m_pVBoxLayout);

    m_pNormalView = new QGraphicsView(this);
    m_pVBoxLayout->addWidget(m_pNormalView);
    m_pNormalScene = new QGraphicsScene(this);
    m_pNormalView->setScene(m_pNormalScene);
    m_pNormalItem = new QGraphicsPixmapItem();
    m_pNormalScene->addItem(m_pNormalItem);

    /////////////
    QWidget *pWidget = new QWidget(this);
    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addStretch();
    QLabel *pLabel = new QLabel(pWidget);
    pLayout->addWidget(pLabel);
    pLayout->addStretch();
    pWidget->setLayout(pLayout);
    pLabel->setText(QString("解码准备"));

    m_pVBoxLayout->addWidget(pWidget);

    m_pVideoPlayer = new VideoPlayer;
    connect(m_pVideoPlayer, SIGNAL(sigVideoFrame(QPixmap)), this, SLOT(freshPixmap(QPixmap)), Qt::QueuedConnection);
    QString strPath("/aaa.mp4");
    m_pVideoPlayer->play(strPath);

    QTimer::singleShot(10000, this, [ = ] {
        m_pVideoPlayer->start();
        pLabel->setText("播放中");
    });
}

MainWindow::~MainWindow()
{
    m_pVideoPlayer->quit();
    m_pVideoPlayer->deleteLater();
}

void MainWindow::startDecode()
{
//    m_pDecoder->decode("/home/zyl/Videos/1.mp4",freshPixmap);
//    m_pDecoder->decode("movie.avi",freshPixmap);
    //    m_pDecoder->decoder();
}

void MainWindow::freshPixmap(QPixmap pixmap)
{
    m_pNormalItem->setPixmap(pixmap);
}
