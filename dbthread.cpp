#include "dbthread.h"

using namespace std;


dbThread::dbThread(){

    db.setHostName(clientAddress);
    db.setDatabaseName(dbName);
    db.setUserName(dbUser);
    db.setPassword(dbPass);
}

dbThread::~dbThread(){

}

void dbThread::run(){

    if (cmdConnect){
        connectToDB();
        cmdConnect = false;
    }

    if (cmdAnalyzeAllZones) {
        analyzeAllZones();
        cmdAnalyzeAllZones = false;
    }

    if (cmdAnalyzeZone) {
        analyzeZone();
        cmdAnalyzeZone = false;
    }

}

void dbThread::stop(){

}

void dbThread::connectToDB(){

    if (!db.open()) {
        qDebug() <<  db.lastError().text() << db.lastError().number();
        emit unconnected();
    } else {
        emit connected();
    }
}

void dbThread::analyzeAllZones(){

    QString qryStr = QString( "SELECT * FROM zones WHERE date <= '%1' AND date >= '%2' AND time <= '%3' AND time >= '%4'").arg(endDate).arg(beginDate).arg(endTime).arg(beginTime);
    //qDebug() << qryStr.toUtf8().constData() << endl;

    if (db.open()) {

        qry.prepare( qryStr );

        if( !qry.exec() )

            qDebug() << qry.lastError();

        else {

            QSqlRecord allZonesRecord;
            //allZonesRecord.clear();
            allZonesRecord = qry.record();

            if ( qry.size() > 0 ) {

                int colNum = allZonesRecord.count();

                for (int i=0; i<zoneNumber+1; i++){
                    statTotalActiveZones[i]=0;
                    statTotalActiveZonesDurations[i]=0;
                }

                QString temp = "";
                if (verbose){
                    for( int c=0; c<colNum; c++ )
                        temp += allZonesRecord.fieldName(c) + " ";
                    qDebug() << temp;
                }

                QStringList timeList;

                qry.last();
                do {
                    timeList.append(qry.value(2).toString());

                    int count = 0;
                    for( int c=3; c<colNum; c++ )
                        if ( qry.value(c).toString() == "1") count++;
                    statTotalActiveZones[count]++;
                    totalActiveZoneList.append(count);

                    if (verbose){
                        temp = "";
                        for( int c=0; c<colNum; c++ )
                            temp += qry.value(c).toString() + " ";
                        qDebug() << temp << count;
                    }
                } while( qry.previous() && qry.value(3).toString() != "*");

                QTime last, first;
                qint64 diff = 0;

                for (int i=0; i<timeList.count()-1; i++) {

                    last = QTime::fromString(timeList.at(i), "hh:mm:ss");
                    first = QTime::fromString(timeList.at(i+1), "hh:mm:ss");
                    diff = first.msecsTo(last) / 1000;
                    statTotalActiveZonesDurations[ totalActiveZoneList.at(i+1)] += diff;
                    //if (verbose){ qDebug() << last.toString() << " - " << first.toString() << " is " << diff; }
                }

                if (verbose){
                    for (int i=0; i<zoneNumber+1; i++)
                        qDebug() << "Active zone count" << i << ": " << statTotalActiveZones[i] << "  Duration: " << statTotalActiveZonesDurations[i];
                }

                emit allZonesProcessed();

            } else
                qDebug() << "no record returned";
        }
    }
}

void dbThread::analyzeZone(){

    QString qryStr = QString( "SELECT * FROM %1 WHERE date <= '%2' AND date >= '%3' AND time <= '%4' AND time >= '%5'").arg(tableNames[currentZone]).arg(endDate).arg(beginDate).arg(endTime).arg(beginTime);
    //qDebug() << qryStr.toUtf8().constData() << endl;

    if (db.open()) {

        qry.prepare( qryStr );

        if( !qry.exec() )

            qDebug() << qry.lastError();

        else {

            QSqlRecord record;
            record = qry.record();

            if ( qry.size() > 0 ) {

                int colNum = record.count();

                QString temp = "";
                if (verbose){
                    for( int c=0; c<colNum; c++ )
                        temp += record.fieldName(c) + " ";
                    qDebug() << temp;
                }

                QStringList timeList;

                qry.last();
                do {
                    timeList.append(qry.value(2).toString());
                    stateList.append(qry.value(3).toInt());

                    if (verbose){
                        temp = "";
                        for( int c=0; c<colNum; c++ )
                            temp += qry.value(c).toString() + " ";
                        qDebug() << temp;
                    }
                } while( qry.previous() && qry.value(3).toString() != "*");


                QTime last, first;
                qint64 diff = 0;

                timeDiffList.clear();
                timeDiffListOn.clear();
                timeDiffListOff.clear();
                ONtime = 0;
                OFFtime = 0;
                ONcount = 0;
                OFFcount = 0;
                for (int i=0; i<timeList.count()-1; i++) {

                    last = QTime::fromString(timeList.at(i), "hh:mm:ss");
                    first = QTime::fromString(timeList.at(i+1), "hh:mm:ss");
                    diff = first.msecsTo(last) / 1000;
                    timeDiffList.append(diff);
                    if (stateList.at(i+1)==1){
                        timeDiffListOn.append(diff);
                        ONtime += diff;
                        ONcount++;
                    } else {
                        timeDiffListOff.append(diff);
                        OFFtime += diff;
                        OFFcount++;
                    }
                    if (verbose){ qDebug() << last.toString() << " - " << first.toString() << " is " << diff; }
                }

                if (verbose){
                    qDebug() << "ON times:";
                    for (int i=0; i<timeDiffListOn.count(); i++)
                        qDebug() << timeDiffListOn.at(i);
                    qDebug() << "OFF times:";
                    for (int i=0; i<timeDiffListOff.count(); i++)
                        qDebug() << timeDiffListOff.at(i);
                }

                emit zoneProcessed();

            } else
                qDebug() << "no record returned";
        }
    }
}
