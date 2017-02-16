#include "dbthread.h"

using namespace std;


dbThread::dbThread(){

    db.setHostName(clientAddress);
    db.setDatabaseName(dbName);
    db.setUserName(dbUser);
    db.setPassword(dbPass);
}

dbThread::~dbThread(){}

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

    if (cmdSummaryReport) {
        summaryReport();
        cmdSummaryReport = false;
    }

    if (cmdInsertToSummaryTable) {
        insertToSummaryTable();
        cmdInsertToSummaryTable = false;
    }

    if (cmdGraphData) {
        getGraphData();
        cmdGraphData = false;
    }

    if (cmdTempData) {
        getTemperature();
        cmdTempData = false;
    }

}

void dbThread::stop(){}

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

                stateList.clear();
                qry.last();
                QString queryEndTime = qry.value(2).toString();
                delimiterEncountered = false;
                do {
                    timeList.append(qry.value(2).toString());

                    int count = 0;
                    for( int c=3; c<colNum; c++ )
                        if ( qry.value(c).toString() == "1") count++;
                    statTotalActiveZones[count]++;
                    stateList.append(count);

                    if (verbose){
                        temp = "";
                        for( int c=0; c<colNum; c++ )
                            temp += qry.value(c).toString() + " ";
                        qDebug() << temp << count;
                    }
                } while( qry.previous() && qry.value(3).toString() != "*");


                // is there a delimiter
                if ( qry.size() != stateList.count()) {
                    beginTimeDelimiter = qry.value(2).toString();
                    qry.next();
                    queryBeginTime = qry.value(2).toString();
                    delimiterEncountered = true;
                } else {
                    qry.first();
                    queryBeginTime = qry.value(2).toString();
                }
                //---

                QTime last, first;
                qint64 diff = 0;
                zeroStateLowTime = 0;
                zeroStateLowCount = 0;
                zeroStateHighTime = 0;
                zeroStateHighCount = 0;
                thresholdIndexList.clear();

                //calc for between [query end time - end time]
                last = QTime::fromString(endTime, "hh:mm:ss");
                first = QTime::fromString(queryEndTime, "hh:mm:ss");
                diff = first.msecsTo(last) / 1000;

                statTotalActiveZonesDurations [ stateList.at(0) ] += diff;

                if (stateList.at(0)==0){
                    if (diff < zeroStateLowThreshold){
                        zeroStateLowTime += diff;
                        zeroStateLowCount++;
                    } else {
                        zeroStateHighTime += diff;
                        zeroStateHighCount++;
                        thresholdIndexList.append(0);
                    }
                }
                //---

                for (int i=0; i<timeList.count()-1; i++) {

                    last = QTime::fromString(timeList.at(i), "hh:mm:ss");
                    first = QTime::fromString(timeList.at(i+1), "hh:mm:ss");
                    diff = first.msecsTo(last) / 1000;
                    statTotalActiveZonesDurations[ stateList.at(i+1) ] += diff;
                    if (stateList.at(i+1)==0){
                        if (diff < zeroStateLowThreshold){
                            zeroStateLowTime += diff;
                            zeroStateLowCount++;
                        } else {
                            zeroStateHighTime += diff;
                            zeroStateHighCount++;
                            thresholdIndexList.append(i+1);
                        }
                    }

                    if (verbose){ qDebug() << last.toString() << " - " << first.toString() << " is " << diff <<"."<<stateList.at(i+1); }
                }

                totalTime = 0;
                for (int i=0; i<zoneNumber+1; i++)
                    totalTime += statTotalActiveZonesDurations[i];

                for (int i=0; i<zoneNumber+1; i++)
                    statTotalActiveZonesPercent[i] = statTotalActiveZonesDurations[i] * 100.0 / totalTime;

                if (verbose){
                    for (int i=0; i<zoneNumber+1; i++)
                        qDebug() << "Active zone count" << i << ": " << statTotalActiveZones[i] << "  Duration: " << statTotalActiveZonesDurations[i];
                }

                summaryData.date = endDate;
                summaryData.zone0_rate = statTotalActiveZonesPercent[0];
                summaryData.zone1_rate = statTotalActiveZonesPercent[1];
                summaryData.zone2_rate = statTotalActiveZonesPercent[2];
                summaryData.zone3_rate = statTotalActiveZonesPercent[3];
                summaryData.zone4_rate = statTotalActiveZonesPercent[4];
                summaryData.zone5_rate = statTotalActiveZonesPercent[5];
                summaryData.zone6_rate = statTotalActiveZonesPercent[6];
                summaryData.zone7_rate = statTotalActiveZonesPercent[7];
                summaryData.zone0_thr_count = zeroStateHighCount;
                summaryData.zone0_thr_time = QDateTime::fromTime_t( zeroStateHighTime ).toUTC().toString("hh:mm:ss");

                if (cmdAnalyzeAllZones)
                    emit allZonesProcessed();

                if (cmdSummaryReport)
                    emit summaryReportProcess();

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

                if (verbose) qDebug() << tableNames[currentZone];

                QString temp = "";
                if (verbose){
                    for( int c=0; c<colNum; c++ )
                        temp += record.fieldName(c) + " ";
                    qDebug() << temp;
                }

                QStringList timeList;

                stateList.clear();
                qry.last();
                QString queryEndTime = qry.value(2).toString();
                delimiterEncountered = false;
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

                QString queryBeginTime;

                if ( qry.size() != stateList.count()) {
                    beginTimeDelimiter = qry.value(2).toString();
                    qry.next();
                    queryBeginTime = qry.value(2).toString();
                    delimiterEncountered = true;
                } else {
                    qry.first();
                    queryBeginTime = qry.value(2).toString();
                }


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
                    //if (verbose){ qDebug() << last.toString() << " - " << first.toString() << " is " << diff; }
                }

                last = QTime::fromString(endTime, "hh:mm:ss");
                first = QTime::fromString(queryEndTime, "hh:mm:ss");
                diff = first.msecsTo(last) / 1000;

                if (stateList.first()==1) {
                    ONtime += diff;
                    ONcount++;
                } else {
                    OFFtime += diff;
                    OFFcount++;
                }

                last = QTime::fromString(queryBeginTime, "hh:mm:ss");

                if (!delimiterEncountered){
                    first = QTime::fromString(beginTime, "hh:mm:ss");
                    diff = first.msecsTo(last) / 1000;
                    if (stateList.last()==0) {
                        ONtime += diff;
                    } else {
                        OFFtime += diff;
                    }
                } else {
                    first = QTime::fromString(beginTimeDelimiter, "hh:mm:ss");
                    diff = first.msecsTo(last) / 1000;
                    if (stateList.last()==1) {
                        ONtime += diff;
                    } else {
                        OFFtime += diff;
                    }
                }


                if (verbose){
                    qDebug() << "ON times:";
                    for (int i=0; i<timeDiffListOn.count(); i++)
                        qDebug() << timeDiffListOn.at(i);
                    qDebug() << "OFF times:";
                    for (int i=0; i<timeDiffListOff.count(); i++)
                        qDebug() << timeDiffListOff.at(i);
                }

                int total = ONtime + OFFtime;
                float rate = 0;
                if (total != 0)
                    rate = 100.0 * ONtime / total;

                switch (currentZone) {
                    case 1: summaryData.on_rate_oto = rate; break;
                    case 2: summaryData.on_rate_sln = rate; break;
                    case 3: summaryData.on_rate_blk = rate; break;
                    case 4: summaryData.on_rate_mut = rate; break;
                    case 5: summaryData.on_rate_eyo = rate; break;
                    case 6: summaryData.on_rate_cyo = rate; break;
                    case 7: summaryData.on_rate_yod = rate; break;
                    default:
                        break;
                }

            } else {

                switch (currentZone) {
                    case 1: summaryData.on_rate_oto = 0; break;
                    case 2: summaryData.on_rate_sln = 0; break;
                    case 3: summaryData.on_rate_blk = 0; break;
                    case 4: summaryData.on_rate_mut = 0; break;
                    case 5: summaryData.on_rate_eyo = 0; break;
                    case 6: summaryData.on_rate_cyo = 0; break;
                    case 7: summaryData.on_rate_yod = 0; break;
                    default:break;
                }

                qDebug() << "no record returned";
            }

            if (cmdAnalyzeZone)
                emit zoneProcessed();

            if (cmdSummaryReport)
                emit summaryReportProcess();
        }
    }
}

