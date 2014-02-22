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

#include "MobiMetaDataExtractor.h"

#include <QString>
#include <QFileInfo>
#include <QDebug>
#include <QStringList>
#include <QMutex>
#include <QMutexLocker>
#include <QImage>

#include "pdbfmt.h"

#define PALM_DOC_HEADER 16
#define FIRST_RECORD_OFFSET 78
#define FOUR_BYTE_SIZE 4
#define MOBI_HEADER_MIN_LENGTH 228
#define FIRST_IMAGE_INDEX 108
#define EXT_HEADER 12
#define FULL_NAME_OFFSET 84
#define FULL_NAME_LENGTH 88
#define RECORD_HEADER_LENGTH 8

#define AUTHOR_TYPE 100
#define PUBLISHER_TYPE 101
#define DESCRIPTION_TYPE 103
#define TITLE_TYPE 503
#define COVER_TYPE 201
#define LANG_TYPE  524

/*static*/ bool MobiMetaDataExtractor::getMetaData(const QString& filename, QString& title, QString& author, QString& publisher, QDateTime& date, QString& description, bool& isDRMBook, QString& language)
{

    LVStreamRef m_stream = LVOpenFileStream(filename.toUtf8().constData(), LVOM_READ);
    if(!m_stream.isNull())
    {

        //Get offset to the PALM-DOC header in file. The records number.
        int offset = 0;
        int recordsNum = 0;
        int dataOffset = 0;

        if(!uncompressStream(m_stream, offset, dataOffset, recordsNum))
            return false;

        //Loop for get all the records info.
        int recordOffset = dataOffset + EXT_HEADER;
        for (int i = 0; i < recordsNum; i++)
        {
            m_stream->SetPos(recordOffset);
            int type = getIntFromArray(m_stream, FOUR_BYTE_SIZE);
            switch (type) {
            case AUTHOR_TYPE:
                if(author.isEmpty())
                    author = getRecordInfo(m_stream, recordOffset);
                break;
            case PUBLISHER_TYPE:
                if(publisher.isEmpty())
                    publisher = getRecordInfo(m_stream, recordOffset);
                break;
            case DESCRIPTION_TYPE:
                if(description.isEmpty())
                    description = getRecordInfo(m_stream, recordOffset);
                break;
            case TITLE_TYPE:
                if(title.isEmpty())
                    title = getRecordInfo(m_stream, recordOffset);
                break;
            case LANG_TYPE:
                if(language.isEmpty())
                    language = getRecordInfo(m_stream, recordOffset);
                break;
            default:
                break;
            }
            m_stream->SetPos(recordOffset + FOUR_BYTE_SIZE);
            recordOffset += getIntFromArray(m_stream, FOUR_BYTE_SIZE);
        }

        //If not found a title type record use the full name for the book title.
        if(title.isEmpty())
        {
            m_stream->SetPos(offset+FULL_NAME_OFFSET);
            int nameOffset = getIntFromArray(m_stream, FOUR_BYTE_SIZE);
            m_stream->SetPos(offset+FULL_NAME_LENGTH);
            int namelength = getIntFromArray(m_stream, FOUR_BYTE_SIZE);
            m_stream->SetPos(offset+nameOffset);
            LVArray<char> fullName( namelength+1, 0 );
            if ( m_stream->Read( fullName.get(), namelength, NULL )==LVERR_OK )
            {
                char* data (fullName.get());
                QString fullNameTitle(data);
                title = fullNameTitle;
                qDebug() << title;
            }
        }
    }
    else
        return false;

    return true;
}

