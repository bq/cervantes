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

#include "EpubMetaDataExtractor.h"

#include <QString>
#include <QFileInfo>
#include <QDebug>
#include <QStringList>
#include <QMutex>
#include <QMutexLocker>

#include <zip.h>
#include <epub.h>
#include <epub_shared.h>


#define N_ELEMENTS(arr) (signed)(sizeof (arr) / sizeof ((arr)[0]))

/*static*/ QString *EpubMetaDataExtractor::getMetaString(struct epub *book, epub_metadata field, const char *stringOffset) {
	if (book == NULL)
		return NULL;

	QString *result;
	unsigned char **metadata;
	int size;
        metadata = epub_get_metadata(book, field, &size);
	if (size <= 0 || !metadata)
		return NULL;

	if (stringOffset != NULL) {
		char *off_index = strstr((char*)metadata[0], stringOffset);
		if (!off_index)
			return NULL;
		result = new QString (QString::fromUtf8(off_index+strlen(stringOffset)));
	} else
		result = new QString (QString::fromUtf8((char*)metadata[0]));

	return result;
}

struct epub *EpubMetaDataExtractor::openEpub(const QString& filename) {
	/* zero sized files make libepub crash */
	QFileInfo fi = QFileInfo(filename);
	if (fi.size() <= 0) {
        qDebug() << Q_FUNC_INFO << "Epub file with size zero. Not trying to open: " << filename;
		return NULL;
	}

	QByteArray ba = filename.toLocal8Bit();
    return epub_open (ba.data(), 0);
}

/*static*/ bool EpubMetaDataExtractor::getMetaData( const QString& filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString& description, bool& isDRMBook, QString& language ) {
	static const char *ePubAuthorPrefix[] = { "aut: ", "Author: ", NULL };

    struct epub *book = openEpub(filename);
	if (book == NULL) {
        qDebug() << Q_FUNC_INFO << "Cannot open: " << filename;
		return false;
	}

    QString* _title = NULL;
    QString* _author = NULL;
    QString* _publisher = NULL;
    QString* _date = NULL;
    QString* _description = NULL;
    isDRMBook = false;
    QString* _language = NULL;

    qDebug () << Q_FUNC_INFO << "Reading metadata for " << filename;
    _title = getMetaString(book, EPUB_TITLE, NULL);
    qDebug () << Q_FUNC_INFO << "  title: " << (_title ? *_title : "NOT FOUND");

	// We support only one (first) author
	// FIXME: makes it sense to concatenate several authors?
	int i;
    for (i = 0; i < N_ELEMENTS(ePubAuthorPrefix); i++)
    {
        _author = getMetaString(book, EPUB_CREATOR, ePubAuthorPrefix[i]);
        if (_author)
        {
            (*_author) = _author->left(_author->indexOf("("));
			break;
        }
	}
    qDebug () << Q_FUNC_INFO << "  author: " << (_author ? *_author : "NOT FOUND");

    _publisher = getMetaString(book, EPUB_PUBLISHER, NULL);
    qDebug () << Q_FUNC_INFO << "  publisher: " << (_publisher ? *_publisher : "NOT FOUND");
    _date = getMetaString(book, EPUB_DATE, "ops-publication: ");
    qDebug () << Q_FUNC_INFO << "  date: " << (_date ? *_date : "NOT FOUND");
    _description = getMetaString(book, EPUB_DESCRIPTION, NULL);
    qDebug () << Q_FUNC_INFO << "  description: " << (_description ? *_description : "NOT FOUND");
#ifdef Q_WS_QWS
    if(epub_has_encryption(book) && epub_has_rights(book))
        isDRMBook = true;
#endif
    _language = getMetaString(book, EPUB_LANG, NULL);
    qDebug () << Q_FUNC_INFO << "  language: " << (_language ? *_language : "NOT FOUND");

    epub_close (book);

    if(_title)
        title = *_title;
    if(_author)
        author = *_author;
    if(_publisher)
        publisher = *_publisher;
    if(_date)
        date = QDateTime::fromString(*_date, Qt::ISODate);
    if(_description)
        description = *_description;
    if(_language)
        language = *_language;

    delete _title;
    delete _author;
    delete _publisher;
    delete _date;
    delete _description;
    delete _language;

	return true;
}


