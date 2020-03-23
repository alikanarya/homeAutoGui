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

    if (cmdAvgTempData) {
        getAvgTemperature();
        cmdAvgTempData = false;
    }

    if (cmdAnalyzeZones) {
        analyzeZones();
        cmdAnalyzeZones = false;
    }

    if (cmdSingleZone) {
        analyzeZone();
        cmdSingleZone = false;
    }

    if (cmdNgConsumption) {
        ngConsumption();
        cmdNgConsumption = false;
    }

    if (cmdUpdateMeterValue) {
        updateMeterValue();
        cmdUpdateMeterValue = false;
    }

    verbose = false;
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
                summaryData.zone0_rate = QString::number(statTotalActiveZonesPercent[0],'f',1).toFloat();
                summaryData.zone1_rate = QString::number(statTotalActiveZonesPercent[1],'f',1).toFloat();
                summaryData.zone2_rate = QString::number(statTotalActiveZonesPercent[2],'f',1).toFloat();
                summaryData.zone3_rate = QString::number(statTotalActiveZonesPercent[3],'f',1).toFloat();
                summaryData.zone4_rate = QString::number(statTotalActiveZonesPercent[4],'f',1).toFloat();
                summaryData.zone5_rate = QString::number(statTotalActiveZonesPercent[5],'f',1).toFloat();
                summaryData.zone6_rate = QString::number(statTotalActiveZonesPercent[6],'f',1).toFloat();
                summaryData.zone7_rate = QString::number(statTotalActiveZonesPercent[7],'f',1).toFloat();
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

    QString qryStr = "";
    if (endDate == beginDate)
        qryStr = QString( "SELECT * FROM %1 WHERE (STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%2', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') <= STR_TO_DATE('%4', '%H:%i:%s')) AND"
                                                 "(STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%3', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') >= STR_TO_DATE('%5', '%H:%i:%s'))").arg(tableNames[currentZone]).arg(endDate).arg(beginDate).arg(endTime).arg(beginTime);
    else
        qryStr = QString( "SELECT * FROM %1 WHERE (STR_TO_DATE(date, '%d/%m/%y') < STR_TO_DATE('%2', '%d/%m/%y') AND STR_TO_DATE(date, '%d/%m/%y') > STR_TO_DATE('%3', '%d/%m/%y')) OR"
                                                     "(STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%2', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') <= STR_TO_DATE('%4', '%H:%i:%s')) OR"
                                                     "(STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%3', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') >= STR_TO_DATE('%5', '%H:%i:%s'))").arg(tableNames[currentZone]).arg(endDate).arg(beginDate).arg(endTime).arg(beginTime);
    //QString qryStr = QString( "SELECT * FROM %1 WHERE date <= '%2' AND date >= '%3' AND time <= '%4' AND time >= '%5'").arg(tableNames[currentZone]).arg(endDate).arg(beginDate).arg(endTime).arg(beginTime);
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
                QTime last, first;
                qint64 diff = 0;

                ONtime = 0;
                OFFtime = 0;
                ONcount = 0;
                OFFcount = 0;
                QTime zero = QTime::fromString("00:00:00", "hh:mm:ss");

                int lastSec = 0;
                int firstSec = 0;

                stateList.clear();
                qry.last();
                QDate endDateForm = QDate::fromString(endDate,"dd/MM/yy");
                QDate endDateRecord = QDate::fromString(qry.value(1).toString(),"dd/MM/yy");
                qint64 endDateDiff = endDateRecord.daysTo(endDateForm);

                QString queryEndTime = qry.value(2).toString();

                int listSeq = currentZone - 1;
                graphData start;
                start.state = qry.value(3).toInt();
                start.timeDiff = 0;
                graphList[listSeq].append(start);

                // End time calcualtions

                last = QTime::fromString(endTime, "hh:mm:ss");
                first = QTime::fromString(queryEndTime, "hh:mm:ss");

                if (endDateDiff == 0)
                    diff = first.msecsTo(last) / 1000;
                else{
                    lastSec = -1*last.secsTo(zero);
                    firstSec = -1*first.secsTo(zero);
                    diff = lastSec+86400*endDateDiff-firstSec;
                }


                if (qry.value(3).toInt()==1) {
                    ONtime += diff;
                    ONcount++;
                } else {
                    OFFtime += diff;
                    OFFcount++;
                }

                int totalDiff = 0;
                totalDiff += diff;

                graphData x;
                x.state = qry.value(3).toInt();
                x.timeDiff = totalDiff;
                graphList[listSeq].append(x);

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



                timeDiffList.clear();
                timeDiffListOn.clear();
                timeDiffListOff.clear();

                for (int i=0; i<timeList.count()-1; i++) {

                    last = QTime::fromString(timeList.at(i), "hh:mm:ss");
                    first = QTime::fromString(timeList.at(i+1), "hh:mm:ss");

                    lastSec = -1*last.secsTo(zero);
                    firstSec = -1*first.secsTo(zero);

                    if (lastSec >= firstSec)
                        diff = first.msecsTo(last) / 1000;
                    else {
                        diff = lastSec+86400-firstSec;
                    }


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
                   // if (verbose){ qDebug() << last.toString() << " - " << first.toString() << " is " << last.secsTo(zero); }

                    totalDiff += diff;
                    graphData x;
                    x.state = stateList.at(i+1);
                    x.timeDiff = totalDiff;
                    graphList[listSeq].append(x);
                }




                // Begin time calcualtions
                QDate beginDateForm = QDate::fromString(beginDate,"dd/MM/yy");
                QDate beginDateRecord = QDate::fromString(qry.value(1).toString(),"dd/MM/yy");
                qint64 beginDateDiff = beginDateForm.daysTo(beginDateRecord);


                last = QTime::fromString(queryBeginTime, "hh:mm:ss");

                if (!delimiterEncountered){
                    first = QTime::fromString(beginTime, "hh:mm:ss");

                    if (beginDateDiff == 0)
                        diff = first.msecsTo(last) / 1000;
                    else{
                        lastSec = -1*last.secsTo(zero);
                        firstSec = -1*first.secsTo(zero);
                        diff = lastSec+86400*beginDateDiff-firstSec;
                    }

                    int lastState = 0;

                    if (stateList.last()==0) {
                        ONtime += diff;
                        lastState = 1;
                    } else {
                        OFFtime += diff;
                        lastState = 0;
                    }

                    totalDiff += diff;
                    graphData x;
                    x.state = lastState;
                    x.timeDiff = totalDiff;
                    graphList[listSeq].append(x);

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
                    qDebug() << "ON times: " << ONtime;
                    for (int i=0; i<timeDiffListOn.count(); i++)
                        qDebug() << timeDiffListOn.at(i);
                    qDebug() << "OFF times:" << OFFtime;
                    for (int i=0; i<timeDiffListOff.count(); i++)
                        qDebug() << timeDiffListOff.at(i);
                }


                if (verbose){
                    for (int c=0; c<graphList[listSeq].size(); c++)
                        qDebug() << graphList[listSeq].at(c).state << "," << graphList[listSeq].at(c).timeDiff;
                }

                int total = ONtime + OFFtime;
                float rate = 0;
                if (total != 0)
                    rate = 100.0 * ONtime / total;

                switch (currentZone) {
                    case 1: summaryData.on_rate_oto = QString::number(rate,'f',1).toFloat(); break;
                    case 2: summaryData.on_rate_sln = QString::number(rate,'f',1).toFloat(); break;
                    case 3: summaryData.on_rate_blk = QString::number(rate,'f',1).toFloat(); break;
                    case 4: summaryData.on_rate_mut = QString::number(rate,'f',1).toFloat(); break;
                    case 5: summaryData.on_rate_eyo = QString::number(rate,'f',1).toFloat(); break;
                    case 6: summaryData.on_rate_cyo = QString::number(rate,'f',1).toFloat(); break;
                    case 7: summaryData.on_rate_yod = QString::number(rate,'f',1).toFloat(); break;
                    default:break;
                }

                switch (currentZone) {
                    case 1: summaryData.on_count_oto = ONcount; break;
                    case 2: summaryData.on_count_sln = ONcount; break;
                    case 3: summaryData.on_count_blk = ONcount; break;
                    case 4: summaryData.on_count_mut = ONcount; break;
                    case 5: summaryData.on_count_eyo = ONcount; break;
                    case 6: summaryData.on_count_cyo = ONcount; break;
                    case 7: summaryData.on_count_yod = ONcount; break;
                    default:break;
                }

                switch (currentZone) {
                    case 1: summaryData.on_time_oto = ONtime; break;
                    case 2: summaryData.on_time_sln = ONtime; break;
                    case 3: summaryData.on_time_blk = ONtime; break;
                    case 4: summaryData.on_time_mut = ONtime; break;
                    case 5: summaryData.on_time_eyo = ONtime; break;
                    case 6: summaryData.on_time_cyo = ONtime; break;
                    case 7: summaryData.on_time_yod = ONtime; break;
                    default:break;
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

                switch (currentZone) {
                    case 1: summaryData.on_count_oto = 0; break;
                    case 2: summaryData.on_count_sln = 0; break;
                    case 3: summaryData.on_count_blk = 0; break;
                    case 4: summaryData.on_count_mut = 0; break;
                    case 5: summaryData.on_count_eyo = 0; break;
                    case 6: summaryData.on_count_cyo = 0; break;
                    case 7: summaryData.on_count_yod = 0; break;
                    default:break;
                }

                switch (currentZone) {
                    case 1: summaryData.on_time_oto = 0; break;
                    case 2: summaryData.on_time_sln = 0; break;
                    case 3: summaryData.on_time_blk = 0; break;
                    case 4: summaryData.on_time_mut = 0; break;
                    case 5: summaryData.on_time_eyo = 0; break;
                    case 6: summaryData.on_time_cyo = 0; break;
                    case 7: summaryData.on_time_yod = 0; break;
                    default:break;
                }
                qDebug() << "table: " << tableNames[currentZone] << " > no record returned";
            }

            if (cmdAnalyzeZone)
                emit zoneProcessed();

            if (cmdSummaryReport)
                emit summaryReportProcess();

            if (cmdSingleZone)
                emit singleZoneDone();
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
                              "on_rate_oto, on_rate_sln, on_rate_blk, on_rate_mut, on_rate_eyo, on_rate_cyo, on_rate_yod, "
                              "tempAvg, tempMin, tempMax, ngMeter) VALUES ('%1', %2, %3, %4, %5, %6, %7, %8, %9, %10, '%11', %12, %13, %14, %15, %16, %17, %18, %19, %20, %21, %22)"
                              ).arg(summaryData.date)
            .arg(summaryData.zone0_rate).arg(summaryData.zone1_rate).arg(summaryData.zone2_rate).arg(summaryData.zone3_rate).arg(summaryData.zone4_rate).arg(summaryData.zone5_rate).arg(summaryData.zone6_rate).arg(summaryData.zone7_rate)
            .arg(summaryData.zone0_thr_count).arg(summaryData.zone0_thr_time)
            .arg(summaryData.on_rate_oto).arg(summaryData.on_rate_sln).arg(summaryData.on_rate_blk).arg(summaryData.on_rate_mut).arg(summaryData.on_rate_eyo).arg(summaryData.on_rate_cyo).arg(summaryData.on_rate_yod)
            .arg(summaryData.tempAvg).arg(summaryData.tempMin).arg(summaryData.tempMax).arg(summaryData.ngMeter);

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

                qDebug() << "table: " << tableNames[currentZone] << " > no record returned";
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
                tempDate = qry.value(1).toString();
                tempTime = qry.value(2).toString();
            } else {
                qDebug() << "no record returned";
            }

            emit tempOutDone();
        }
    }
}

