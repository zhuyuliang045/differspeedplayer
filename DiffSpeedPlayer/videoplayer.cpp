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
    static int nStatic = 0;
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

void VideoPlayer::transPixmap(AVFrame *pAVFrame)
{
    AVFrame *pFrameYUV = av_frame_alloc();
    QPixmap pixMap;
    unsigned char *outBuffer;

    AVCodecContext *pAVCodecCtx = m_pVideoState->videoDecoder.pAVCodecCtx;

    outBuffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, pAVCodecCtx->width, pAVCodecCtx->height, 1));
    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, outBuffer, AV_PIX_FMT_RGB24, pAVCodecCtx->width, pAVCodecCtx->height, 1);


    m_pSwsCtx = sws_getContext(pAVCodecCtx->width, pAVCodecCtx->height, pAVCodecCtx->pix_fmt,
                               pAVCodecCtx->width, pAVCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

    sws_scale(m_pSwsCtx, (const unsigned char *const *)pAVFrame->data,
              pAVFrame->linesize, 0, pAVCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);


    QImage tmpImg(pFrameYUV->data[0], pAVFrame->width, pAVFrame->height, QImage::Format_RGB888);

    QImage tmpImg1 =  tmpImg.scaled(1920, 1080);

    pixMap = QPixmap::fromImage(tmpImg1);

    sigVideoFrame(pixMap);

    free(outBuffer);
    av_frame_free(&pFrameYUV);
    av_frame_free(&pAVFrame);
    sws_freeContext(m_pSwsCtx);
}

