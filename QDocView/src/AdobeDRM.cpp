/*************************************************************************

bq Cervantes e-book reader application
Copyright (C) 2011-2016  Mundoreader, S.L

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

#include <unistd.h>
#include "AdobeDRM.h"

#include "QBook.h"
#include "QBookApp.h"

#include "Storage.h"

#ifdef Q_WS_QWS
#include <QtNetwork>
#include <QProcess>
#include <stddef.h>
#include <stdlib.h>
#endif
#include "dp_core.h"
#include "dp_doc.h"
#include "dp_io.h"


#include <QUrl>
#include <QFile>
#include <QDebug>


#include "curlnetprovider.h"

/*
device.xml file should look lile this:
<deviceInfo xmlns="http://ns.adobe.com/adept">
  <deviceClass>Manufacturer and device code/name, e.g. "ACMEBook EBR120"</deviceClass>
  <deviceSerial>device serial number</deviceSerial>
  <deviceName>User-friendly name of the device</deviceName>
  <deviceType>ACS4 device type</deviceType>
  <version name="hobbes" value="X.X.XXXX"/>
  <fingerprint>base64-encoded fingerprint</fingerprint>
</deviceInfo>

*/

void AdobeDRMProcessorClient::workflowsDone(unsigned int workflows, const dp::Data&)
{
    qDebug() << Q_FUNC_INFO << workflows;
    emit workflowDone(workflows);
}

void AdobeDRMProcessorClient::requestInput(const dp::Data&)
{
    qDebug() << Q_FUNC_INFO;
}

void AdobeDRMProcessorClient::requestConfirmation(const dp::String& code)
{
    qDebug() << Q_FUNC_INFO << code.utf8();
}

void AdobeDRMProcessorClient::reportWorkflowProgress(unsigned int workflow, const dp::String& title, double progress)
{
    qDebug() << Q_FUNC_INFO << workflow << title.utf8() << progress;
//    emit workflowProgress(QString::fromUtf8(title.utf8()), progress);
}

void AdobeDRMProcessorClient::reportWorkflowError(unsigned int errorCode, const dp::String& errorMsg)
{
    qDebug() << Q_FUNC_INFO << errorCode << errorMsg.utf8();
    emit workflowError(errorCode, QString::fromUtf8(errorMsg.utf8()));
}
void AdobeDRMProcessorClient::requestPasshash( const dp::ref<dpdrm::FulfillmentItem>& )
{
    qDebug() << Q_FUNC_INFO;
    emit requestPasshash();
}

/*-------------------------------------------------------------------------*/

QDebug operator<<(QDebug dbg, const AdobeDRM::Permission& p)
{
    dbg.nospace() << "( type:" << p.type
                  << ", expiration: " << p.expiration
                  << ", loadId: " << p.loanId
                  << ", deviceId: " << p.deviceId
                  << ", deviceType: " << p.devicetype
                  << ", maxResolution: " << p.maxResolution
                  << ", valid: " << p.valid
                  << ", isConsumable: " << p.isConsumable
                  << ", initialCount: " << p.initialCount
                  << ", maxCount: " << p.maxCount
                  << ", incrementalInterval: " << p.incrementInterval << ")";

    return dbg.space();
}

AdobeDRM* AdobeDRM::s_pInstance = NULL;

#include "ADConverter.h"

AdobeDRM::AdobeDRM()
    : m_device(NULL)
    , m_proc(NULL)
    , m_client(NULL)
    , m_netProvider(NULL)
{
    qDebug() << Q_FUNC_INFO;
}

AdobeDRM::~AdobeDRM()
{
    done();
}

void AdobeDRM::staticInit()
{
    qDebug() << Q_FUNC_INFO;

    if(s_pInstance) return;

    s_pInstance = new AdobeDRM();
    s_pInstance->init();
}

void AdobeDRM::staticDone()
{
    qDebug() << Q_FUNC_INFO;

    delete s_pInstance;
    s_pInstance = NULL;
}

