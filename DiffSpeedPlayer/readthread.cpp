#include "readthread.h"
#include <QDebug>
#include <QtMath>

ReadThread::ReadThread()
{

}

void ReadThread::setVideoState(VideoState *pVideoState)
{
    m_pVideoState = pVideoState;
}

void ReadThread::run()
{
    AVPacket *pAVPacket = av_packet_alloc();
    AVPacket *pLastAVpacket = nullptr;
    static int nStatic = 0;

    while (av_read_frame(m_pVideoState->pFormatCtx, pAVPacket) >= 0) {
        if (pAVPacket->stream_index == m_pVideoState->nVideoIndex) {
            if (m_pVideoState->videoQueue.nPacketSize > 125) {
                msleep(10);
            }

            if (!pLastAVpacket || qAbs(pLastAVpacket->buf->size - pAVPacket->buf->size) < 20) {
//                nStatic ++;
//                qInfo() << pAVPacket->buf->size << nStatic;
                pLastAVpacket = av_packet_alloc();
                *pLastAVpacket = *pAVPacket;
                put_packet(&m_pVideoState->videoQueue, pAVPacket, true);
            } else {
                *pLastAVpacket = *pAVPacket;
                put_packet(&m_pVideoState->videoQueue, pAVPacket);
            }
        }
    }
}

void ReadThread::put_packet(PacketQueue *pQueuePacket, AVPacket *pPkt, bool bFast)
{
    RePacket *pPacketList = new RePacket;
    pPacketList->pkt = *pPkt;
    pPacketList->pNext = nullptr;
    pPacketList->bFast = bFast;

    pQueuePacket->pMutex->lock();

    if (!pQueuePacket->pLast) {
        pQueuePacket->pFirst = pPacketList;
    } else {
        pQueuePacket->pLast->pNext = pPacketList;
    }
    pQueuePacket->pLast = pPacketList;
    pQueuePacket->nPacketSize++;

    pQueuePacket->pWaitCondition->wakeAll();
    pQueuePacket->pMutex->unlock();
}
