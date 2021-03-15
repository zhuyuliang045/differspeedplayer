#include "streamopen.h"
#include <QDebug>

StreamOpen::StreamOpen()
{
    m_pVideoState = new VideoState;
    m_pReadThread = new ReadThread;
    m_pVideoThread = new VideoThread;

    m_pVideoState->videoQueue.nPacketSize = 0;
    m_pVideoState->videoQueue.pLast = nullptr;
    m_pVideoState->videoQueue.pFirst = nullptr;
    m_pVideoState->videoQueue.pMutex = new QMutex;
    m_pVideoState->videoQueue.pWaitCondition = new QWaitCondition;

//    m_pVideoState->picQueue.nSize = 0;
//    m_pVideoState->picQueue.nRIndex = 0;
    m_pVideoState->picQueue.nMaxSize = 640;
    m_pVideoState->picQueue.pMutex = new QMutex;
    m_pVideoState->picQueue.pCondition_Not_Full = new QWaitCondition;
    m_pVideoState->picQueue.pCondition_Not_Empty = new QWaitCondition;
}

StreamOpen::~StreamOpen()
{
    m_pReadThread->quit();
    m_pReadThread->deleteLater();

    m_pVideoThread->quit();
    m_pVideoThread->deleteLater();
}

void StreamOpen::streamOpen(int nIndex)
{
    int nRet = -1;
    AVFormatContext *pFormatCtx = m_pVideoState->pFormatCtx;
    AVCodecContext *pAVCodecCtx;
    AVCodec *pCodec;

    if (nIndex < 0 || nIndex >= pFormatCtx->nb_streams) {
        qDebug() << "error";
    }
    pAVCodecCtx = pFormatCtx->streams[nIndex]->codec;
//    pAVCodecCtx = avcodec_alloc_context3(nullptr);
//    if (!pAVCodecCtx) {
//        return;
//    }

//    nRet = avcodec_parameters_to_context(pAVCodecCtx, pFormatCtx->streams[nIndex]->codecpar);
//    if (nRet < 0) {
//        qDebug() << "error";
//    }

    pCodec = avcodec_find_decoder(pAVCodecCtx->codec_id);

    if (!pCodec) {
        qDebug() << "the codec didn't find";
    }

    nRet = avcodec_open2(pAVCodecCtx, pCodec, nullptr);
    if (nRet < 0) {
        qDebug() << "error";
    }

    switch (pAVCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        m_pVideoState->nVideoIndex = nIndex;
        m_pVideoState->pVideoStream = pFormatCtx->streams[nIndex];

        m_pVideoState->videoDecoder.pAVCodecCtx = pAVCodecCtx;
        m_pVideoState->videoDecoder.pPktQueue = &m_pVideoState->videoQueue;

        //decode thread
        m_pVideoThread->setDecoder(m_pVideoState);
        m_pVideoThread->start();

        break;

    default:
        break;
    }
}

VideoState *StreamOpen::getVideoState()
{
    return m_pVideoState;
}

void StreamOpen::fileOpen(QString strFileName)
{
    int nRet = -1;
    AVFormatContext *pForamtCtx = avformat_alloc_context();
    int streamType[AVMEDIA_TYPE_NB];

    nRet = avformat_open_input(&pForamtCtx,  /*"http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8"*/ strFileName.toLocal8Bit(), nullptr, nullptr);   //解封装，AVInputFormat
    if (nRet < 0) {
        qDebug() << "error";
    }
    nRet = avformat_find_stream_info(pForamtCtx, nullptr);
    if (nRet < 0) {
        qDebug() << "error";
    }

    m_pVideoState->pFormatCtx = pForamtCtx;

    for (int i = 0; i < pForamtCtx->nb_streams; i++) {
        AVStream *avStream = pForamtCtx->streams[i];
        enum AVMediaType type = avStream->codecpar->codec_type;
        if (type >= 0) {
            streamType[type] = i;
        }
    }

    //设在窗口大小和比例
    if (streamType[AVMEDIA_TYPE_VIDEO] >= 0) {
        AVStream *avStream = pForamtCtx->streams[streamType[AVMEDIA_TYPE_VIDEO]];
        AVCodecParameters *codecpar = avStream->codecpar;
        AVRational ration = av_guess_sample_aspect_ratio(pForamtCtx, avStream, nullptr);
        //Todo 设置窗口大小和比例
    }

    //readpacket
    m_pReadThread->setVideoState(m_pVideoState);
    m_pReadThread->start();

    //open the streams
    if (streamType[AVMEDIA_TYPE_AUDIO] >= 0) {
        streamOpen(1);
    }

    if (streamType[AVMEDIA_TYPE_VIDEO] >= 0) {
        streamOpen(streamType[AVMEDIA_TYPE_VIDEO]);
    }
}
