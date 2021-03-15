#include "videoplayer.h"
#include <QDebug>
#include <QImage>
#include <QDateTime>
#include <QtMath>
#include <QDateTime>

VideoPlayer::VideoPlayer()
{
    m_pStreamOpen = new StreamOpen;
}

void VideoPlayer::play(QString &strPath)
{
    m_strFile = QString(strPath);

    m_pStreamOpen->fileOpen(m_strFile);

    m_pVideoState = m_pStreamOpen->getVideoState();
}

void VideoPlayer::run()
{
    FrameQueue *pFrameQueue;
    int remianTime;
    QPixmap pixmap;
    AVFrame *pLastFrame = av_frame_alloc();

    pFrameQueue = &m_pVideoState->picQueue;

    while (true) {
        AVFrame *pAVFrame = av_frame_alloc();

        pFrameQueue->pMutex->lock();
        if (pFrameQueue->queue.size() <= 1) {
            pFrameQueue->pCondition_Not_Empty->wait(pFrameQueue->pMutex);
        }

        *pAVFrame = pFrameQueue->queue.first().avFrame;

        remianTime = (pAVFrame->pts - pLastFrame->pts) * av_q2d(m_pVideoState->pVideoStream->time_base) * 1000;
        if (remianTime <= 0 || remianTime > 3600) {
            remianTime = 41;
        }

        pixmap = pFrameQueue->queue.first().pixmap;


        pFrameQueue->pCondition_Not_Full->wakeAll();

        pFrameQueue->pMutex->unlock();

        if (pFrameQueue->queue.first().bFast) {
            remianTime = remianTime * 0.2;
        } else {
            remianTime = remianTime * 1;
        }

        *pLastFrame = *pAVFrame;

        msleep(remianTime);

        av_frame_free(&pAVFrame);
        //transPixmap(pAVFrame);      //视频输出

        sigVideoFrame(pixmap);

        pFrameQueue->queue.dequeue();
    }
}

