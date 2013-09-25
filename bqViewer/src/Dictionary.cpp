/*************************************************************************

bq Cervantes e-book reader application
Copyright (C) 2011-2013  Mundoreader, S.L

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the source code.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/

#include "Dictionary.h"

#include "QBookApp.h"
#include "QBook.h"
#include "Storage.h"
#include "semantix_dixio.h"
#include "json.h"
#include "Storage.h"
#include "bqDeviceServices.h"

#include <QDebug>
#include <QSettings>
#include <QByteArray>
#include <map>
#include <string>
#include <QDir>
#include <dlfcn.h>

#define SEMANTIX_LIB_PATH   "/usr/lib/libsemantix_dixio.so"
#define EMPTY_DICTIO_RESULT 1234 // TODO: requested new version to Semantix to avoid this check. This is the size of an "empty" html return

// Singleton instance
Dictionary* Dictionary::m_spInstance = NULL;

void Dictionary::staticInit()
{
    if(!m_spInstance)
        m_spInstance = new Dictionary();
}

void Dictionary::staticDone()
{
    if(m_spInstance)
    {
        delete m_spInstance;
        m_spInstance = NULL;
    }
}

void Dictionary::loadSemantixLib( const QString& sofile )
{
    qDebug() << Q_FUNC_INFO;
    char *error;

    handle = dlopen(sofile.toUtf8().constData(), RTLD_NOW);
    if (!handle) {
        qDebug() << "QBookDic::loadSemantixLib: cannot open semantix library: " << dlerror();
        return;
    }

    dlerror();

    *(void **) (&initialize) = dlsym(handle, "_ZN8Semantix10initializeESsSs");
    if ((error = dlerror()) != NULL)  {
        qDebug() << "Cannot load Semantix::initialize: " << error;
        return;
    }

    *(void **) (&getDictionaries) = dlsym(handle, "_ZN8Semantix15getDictionariesEv");
    if ((error = dlerror()) != NULL)  {
        qDebug() << "Cannot load Semantix::getDictionaries: " << error;
        return;
    }

    *(void **) (&finalize) = dlsym(handle, "_ZN8Semantix8finalizeEv");
    if ((error = dlerror()) != NULL)  {
        qDebug() << "Cannot load Semantix::finalize: " << error;
        return;
    }

    *(void **) (&getVersion) = dlsym(handle, "_ZN8Semantix10getVersionEv");
    if ((error = dlerror()) != NULL)  {
        qDebug() << "Cannot load Semantix::getVersion: " << error;
        return;
    }

    *(void **) (&getWorkingDictionariesEx) = dlsym(handle, "_ZN8Semantix24getWorkingDictionariesExEv");
    if ((error = dlerror()) != NULL)  {
        qDebug() << "Cannot load Semantix::getWorkingDictionaries: " << error;
        return;
    }

    *(void **) (&getActivationData) = dlsym(handle, "_ZN8Semantix17getActivationDataESsSsRSsS0_");
    if ((error = dlerror()) != NULL)  {
        qDebug() << "Cannot load Semantix::getActivationData: " << error;
        return;
    }

    *(void **) (&setActivationData) = dlsym(handle, "_ZN8Semantix17setActivationDataESsSs");
    if ((error = dlerror()) != NULL)  {
        qDebug() << "Cannot load Semantix::setActivationData: " << error;
        return;
    }

// Dixio lib for x86 does not contains Semantix::searchHtml, so we finish here
    #ifndef Q_WS_QWS
    qDebug() << "QBookDic::loadSemantixLib: symbols loaded correctly";
    initialized = true;
    return;
    #endif


    *(void **) (&searchHtml) = dlsym(handle, "_ZN8Semantix10searchHtmlESsSsSsiSsii");
	if ((error = dlerror()) != NULL)  {
            qDebug() << "Cannot load Semantix::searchHtml: " << error;
            return;
	}

	qDebug() << "QBookDic::loadSemantixLib: symbols loaded correctly";
	initialized = true;
}

Dictionary::Dictionary() {
    
    loadSemantixLib(SEMANTIX_LIB_PATH);

    if (!initialized)
	return;
    QString dictPath = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + "dictionaries" + QDir::separator();

    QDir dirPrivateDictionaries(Storage::getInstance()->getPrivatePartition()->getMountPoint() + QDir::separator());
    if(!dirPrivateDictionaries.exists( "dictionaries/"))
        dirPrivateDictionaries.mkdir( "dictionaries");

    QString dictPrivatePath = dirPrivateDictionaries.absolutePath() + QDir::separator() + "dictionaries" + QDir::separator();
    QString repositories = dictPath + QString(":") + dictPrivatePath;
    qDebug() << Q_FUNC_INFO << "repositories " << repositories;
    initialize(dictPath.toUtf8().constData(),repositories.toUtf8().constData());
    qDebug() << Q_FUNC_INFO << "Dictionaires lib initialized. Version: " << QString::fromUtf8(getVersion().c_str());
}

Dictionary::~Dictionary()
{
    qDebug() << Q_FUNC_INFO;
    if (!initialized)
	return;
    finalize();
    dlclose(handle);
}

QString Dictionary::translate(const QString& original, const QString& context)
{

    if (!initialized)
        return tr("Couldn't find any result for the given word");
    #ifndef Q_WS_QWS
        return tr("x86 dixio lib cannot translate words");
    #endif

    if(m_dixioKey.isEmpty())
        m_dixioKey = Dictionary::instance()->getDixioKey();

    string key = m_dixioKey.toUtf8().constData();;
    string dictCode = getDictCode();

    string originalWord = original.toUtf8().constData();
    string contextStr = context.toUtf8().constData();

    qDebug() << Q_FUNC_INFO << "key: " << key.c_str() << ", dictCode: " << dictCode.c_str() << ", originalWord: " << originalWord.c_str() << ", contextStr: " << contextStr.c_str();

    #ifdef Q_WS_QWS
        string translatedWord = searchHtml(key, originalWord, contextStr, 3,dictCode,1,1);
    #else
        string translatedWord = QString("").toUtf8().constData();
    #endif

    qDebug() << Q_FUNC_INFO << "Translated word:" << QString::fromUtf8(translatedWord.c_str()) << " size:" <<translatedWord.size();

    if(!QBookApp::instance()->isActivated() && !QBookApp::instance()->isLinked()){
        return "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html><head></head><body><div>"
                "<p style=\"text-align: left; font-family: 'Lato'; font-size: 24pt; padding-bottom:5pt;\">"
                + tr("Please, remember to activate the device to use")
                + "</p></div></body></html>";
    }

    // TODO: requested new version to Semantix that returns empty string to avoid this check
    if(translatedWord.empty() || translatedWord.size() <= EMPTY_DICTIO_RESULT )
        return "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"><html><head></head><body>"
                "<div style=\"font-family:Verdana,Arial,Helvetica,sans-serif;line-height:25px;font-size:80%;\">"
                "<p style=\"color:#233D94;font-size:1.25em;text-align:left;font-weight:bold;margin-top:10px;margin-bottom:10px;\">"
                + tr("%1").arg(original)
                + "</p></div>"
                + "<p style=\"text-align: left; font-family: 'Lato'; font-size: 24pt; padding-bottom:5pt;\">"
                + tr("Couldn't find any result for the given word")
                + "</p></body></html>";
    else
        return QString::fromUtf8(translatedWord.c_str());
}

string
Dictionary::getDictCode() const
{
    QString dictSettingsCode = QBook::settings().value("setting/dictionary", "").toString();

    if(dictSettingsCode.isEmpty() || dictSettingsCode == "0")
        return "";

    return dictSettingsCode.toStdString();
}

QString Dictionary::getCurrentDictCode() const
{
    QString dictSettingsCode = QBook::settings().value("setting/dictionary", "").toString();

    if(dictSettingsCode.isEmpty() || dictSettingsCode == "0")
        return "";

    return dictSettingsCode;
}

QByteArray Dictionary::getJsonDictionaries() const
{
    qDebug() << Q_FUNC_INFO;

    if (!initialized)
        return tr("You need to link the device to see available dictionaries").toAscii();

    //QByteArray dictsJson = "{\"dictionaries\":[{\"id\":\"000200\",\"commercialName\":\"Definiciones Inglés\",\"shortName\":\"wordne\"},{\"id\":\"000300\",\"commercialName\":\"Semantix Inglés-Español\",\"shortName\":\"semEnEs\"},{\"id\":\"000400\",\"commercialName\":\"Semantix Español-Inglés\",\"shortName\":\"semEsEn\"},{\"id\":\"001000\",\"commercialName\":\"Definiciones Español\",\"shortName\":\"clave\"},{\"id\":\"001300\",\"commercialName\":\"Semantix Inglés básico\",\"shortName\":\"simple\"}]}";
    //return dictsJson;

    return QString::fromStdString(getWorkingDictionariesEx()).toAscii();
}

QByteArray Dictionary::getJsonAvailableDicts() const
{
    qDebug() << Q_FUNC_INFO;

    if (!initialized)
        return tr("You need to link the device to see available dictionaries").toAscii();

    bqDeviceServices* services = QBookApp::instance()->getDeviceServices();
    QByteArray availableDictsJson = services->getDictionaries().toAscii();
    qDebug() << Q_FUNC_INFO << "availableDictsJson: " << availableDictsJson;
    return availableDictsJson;
}

QMap<QString, DictionaryParams> Dictionary::parseCurrentDictionaries( const QString& json )
{
    qDebug() << Q_FUNC_INFO << json;
    QMap<QString, DictionaryParams> dictionaryList;

    bool parseOk;
    QVariantMap result = Json::parse(json, parseOk).toMap();

    if(!parseOk)
        return dictionaryList;

    QList<QVariant> dictionaries = result["dictionaries"].toList();

    if(!dictionaries.size())
        return dictionaryList;

    QList<QVariant>::const_iterator it = dictionaries.constBegin();
    QList<QVariant>::const_iterator itEnd = dictionaries.constEnd();

    //Automatic dictionary:
    DictionaryParams automatic;
    automatic.id = "0";
    automatic.commercialName = QString(tr("Automatic"));
    automatic.activationState = true;
    dictionaryList.insert(automatic.commercialName, automatic);

    for(; it != itEnd; ++it)
    {
        DictionaryParams dictionary;
        dictionary.id = it->toMap()["id"].toString();
        dictionary.commercialName = rename(dictionary.id);
        dictionary.shortName = it->toMap()["shortName"].toString();
        dictionary.downloadUrl = it->toMap()["url"].toString();
        dictionary.activationState = true;

        if(!dictionary.commercialName.isEmpty()) //Not included simple english words dictionary.
            dictionaryList.insert(dictionary.commercialName, dictionary);
    }

    qDebug() << Q_FUNC_INFO << "size: " << dictionaryList.size();
    return dictionaryList;
}

QMap<QString, DictionaryParams> Dictionary::parseAvailableDictionaries(const QString& json, QString& dixioToken)
{
    qDebug() << Q_FUNC_INFO << json;
    QMap<QString, DictionaryParams> dictionaryList;

    bool parseOk;
    QVariantMap result = Json::parse(json, parseOk).toMap();

    if(!parseOk)
        return dictionaryList;

    dixioToken = result["tokenDixio"].toString();
    QVariantMap dictionaries = result["dictionaries"].toMap();
    if(!dictionaries.size())
        return dictionaryList;

    QMap<QString, QVariant>::iterator i;

    for (i = dictionaries.begin(); i != dictionaries.end(); ++i)
    {
        QVariantMap dictionaryMap = i.value().toMap();
        DictionaryParams dictionary;
        dictionary.id = dictionaryMap["id"].toString();
        dictionary.commercialName = dictionaryMap["commercialName"].toString();
        dictionary.shortName = dictionaryMap["shortName"].toString();
        dictionary.downloadUrl = dictionaryMap["url"].toString();
        dictionary.dixFileSize = dictionaryMap["url_size"].toInt();
        dictionary.downloadUrlComp = dictionaryMap["url_comp"].toString();
        dictionary.compFileSize = dictionaryMap["url_comp_size"].toInt();
        dictionary.downloadFileName = dictionaryMap["downloadFileName"].toString();
        dictionary.downloadFileNameComp = dictionaryMap["downloadFileName_comp"].toString();
        dictionary.activationState = true;
        dictionaryList.insert(dictionary.commercialName, dictionary);
    }

    return dictionaryList;
}

bool Dictionary::getActivation(QString dixioKey, QString& _callUrl, QString& _postParameters)
{
    qDebug() << Q_FUNC_INFO << "dixioKey: " << dixioKey;
    String key = dixioKey.toStdString();
    String callUrl = _callUrl.toStdString();
    String postParameters = _postParameters.toStdString();

    getActivationData(key, "", callUrl, postParameters);
    _postParameters = QString::fromStdString(postParameters);
    _callUrl = QString::fromStdString(callUrl);

    return _callUrl.size() && _postParameters.size();
}

String Dictionary::setActivation(QString key, QString activationResponse)
{
    return setActivationData(key.toStdString(), activationResponse.toStdString());
}

QString Dictionary::getDixioKey()
{
    QString path = Storage::getInstance()->getDataPartition()->getMountPoint() + QDir::separator() + "dixioKey";
    QString dixioKey = "";
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << Q_FUNC_INFO << "Cannot open dixioKey file";
    }else{
        QTextStream in(&file);
        dixioKey = in.readAll();
        file.close();
    }

    return dixioKey;
}

QString Dictionary::rename( QString& codigo )
{
    qDebug() << Q_FUNC_INFO << "codigo:" << codigo;

    if(codigo == "000200" || codigo == "138")
        return QString("English definitions");
    else if(codigo == "001000" || codigo == "354")
        return QString("Definiciones Espa%1ol").arg(QChar(241));
    else if(codigo == "000300" || codigo == "111")
        return QString("Semantix Ingl%1s - Espa%2ol").arg(QChar(233)).arg(QChar(241));
    else if(codigo == "000400" || codigo == "112")
        return QString("Spanish - English by Semantix");
    else if(codigo == "224")
        return QString("Definicions Catal%1").arg(QChar(225));
    else return "";
}

void Dictionary::renewDixioKey()
{
    qDebug() << Q_FUNC_INFO;
    m_dixioKey.clear();
}
