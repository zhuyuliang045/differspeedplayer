#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QThread>
#include "support.h"

class VideoThread: public QThread
{
    Q_OBJECT

public:
    VideoThread();

    void setDecoder(VideoState *pVideoState);

    void run();

    void get_queue_packet(PacketQueue *pQueue, RePacket *&packet);

    void queue_picture(FrameQueue *pFrameQueue, AVFrame *frame, bool bFast = false);

    QPixmap transPixmap(AVFrame *pAVFrame);

private:
    VideoState *m_pVideoState;
    struct SwsContext *m_pSwsCtx;
};

#endif // VIDEOTHREAD_H
