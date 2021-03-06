#include "settingscontroller.h"

#include "models/deviceobject.h"
#include "models/localeobject.h"

#include "system/settingsvalult.h"
#include "utills/singleton.h"

#include <QDebug>
#include <QSettings>
#include <QApplication>
#include <QLocale>

extern "C" {
    #include "utills/OpenAL/openal_wrapper.h"
}

SettingsController::SettingsController(QObject *parent) : QObject(parent)
{    
    this->settingsValult = &Singleton<SettingsValult>::Instance();

    LocaleObject * byLang = new LocaleObject(new QLocale(QLocale::Belarusian, QLocale::Belarus));
    LocaleObject * ruLang = new LocaleObject(new QLocale(QLocale::Russian, QLocale::Russia));
    LocaleObject * enLang = new LocaleObject(new QLocale(QLocale::English, QLocale::UnitedStates));

    availableLanguages[byLang->locale()->name()] = byLang;
    availableLanguages[ruLang->locale()->name()] = ruLang;
    availableLanguages[enLang->locale()->name()] = enLang;

    availableUiLanguages.append(byLang);
    availableUiLanguages.append(ruLang);
    availableUiLanguages.append(enLang);

    availableSttLanguages.append(ruLang);
    availableSttLanguages.append(enLang);

    availableTtsLanguages.append(ruLang);

    this->settingsValult->setUiLocale(ruLang);
    this->settingsValult->setTtsLocale(ruLang);
    this->settingsValult->setSttLocale(ruLang);

    initAudio();
    loadSettings();
}

SettingsController::~SettingsController()
{

}

void SettingsController::initAudio()
{
    int index = 0;
    oal_devices_list *outputDevices = getOutputDevices();
    while(outputDevices)
    {
        QString name = QString::fromLocal8Bit(outputDevices->device->name);
        this->settingsValult->mOutputDevices.append(
            new DeviceObject(name, index, outputDevices->device)
        );
        outputDevices = outputDevices->next;
        index++;
    }
    this->settingsValult->mCurrentOutputDevice = this->settingsValult->mOutputDevices.first();

    index = 0;
    oal_devices_list *inputDevices = getInputDevices();
    while(inputDevices)
    {
        QString name = QString::fromLocal8Bit(inputDevices->device->name);
        this->settingsValult->mInputDevices.append(
            new DeviceObject(name, index, inputDevices->device)
        );
        inputDevices = inputDevices->next;
        index++;
    }
    this->settingsValult->mCurrentInputDevice = this->settingsValult->mInputDevices.first();
}

QList<QObject *> SettingsController::getInputDeviceList()
{
    QList<DeviceObject *> * devices = &(this->settingsValult->mInputDevices);
    QList<QObject *> * objects = reinterpret_cast<QList<QObject *> * >(devices);
    return *objects;
}

QList<QObject *> SettingsController::getOutputDeviceList()
{
    QList<DeviceObject *> * devices = &(this->settingsValult->mOutputDevices);
    QList<QObject *> * objects = reinterpret_cast<QList<QObject *> * >(devices);
    return *objects;
}

void SettingsController::setInputDevice(const int index)
{
    if (index < this->settingsValult->mInputDevices.size())
    {
        this->settingsValult->mCurrentInputDevice = this->settingsValult->mInputDevices.at(index);
        qDebug() << "SettingsController::setInputDevice >> " << this->settingsValult->mCurrentInputDevice->name();
    }
}

int SettingsController::getInputDevice() const
{
    qDebug() << "SettingsController::getInputDevice" << this->settingsValult->mCurrentInputDevice->name();
    return this->settingsValult->mInputDevices.indexOf(this->settingsValult->mCurrentInputDevice);
}

void SettingsController::setOutputDevice(const int index)
{
    if (index < this->settingsValult->mOutputDevices.size())
    {
        this->settingsValult->mCurrentOutputDevice = this->settingsValult->mOutputDevices.at(index);
        qDebug() << "SettingsController::setOutputDevice >> " << this->settingsValult->mCurrentOutputDevice->name();
    }
}

