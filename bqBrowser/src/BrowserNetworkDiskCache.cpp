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

#include <QSettings>

#include "QBook.h"
#include "BrowserNetworkDiskCache.h"
#include "Storage.h"

#define MAX_CACHE_SIZE 50
#define BROWSER_CACHE_DIRECTORY	"browser/cache"

BrowserNetworkDiskCache::BrowserNetworkDiskCache(QObject *parent)
: QNetworkDiskCache(parent)
, m_private(false)
{
        setCacheDirectory(QString (Storage::getInstance()->getDataPartition()->getMountPoint() + "/" + BROWSER_CACHE_DIRECTORY));
}

BrowserNetworkDiskCache::~BrowserNetworkDiskCache()
{}

void BrowserNetworkDiskCache::loadSettings()
{
	qint64 maximumCacheSize = QBook::settings().value(QLatin1String("webkit/maxCacheSize"), 20).toInt();
	if (maximumCacheSize >= MAX_CACHE_SIZE) 
		maximumCacheSize = MAX_CACHE_SIZE * 1024 * 1024;
	else 
		maximumCacheSize = maximumCacheSize * 1024 * 1024;
	setMaximumCacheSize(maximumCacheSize);
}

void BrowserNetworkDiskCache::privacyChanged(bool isPrivate)
{
	m_private = isPrivate;
}

QIODevice *BrowserNetworkDiskCache::prepare(const QNetworkCacheMetaData &metaData)
{
	if (m_private)
		return 0;
	return QNetworkDiskCache::prepare(metaData);
}
