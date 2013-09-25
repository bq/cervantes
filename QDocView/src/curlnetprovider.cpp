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

/*************************************************************************
*
* ADOBE CONFIDENTIAL
* ___________________
*
*  Copyright 2009 Adobe Systems Incorporated
*  All Rights Reserved.
*
* NOTICE:  All information contained herein is, and remains
* the property of Adobe Systems Incorporated and its suppliers,
* if any.  The intellectual and technical concepts contained
* herein are proprietary to Adobe Systems Incorporated and its
* suppliers and are protected by trade secret or copyright law.
* Dissemination of this information or reproduction of this material
* is strictly forbidden unless prior written permission is obtained
* from Adobe Systems Incorporated.
**************************************************************************/
 
 #include <string.h>
#include <stdlib.h> //atoi
#include <curl/curl.h>
#include <dp_utils.h>

#include "curlnetprovider.h"

#if (LIBCURL_VERSION_NUM >= 0x071200)
#define USE_CURL_PAUSE 
#endif

static size_t CurlStream_header( void *ptr, size_t size, size_t nmemb, void *handle);
static size_t CurlStream_writer( void *ptr, size_t size, size_t nmemb, void *handle);
static size_t CurlStream_reader( void *ptr, size_t size, size_t nmemb, void *handle);

class CurlStream : public dputils::GuardedStream, public dpio::StreamClient
{
public:
	CurlStream(const dp::String& method, const dp::String& url, dpio::StreamClient * client,
		 dpio::Stream * dataToPost, bool verbose) :
		m_method(method),
		m_client(client),
		m_dataToPost(dataToPost),
		m_readOffset(0),
		m_writeOffset(0),
		m_pDataRead(NULL),
		m_DataCapacity(0),
		m_DataBytesRead(0),
		m_state(0),
		m_saved(NULL),
		m_savedSize(0),
		m_verbose(verbose)
	{
		char * szHeaders = NULL;
		char szContentTypeHeader[] = "Content-type: ";
		size_t contentTypeHeaderLen = sizeof(szContentTypeHeader)/sizeof(char) - 1;
		
		// Create our m_curl handle  
		m_curl = curl_easy_init();  
		m_headers = NULL;
			
		if( m_verbose )
			printf( "Created stream %08x, %s %s\n", this, method.utf8(), url.utf8() );

		if (m_curl)  
		{  
		
			// Now set up all of the m_curl options  
			curl_easy_setopt(m_curl, CURLOPT_URL, url.utf8());  
			curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1);  
			curl_easy_setopt(m_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);		

			if( 0 == strcmp(method.utf8(), "POST") )
			{
				curl_easy_setopt(m_curl, CURLOPT_POST, true );

				// Read all the data to be posted
				// WARNING: this code works only for the synchronous stream
				if (dataToPost)
				{
					dataToPost->setStreamClient(this);
					dataToPost->requestInfo();
					dataToPost->requestBytes(0, (size_t)-1);
				}

				//const char *strContentType = contentTypeUTF16.utf16();
				size_t contentTypeLen = m_contentType.length();

				size_t headersLen = 0;
				if( contentTypeLen != 0 && m_DataBytesRead != 0 )
				{
					headersLen = contentTypeLen + contentTypeHeaderLen;
					szHeaders = new char[headersLen + 1];
					::strcpy( szHeaders, szContentTypeHeader );
					::strcpy( szHeaders + contentTypeHeaderLen, m_contentType.utf8() );

					m_headers = curl_slist_append (m_headers, szHeaders);
					curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headers);
					curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, CurlStream_reader );
					curl_easy_setopt(m_curl, CURLOPT_READDATA, static_cast<void*>(this));
					curl_easy_setopt(m_curl, CURLOPT_POSTFIELDSIZE, m_DataBytesRead);
				}
				else
				{
					curl_easy_setopt(m_curl, CURLOPT_HEADER, 0);  
				}
			}

			curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, CurlStream_header);  
			curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, static_cast<void *>(this));  			
			curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, CurlStream_writer);  
			curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, static_cast<void *>(this));  			
		}

		if (szHeaders)
		{
			delete[] szHeaders;
		}

	}

	~CurlStream()
	{
			// Always cleanup  
			curl_easy_cleanup(m_curl);  
			if( m_headers )
			{
				curl_slist_free_all( m_headers );
			}
			if (m_saved)
				delete[] m_saved;
	}

	void deleteThis()
	{
		delete this;
	}

	void setStreamClient( dpio::StreamClient * client )
	{
		m_client = client;
	}

	unsigned int getCapabilities()
	{
		return dpio::SC_SYNCHRONOUS;
	}

	void requestInfo()
	{
		if (m_state == 0)
		{
			m_state = 1; 
			perform();
		}
	}

	void requestBytes( size_t offset, size_t len )
	{
		switch (m_state)
		{
		case 0: // requestInfo has not be called
			m_state = 3;
			perform();
			break;
		case 1: // requestInfo has been called but we are not blocked yet
			m_state = 3;
			break;
		case 2: // we are blocked 
			m_state = 3;
#ifdef USE_CURL_PAUSE
			curl_easy_pause(m_curl, CURLPAUSE_SEND_CONT);
#endif
			break;
#ifndef USE_CURL_PAUSE
		case 4: 
			m_state = 3;
			break;
#endif
		}
	}

	void reportWriteError( const dp::String& error )
	{
		dputils::StreamGuard guard(this);

		if (m_client)
			m_client->reportError( error );
	}

	virtual void propertyReady( const dp::String& name, const dp::String& value )
	{
		if( ::strcmp(name.utf8(), "Content-Type") == 0) 
			m_contentType = value;
	}

	virtual void propertiesReady()
	{
	}

	virtual void totalLengthReady( size_t length )
	{
		if (!m_pDataRead) {
			m_pDataRead = new unsigned char[length];
			m_DataCapacity = length;
		}
	}

	virtual void bytesReady( size_t offset, const dp::Data& data, bool eof )
	{
		if( !data.isNull() )
		{
			size_t buflen;
			const unsigned char * buf = data.data(&buflen);

			if (offset != m_DataBytesRead) 
			{
				reportError("Stream received non-sequentially");
				return;
			}

			if (m_DataCapacity < m_DataBytesRead + buflen) 
			{

				m_DataCapacity = m_DataBytesRead + buflen;
				unsigned char * tmp = new unsigned char[m_DataCapacity];
				if (m_pDataRead)
				{
					::memcpy(tmp, m_pDataRead, m_DataBytesRead);
					delete[] m_pDataRead;
				}
				m_pDataRead = tmp;
			}

			if( m_verbose )
				reportData( "out", offset, data );

			memcpy(m_pDataRead + m_DataBytesRead, buf, buflen);
			m_DataBytesRead += buflen;
		}
	}

	virtual void reportError( const dp::String& error )
	{
		dputils::StreamGuard guard(this);

		if (m_client)
			m_client->reportError (error);
	}

