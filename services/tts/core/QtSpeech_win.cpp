/*  QtSpeech -- a small cross-platform library to use TTS
    Copyright (C) 2010-2011 LynxLine.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General
    Public License along with this library; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301 USA */

#include <QtCore>
#include "services/tts/core/QtSpeech.h"

#include <mmintrin.h>
#include <wtypes.h>

#include <sapi.h>
#include <sphelper.h>
#include <comdef.h>

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <QDebug>

// some defines for throwing exceptions
#define Where QString("%1:%2:").arg(__FILE__).arg(__LINE__)
#define SysCall(x,e) {\
    HRESULT hr = x;\
    if (FAILED(hr)) {\
        QString msg = #e;\
        msg += ":"+QString(__FILE__);\
        msg += ":"+QString::number(__LINE__)+":"+#x+":";\
        msg += _com_error(hr).ErrorMessage();\
        throw e(msg);\
    }\
}

// internal data
class QtSpeech::Private {
public:
    Private()
        :onFinishSlot(0L),waitingFinish(false) {}

    VoiceName name;

    static const QString VoiceId;
    typedef QPointer<QtSpeech> Ptr;
    static QList<Ptr> ptrs;

    CComPtr<ISpVoice> voice;

    const char * onFinishSlot;
    QPointer<QObject> onFinishObj;
    bool waitingFinish;

    class WCHAR_Holder {
    public:
        WCHAR * w;
        WCHAR_Holder(QString s)
            :w(0) {
            w = new WCHAR[s.length()+1];
            s.toWCharArray(w);
            w[s.length()] =0;
        }

        ~WCHAR_Holder() { delete[] w; }
    };
};
const QString QtSpeech::Private::VoiceId = QString("win:%1");
QList<QtSpeech::Private::Ptr> QtSpeech::Private::ptrs = QList<QtSpeech::Private::Ptr>();

// implementation
QtSpeech::QtSpeech(QObject * parent)
    :QObject(parent), d(new Private)
{
    this->setVoice();
    d->ptrs << this;
}

QtSpeech::QtSpeech(VoiceName voice, QObject * parent)
    :QObject(parent), d(new Private)
{
    this->setVoice(voice);
    d->ptrs << this;
}

QtSpeech::~QtSpeech()
{
    d->ptrs.removeAll(this);
//    d->voice.Release();
//    d->voice = NULL;
    CoUninitialize();
//    d->voice = NULL;
//    delete d;
}

const QtSpeech::VoiceName & QtSpeech::name() const {
    return d->name;
}

const QtSpeech::VoiceNames QtSpeech::voices()
{
    VoiceNames vs;       
    ULONG count = 0;
    CComPtr<IEnumSpObjectTokens> voices;

    CoInitialize(NULL);
    SysCall( SpEnumTokens(SPCAT_VOICES, NULL, NULL, &voices), LogicError);
    SysCall( voices->GetCount(&count), LogicError);

    for(int i=0; i< count; i++) {
        WCHAR * w_id = 0L;
        WCHAR * w_name = 0L;
        CComPtr<ISpObjectToken> voice;
        SysCall( voices->Next( 1, &voice, NULL ), LogicError);
        SysCall( SpGetDescription(voice, &w_name), LogicError);
        SysCall( voice->GetId(&w_id), LogicError);

        QString id = QString::fromWCharArray(w_id);
        QString name = QString::fromWCharArray(w_name);
        VoiceName n = { id, name };
        vs << n;

        voice.Release();
    }
    return vs;
}

void QtSpeech::tell(QString text) const {
    tell(text, 0L,0L);
}

void QtSpeech::tell(QString text, QObject * obj, const char * slot) const
{
    if (d->waitingFinish)
        throw LogicError(Where+"Already waiting to finish speech");

    d->onFinishObj = obj;
    d->onFinishSlot = slot;
    if (obj && slot)
        connect(const_cast<QtSpeech *>(this), SIGNAL(finished()), obj, slot);

    d->waitingFinish = true;
    const_cast<QtSpeech *>(this)->startTimer(100);

    Private::WCHAR_Holder w_text(text);
    SysCall( d->voice->Speak( w_text.w, SPF_ASYNC | SPF_IS_NOT_XML, 0), LogicError);
}

void QtSpeech::say(QString text) const
{
    Private::WCHAR_Holder w_text(text);
    SysCall( d->voice->Speak( w_text.w, SPF_IS_NOT_XML, 0), LogicError);
}

void QtSpeech::setVoice(VoiceName voice)
{
    qDebug() << "QtSpeech::setVoice >> " << voice.name << " " << voice.id;
    ULONG count = 0;
    CComPtr<IEnumSpObjectTokens> voices;

    CoInitialize(NULL);
    SysCall( d->voice.CoCreateInstance( CLSID_SpVoice ), InitError);

    if (voice.id.isEmpty()) {
        WCHAR * w_id = 0L;
        WCHAR * w_name = 0L;
        CComPtr<ISpObjectToken> systemVoice;
        SysCall( d->voice->GetVoice(&systemVoice), InitError);
        SysCall( SpGetDescription(systemVoice, &w_name), InitError);
        SysCall( systemVoice->GetId(&w_id), InitError);
        voice.name = QString::fromWCharArray(w_name);
        voice.id = QString::fromWCharArray(w_id);
        systemVoice.Release();
    }
    else {
        SysCall( SpEnumTokens(SPCAT_VOICES, NULL, NULL, &voices), InitError);
        SysCall( voices->GetCount(&count), InitError);
        for (int i =0; i< count; i++) {
            WCHAR * w_id = 0L;
            CComPtr<ISpObjectToken> systemVoice;
            SysCall( voices->Next( 1, &systemVoice, NULL ), InitError);
            SysCall( systemVoice->GetId(&w_id), InitError);
            QString id = QString::fromWCharArray(w_id);
            if (id == voice.id) d->voice->SetVoice(systemVoice);
            systemVoice.Release();
        }
    }

    if (voice.id.isEmpty())
        throw InitError(Where+"No default voice in system");

    d->name = voice;
}

void QtSpeech::timerEvent(QTimerEvent * te)
{
    QObject::timerEvent(te);

    if (d->waitingFinish) {
        SPVOICESTATUS es;
        d->voice->GetStatus( &es, NULL );
        if (es.dwRunningState == SPRS_DONE) {
            d->waitingFinish = false;
            killTimer(te->timerId());
            finished();
        }
    }
}