bool AdobeDRM::init()
{
    done();

    qDebug() << Q_FUNC_INFO;

    dpdev::DeviceProvider* dev_provider = dpdev::DeviceProvider::getProvider(0);
    if (!dev_provider) return false;

    qDebug() << Q_FUNC_INFO << "DeviceProvider created.";

    // getDevice will force generating device.xml. It's written inside the constructor of a static variable, hence it's created only once.
    m_device = dev_provider->getDevice(0);
    if (!m_device) return false;

    qDebug() << Q_FUNC_INFO << "Device created" << m_device->getDeviceName().utf8()
             << ", fingerprint: " << dp::String::base64Encode(m_device->getFingerprint()).utf8()
//             << ", activation record: " << dp::String::base64Encode(m_device->getActivationRecord()).utf8()
             << ", devicekey: " << dp::String::base64Encode(m_device->getDeviceKey()).utf8();

    dpdrm::DRMProvider* drm_provider = dpdrm::DRMProvider::getProvider();
    if (!drm_provider) return false;

    qDebug() << Q_FUNC_INFO << "DRM Provider created";

    m_client = new AdobeDRMProcessorClient();
    m_proc = drm_provider->createDRMProcessor(m_client, m_device);
    if(!m_proc) return false;

    qDebug() << Q_FUNC_INFO << "m_proc created";

    // TODO: Establecer las descargas en /mnt/public
//    char tmp[2048];
//    ::strcpy( tmp, "file://");
//    ::strcat( tmp, qgetenv("ADEPT_ACTIVATION_FILE"));
//    dp::String url = dp::String(tmp);
//    qDebug() << Q_FUNC_INFO << "Setting AdobeDRM partition: " << url.utf8();
//    dpio::Partition* partition = dpio::Partition::findPartitionForURL(url);
//    if( partition != NULL )
//    {
//        qDebug() << Q_FUNC_INFO << "Setting AdobeDRM partition: " << partition->getPartitionName().utf8();
//        m_proc->setPartition(partition);
//    }

    m_netProvider = new NETPROVIDERIMPL(0);
    dpnet::NetProvider::setProvider(m_netProvider);

#ifdef Q_WS_QWS
    if( isLinked() )
    {
        gatherCredentials();
    }
#endif

    return true;
}

void AdobeDRM::done()
{
    clearCredentials();

    if (m_proc)
    {
        m_proc->release();
        m_proc = NULL;
    }

    delete m_client;
    m_client = NULL;

    delete m_netProvider;
    m_netProvider = NULL;
}

//////////// Link stuff
bool AdobeDRM::isLinked() const
{
    // From the Documentation: "If the list that it returns has at least 1 item, the device is activated."
    if (m_proc)
    {
        qDebug() << Q_FUNC_INFO << m_proc->getActivations().length();
        return m_proc->getActivations().length() != 0;
    }
    qDebug() << Q_FUNC_INFO << 0;

    return false;
}

void AdobeDRM::link(const QString& userID, const QString& password)
{
    qDebug() << Q_FUNC_INFO;

    if (isLinked())
    {
        emit linkOK();
        return;
    }

    if (!m_proc)
    {
        emit linkKO(2);
        return;
    }

    m_DRM_ID = userID;
    m_DRM_PW = password;

    #ifdef Q_WS_QWS
    connect(m_client, SIGNAL(workflowDone(unsigned int)),                   this, SLOT(linkWorkflowDone(unsigned int)), Qt::UniqueConnection);
    connect(m_client, SIGNAL(workflowError(int, QString)),                  this, SLOT(linkWorkflowError(int, QString)), Qt::UniqueConnection);
    #endif

    QTimer::singleShot(110, this, SLOT(processLink()));
}

void AdobeDRM::processLink()
{
    qDebug() << Q_FUNC_INFO;

#ifdef Q_WS_QWS
    qDebug() << Q_FUNC_INFO << "user: " << m_DRM_ID << " password: " << m_DRM_PW;

    uint workflow = m_proc->initSignInWorkflow(
            dpdrm::DW_AUTH_SIGN_IN|dpdrm::DW_ACTIVATE|dpdrm::DW_NOTIFY,
            dp::String("AdobeID"),
            dp::String(m_DRM_ID.toUtf8().constData()),
            dp::String(m_DRM_PW.toUtf8().constData()));

    m_proc->startWorkflows(workflow);

#else
    if(m_DRM_ID != "test" || m_DRM_PW != "test")
    {
        linkWorkflowError(2, "DW_AUTH_SIGN_IN");
        return;
    }
    linkWorkflowDone(dpdrm::DW_ACTIVATE);
#endif
}

