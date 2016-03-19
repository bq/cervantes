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

#ifndef VIEWERSTEPSMANAGER_H
#define VIEWERSTEPSMANAGER_H

#include <QString>
#include <QDateTime>

class ViewerStepsManager{

    struct Range{
        double start;
        double end;
        qint64 initialTimestamp;
        qint64 finalTimestamp;
    };

public:

    ViewerStepsManager();
    virtual ~ViewerStepsManager();

    double                              timePerPercentage;
    QList<Range>                        ranges;

    void                                init                                    ( double _timePerView );
    void                                stepRead                                ( double _initialRendererPos , double _finalRendererPos, qint64 initialTimestamp, qint64 finalTimestamp );
    QString                             getStringFromRanges                     ();
    void                                cleanRanges                             ();
    double                              getNeededMsecsToBeRead                  (double _initialRendererPos ,double _finalRendererPos);

private:
    bool                                alreadyExists                           ( Range );


};


#endif // VIEWERSTEPSMANAGER_H
