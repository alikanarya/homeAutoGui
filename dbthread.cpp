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
