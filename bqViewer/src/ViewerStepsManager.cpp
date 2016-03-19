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


#include "ViewerStepsManager.h"
#include <QStringList>
#include <QDebug>
#include <QtCore/qmath.h>

#define MAX_MSECS_FOR_STEP 300 * 1000 //5 minutes
ViewerStepsManager::ViewerStepsManager()
    : timePerPercentage(0)
{
}

ViewerStepsManager::~ViewerStepsManager()
{
}

void ViewerStepsManager::init(double _totalReadingTime)
{
    qDebug() << Q_FUNC_INFO << "_totalReadingTime: " << _totalReadingTime;
    timePerPercentage = (double)(_totalReadingTime / 100);
}

void ViewerStepsManager::stepRead(double _initialRendererPos ,double _finalRendererPos, qint64 initialTimestamp, qint64 finalTimestamp)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << Q_FUNC_INFO << "initialTimestamp: " << initialTimestamp;
    qDebug() << Q_FUNC_INFO << "finalTimestamp: " << finalTimestamp;
    qDebug() << Q_FUNC_INFO << "diferencia: " << fabs(initialTimestamp - finalTimestamp);

    if(_initialRendererPos <= 0 || _finalRendererPos <= 0 || initialTimestamp <= 0 || finalTimestamp <= 0 || timePerPercentage <= 0)
        return;

    Range range;
    range.start = _initialRendererPos;
    range.end = _finalRendererPos;
    range.initialTimestamp = initialTimestamp;
    range.finalTimestamp = finalTimestamp;

    if(!alreadyExists(range))
        ranges.append(range);
}

double ViewerStepsManager::getNeededMsecsToBeRead(double _initialRendererPos ,double _finalRendererPos)
{
    qDebug() << Q_FUNC_INFO;
    double mSecsNeeded = (_finalRendererPos - _initialRendererPos) * 100 * timePerPercentage;
    qDebug() << Q_FUNC_INFO << "_finalRendererPos: " << _finalRendererPos;
    qDebug() << Q_FUNC_INFO << "_initialRendererPos: " << _initialRendererPos;
    qDebug() << Q_FUNC_INFO << "range: " << (_finalRendererPos - _initialRendererPos);
    qDebug() << Q_FUNC_INFO << "timePerPercentage: " << timePerPercentage;
    qDebug() << Q_FUNC_INFO << "mSecsNeeded: " << mSecsNeeded;
    return mSecsNeeded;
}

QString ViewerStepsManager::getStringFromRanges()
{
    qDebug() << Q_FUNC_INFO;
    QList<Range>::iterator it = ranges.begin();
    QList<Range>::iterator itEnd = ranges.end();
    QString stringToReturn = "";
    double lastRangeEnd = 0;
    qint64 finalTimestamp = 0;
    qint64 initialBeforeTimestamp = 0;

    while (it != itEnd)
    {
        if(stringToReturn.isEmpty())
        {
            stringToReturn += QString(QString::number((*it).initialTimestamp) + ":" + QString::number((*it).start) + "..");
            finalTimestamp = (*it).finalTimestamp;
            lastRangeEnd = (*it).end;
            initialBeforeTimestamp = (*it).initialTimestamp;
        }
        else
        {
            qDebug() << Q_FUNC_INFO << "initialBeforeTimestamp: " << initialBeforeTimestamp;
            qDebug() << Q_FUNC_INFO << "(*it).initialTimestamp): " << (*it).initialTimestamp;
            qDebug() << Q_FUNC_INFO << "diferencia: " << fabs(initialBeforeTimestamp - (*it).initialTimestamp);
            if(fabs((*it).start - lastRangeEnd) > 0.0001 || (initialBeforeTimestamp > 0 && fabs(initialBeforeTimestamp - (*it).initialTimestamp) > MAX_MSECS_FOR_STEP))
            {
                stringToReturn += QString(QString::number(finalTimestamp) + ":");
                stringToReturn += QString(QString::number(lastRangeEnd) + "," );
                stringToReturn += QString(QString::number((*it).initialTimestamp) + ":");
                stringToReturn += QString(QString::number((*it).start) + "..");
                finalTimestamp = (*it).finalTimestamp;
                lastRangeEnd = (*it).end;
            }

            initialBeforeTimestamp = (*it).initialTimestamp;
            finalTimestamp = (*it).finalTimestamp;
            lastRangeEnd = (*it).end;
        }
        it++;
    }

    if(lastRangeEnd > 0)
        stringToReturn += QString::number(finalTimestamp) + ":" + QString::number(lastRangeEnd);

    qDebug() << Q_FUNC_INFO << "stringToReturn: " << stringToReturn;
    cleanRanges();

    return stringToReturn;
}

void ViewerStepsManager::cleanRanges()
{
    ranges.clear();
}

bool ViewerStepsManager::alreadyExists(Range range)
{
    QList<Range>::iterator it = ranges.begin();
    QList<Range>::iterator itEnd = ranges.end();
    while (it != itEnd)
    {
        if( ( (*it).start == range.start ) && ( (*it).initialTimestamp == range.initialTimestamp) )
            return true;

        it++;
    }
    return false;
}
