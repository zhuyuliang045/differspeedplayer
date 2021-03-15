#ifndef STREAMOPEN_H
#define STREAMOPEN_H

//#ifdef __cplusplus
//extern "C" {
//#include "libavformat/avformat.h"
//}
//#endif

#include <QString>
#include <QMutex>
#include "support.h"
#include "readthread.h"
#include "videothread.h"

class StreamOpen
{
public:
    StreamOpen();
    ~StreamOpen();

    //打开文件查找流
    void fileOpen(QString strFileName);

    //打开流
    void streamOpen(int nIndex);

    VideoState *getVideoState();

private:
    VideoState *m_pVideoState;
    ReadThread *m_pReadThread;
    VideoThread *m_pVideoThread;
};

#endif // STREAMOPEN_H
