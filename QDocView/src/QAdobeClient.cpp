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

#include "QAdobeClient.h"
#include "QDocView.h"
#include "QBook.h"
#include "BookInfo.h"

#include <QUrl>
#include <QFile>
#include <QDebug>

#define OTF_DUMMY_DEVID     "0123456789AB0000"
#define OTF_AES_KEY_BITS    128
#define OTF_BLOCK_LEN       16
#define OTF_MACADDR_FILE    "/macaddr"

#ifdef QDEF_ADOBE_OTF_DECRYPTION
    #include "openssl/aes.h"
    static unsigned char _dev_id[OTF_BLOCK_LEN];

    #ifdef Q_WS_QWS
        extern "C" 
    #endif
    int genkey(unsigned char* devid, unsigned char* iv, unsigned char* key);
#else
    struct AES_KEY;
#endif

/*-------------------------------------------------------------------------*/

class QAdobeFileStream : public dpio::Stream
{
public:
    static QAdobeFileStream* create(const QUrl& url);
    QAdobeFileStream(QFile* file, AES_KEY *key = 0);
    virtual ~QAdobeFileStream();

    virtual void release();
    virtual void setStreamClient(dpio::StreamClient* receiver);
    virtual unsigned int getCapabilities();
    virtual void requestInfo();
    virtual void requestBytes(size_t offset, size_t len);
    virtual void reportWriteError(const dp::String& error);

private:
    dpio::StreamClient* m_receiver;
    unsigned char m_buffer[32 * 1024];
    QString m_mimeType;
    QFile* m_file;
    AES_KEY* m_aes_key;
    int m_magic_count;
};

/*-------------------------------------------------------------------------*/

QAdobeFileStream* QAdobeFileStream::create(const QUrl& url)
{
    if (url.scheme() != QLatin1String("file")) return 0;

    QFile* file = new QFile(url.toLocalFile());
    if (!file->open(QFile::ReadOnly)) {
        delete file;
        return 0;
    }

    return new QAdobeFileStream(file);
}

QAdobeFileStream::QAdobeFileStream(QFile* file, AES_KEY *key)
	: m_receiver(0)
    , m_file(file)
    , m_aes_key(0)
{
    qDebug() << "--->" << Q_FUNC_INFO;

    qDebug() << "key:" << key;

    m_mimeType = QDocView::guessMimeType(QUrl::fromLocalFile(m_file->fileName()));
    if (m_mimeType == QLatin1String("file")) m_mimeType = QString("application/octet-stream");

#ifdef QDEF_ADOBE_OTF_DECRYPTION
    if (key) {
        m_aes_key = new AES_KEY;
        *m_aes_key = *key;

        m_magic_count = 5;
        if (m_file->fileName().toLower().endsWith(QString(".enc.epub"), Qt::CaseInsensitive))
            --m_magic_count ;
    }
#endif
}

QAdobeFileStream::~QAdobeFileStream()
{
    if (m_file) m_file->close();
        delete m_file;
    m_file = NULL;

    if (m_aes_key)
        delete m_aes_key;
    m_aes_key = NULL;

}

void QAdobeFileStream::release()
{
	delete this;
}

unsigned int QAdobeFileStream::getCapabilities()
{
	return dpio::SC_SYNCHRONOUS|dpio::SC_BYTE_RANGE;
}

void QAdobeFileStream::setStreamClient(dpio::StreamClient* receiver)
{
	m_receiver = receiver;
}

void QAdobeFileStream::requestInfo()
{
    if (!m_receiver) return;
    if (m_aes_key) {
	    m_receiver->totalLengthReady(m_file->size() - OTF_BLOCK_LEN);
    } else {
		m_receiver->totalLengthReady(m_file->size());
	}
	m_receiver->propertyReady("Content-Type", m_mimeType.toAscii().constData());
    m_receiver->propertiesReady();
}