/*static*/ bool EpubMetaDataExtractor::extractCover(const QString& epubFilename, const QString& coverPath)
{
    qDebug() << Q_FUNC_INFO << "entering" << epubFilename << coverPath;

    qDebug() << Q_FUNC_INFO << "after mutex" << epubFilename;

    int size;
	char *data = NULL;

    struct epub *book = openEpub(epubFilename);
	if (book == NULL) {
        qDebug() << Q_FUNC_INFO << "Cannot open: " << epubFilename;
		return false;
	}

#ifdef Q_WS_QWS
	/* Check for Adobe DRM */
    if (epub_has_encryption(book) && epub_has_rights(book)) {
        qDebug() << Q_FUNC_INFO << epubFilename << " has Adobe DRM. Not trying to extract cover";
        return false;
    }
#endif

	unsigned char **meta;
    meta = epub_get_metadata(book, EPUB_META, &size);
	if (!meta) {
		qDebug() << Q_FUNC_INFO << "Cannot get EPUB_META from " << epubFilename;
		epub_close (book);
		return false;
	}

	char *cover_id = NULL;
	char *cover_file = NULL;
	
	int i;
	for (i = 0; i < size; i++) {
		qDebug() << "extractCover: meta[" << i <<"]:" << (char*)meta[i];
		if (strncmp("cover: ", (char*)meta[i], 7) == 0) {
			cover_id = strdup((char*)meta[i] + 7);	
            qDebug() << Q_FUNC_INFO << "ExtractCover: found cover item: " << cover_id;
			break;
		}
	}

	if (!cover_id) {
		epub_close (book);
		return false;
	}

	struct titerator *tit;
	tit = epub_get_titerator(book, TITERATOR_MANIFEST, 0);

	if (!tit) {
        qDebug () << Q_FUNC_INFO << "ExtractCover: cannot get iterator fot navmap or guide!";
		epub_close (book);
		return false;
	}
	do {
		if (epub_tit_curr_valid(tit)) {
			char *clink = epub_tit_get_curr_link(tit);
			char *clabel = epub_tit_get_curr_label(tit);
			if (strcmp(clabel,cover_id) == 0) {
                qDebug() << Q_FUNC_INFO << "Found cover file: " << clabel << " link: " << clink;
				cover_file = clink;
				free(clabel);
				break;
			}
			free(clink);
			free(clabel);
		}
	} while (epub_tit_next(tit));

	epub_free_titerator(tit);

	if (!cover_file) {
        qDebug() << Q_FUNC_INFO << "Cover file not found in manifest!";
		epub_close (book);
		return false;
	}

	int coverSize;
	coverSize = epub_get_data(book, cover_file, &data);
	if (coverSize <= 0 || !data) {
        qDebug() << Q_FUNC_INFO << "Cover file not found in zipfile: " << cover_file;
		epub_close (book);
		return false;
	}
	QFile dataFile (coverPath);
        if (!dataFile.open(QFile::WriteOnly)) {
        qDebug() << Q_FUNC_INFO << "Cannot open thumbnail file for writing: " << coverPath;
		free(data);
		epub_close (book);
		return false;
        }
    qDebug() << Q_FUNC_INFO << "Writing cover file: " << coverPath << ": " << coverSize << " bytes";
	int res = dataFile.write(data, coverSize);
    qDebug() << Q_FUNC_INFO << "Wrote " << res << " bytes";
	
	dataFile.close();

	free(data);

	epub_close (book);
	return true;
}

/*static*/ QStringList EpubMetaDataExtractor::extractCSS(const QString& epubFilename)
{
	qDebug() << Q_FUNC_INFO << "Extracting CSS from" << epubFilename;
	QStringList epubContentEntries;

        int err = 0;
        QByteArray baInput = epubFilename.toLocal8Bit();
        char *c_input = baInput.data();
        zip *z = zip_open(c_input, 0, &err);
        if (!z) {
                printf("Cannot open %s as zip file. Zip error code: %d", c_input, err);
                return epubContentEntries;
        }

	int entries = zip_get_num_entries(z, 0);
	if (entries <= 0) {
		qDebug() << "Cannot get entries in epub file";
		zip_close(z);
                return epubContentEntries;
	}

	for (int i = 0; i < entries; i++) {
		struct zip_stat st;
        	zip_stat_init(&st);
        	if (zip_stat_index(z, i, 0, &st) == -1) 
			continue;
		const char *ext = rindex(st.name, '.');
		if (ext == NULL)
			continue;
		if (strncmp(ext, ".css", 4) == 0) 
			epubContentEntries << st.name;
	}
		
	zip_close(z);
	qDebug() << "CSS: " << epubContentEntries;

	return epubContentEntries;
}


QString EpubMetaDataExtractor::getCollection(const QString& epubFilename)
{
    int size;
    char *data = NULL;

    struct epub *book = openEpub(epubFilename);
    if (book == NULL) {
        qDebug() << Q_FUNC_INFO << "Cannot open: " << epubFilename;
        return "";
    }

    unsigned char **meta;
    char* info;
    meta = epub_get_metadata(book, EPUB_META, &size);
    if (!meta) {
        qDebug() << Q_FUNC_INFO << "Cannot get EPUB_META from " << epubFilename;
        epub_close (book);
        return "";
    }

    char *metadata = NULL;

    int i;
    for (i = 0; i < size; i++) {
        if (strncmp("calibre:series", (char*)meta[i], 14) == 0 && strncmp("calibre:series_index", (char*)meta[i], 20) != 0) {
            metadata = strdup((char*)meta[i] + 16);
            break;
        }
    }
    QString collection = QString::fromUtf8( metadata );
    if(!collection.isEmpty())
        qDebug() << Q_FUNC_INFO << collection;

    epub_close(book);
    return collection;
}

double EpubMetaDataExtractor::getCollectionIndex(const QString& epubFilename)
{
    int size;
    struct epub *book = openEpub(epubFilename);
    if (book == NULL) {
        qDebug() << Q_FUNC_INFO << "Cannot open: " << epubFilename;
        return 0;
    }

    unsigned char **meta;
    meta = epub_get_metadata(book, EPUB_META, &size);
    if (!meta) {
        qDebug() << Q_FUNC_INFO << "Cannot get EPUB_META from " << epubFilename;
        epub_close (book);
        return 0;
    }

    char *metadata = NULL;

    int i;
    for (i = 0; i < size; i++) {
        if (strncmp("calibre:series_index", (char*)meta[i], 20) == 0) {
            metadata = strdup((char*)meta[i] + 22);
            break;
        }
    }
    QString indexString(metadata);
    double collectionIndex = indexString.toDouble();
    qDebug() << Q_FUNC_INFO << collectionIndex;
    epub_close (book);
    return collectionIndex;
}
