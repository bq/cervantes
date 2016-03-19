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

 
 #ifndef _CURLNETPROVIDER_H
#define _CURLNETPROVIDER_H

#include <dp_net.h>
#include <dp_io.h>

class CurlNetProvider : public dpnet::NetProvider 
{
public:
	CurlNetProvider(bool verbose);

	virtual ~CurlNetProvider();

	/**
	 *  Initiate a raw network request (HTTP or HTTPS, GET or POST). When request is processed,
	 *  StreamClient methods should be called with the result obtained from the server. Note that for best
	 *  performance this call should not block waiting for result from the server. Instead the communication
	 *  to the server should happen on a separate thread or through event-based system.
	 */
	virtual dpio::Stream * open( const dp::String& method, const dp::String& url, dpio::StreamClient * client,
		unsigned int cap, dpio::Stream * dataToPost = 0 );

private:
	bool m_verbose;
};

#define NETPROVIDERIMPL CurlNetProvider

#endif

