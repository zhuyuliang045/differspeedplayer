#ifndef READTHREAD_H
#define READTHREAD_H

#include <QThread>
#include "support.h"

class ReadThread:  public QThread
{
    Q_OBJECT

public:
    ReadThread();

    void setVideoState(VideoState *pVideoState);

    void run();

private:
    void put_packet(PacketQueue *pQueuePacket, AVPacket *pPkt, bool bFast = false);

private:
    VideoState *m_pVideoState;
};

#endif // READTHREAD_H
