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

#ifndef ADOBEDRM_H
#define ADOBEDRM_H

#include <QObject>
#include <QPointer>
#include <QDateTime>
#include <QFile>

#include "dp_drm.h"

// Predeclarations
namespace dpdrm { class DRMProcessor; }
namespace dpdev { class Device; }
namespace dpdoc { class Document; }

namespace dpnet { class NetProvider; }
namespace dpio { class Partition; }
namespace dp { class Data; }

class StorageDevice;
class QAdobeDocView;


// Client to notify progress in the workflow operations
class AdobeDRMProcessorClient : public QObject, public dpdrm::DRMProcessorClient
{
    Q_OBJECT
public:
    virtual void workflowsDone(unsigned int workflows, const dp::Data& followUp);
    virtual void requestInput(const dp::Data& inputXHTML);
    virtual void requestConfirmation(const dp::String& code);
    virtual void reportWorkflowProgress(unsigned int workflow, const dp::String& title, double progress);
    virtual void reportWorkflowError(unsigned int workflow, const dp::String& errorCode);
    virtual void requestPasshash( const dp::ref<dpdrm::FulfillmentItem>& fulfillmentItem );
    virtual void reportFollowUpURL( unsigned int, const dp::String&) {}

signals:
    void                                workflowDone                        ( unsigned int workflows );
    void                                requestPasshash                     ();

    void                                workflowError                       ( int errorCode, QString errorMsg );
    void                                workflowProgress                    ( QString title, double progress );
};

class AdobeDRM : public QObject
{
    Q_OBJECT

public:

    struct Permission
    {
        QString     type;
        QDateTime	expiration;
        QString 	loanId;
        QString 	deviceId;
        QString		devicetype;
        double		maxResolution;
        bool		valid;
        bool 		isConsumable;
        int 		initialCount;
        int 		maxCount;
        double 		incrementInterval;
    };

public:

    static void         staticInit                                  ();
    static void         staticDone                                  ();
    static AdobeDRM*    getInstance                                 () { return s_pInstance; }

    bool                isLinked                                    () const;
    void                link                                        ( const QString& userID, const QString& password );
    void                unlink                                      ();

    void                fulfillment                                 ( QFile& file );

    QString             getPermissionString                         ( dpdoc::Document* doc, const char* type , bool* drmFile );

    QDateTime           expiredTime                                 ( dpdoc::Document* doc );

    bool                addHashPass                                 ( const QString& op, const QString& user, const QString& pass );
    bool                removeHashPass                              ( const QString& op, const QString& user, const QString& pass );

    const QString&      getAdobeID                                  () const { return m_DRM_ID; }

public slots:

    void                connectedToPc                               ( bool connected );

protected:

    Permission          getPermission                               ( dpdoc::Document* doc, const char* type );

protected slots:

    void                processLink                                 ();
    void                linkWorkflowDone                            ( unsigned int workflows );
    void                linkWorkflowError                           ( int, QString );

    void                processFulfillment                          ();
    void                fulfillmentWorkflowDone                     ( unsigned int workflows );
    void                fulfillmentWorkflowError                    ( int, QString );

signals:

    void                linkOK                                      ();
    void                linkKO                                      ( int );

    void                unlinkOK                                    ();
    void                unlinkKO                                    ();

    void                fulfillmentOK                               ( QString fulfillmentId, bool bReturnable, QString fulfillmentDocPath );
    void                fulfillmentKO                               ( QString errorMsg );

private:

                        AdobeDRM                                    ();
    virtual             ~AdobeDRM                                   ();
    static AdobeDRM*    s_pInstance;
    bool                init                                        ();
    void                done                                        ();

    dpdev::Device*      m_device;
    dpdrm::DRMProcessor* m_proc;
    AdobeDRMProcessorClient* m_client;
    dpnet::NetProvider* m_netProvider;

    QByteArray          m_fulfillmentACSMInProgress;

    QString             m_DRM_ID;
    QString             m_DRM_PW;

    void                clearCredentials                            ();
    void                gatherCredentials                           ();
    bool                eraseActivationFile                         ();
    bool                readUserNameFromXML                         ();
    bool                readUserNameFromActivationList              ();
};

QDebug operator<<(QDebug dbg, const AdobeDRM::Permission& p);


#endif // ADOBEDRM_H
