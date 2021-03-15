#include "videothread.h"
#include <QDebug>
#include <QDateTime>

VideoThread::VideoThread()
{

}

void VideoThread::setDecoder(VideoState *pVideoState)
{
    m_pVideoState = pVideoState;
}

void VideoThread::run()
{
    RePacket *pPacketList;
    AVPacket packet;
    AVFrame *frame;
    int nRet;

    frame = av_frame_alloc();
    while (true) {
        //1.get packet
        get_queue_packet(&m_pVideoState->videoQueue, pPacketList);

        //2.send packet
        if (avcodec_send_packet(m_pVideoState->videoDecoder.pAVCodecCtx, &pPacketList->pkt) == AVERROR(EAGAIN)) {
            qInfo() << "error";
        }

        //3.recive frame
        nRet = avcodec_receive_frame(m_pVideoState->videoDecoder.pAVCodecCtx, frame);

        if (nRet == AVERROR_EOF) {
            return;
        }

        if (nRet != 0) {
            continue;
        }

        queue_picture(&m_pVideoState->picQueue, frame, pPacketList->bFast);
    }
}

void VideoThread::get_queue_packet(PacketQueue *pQueue, RePacket *&packet)
{
    RePacket *pPktList;

    pQueue->pMutex->lock();
    pPktList = pQueue->pFirst;
    if (!pPktList) {
        pQueue->pWaitCondition->wait(pQueue->pMutex);
    }

    pQueue->pFirst = pPktList->pNext;
    if (!pQueue->pFirst) {
        pQueue->pFirst = nullptr;
    }

    packet = pPktList;

    pQueue->nPacketSize--;

    pQueue->pMutex->unlock();
}

void VideoThread::queue_picture(FrameQueue *pFrameQueue, AVFrame *frame, bool bFast)
{
    AVFrame *avFrame = av_frame_alloc();
    ReFrame reFrame;

    reFrame.pixmap = transPixmap(frame);

    pFrameQueue->pMutex->lock();
    while (pFrameQueue->queue.size() == pFrameQueue->nMaxSize) {
        pFrameQueue->pCondition_Not_Full->wait(pFrameQueue->pMutex);
    }
    av_frame_move_ref(avFrame, frame);
    reFrame.bFast = bFast;
    reFrame.avFrame = *avFrame;
    pFrameQueue->queue.enqueue(reFrame);

    if (pFrameQueue->queue.size() >= 2) {
        pFrameQueue->pCondition_Not_Empty->wakeAll();
    }
    pFrameQueue->pMutex->unlock();
}

QPixmap VideoThread::transPixmap(AVFrame *pAVFrame)
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

    free(outBuffer);
    av_frame_free(&pFrameYUV);
    //av_frame_free(&pAVFrame);
    sws_freeContext(m_pSwsCtx);

    return pixMap;
}
