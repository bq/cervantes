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

#ifndef QADOBECLIENT_H
#define QADOBECLIENT_H

#include <stddef.h>
#include "dp_doc.h"
#include <QObject>

class QRect;
class QUrl;

/*-------------------------------------------------------------------------*/

class QAdobeClient : public QObject, public dpdoc::DocumentClient, public dpdoc::RendererClient
{
    Q_OBJECT

public:
    QAdobeClient();
    virtual ~QAdobeClient();

	virtual void* getOptionalInterface(const char * name);
	virtual int getInterfaceVersion();

	virtual dpio::Stream* getResourceStream(const dp::String& url, unsigned int caps);
	virtual bool canContinueProcessing(int kind);
	virtual void reportLoadingState(int state);
    virtual void reportDocumentError(const dp::String& errorString);
    virtual void reportErrorListChange();
	virtual void requestLicense(const dp::String& type, const dp::String& resourceId, const dp::Data& requestData);
	virtual void requestDocumentPassword();
	virtual void documentSerialized();

    virtual double getUnitsPerInch();
    virtual void requestRepaint(int xMin, int yMin, int xMax, int yMax);
	virtual void navigateToURL(const dp::String& url, const dp::String& target);    
	virtual void reportMouseLocationInfo(const dpdoc::MouseLocationInfo& info);    
	virtual void reportInternalNavigation();
	virtual void reportDocumentSizeChange();
	virtual void reportHighlightChange(int highlightType);
	virtual void reportRendererError(const dp::String& errorString);
	virtual void finishedPlaying();

    void stopProcessing();
    void clearAbortProcessing();
    void setEditorFonts ( bool editorFonts ) { m_editorFonts = editorFonts; }
    void setIncorrectCSS ( bool incorrectCSS ) { m_hasIncorrectCSS = incorrectCSS; }

signals:
    void stateChanged(int state);
    void navigationChanged();
    void sizeChanged();
    void errorOccurred(const QString& msg);
    void repaintRequested(const QRect& bound);
    void navigationRequested(const QUrl& url);
    void passwordRequested();
    void licenseRequested(const QString& type, const QString& res, const uchar* data, size_t len);

private:
    bool m_loaded;
    bool m_aborted;
    bool m_editorFonts;
    bool m_hasIncorrectCSS;
};

/*-------------------------------------------------------------------------*/

#endif
