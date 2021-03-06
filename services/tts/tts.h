#ifndef TTS_H
#define TTS_H

#include <QObject>
#include <QMap>

#include <QtSpeech>

class TTS : public QObject
{
    Q_OBJECT
public:
    explicit TTS(QObject *parent = 0);
    ~TTS();

    void say(QString);
    void tell(QString);

    QStringList getVoiceList();
    void setVoice(QString);

private:
    QtSpeech * speech;
    QString voice;
    QMap<QString, QtSpeech::VoiceName> voices;
    // TODO: message to say thead
    
signals:
    void finished();
    
};

#endif // TTS_H
