#include "videodecoder.h"
#include <QFile>
#include <QPixmap>
#include <QTimer>
#include <QEventLoop>
#include <QTime>
#include <QCoreApplication>

#include <unistd.h>

#include <QDebug>

QPixmap *videoBuffer[100];

VideoDecoder::VideoDecoder()
{
}

void VideoDecoder::run()
{
    decode();
}

void VideoDecoder::decode(/*QString strInFile, void(*callback)(QPixmap &pixmap)*/)
{
    int nRet;
    int nGotPicture;
    int nVideoIndex = -1;
    QPixmap pixMap;
    unsigned char *rgb24Buffer;

    //av_register_all();
    QString strInFile = "/home/zyl/Videos/3.mp4";
    m_pFormatCtx = avformat_alloc_context();                                             //分配AVFormatContext
    if (avformat_open_input(&m_pFormatCtx, strInFile.toLocal8Bit(), nullptr, nullptr) != 0) { //打开视频流
        return;
    }

    if (avformat_find_stream_info(m_pFormatCtx, nullptr) < 0) {                          //获取流信息，帧率等。。
        return;
    }

    for (int i = 0; i < m_pFormatCtx->nb_streams; i++) {                                 //查找视频流
        if (m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            nVideoIndex = i;
            break;
        }
    }

    if (nVideoIndex == -1) {
        return;
    }

    m_pCodecCtx = m_pFormatCtx->streams[nVideoIndex]->codec;

    m_pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);                                  //查找解码器
    if (m_pCodec == nullptr) {
        return;
    }

    if (avcodec_open2(m_pCodecCtx, m_pCodec, nullptr) < 0) {                                 //打开解码器
        return;
    }

    m_pFrame = av_frame_alloc();
    m_pFrameYUV = av_frame_alloc();

    m_outBuffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, m_pCodecCtx->width, m_pCodecCtx->height, 1));
    av_image_fill_arrays(m_pFrameYUV->data, m_pFrameYUV->linesize, m_outBuffer, AV_PIX_FMT_YUV420P, m_pCodecCtx->width, m_pCodecCtx->height, 1);
    m_pPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    rgb24Buffer = (unsigned char *)malloc(m_pCodecCtx->width * m_pCodecCtx->height * 3);

    m_pSwsCtx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
                               m_pCodecCtx->width, m_pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    while (av_read_frame(m_pFormatCtx, m_pPacket) >= 0) {
        if (m_pPacket->stream_index == nVideoIndex) {
            nRet = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &nGotPicture, m_pPacket);
            if (nRet < 0) {
                return;
            }

            if (nGotPicture >= 1) {
                sws_scale(m_pSwsCtx, (const unsigned char *const *)m_pFrame->data,
                          m_pFrame->linesize, 0, m_pCodecCtx->height, m_pFrameYUV->data, m_pFrameYUV->linesize);

                yuv420_to_rgb24(m_pFrameYUV->data[0], m_pFrameYUV->data[1], m_pFrameYUV->data[2], rgb24Buffer, m_pFrame->width, m_pFrame->height);

                QImage tmpImg(rgb24Buffer, m_pFrame->width, m_pFrame->height, QImage::Format_RGB888);

                QImage tmpImg1 =  tmpImg.scaled(1920, 1080);

                pixMap = QPixmap::fromImage(tmpImg1);

            }
            sigVideoFrame(pixMap);

            qInfo() << "pts:" << m_pPacket->pts;
        }

        av_packet_unref(m_pPacket);
    }

    while (true) {
        if (!(m_pCodec->capabilities & AV_CODEC_CAP_DELAY))
            return;

        nRet = avcodec_decode_video2(m_pCodecCtx, m_pFrame, &nGotPicture, m_pPacket);
        if (nRet < 0) {
            break;
        }
        if (!nGotPicture) {
            break;
        }

        sws_scale(m_pSwsCtx, (const unsigned char *const *)m_pFrame->data,
                  m_pFrame->linesize, 0, m_pCodecCtx->height, m_pFrameYUV->data, m_pFrameYUV->linesize);


        yuv420_to_rgb24(m_pFrameYUV->data[0], m_pFrameYUV->data[1], m_pFrameYUV->data[2], rgb24Buffer, m_pFrame->width, m_pFrame->height);

        QImage tmpImg(rgb24Buffer, m_pFrame->width, m_pFrame->height, QImage::Format_RGB888);
        QImage tmpImg1 = tmpImg.scaled(1920, 1080);

        pixMap = QPixmap::fromImage(tmpImg1);
    }

    sws_freeContext(m_pSwsCtx);
    av_frame_free(&m_pFrameYUV);
    av_frame_free(&m_pFrame);
    avcodec_close(m_pCodecCtx);
    avformat_close_input(&m_pFormatCtx);
}

void VideoDecoder::yuv420_to_rgb24(unsigned char *pY, unsigned char *pU, unsigned char *pV, unsigned char *pRGB24, int width, int height)
{
    int y, u, v;
    int off_set = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            y = i * width + j;
            v = (i / 4) * width + j / 2;
            u = (i / 4) * width + j / 2;

            int R = (pY[y] - 16) + 1.370805 * (pV[u] - 128);
            int G = (pY[y] - 16) - 0.69825 * (pV[u] - 128) - 0.33557 * (pU[v] - 128);
            int B = (pY[y] - 16) + 1.733221 * (pU[v] - 128);

            R = R < 255 ? R : 255;
            G = G < 255 ? G : 255;
            B = B < 255 ? B : 255;

            R = R < 0 ? 0 : R;
            G = G < 0 ? 0 : G;
            B = B < 0 ? 0 : B;

            pRGB24[off_set++] = (unsigned char)R;
            pRGB24[off_set++] = (unsigned char)G;
            pRGB24[off_set++] = (unsigned char)B;
        }
    }
}
