#ifndef VIDEODECODER_H
#define VIDEODECODER_H


#ifdef __cplusplus
extern "C"
{
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif

#include <QString>
#include <QObject>
#include <QThread>
#include <QPixmap>

class VideoDecoder: public QThread
{
    Q_OBJECT

signals:
    void sigVideoFrame(QPixmap pPixmap);

public:
    VideoDecoder();

    void run();

    void decode(/*QString strInFile, void(*callback)(QPixmap &pixmap)*/);

    void yuv420_to_rgb24(unsigned char *pY, unsigned char *pU, unsigned char *pV, unsigned char *pRGB24, int width, int height);

private:
    AVFormatContext *m_pFormatCtx;
    AVCodecContext *m_pCodecCtx;
    AVCodec *m_pCodec;
    AVFrame *m_pFrame;
    AVFrame *m_pFrameYUV;
    AVPacket *m_pPacket;
    unsigned char *m_outBuffer;
    struct SwsContext *m_pSwsCtx;
};

#endif // DECODER_H