void dbThread::summaryReport(){

    currentZone = 0;
    analyzeAllZones();

    for (int i=0; i<zoneNumber; i++) {
        currentZone = i+1;
        analyzeZone();
    }

    emit summaryReportDone();
}

void dbThread::insertToSummaryTable(){

    QString qryStr = QString( "INSERT INTO summary (date, "
                              "0_zone_rate, 1_zone_rate, 2_zone_rate, 3_zone_rate, 4_zone_rate, 5_zone_rate, 6_zone_rate, 7_zone_rate, "
                              "0_zone_thr_count, 0_zone_thr_time, "
                              "on_rate_oto, on_rate_sln, on_rate_blk, on_rate_mut, on_rate_eyo, on_rate_cyo, on_rate_yod) VALUES ('%1', %2, %3, %4, %5, %6, %7, %8, %9, %10, '%11', %12, %13, %14, %15, %16, %17, %18)"
                              ).arg(summaryData.date)
            .arg(summaryData.zone0_rate).arg(summaryData.zone1_rate).arg(summaryData.zone2_rate).arg(summaryData.zone3_rate).arg(summaryData.zone4_rate).arg(summaryData.zone5_rate).arg(summaryData.zone6_rate).arg(summaryData.zone7_rate)
            .arg(summaryData.zone0_thr_count).arg(summaryData.zone0_thr_time)
            .arg(summaryData.on_rate_oto).arg(summaryData.on_rate_sln).arg(summaryData.on_rate_blk).arg(summaryData.on_rate_mut).arg(summaryData.on_rate_eyo).arg(summaryData.on_rate_cyo).arg(summaryData.on_rate_yod);

    //qDebug() << qryStr.toUtf8().constData() << endl;

    if (db.open()) {

        qry.prepare( qryStr );

        if( !qry.exec() ){
            qDebug() << qry.lastError().type();
            qDebug() << qry.lastError().databaseText();
        }
        else {
            qDebug( "Inserted!" );
        }
    }
}