void dbThread::getAvgTemperature(){

//    QString qryStr = QString( "SELECT * FROM tempout ORDER BY `index` DESC LIMIT 1");
//    QString qryStr = QString( "SELECT * FROM tempout WHERE (date = '%1' AND time <= '%2') OR (date = '%3' AND time > '%4') ORDER BY `index` DESC").arg(endDate).arg(endTime).arg(beginDate).arg(beginTime);

    QString qryStr = "";
    if (endDate == beginDate)
        qryStr = QString( "SELECT * FROM %1 WHERE (STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%2', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') <= STR_TO_DATE('%4', '%H:%i:%s')) AND"
                                                 "(STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%3', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') >= STR_TO_DATE('%5', '%H:%i:%s'))").arg("tempout").arg(endDate).arg(beginDate).arg(endTime).arg(beginTime);
    else
        qryStr = QString( "SELECT * FROM %1 WHERE (STR_TO_DATE(date, '%d/%m/%y') < STR_TO_DATE('%2', '%d/%m/%y') AND STR_TO_DATE(date, '%d/%m/%y') > STR_TO_DATE('%3', '%d/%m/%y')) OR"
                                                     "(STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%2', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') <= STR_TO_DATE('%4', '%H:%i:%s')) OR"
                                                     "(STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%3', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') >= STR_TO_DATE('%5', '%H:%i:%s'))").arg("tempout").arg(endDate).arg(beginDate).arg(endTime).arg(beginTime);

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
                tempAvg = 0;
                tempMax = -99;
                tempMin =99;
                int count = 0;
                float val = 0;

                qry.first();
                tempList.clear();
                //QTime last, first;
                qint64 diff = 0;
                //last = QTime::fromString(endTime, "hh:mm:ss");

                QDateTime endDT;
                endDT.setDate( QDate::fromString(endDate, "dd/MM/yy") );
                endDT.setTime( QTime::fromString(endTime, "hh:mm:ss") );

                QDateTime beginDT;

                //int timeDifference = beginDT.secsTo(endDT);

                do {

                    count++;

                    val = qry.value(3).toFloat();

                    beginDT.setDate( QDate::fromString(qry.value(1).toString(), "dd/MM/yy") );
                    beginDT.setTime( QTime::fromString(qry.value(2).toString(), "hh:mm:ss") );

                    tempAvg += val;

                    if (val > tempMax)
                        tempMax = val;

                    if (val < tempMin)
                        tempMin = val;

                    //first = QTime::fromString(qry.value(2).toString(), "hh:mm:ss");
                    diff = beginDT.secsTo(endDT);

                    tempData x;
                    x.value = val;
                    x.timeDiff = diff;

                    tempList.append(x);

                    if (verbose){
                        temp = "";
                        for( int c=0; c<colNum; c++ )
                            temp += qry.value(c).toString() + " ";
                        temp += QString::number(diff);
                        qDebug() << temp ;
                    }

                } while( qry.next() && qry.value(3).toString() != "-99");

                if (count!=0) {
                    tempAvg /= count;
                    summaryData.tempAvg = QString::number(tempAvg,'f',1).toFloat();
                    summaryData.tempMin = QString::number(tempMin,'f',1).toFloat();
                    summaryData.tempMax = QString::number(tempMax,'f',1).toFloat();
                    //qDebug() << "min: " << summaryData.tempMin << " avg: " << summaryData.tempAvg << " max: " << summaryData.tempMax;
                } else {
                    summaryData.tempAvg = summaryData.tempMin = summaryData.tempMax = 0;
                }

                if (verbose) {
                    qDebug() << "min: " << tempMin << " avg: " << tempAvg << " max: " << tempMax;
                }
            } else {
                qDebug() << "no record returned";
            }

            emit tempOutAvgDone();
        }
    }
}