void QAdobeFileStream::requestBytes(size_t offset, size_t len)
{
    if (!m_receiver) return;

	if (!m_aes_key) {
        if (!m_file->seek(offset)) {
            m_receiver->bytesReady(offset, dp::Data(), true);
            return;
	    }
    
	    while (len > 0) {
		    qint64 to_read = len;
		    if (to_read > sizeof(m_buffer)) to_read = sizeof(m_buffer);
		    qint64 had_read = m_file->read(reinterpret_cast<char*>(m_buffer), to_read);
            if (had_read <= 0) {
                m_receiver->bytesReady(offset, dp::Data(), true);
                break;
            } else {
                bool eof = m_file->atEnd();
                m_receiver->bytesReady(offset, dp::TransientData(m_buffer, had_read), eof);
                if (eof) break;
            }
		    offset += had_read;
		    len -= had_read;
	    }
	}

#ifdef QDEF_ADOBE_OTF_DECRYPTION
    else {
        unsigned char iv[OTF_BLOCK_LEN] = {0};
        size_t sbidx = offset / OTF_BLOCK_LEN;             // the block index of the begin of data
        size_t tbidx = (offset + len - 1) / OTF_BLOCK_LEN; // the block index of the end of data
        size_t sboff = offset % OTF_BLOCK_LEN;             // byte offset in the begining block.
        size_t tboff = (offset + len - 1) % OTF_BLOCK_LEN; // byte offset in the ending block.
        int    ivnum = 0;  // Needed by the AES CFB decryptor, not meaningful here. Must be 0.

        /* Decide what the IV should be. */
        qint64 fpos = OTF_BLOCK_LEN + ((sbidx-1)*OTF_BLOCK_LEN);
        if (!m_file->seek(fpos)) {
            m_receiver->bytesReady(offset, dp::Data(), true);
            return; // I/O error.
        }
        if (sbidx) {
            if (OTF_BLOCK_LEN != m_file->read((char*)&iv[0], OTF_BLOCK_LEN)) {
                qDebug("Error: Beyound the valid file range.");
                m_receiver->bytesReady(offset, dp::Data(), true);
                return; // I/O error.
            }
        } else {
            unsigned char foo[OTF_BLOCK_LEN];
            if (OTF_BLOCK_LEN != m_file->read((char*)&iv[0], OTF_BLOCK_LEN)) {
                qDebug("Error: Beyound the valid file range.");
                m_receiver->bytesReady(offset, dp::Data(), true);
                return; // I/O error
            }
            ::genkey(_dev_id, iv, foo); // retrieve valid iv.
        }

        /* Do the on-the-fly decrypt */
        while (sbidx <= tbidx && len > 0) {
            qint64 tr;                      // Bytes to read
            qint64 hr;                      // Bytes has been read
            qint64 cnt;                     // Length of chosen bytes
            unsigned char cbuf[4096];       // Buffer for ciphered data.
            bool cut = true;                // Whether to do cut off at the last block.
            int rx = OTF_BLOCK_LEN;         // Length of remainder bytes in the last block.
            int cx;                         // Length of bytes to be cut-off.

            /* How many bytes to read? */
            tr = (tbidx - sbidx + 1) * OTF_BLOCK_LEN;

            /* If > 4096, read it as multiple chrunks, each as 4096 bytes. */
            if (tr > 4096) {
                cut = false; // Don't do cut off for internal chrunks.
                tr = 4096;
            }

            /* Read payload */
            hr = m_file->read((char*)&cbuf[0], tr);
            if (!hr) {
                m_receiver->bytesReady(offset, dp::Data(), true);
                return;
            }

            /* Decrypt payload */
            AES_cfb128_encrypt((const unsigned char *)&cbuf[0], (unsigned char *)&m_buffer[0],
                    hr, m_aes_key, (unsigned char *)&iv[0], &ivnum, AES_DECRYPT);

            // Magic replacement is needed for the first block.
            if (sbidx == 0) {
                if (m_magic_count == 4) {
                    m_buffer[0] = 'P';
                    m_buffer[1] = 'K';
                    m_buffer[2] = '\003';
                    m_buffer[3] = '\004';
                } else {
                    m_buffer[0] = '%';
                    m_buffer[1] = 'P';
                    m_buffer[2] = 'D';
                    m_buffer[3] = 'F';
                    m_buffer[4] = '-';
                }
            }

            /* Compute how many remainder bytes there in the last block */
            if (hr < tr) {
                rx = hr % OTF_BLOCK_LEN;
                if (!rx) rx = OTF_BLOCK_LEN;
            }

            /* Compute how many bytes to be cut off */
            cx = rx - tboff - 1;
            cnt = hr - sboff;                          // Drop those bytes ahead of the begining byte.
            if (cut && (cx > 0)) cnt -= cx;            // Do the cut-off.
            m_receiver->bytesReady(offset, dp::Data(&m_buffer[sboff], cnt), hr < tr); 

            len -= cnt;
            offset += cnt;
            sbidx += tr / OTF_BLOCK_LEN;
            sboff = 0; // sboff should be always 0 in subsequent loop.
        }
	}
#endif
}

void QAdobeFileStream::reportWriteError(const dp::String& error)
{
    qWarning() << Q_FUNC_INFO << "error =" << error.utf8();
}

/*-------------------------------------------------------------------------*/

QAdobeClient::QAdobeClient()
    : m_loaded(false)
    , m_aborted(false)
    , m_editorFonts(true)
    , m_hasIncorrectCSS(false)
{
    // do nothing
}

QAdobeClient::~QAdobeClient()
{
    // do nothing
}

/*-------------------------------------------------------------------------*/

int QAdobeClient::getInterfaceVersion()
{
    return 1;
}

void* QAdobeClient::getOptionalInterface(const char*)
{
    return NULL;
}

/*-------------------------------------------------------------------------*/

bool QAdobeClient::canContinueProcessing(int kind)
{
	return !m_aborted && kind != dpdoc::PK_BACKGROUND;
}

void QAdobeClient::stopProcessing()
{
    m_aborted = true;
}

void QAdobeClient::clearAbortProcessing()
{
    m_aborted = false;
}