void AdobeDRM::linkWorkflowDone(unsigned int workflows)
{
    qDebug() << Q_FUNC_INFO << "workflows: " << workflows;

    if(dpdrm::DW_ACTIVATE != workflows)
        return;

    #ifdef Q_WS_QWS
    // Disconnect signals
    disconnect(m_client, SIGNAL(workflowDone(unsigned int)),                    this, SLOT(linkWorkflowDone(unsigned int)));
    disconnect(m_client, SIGNAL(workflowError(int, QString)),                   this, SLOT(linkWorkflowError(int, QString)));
    #endif

    emit linkOK();
}

void AdobeDRM::linkWorkflowError(int err, QString errorMsg)
{
    #ifdef Q_WS_QWS
    // Disconnect signals
    disconnect(m_client, SIGNAL(workflowDone(unsigned int)),                    this, SLOT(linkWorkflowDone(unsigned int)));
    disconnect(m_client, SIGNAL(workflowError(int, QString)),                   this, SLOT(linkWorkflowError(int, QString)));
    #endif

    qDebug() << Q_FUNC_INFO << err << errorMsg;

    clearCredentials();
    eraseActivationFile();

    emit linkKO(err);
}

void AdobeDRM::unlink()
{
    clearCredentials();

    if(eraseActivationFile())
        emit unlinkOK();
    else
        emit unlinkKO();

}

//////////// Fulfillment stuff

// Provisioning new content to the user and the device is called fulfillment
void AdobeDRM::fulfillment( QFile& file )
{
    if (!m_proc || !m_fulfillmentACSMInProgress.isEmpty() )
    {
        emit fulfillmentKO("E_INITIALIZATION");
        return;
    }

    m_fulfillmentACSMInProgress = file.readAll();

    qDebug() << Q_FUNC_INFO << m_fulfillmentACSMInProgress;

    #ifdef Q_WS_QWS
    connect(m_client, SIGNAL(workflowDone(unsigned int)),                   this, SLOT(fulfillmentWorkflowDone(unsigned int)), Qt::UniqueConnection);
    connect(m_client, SIGNAL(workflowError(int, QString)),                  this, SLOT(fulfillmentWorkflowError(int, QString)), Qt::UniqueConnection);
    #endif

    QTimer::singleShot(110, this, SLOT(processFulfillment()));
}

void AdobeDRM::processFulfillment()
{
    qDebug() << Q_FUNC_INFO;

    dp::TransientData data(reinterpret_cast<const unsigned char*>(m_fulfillmentACSMInProgress.constData()), m_fulfillmentACSMInProgress.length());
    uint workflow = m_proc->initWorkflows(dpdrm::DW_FULFILL|dpdrm::DW_DOWNLOAD/*|dpdrm::DW_NOTIFY*/, data);
    m_proc->startWorkflows(workflow);
}

void AdobeDRM::fulfillmentWorkflowDone(unsigned int workflows)
{
    qDebug() << Q_FUNC_INFO << "workflows: " << workflows;

    if(dpdrm::DW_DOWNLOAD != workflows)
        return;

    #ifdef Q_WS_QWS
    // Disconnect signals
    disconnect(m_client, SIGNAL(workflowDone(unsigned int)),                    this, SLOT(fulfillmentWorkflowDone(unsigned int)));
    disconnect(m_client, SIGNAL(workflowError(int, QString)),                   this, SLOT(fulfillmentWorkflowError(int, QString)));
    #endif

    m_fulfillmentACSMInProgress.clear();

    bool bReturnable = m_proc->isReturnable();

    QString fulfillmentId;
//    if(bReturnable)
        fulfillmentId = QString::fromUtf8(m_proc->getFulfillmentID().utf8());

    QString fulfillmentDocPath;
//    QString fileUrl = QString::fromUtf8(m_proc->getFulfillmentDocPath().utf8());
//    #ifdef Q_WS_QWS
//    fulfillmentDocPath = fileUrl.mid(7);
//    #else
//    fulfillmentDocPath = fileUrl.mid(8);
//    #endif

    qDebug() << Q_FUNC_INFO << "fulfillmentId, bReturnable, fulfillmentDocPath" << fulfillmentId << bReturnable << fulfillmentDocPath;

    emit fulfillmentOK(fulfillmentId, bReturnable, fulfillmentDocPath);
}