private:

	void perform ()
	{
		dputils::StreamGuard guard(this);

		int result = curl_easy_perform(m_curl);
		if (result != 0) 
		{
			char err[1024];
			sprintf(err, "E_STREAM_ERROR: CURL returned %d (%X)", result, result);
			if (m_client)
				m_client->reportError(err);
		}
		if (m_saved != NULL)
		{
			if (m_client)
				m_client->bytesReady(m_writeOffset, dp::Data(m_saved, m_savedSize), true);
			delete[] m_saved;
			m_savedSize = 0;
		}
		else
		{
			if (m_client)
				m_client->bytesReady(m_writeOffset, dp::Data(), true);
		}
	}

	void append (unsigned char * ptr, size_t bytes)
	{
		if (m_saved == NULL)
		{
			m_saved = new unsigned char[bytes];
			memcpy(m_saved, ptr, bytes);
			m_savedSize = bytes;
		} else {
			unsigned char * tmp = m_saved;
			m_saved = new unsigned char[m_savedSize + bytes];
			memcpy(m_saved, tmp, m_savedSize);
			memcpy(m_saved + m_savedSize, ptr, bytes);
			m_savedSize += bytes;
			delete[] tmp;
		}
	}

	size_t header_callback( void *ptr, size_t size, size_t nmemb)
	{
		dputils::StreamGuard guard(this);

		if (!m_client)
			return 0;

		// What we will return  
		size_t result = 0;  
		size_t bytes = size * nmemb;

		if (m_state > 1)
			return  bytes;


		// we are not guaranteed that ptr is null-terminated
		char *buf = new char[bytes + 1];
		if (ptr != NULL && bytes != 0)
			::memcpy (buf, ptr, bytes);
		buf[bytes] = (char)0;
		size_t len = ::strlen(buf);
		while (len > 0 && buf[len-1] <= ' ')
		{
			len--;
			buf[len] = 0;
		}

		if (len == 0)
		{
#ifdef USE_CURL_PAUSE
			curl_easy_pause (m_curl, CURLPAUSE_SEND);
			m_state = 2;
#else
			m_state = 4;
#endif
			if (m_client)
			{
				m_client->propertiesReady();
			}
		} 
		else
		{
			char * colon = ::strchr(buf, ':');
			if (colon)
			{
				*colon = (char)0;
				do
				{
					++colon;
				} while (*colon == ' ');

				int t = ::strlen(colon);

				if (m_client)
				{
					if (::strcmp(buf,  "Content-Length") == 0)
					{
						int len = ::atoi(colon);
						if (len > 0)
						{
							m_client->totalLengthReady(len);
						}
					}
					else
					{
						if( m_verbose )
							printf( "Stream %08x header: %s: %s\n", this, buf, colon );
						m_client->propertyReady(buf, colon);
					}
				}
			}
		}

		delete[] buf;

		if (m_client)
			result = bytes;

		return result;  
	}

	// This is the writer call back function used by m_curl  
	size_t writer_callback( void *ptr, size_t size, size_t nmemb)
	{
		dputils::StreamGuard guard(this);

		if (!m_client)
			return 0;

		// What we will return  
		size_t result = 0;  
		size_t bytes = size * nmemb;
		switch (m_state)
		{
		case 0: //  broken state;
			return 0;
		case 1: // unexpected, but let's recover
#ifdef USE_CURL_PAUSE
			curl_easy_pause (m_curl, CURLPAUSE_SEND);
			m_state = 2;
#else
			m_state = 4;
#endif
			append ((unsigned char *)ptr, bytes);
			if (m_client)
			{
				m_client->propertiesReady();
			}
			break;
		case 2:
			{
				//it's the first time after requestBytes was called
				append((unsigned char *)ptr, bytes);
				dp::Data data (m_saved, m_savedSize);
				m_state = 3;
				if (m_client)
				{
					size_t oldOffset = m_writeOffset;
					m_writeOffset += bytes;
					if( m_verbose )
						reportData( "in", oldOffset, data );
					m_client->bytesReady(oldOffset, data, bytes == 0);
				}
			}
			break;
		case 3:
			{
				dp::Data data (static_cast<unsigned char *>(ptr), bytes);
				if (m_client)
				{
					size_t oldOffset = m_writeOffset;
					m_writeOffset += bytes;
					if( m_verbose )
						reportData( "in", oldOffset, data );
					m_client->bytesReady(oldOffset, data, bytes == 0);
				}
			}
			break;

#ifndef USE_CURL_PAUSE
		case 4:
			{
				append ((unsigned char *)ptr, bytes);
			}
			break;
#endif
		}
		if (m_client)
			result = bytes;

		return result;  
	}

	// This is the reader call back function used by m_curl  
	size_t reader_callback ( void *ptr, size_t size, size_t nmemb)
	{
		// What we will return  
		size_t result = 0;
		size_t requestedBytes = size * nmemb;
		size_t actualBytes = m_DataBytesRead - m_readOffset;
		
		if( actualBytes == 0 )
			return 0;
		
		if( actualBytes <= requestedBytes )
		{
			memcpy( ptr, m_pDataRead + m_readOffset, actualBytes );
			result = actualBytes;
		}
		else
		{
			memcpy(ptr, m_pDataRead + m_readOffset, requestedBytes);
			result = requestedBytes;
		}

		m_readOffset += result;
		
		return result;  
	}

	void reportData( const char * dir, size_t offset, const dp::Data& data )
	{
		printf("Stream %08x %s offset=%d\n", this, dir, offset );
		size_t len;
		size_t i;
		const unsigned char * p = data.data(&len);
		for( i = 0 ; i < len && i < 4096 ; i++ )
		{
			if( p[i] == '\n' || p[i] == '\t' || (' ' <= p[i] && p[i] < 0x7F) )
					putchar( p[i] );
			else
					putchar( '.' );
		}
		if( i < len )
			printf("...[%d bytes]...", (len-i));
		printf("\n");
	}