int SettingsController::getOutputDevice() const
{
    qDebug() << "SettingsController::getOutputDevice" << this->settingsValult->mCurrentOutputDevice->name();
    return this->settingsValult->mOutputDevices.indexOf(this->settingsValult->mCurrentOutputDevice);
}

void SettingsController::loadSettings()
{
    qDebug() << "SettingsController::loadSettings() >> " << SETTINGS_FILE;
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
    settings.setPath(QSettings::IniFormat, QSettings::UserScope, QApplication::applicationDirPath());

    if(settings.contains("devices/input"))
        for(DeviceObject * device: this->settingsValult->mInputDevices)
            if(settings.value("devices/input").toString() == device->name())
                this->settingsValult->mCurrentInputDevice = device;

    if(settings.contains("devices/output"))
        for(DeviceObject * device: this->settingsValult->mOutputDevices)
            if(settings.value("devices/output").toString() == device->name())
                this->settingsValult->mCurrentOutputDevice = device;

    if(settings.contains("language/ui"))
    {
        QString locale = settings.value("language/ui").toString();
        this->settingsValult->setUiLocale(this->availableLanguages[locale]);
    }

    if(settings.contains("language/tts"))
    {
        QString locale = settings.value("language/tts").toString();
        this->settingsValult->setTtsLocale(this->availableLanguages[locale]);
    }

    if(settings.contains("language/stt"))
    {
        QString locale = settings.value("language/stt").toString();
        this->settingsValult->setSttLocale(this->availableLanguages[locale]);
    }

}

void SettingsController::saveSettings()
{
    qDebug() << "SettingsController::saveSettings() " << SETTINGS_FILE;
    QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
    settings.setPath(QSettings::IniFormat, QSettings::UserScope, QApplication::applicationDirPath());

    settings.setValue("devices/input", this->settingsValult->mCurrentInputDevice->name());
    settings.setValue("devices/output", this->settingsValult->mCurrentOutputDevice->name());
    settings.setValue("language/ui", this->settingsValult->getUiLocale()->locale()->name());
    settings.setValue("language/tts", this->settingsValult->getTtsLocale()->locale()->name());
    settings.setValue("language/stt", this->settingsValult->getSttLocale()->locale()->name());
}

QList<QObject *> SettingsController::getUiLanguageList()
{
    QList<LocaleObject *> * list = &(this->availableUiLanguages);
    QList<QObject *> * objects = reinterpret_cast<QList<QObject *> * >(list);
    return *objects;
}

void SettingsController::setUiLanguage(const QString code)
{
    if (this->availableLanguages.contains(code))
        this->settingsValult->setUiLocale(this->availableLanguages[code]);
}

int SettingsController::getUiLanguage() const
{
    qDebug() << "getUiLanguage >> " << this->settingsValult->getUiLocale()->name();
    return this->availableUiLanguages.indexOf(this->settingsValult->getUiLocale());
}

QList<QObject *> SettingsController::getSttLanguageList()
{
    QList<LocaleObject *> * list = &(this->availableSttLanguages);
    QList<QObject *> * objects = reinterpret_cast<QList<QObject *> * >(list);
    return *objects;
}

void SettingsController::setSttLanguage(const QString code)
{
    if (this->availableLanguages.contains(code))
        this->settingsValult->setSttLocale(this->availableLanguages[code]);
}

int SettingsController::getSttLanguage() const
{
    qDebug() << "getSttLanguage >> " << this->settingsValult->getSttLocale()->name();
    return this->availableSttLanguages.indexOf(this->settingsValult->getSttLocale());
}

QList<QObject *> SettingsController::getTtsLanguageList()
{
    QList<LocaleObject *> * list = &(this->availableTtsLanguages);
    QList<QObject *> * objects = reinterpret_cast<QList<QObject *> * >(list);
    return *objects;
}

void SettingsController::setTtsLanguage(const QString code)
{
    if (this->availableLanguages.contains(code))
        this->settingsValult->setTtsLocale(this->availableLanguages[code]);
}

int SettingsController::getTtsLanguage() const
{
    qDebug() << "getTtsLanguage >> " << this->settingsValult->getTtsLocale()->name();
    return this->availableTtsLanguages.indexOf(this->settingsValult->getTtsLocale());
}