void AdobeDRM::fulfillmentWorkflowError(int err, QString errorMsg)
{
    #ifdef Q_WS_QWS
    // Disconnect signals
    disconnect(m_client, SIGNAL(workflowDone(unsigned int)),                    this, SLOT(fulfillmentWorkflowDone(unsigned int)));
    disconnect(m_client, SIGNAL(workflowError(int, QString)),                   this, SLOT(fulfillmentWorkflowError(int, QString)));
    #endif

    qDebug() << Q_FUNC_INFO << err << errorMsg;

    m_fulfillmentACSMInProgress.clear();

    emit fulfillmentKO(errorMsg);
}

/////////////////
bool AdobeDRM::addHashPass( const QString& op, const QString& user, const QString& pass )
{
    qDebug() << Q_FUNC_INFO;

    if (!m_proc) return false;
    dp::Data hash = m_proc->calculatePasshash(dp::String(user.toUtf8().constData()), dp::String(pass.toUtf8().constData()));
    int err = m_proc->addPasshash(dp::String(op.toUtf8().constData()), hash);

    return err != dpdrm::PH_ERROR;
}

bool AdobeDRM::removeHashPass( const QString& op, const QString& user, const QString& pass )
{
    qDebug() << Q_FUNC_INFO;

    if (!m_proc) return false;
      dp::Data hash = m_proc->calculatePasshash(dp::String(user.toUtf8().constData()), dp::String(pass.toUtf8().constData()));
    int err = m_proc->removePasshash(dp::String(op.toUtf8().constData()), hash);
    return err != dpdrm::PH_ERROR;
}

QDateTime AdobeDRM::expiredTime( dpdoc::Document* doc )
{
    qDebug() << Q_FUNC_INFO;

    if (!m_proc || !doc) return QDateTime();
    dp::ref<dpdrm::Rights> rights = doc->getRights();
    if (!rights) return QDateTime();

    dp::list<dpdrm::License> licenses = rights->getValidLicenses(m_device);
    dp::time_t max_time = 0;
    unsigned int length = licenses.length();
    for (unsigned int i = 0; i < length; ++i)
    {
        dp::ref<dpdrm::License> license = licenses[i];
        dp::list<dpdrm::Permission> perms = license->getPermissions(dp::String("display"));
        unsigned int permsLength = perms.length();
        for (unsigned int j = 0; j < permsLength; ++j)
        {
            dp::ref<dpdrm::Permission> perm = perms[j];
            dp::time_t time = perm->getExpiration();
            if (time > max_time) max_time = time;
        }
    }

    if (!max_time) return QDateTime();
    return QDateTime::fromTime_t(max_time / 1000);
}