void dbThread::analyzeZonesForGraph(){

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

                if (verbose) qDebug() << tableNames[currentZone];

                QString temp = "";
                if (verbose){
                    for( int c=0; c<colNum; c++ )
                        temp += record.fieldName(c) + " ";
                    qDebug() << temp;
                }

                QTime last, first;
                qint64 diff = 0;

                qry.last();

                int listSeq = currentZone - 1;
                graphData start;
                start.state = qry.value(3).toInt();
                start.timeDiff = 0;
                graphList[listSeq].append(start);

                if (verbose)
                    qDebug() << graphList[listSeq].at(0).state << "," << graphList[listSeq].at(0).timeDiff;

                last = QTime::fromString(endTime, "hh:mm:ss");

                delimiterEncountered = false;
                int index=0;
                do {
                    first = QTime::fromString(qry.value(2).toString(), "hh:mm:ss");
                    diff = first.secsTo(last);

                    graphData x;
                    x.state = qry.value(3).toInt();
                    x.timeDiff = diff;
                    graphList[listSeq].append(x);

                    index++;
                    if (verbose){
                        temp = "";
                        for( int c=0; c<colNum; c++ )
                            temp += qry.value(c).toString() + " ";
                        qDebug() << temp << graphList[listSeq].at(index).state << "," << graphList[listSeq].at(index).timeDiff;
                    }
                } while( qry.previous() && qry.value(3).toString() != "*");

                QString queryBeginTime;

                if ( qry.size() != (graphList[listSeq].count()-1)) {
                    beginTimeDelimiter = qry.value(2).toString();
                    qry.next();
                    queryBeginTime = qry.value(2).toString();
                    delimiterEncountered = true;
                } else {
                    qry.first();
                    queryBeginTime = qry.value(2).toString();
                }


                if (!delimiterEncountered){
                    first = QTime::fromString(beginTime, "hh:mm:ss");
                    diff = first.secsTo(last);
                    int _state = 0;
                    if (graphList[listSeq].last().state==0) {
                        _state = 1;
                    } else {
                        _state = 0;
                    }
                    graphData x;
                    x.state = _state;
                    x.timeDiff = diff;
                    graphList[listSeq].append(x);

                } else {
                    first = QTime::fromString(beginTimeDelimiter, "hh:mm:ss");
                    diff = first.secsTo(last);
                    graphData x;
                    x.state = qry.value(3).toInt();
                    x.timeDiff = diff;
                    graphList[listSeq].append(x);
                }

                if (verbose)
                    qDebug() << graphList[listSeq].last().state << "," << graphList[listSeq].last().timeDiff;

            } else {

                qDebug() << "no record returned";
            }

            //emit summaryReportProcess();
        }
    }

}

void dbThread::getGraphData(){

    for (int i=0; i<zoneNumber; i++)
        graphList[i].clear();

    for (int i=1; i<8; i++) {
        currentZone = i;
        analyzeZonesForGraph();
    }

    emit graphDataDone();
}

void dbThread::getTemperature(){

    QString qryStr = QString( "SELECT * FROM tempout ORDER BY `index` DESC LIMIT 1");
    //qDebug() << qryStr.toUtf8().constData() << endl;


    if (db.open()) {

        qry.prepare( qryStr );

        if( !qry.exec() )

            qDebug() << qry.lastError();

        else {

            QSqlRecord record;
            record = qry.record();

            if ( qry.size() > 0 ) {

                qry.last();

                tempOut = qry.value(3).toFloat();


            } else {

                qDebug() << "no record returned";
            }

            emit tempOutDone();
        }
    }

}

