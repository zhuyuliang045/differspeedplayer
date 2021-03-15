#ifndef SUPPORT_H
#define SUPPORT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
};
#endif

#include <QString>
#include <QMutex>
#include <QWaitCondition>

#include <QQueue>
#include <QPixmap>

#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1

struct RePacket {
    AVPacket pkt;
    bool bFast = false;
    RePacket *pNext;
};

struct PacketQueue {
    RePacket *pFirst;
    RePacket *pLast;
    int nPacketSize;
    int nSize;
    QMutex *pMutex;
    QWaitCondition *pWaitCondition;
};

struct ReFrame {
    AVFrame avFrame;
    QPixmap pixmap;
    bool bFast;
};

struct FrameQueue {
    QQueue<ReFrame> queue;
    int nMaxSize;
    QMutex *pMutex;
    QWaitCondition *pCondition_Not_Full;
    QWaitCondition *pCondition_Not_Empty;
};

struct Decoder {
    AVPacket pkt;
    AVCodecContext *pAVCodecCtx;
    PacketQueue *pPktQueue;
};

typedef struct VideoState {
    AVInputFormat *pInputFormat;
    AVFormatContext *pFormatCtx;
    AVStream *pVideoStream;
    AVCodec *pVideoCodec;
    int nVideoIndex;

    PacketQueue videoQueue;

    FrameQueue picQueue;

    Decoder videoDecoder;
} VideoState;

#endif // SUPPORT_H