AdobeDRM::Permission AdobeDRM::getPermission( dpdoc::Document* doc, const char* type )
{
    struct Permission rperm;

    rperm.type = QString(type);
    rperm.valid = false;

    if (!m_proc || !doc) return rperm;

    dp::ref<dpdrm::Rights> rights = doc->getRights();
    if (!rights) return rperm;

    dp::list<dpdrm::License> licenses = rights->getValidLicenses(m_device);
    dp::time_t max_time = 0;

    unsigned int licSize = licenses.length();
    for (unsigned int i = 0; i < licSize; ++i)
    {
        dp::ref<dpdrm::License> license = licenses[i];
        dp::list<dpdrm::Permission> perms = license->getPermissions(dp::String(type));

        qDebug() << Q_FUNC_INFO << "License " << i
                 << "UserID: " << QString::fromUtf8(license->getUserID().utf8())
                 << ", ResourceID: " << QString::fromUtf8(license->getResourceID().utf8())
                 << ", VoucherID: " << QString::fromUtf8(license->getVoucherID().utf8())
                 << ", Licensee: " << QString::fromUtf8(license->getLicensee().utf8());

        if(perms.length())
            rperm.valid = true;

        unsigned int permsSize = perms.length();
        for (unsigned int j = 0; j < permsSize ; ++j)
        {
            dp::ref<dpdrm::Permission> perm = perms[j];
            dp::time_t time = perm->getExpiration();
            if (time > max_time) max_time = time;

            qDebug() << Q_FUNC_INFO << "Permission " << j
                     << "PermissionType: " << QString::fromUtf8(perm->getPermissionType().utf8())
                     << ", Expiration: " << QDateTime::fromTime_t(perm->getExpiration()/1000)
                     << ", LoanId: " << QString::fromUtf8(perm->getLoanID().utf8())
                     << ", DeviceID: " << QString::fromUtf8(perm->getDeviceID().utf8())
                     << ", DeviceType: " << QString::fromUtf8(perm->getDeviceType().utf8())
                     << ", MaxResolution: " << perm->getMaxResoultion()
                     << ", isConsumable: " << perm->isConsumable();

            rperm.loanId = QString::fromUtf8(perm->getLoanID().utf8());
            rperm.deviceId = QString::fromUtf8(perm->getDeviceID().utf8());
            rperm.devicetype = QString::fromUtf8(perm->getDeviceType().utf8());
            rperm.maxResolution = perm->getMaxResoultion();
            rperm.isConsumable = perm->isConsumable();
            if(perm->isConsumable())
            {
                rperm.initialCount  = perm->getInitialCount();
                rperm.maxCount  = perm->getMaxCount();
                rperm.incrementInterval = perm->getIncrementInterval();
            }
            else
            {
                rperm.initialCount  = 0;
                rperm.maxCount  = 0;
                rperm.incrementInterval = 0;
            }

            qDebug() << Q_FUNC_INFO << "Iter " << j << rperm;
        }
    }

    if(rperm.valid == true)
        rperm.expiration = QDateTime::fromTime_t(max_time/1000);

    return rperm;
}

QString AdobeDRM::getPermissionString( dpdoc::Document* doc, const char* type ,  bool* drmFile )
{
    QString perm_str;
    Permission perm;

    if (!m_proc || !doc) return QString();

    perm = getPermission(doc, type);

    qDebug() << Q_FUNC_INFO << perm;

    if(!perm.valid)
    {
        perm_str.append("Not allowed");
        if(drmFile) *drmFile = true;
        return perm_str;
    }

    // Legal device ?
    if( perm.type == QString("print"))
    {
        perm_str.append("Any Device");
        return perm_str;
    }

    if( perm.type == QString("excerpt"))
    {
        perm_str.append("Any Device");
        return perm_str;
    }

    if(perm.deviceId.isEmpty())
    {
        perm_str.append("Any Device");
    }
    else
    {
        bool isLegalDevice = false;

        dp::list<dpdrm::Activation> activations = m_proc->getActivations();
        unsigned int size = activations.length();
        for (unsigned int i = 0; i < size; ++i)
        {
            dp::ref<dpdrm::Activation> activation = activations[i];
            QString deviceId = QString::fromUtf8(activation->getDeviceID().utf8());
            if( deviceId == perm.deviceId )
            {
                isLegalDevice = true;
                break;
            }
        }

        if(isLegalDevice)
            perm_str.append("Only this Device");
        else
            perm_str.append("Not this Device");

    }

    // Expiration ?
    if(perm.maxResolution != 0 && perm.type == QString("print"))
    {
        perm_str.append(QString(", Max. resolution is %1 dpi").arg(perm.maxResolution));
    }

    if(perm.expiration.toTime_t() != 0)
    {
        perm_str.append(", Until ");
        perm_str.append(perm.expiration.toLocalTime().toString(Qt::SystemLocaleDate));
    }

    if(drmFile)
    {
        QString drm_string("Any Device");
        if(perm_str.compare(drm_string, Qt::CaseSensitive)!= 0)
            *drmFile = true;
        else
            *drmFile = false;
    }

    return perm_str;
}