private:
	dp::String m_method;
	dpio::StreamClient *m_client;
	dpio::Stream *m_dataToPost;

	size_t m_writeOffset;
	size_t m_readOffset;

	CURL *m_curl;  
	struct curl_slist *m_headers;
	dp::String m_contentType;
	unsigned char * m_pDataRead;
	size_t m_DataCapacity;
	size_t m_DataBytesRead;

	int m_state;
	unsigned char *m_saved;
	size_t m_savedSize;

	bool m_verbose;

	friend size_t CurlStream_header( void *ptr, size_t size, size_t nmemb, void *buffer);
	friend size_t CurlStream_writer( void *ptr, size_t size, size_t nmemb, void *buffer);
	friend size_t CurlStream_reader( void *ptr, size_t size, size_t nmemb, void *buffer);
};


CurlNetProvider::CurlNetProvider( bool verbose )
: m_verbose(verbose)
{
}

CurlNetProvider::~CurlNetProvider()
{
}

// This is the header writer call back function used by m_curl  
static size_t CurlStream_header( void *ptr, size_t size, size_t nmemb, void *handle)
{
	// What we will return  
	size_t result = 0;  
	if( handle != NULL )
	{
		CurlStream * stream = reinterpret_cast<CurlStream *>(handle);
		result = stream->header_callback (ptr, size, nmemb);
	}
 
	return result;  
}

// This is the writer call back function used by m_curl  
static size_t CurlStream_writer( void *ptr, size_t size, size_t nmemb, void *handle)
{
	// What we will return  
	size_t result = 0;  
	if( handle != NULL )
	{
		CurlStream * stream = reinterpret_cast<CurlStream *>(handle);
		result = stream->writer_callback (ptr, size, nmemb);
	}
 
	return result;  
}

// This is the reader call back function used by m_curl  
static size_t CurlStream_reader( void *ptr, size_t size, size_t nmemb, void *handle)
{
	// What we will return  
	size_t result = 0;  
	if( handle != NULL )
	{
		CurlStream * stream = reinterpret_cast<CurlStream *>(handle);
		result = stream->reader_callback (ptr, size, nmemb);
	}
 
	return result;  
}

dpio::Stream* CurlNetProvider::open( const dp::String& method, const dp::String& url, dpio::StreamClient * client,
	unsigned int cap, dpio::Stream * dataToPost)
{
	return new CurlStream (method, url, client, dataToPost, m_verbose);
}
