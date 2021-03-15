#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QThread>
#include <QPixmap>

#include "streamopen.h"
#include "support.h"

class VideoPlayer: public QThread
{
    Q_OBJECT

signals:
    void sigVideoFrame(QPixmap pPixmap);

public:
    VideoPlayer();

    void play(QString &strPath);

    void run();

private:
    StreamOpen *m_pStreamOpen;
    VideoState *m_pVideoState;
    QString m_strFile;
    struct SwsContext *m_pSwsCtx;
};

#endif // VIDEOPLAYER_H