void AdobeDRM::connectedToPc( bool connected )
{
    qDebug() << Q_FUNC_INFO << "Connected:" << connected;

    if(!connected)
    {
        sync();

        system("cat /mnt/public/.adobe-digital-editions/activation.xml");

        QDir publicDir(qgetenv("ADEPT_ACTIVATION_DIR"));
        QStringList entryList(publicDir.entryList());
        qDebug() << Q_FUNC_INFO << entryList.size() << entryList;

        qDebug() << Q_FUNC_INFO << QFile::exists(qgetenv("ADEPT_ACTIVATION_FILE"));

        system("ls /mnt/public/.adobe-digital-editions/");

        init();

        if(!isLinked() || !entryList.contains("activation.xml"))
        {
            qDebug() << Q_FUNC_INFO << "Now it's NOT linked.";
            // Now it's not linked
            clearCredentials();
            eraseActivationFile();// Just in case
            emit unlinkOK();
        }
        else
        {
            // Now it's linked
            qDebug() << Q_FUNC_INFO << "Now it's linked.";
            emit linkOK();
        }
    }
}

bool AdobeDRM::readUserNameFromXML()
{
    qDebug() << Q_FUNC_INFO;

    // TODO: Revisar el parseo!

    QString filePath = qgetenv("ADEPT_ACTIVATION_FILE");

    QFile file(filePath);
    if(!file.exists())
        return false;

    QString userName;

    if(file.open(QFile::ReadOnly))
    {
        QXmlStreamReader xml(&file);
        xml.readNext(); //Skip first tag
        xml.readNext(); //Skip first tag
        xml.readNext();

        while (!xml.atEnd() && !xml.hasError())
        {
            xml.readNext();
            if (xml.name() == "username" && xml.isStartElement()){
                userName = xml.readElementText();
                break;
            }
        }
        file.close();
    }

    if(!userName.isEmpty())
    {
        m_DRM_ID = userName;

        return true;
    }

    return false;
}

bool AdobeDRM::readUserNameFromActivationList()
{
    qDebug() << Q_FUNC_INFO << "Activations:" << m_proc->getActivations().length();

    if (!m_proc) return false;

    dp::list<dpdrm::Activation> activations = m_proc->getActivations();

    unsigned int length = activations.length();

    if(length == 0)
        return false;

    for(unsigned int i = 0; i < length; ++i)
    {
        dp::ref<dpdrm::Activation> activation = activations[i];
        if( activation->hasCredentials() )
        {
            m_DRM_ID = QString::fromUtf8(activation->getUsername().utf8());
            qDebug() << Q_FUNC_INFO << m_DRM_ID;
            return true;
        }
    }

    qDebug() << Q_FUNC_INFO << "Not activation with credentials.";
    m_DRM_ID = QString::fromUtf8(activations[0]->getUsername().utf8());

    return !m_DRM_ID.isEmpty();
}

void AdobeDRM::gatherCredentials()
{
    qDebug() << Q_FUNC_INFO;

    if( readUserNameFromActivationList() || readUserNameFromXML() )
    {
        qDebug() << Q_FUNC_INFO << ": AdobeDRM has credentials: " << m_DRM_ID;
    }
}

void AdobeDRM::clearCredentials()
{
    qDebug() << Q_FUNC_INFO;

    m_DRM_ID.clear();
    m_DRM_PW.clear();
}

bool AdobeDRM::eraseActivationFile()
{
    if( m_device )
        m_device->setActivationRecord( dp::Data() );

    #ifdef Q_WS_QWS
    QString path = qgetenv("ADEPT_ACTIVATION_FILE");
    if(!path.isEmpty())
        QFile::remove(path);

    if(QFile::exists(path))
        return false;
    #endif

    return true;
}