dpio::Stream* QAdobeClient::getResourceStream(const dp::String& spec, unsigned int capabilities)
{
    qDebug() << Q_FUNC_INFO << spec.utf8() << capabilities;
    dp::String url = spec;
    const char* urlUtf8 = url.utf8();

    if( ::strncmp( urlUtf8, "data:", 5 ) == 0 )
            return dpio::Stream::createDataURLStream( url, NULL, NULL );

    //Change to current font
    if(::strncmp( urlUtf8, "res:///userStyle.css", 20 ) == 0)
    {
        if (!m_editorFonts || m_hasIncorrectCSS)
        {
            qDebug() << Q_FUNC_INFO << "Loading our css";
            url = dp::String("file:///tmp/epub.css");
            urlUtf8 = "file:///tmp/epub.css";
        }
        else
        {
            qDebug() << Q_FUNC_INFO << "using editor fonts";
        }
    }
    // resources: user stylesheet, fonts, hyphenation dictionaries and resources they references
    if( ::strncmp( urlUtf8, "res:///", 7 ) == 0 && url.length() < 1024)
    {
            char tmp[2048];
            ::strcpy( tmp, "file://");
        /* We store fonts in a separate dir than resources */
            if ( ::strncmp( urlUtf8, "res:///fonts", strlen("res:///fonts") ) == 0 ) {
                ::strcat( tmp, QBook::fontsDirPath().toUtf8());
                ::strcat( tmp, "/" );
                ::strcat( tmp, urlUtf8 + strlen("res:///fonts"));
            } else {
                ::strcat( tmp, QBook::resDirPath().toUtf8());
                ::strcat( tmp, "/" );
                ::strcat( tmp, urlUtf8 + strlen("res:///"));
            }
            url = dp::String( tmp );
            urlUtf8 = url.utf8();
    }
    qDebug() << Q_FUNC_INFO << "Reading file: " << urlUtf8;
    dpio::Partition* partition = dpio::Partition::findPartitionForURL( url );
    if( partition != NULL )
    {
        dpio::Stream* st = partition->readFile( url, NULL, capabilities );
        qDebug() << Q_FUNC_INFO << st;
        return st;
    }
    return NULL;
}


/*-------------------------------------------------------------------------*/

void QAdobeClient::reportLoadingState(int state)
{
    switch (state) {
    case dpdoc::LS_INITIAL:
        m_loaded = false;
        break;
    case dpdoc::LS_ERROR:
        emit stateChanged(QDocView::LOAD_FAILED);
        break;
    case dpdoc::LS_INCOMPLETE:
    case dpdoc::LS_COMPLETE:
		if (!m_loaded) {
	        m_loaded = true;
			emit stateChanged(QDocView::LOAD_STARTED);
		}
		if (state == dpdoc::LS_COMPLETE) {
			emit stateChanged(QDocView::LOAD_FINISHED);
		}
        break;
	}
}

void QAdobeClient::reportDocumentError(const dp::String& error)
{
    qDebug() << Q_FUNC_INFO;
    QString msg = QString::fromUtf8(error.utf8());
    emit errorOccurred(msg);
}

void QAdobeClient::reportErrorListChange()
{
    qDebug() << Q_FUNC_INFO;
//    emit errorOccurred("!ERRORS");
}

void QAdobeClient::requestLicense(const dp::String& type, const dp::String& res, const dp::Data& req)
{
    qDebug() << Q_FUNC_INFO << "type=" << type.utf8() << ", res=" << res.utf8() << ", req len=" << req.length();
    emit licenseRequested(QString::fromUtf8(type.utf8()), QString::fromUtf8(res.utf8()), req.data(), req.length());
}

void QAdobeClient::requestDocumentPassword()
{
    qDebug() << Q_FUNC_INFO;
    emit passwordRequested();
}

void QAdobeClient::documentSerialized()
{   
    qDebug() << Q_FUNC_INFO;
}

/*-------------------------------------------------------------------------*/

double QAdobeClient::getUnitsPerInch()
{
    return QBook::screenDpi();
}

static inline int clip(int v)
{
    if (v < -0x3fffffff) return -0x3fffffff;
    if (v > 0x3fffffff) return 0x3fffffff;
    return v;
}

void QAdobeClient::requestRepaint(int xMin, int yMin, int xMax, int yMax)
{
    xMin = clip(xMin);
    yMin = clip(yMin);
    xMax = clip(xMax);
    yMax = clip(yMax);
    emit repaintRequested(QRect(xMin, yMin, xMax - xMin, yMax - yMin));
}

void QAdobeClient::navigateToURL(const dp::String& url, const dp::String&)
{
    emit navigationRequested(QUrl::fromEncoded(url.utf8(), QUrl::TolerantMode));
}

void QAdobeClient::reportMouseLocationInfo(const dpdoc::MouseLocationInfo& )
{
    // do nothing
}

void QAdobeClient::reportInternalNavigation()
{
	emit navigationChanged();
}

void QAdobeClient::reportDocumentSizeChange()
{
	emit sizeChanged();
}

void QAdobeClient::reportHighlightChange(int)
{
    // FIXME
}

void QAdobeClient::reportRendererError(const dp::String& error)
{
    qDebug() << Q_FUNC_INFO << "error" << error.utf8();
}

void QAdobeClient::finishedPlaying()
{
    // do nothing
}