void dbThread::analyzeZones(){

    for (int i=0; i<zoneNumber; i++)
        graphList[i].clear();

    for (int i=1; i<8; i++) {
        currentZone = i;
        analyzeZone();
    }

    emit analyzeZonesDone();
}

void dbThread::ngConsumption()
{
    QDateTime refDT;
    refDT.setDate( QDate::fromString(endDate, "dd/MM/yy") );
    refDT.setTime( QTime::fromString("23:59:59") );
    QDateTime end_DT;
    end_DT.setDate( QDate::fromString(endDate, "dd/MM/yy") );
    end_DT.setTime( QTime::fromString(endTime, "hh:mm:ss") );

    if ( end_DT.secsTo(refDT) == 0 ) {
        end_DT = end_DT.addDays(1);
        end_DT.setTime( QTime::fromString("00:00:02") );
    }
    QString _endDate = end_DT.toString("dd/MM/yy");
    QString _endTime = end_DT.toString("hh:mm:ss");
    //qDebug() << _endDate << " " << _endTime;


    QString qryStr = "";
    if (_endDate == beginDate)
        qryStr = QString( "SELECT * FROM %1 WHERE (STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%2', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') <= STR_TO_DATE('%4', '%H:%i:%s')) AND"
                                                 "(STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%3', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') >= STR_TO_DATE('%5', '%H:%i:%s'))").arg("gas_reading").arg(_endDate).arg(beginDate).arg(_endTime).arg(beginTime);
    else
        qryStr = QString( "SELECT * FROM %1 WHERE (STR_TO_DATE(date, '%d/%m/%y') < STR_TO_DATE('%2', '%d/%m/%y') AND STR_TO_DATE(date, '%d/%m/%y') > STR_TO_DATE('%3', '%d/%m/%y')) OR"
                                                     "(STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%2', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') <= STR_TO_DATE('%4', '%H:%i:%s')) OR"
                                                     "(STR_TO_DATE(date, '%d/%m/%y') = STR_TO_DATE('%3', '%d/%m/%y') AND STR_TO_DATE(time, '%H:%i:%s') >= STR_TO_DATE('%5', '%H:%i:%s'))").arg("gas_reading").arg(_endDate).arg(beginDate).arg(_endTime).arg(beginTime);

    //qDebug() << qryStr.toUtf8().constData() << endl;

    if (db.open()) {

        qry.prepare( qryStr );

        if( !qry.exec() )
            qDebug() << qry.lastError();
        else {

            QSqlRecord record;
            record = qry.record();
            float consumption = 0;

            if ( qry.size() > 0 ) {

                int colNum = record.count();

                QString temp = "";
                int count = 0;
                float val = 0;

                qry.first();
                ngMeterList.clear();
                ngMeterTableList.clear();

                //QTime last, first;
                qint64 diff = 0;
                //last = QTime::fromString(endTime, "hh:mm:ss");

                QDateTime endDT;
                endDT.setDate( QDate::fromString(_endDate, "dd/MM/yy") );
                endDT.setTime( QTime::fromString(_endTime, "hh:mm:ss") );

                QDateTime beginDT;

                //int timeDifference = beginDT.secsTo(endDT);

                do {

                    count++;

                    val = qry.value(3).toFloat();

                    beginDT.setDate( QDate::fromString(qry.value(1).toString(), "dd/MM/yy") );
                    beginDT.setTime( QTime::fromString(qry.value(2).toString(), "hh:mm:ss") );

                    //first = QTime::fromString(qry.value(2).toString(), "hh:mm:ss");
                    diff = beginDT.secsTo(endDT);

                    tempData x;
                    x.value = val;
                    x.timeDiff = diff;
                    ngMeterList.append(x);

                    meterData d;
                    d.index = qry.value(0).toInt();
                    d.date = qry.value(1).toString();
                    d.time = qry.value(2).toString();
                    d.value = val;
                    d.note = qry.value(4).toString();
                    ngMeterTableList.append(d);

                    if (verbose){
                        temp = "";
                        for( int c=0; c<colNum; c++ )
                            temp += qry.value(c).toString() + " ";
                        temp += QString::number(diff);
                        qDebug() << temp ;
                    }

                } while( qry.next() && qry.value(3).toString() != "-99");
                consumption = ngMeterList.last().value - ngMeterList.first().value;

                summaryData.ngMeter = QString::number(consumption,'f',1).toFloat();

                /*if (verbose) {
                    qDebug() << "min: " << tempMin << " avg: " << tempAvg << " max: " << tempMax;
                }*/
            } else {
                qDebug() << "no record returned";
            }

            emit ngConsumptionDone(consumption);
            emit ngConsumptionGraph();
        }
    }
}

void dbThread::updateMeterValue()
{
    QString qryStr = "";

    for (int i=0; i<ngMeterUpdateList.size(); i++) {
        qryStr = QString( "UPDATE `%1` SET `value` = %2, `note` = 'fixed'  WHERE `index` = %3" ).arg("gas_reading").arg(ngMeterUpdateList.at(i).value).arg(ngMeterUpdateList.at(i).index);

        //qDebug() << qryStr.toUtf8().constData() << endl;

        if (db.open()) {

            qry.prepare( qryStr );

            if( !qry.exec() ){
                qDebug() << qry.lastError().type();
                qDebug() << qry.lastError().databaseText();
            }
            else {
                //qDebug( "Updated!" );
            }
        }
    }
}

void dbThread::appendMeterUpdateList(int index, float value)
{
    meterData x;
    x.index = index;
    x.value = value;
    ngMeterUpdateList.append(x);
}