bool MobiMetaDataExtractor::extractCover( const QString& filename,const QString& coverPath)
{
    LVStreamRef m_stream = LVOpenFileStream(filename.toUtf8().constData(), LVOM_READ);
    if(!m_stream.isNull())
    {

        //Get offset to the PALM-DOC header in file, and the records number.
        int offset = 0;
        int recordsNum = 0;
        int dataOffset = 0;

        if(!uncompressStream(m_stream, offset, dataOffset, recordsNum))
            return false;

        //Get the Images record number.
        m_stream->SetPos(offset+FIRST_IMAGE_INDEX);
        int imageNumberRecord = getIntFromArray(m_stream, FOUR_BYTE_SIZE);

        //Loop for get all the records info.
        int recordOffset = dataOffset + EXT_HEADER;
        int imageOffset;
        for (int i = 0; i < recordsNum; i++)
        {
            m_stream->SetPos(recordOffset);
            int type = getIntFromArray(m_stream, FOUR_BYTE_SIZE);
            if(type == COVER_TYPE)
            {
                m_stream->SetPos(recordOffset + FOUR_BYTE_SIZE);
                lvsize_t recordLength = getIntFromArray(m_stream, FOUR_BYTE_SIZE) - RECORD_HEADER_LENGTH;
                m_stream->SetPos(recordOffset+FOUR_BYTE_SIZE+FOUR_BYTE_SIZE);
                imageOffset = getIntFromArray(m_stream, recordLength);
                break;
            }
            m_stream->SetPos(recordOffset + FOUR_BYTE_SIZE);
            recordOffset += getIntFromArray(m_stream, FOUR_BYTE_SIZE);
        }


        //Get the cover image record offset.
        int imageRecordOffset = getRecordOffset(m_stream, imageNumberRecord + imageOffset);

        //Get the next record offset.
        int nextRecordOffset = getRecordOffset(m_stream, imageNumberRecord + imageOffset + 1);

        int length = nextRecordOffset - imageRecordOffset;

        //Get array data for the cover image.
        m_stream->SetPos(imageRecordOffset);
        LVArray<u_char> coverImageInfo( length+1, 0 );
        if ( m_stream->Read( coverImageInfo.get(), length, NULL )==LVERR_OK )
        {
            QByteArray imageData = QByteArray::fromRawData((char*)(coverImageInfo.get()), coverImageInfo.size());
            QImage image = QImage::fromData(imageData);
            if(image.isNull())
                return false;
            if (image.save(coverPath,"jpg")) return true;
        }
    }
    else
        return false;
    return true;
}

int MobiMetaDataExtractor::getRecordOffset(LVStreamRef stream, int recordNumber)
{
    int recordPos = FIRST_RECORD_OFFSET + recordNumber*RECORD_HEADER_LENGTH;
    stream->SetPos(recordPos);
    int offset = getIntFromArray(stream, FOUR_BYTE_SIZE);
    return offset;
}

QString MobiMetaDataExtractor::getRecordInfo(LVStreamRef stream, int offset)
{
    stream->SetPos(offset+FOUR_BYTE_SIZE);
    lvsize_t recordLength = getIntFromArray(stream, FOUR_BYTE_SIZE) - RECORD_HEADER_LENGTH;
    stream->SetPos(offset+FOUR_BYTE_SIZE+FOUR_BYTE_SIZE);
    LVArray<char> recordInfo( recordLength+1, 0 );
    if ( stream->Read( recordInfo.get(), recordLength, NULL )==LVERR_OK )
    {
        QString info = QString::fromUtf8( lString8( recordInfo.get()).c_str(), lString8( recordInfo.get()).length() );
        info = info.replace("_", " ");
        return info;
    }
    return "";
}

int MobiMetaDataExtractor::getIntFromArray(LVStreamRef stream, int length)
{
    LVArray<lUInt8> array( length+1, 0 );
    if ( stream->Read( array.get(), length, NULL )==LVERR_OK )
        return array.get(3) + (array.get(2) << 8) + (array.get(1) << 16) + (array.get(0) << 24);
    else
        return 0;
}

bool MobiMetaDataExtractor::uncompressStream(LVStreamRef stream, int &offset, int &dataOffset, int &recordsNum)
{
    //Get offset to the PALM-DOC header in file.
    offset = getRecordOffset(stream, 0);

    //Get mobi header length.
    stream->SetPos(offset + PALM_DOC_HEADER + FOUR_BYTE_SIZE);
    int mobiHeaderLength = getIntFromArray(stream, FOUR_BYTE_SIZE);

    //Check if the mobi header has the correct length.
    if(mobiHeaderLength < MOBI_HEADER_MIN_LENGTH)
        return false;

    dataOffset = offset + PALM_DOC_HEADER + mobiHeaderLength;
    stream->SetPos(dataOffset);
    LVArray<char> EXTH( FOUR_BYTE_SIZE+1, 0 );
    if ( stream->Read( EXTH.get(), FOUR_BYTE_SIZE, NULL )==LVERR_OK )
    {
        char* data (EXTH.get());
        QString exth(data);
        //Return if the header not start with "EXTH".
        if(exth != "EXTH")
            return false;
    }

    //Get the EXTH Header length.
    stream->SetPos(dataOffset+FOUR_BYTE_SIZE);
    int exthHeaderlength = getIntFromArray(stream, FOUR_BYTE_SIZE);

    //Get the records behind the EXTH Header.
    stream->SetPos(dataOffset+FOUR_BYTE_SIZE+FOUR_BYTE_SIZE);
    recordsNum = getIntFromArray(stream, FOUR_BYTE_SIZE);
    return true;
}
