#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "checkclient.h"
#include "dbthread.h"
#include <iostream>

using namespace std;

Server *serverx;
Client *clientx;
checkClient *checkClientX;
dbThread *dbThreadX;

int currentZone=0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){

    ui->setupUi(this);

    ui->dateEdit_BEGIN->setDate(QDate::currentDate());
    ui->dateEdit_END->setDate(QDate::currentDate());
    ui->timeEdit_END->setTime(QTime::currentTime());
    ui->thresholdEdit->setText("300");

    ui->tableAllZones->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableZone->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    fontBold.setBold(true);

    ui->tableAllZones->setRowCount(15);
    ui->tableAllZones->verticalHeader()->setDefaultSectionSize(20);
    for (int i=0; i<15; i++)
        for (int c=0; c<4; c++){
            ui->tableAllZones->setItem(i, c, new QTableWidgetItem( " " ) );
            ui->tableAllZones->item(i, c)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }

    for (int i=0; i<7; i++)
        for (int c=0; c<5; c++){
            ui->tableZone->setItem(i, c, new QTableWidgetItem( " " ) );
            ui->tableZone->item(i, c)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }

    serverx = new Server();
    clientx = new Client();
    checkClientX = new checkClient();

    settings = new QSettings(INIFILENAME, QSettings::IniFormat);
    readSettings();
    clientx->setHost(clientAddress, clientPort);

    for (int i = 0; i < dInpSize; i++) dInpArr[i] = '0';
    dInpArr[dInpSize] = '\0';

    for (int i = 0; i < dOutSize; i++) dOutArr[i] = '1';
    dOutArr[dInpSize] = '\0';

    for (int i = 0; i < dOutSize; i++) dOutReadArr[i] = '0';
    dOutReadArr[dInpSize] = '\0';

    for (int i = 0; i < aInpSize; i++) aInpArr[i] = 0;
    for (int i = 0; i < aInpSize; i++) aInpArrVal[i] = 0.0;

    for (int i = 0; i < aOutSize; i++) aOutArr[i] = 0;

    dbThreadX = new dbThread();

    // 1sec timer
    QTimer *timerSec = new QTimer();
    QObject::connect(timerSec, SIGNAL(timeout()), checkClientX, SLOT(connect()));
    timerSec->start(1000);

    connect(serverx, SIGNAL(readFinished()),this, SLOT(displayInputs()));
    connect(checkClientX, SIGNAL(Connected()),this, SLOT(ConnectedToServer()));
    connect(checkClientX, SIGNAL(notConnected()),this, SLOT(NotConnectedToServer()));
    connect(this, SIGNAL(sendData()),this, SLOT(transferData()));
    connect(dbThreadX, SIGNAL(connected()), this, SLOT(connectedToDB()));
    connect(dbThreadX, SIGNAL(unconnected()), this, SLOT(unconnectedToDB()));

    progress = new ThreadProgressDialog (this);
    progress->setWindowTitle("DB Bağlantısı Bekleniyor");
    connect(dbThreadX, SIGNAL(started()), progress, SLOT(threadStarted()));
    connect(dbThreadX, SIGNAL(finished()), progress, SLOT(threadFinished()));

    connect(dbThreadX, SIGNAL(allZonesProcessed()), this, SLOT(allZonesTable()));
    connect(dbThreadX, SIGNAL(zoneProcessed()), this, SLOT(zoneTable()));
    connect(dbThreadX, SIGNAL(summaryReportDone()), this, SLOT(summaryResult()));
    connect(dbThreadX, SIGNAL(graphDataDone()), this, SLOT(drawGraph()));

    dbThreadX->cmdConnect = true;
    dbThreadX->start();

    // init deviation trend
    scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);
    penAxis.setColor(Qt::red);
    penAxis.setWidth(1);
    penZone.setColor(Qt::blue);
    penZone.setWidth(1);
    sceneRect = ui->graphicsView->geometry();
    sceneZoneStep = sceneRect.height() / zoneNumber;
    sceneWidth = sceneRect.width() - 2;
    xMax = sceneWidth - 1;
    sceneHeight = sceneRect.height() - 2;
    yMax = sceneHeight - 1;

    qDebug() << sceneWidth << "," << sceneHeight << "," << xMax << "," << yMax;
    addAxis();
}

MainWindow::~MainWindow(){

    delete ui;
}

bool MainWindow::readSettings(){

    if (QFile::exists(INIFILENAME)){

        clientAddress = settings->value("clientAddress", _CLIENT_ADR).toString();
        clientPort = settings->value("clientPort", _CLIENT_PORT).toInt();
        dbName = settings->value("dbName", _DB_NAME).toString();
        dbUser = settings->value("dbUser", _DB_USER).toString();
        dbPass = settings->value("dbPass", _DB_PASS).toString();

        //cout << clientAddress.toUtf8().constData() << endl;
        return true;

    } else {
        cout << "ini file not found" << endl;
        return false;

    }
}

void MainWindow::connectedToDB(){

    ui->DBconnStatusLabel->setText(MSG_DB_CON_YES);
    ui->DBconnStatusLabel->setStyleSheet("color: green");
}

void MainWindow::unconnectedToDB(){

    ui->DBconnStatusLabel->setText(MSG_DB_CON_NO);
    ui->DBconnStatusLabel->setStyleSheet("color: red");
}

void MainWindow::ConnectedToServer(){

    ui->connStatusLabel->setText(MSG_BBB_CON_YES);
    ui->connStatusLabel->setStyleSheet("color: green");
}

void MainWindow::NotConnectedToServer(){

    //cout << MSG_CONN_NO.toUtf8().constData() << endl;

    ui->connStatusLabel->setText(MSG_BBB_CON_NO);
    ui->connStatusLabel->setStyleSheet("color: red");

    ui->dINP1->setStyleSheet("background-color: light gray");
    ui->dINP2->setStyleSheet("background-color: light gray");
    ui->dINP3->setStyleSheet("background-color: light gray");
    ui->dINP4->setStyleSheet("background-color: light gray");
    ui->dINP5->setStyleSheet("background-color: light gray");
    ui->dINP6->setStyleSheet("background-color: light gray");
    ui->dINP7->setStyleSheet("background-color: light gray");

}

void MainWindow::displayInputs(){

    if (dInpArr[0] == '0')
        ui->dINP1->setStyleSheet("background-color: red");
    else if (dInpArr[0] == '1')
        ui->dINP1->setStyleSheet("background-color: green");

    if (dInpArr[1] == '0')
        ui->dINP2->setStyleSheet("background-color: red");
    else if (dInpArr[1] == '1')
        ui->dINP2->setStyleSheet("background-color: green");

    if (dInpArr[2] == '0')
        ui->dINP3->setStyleSheet("background-color: red");
    else if (dInpArr[2] == '1')
        ui->dINP3->setStyleSheet("background-color: green");

    if (dInpArr[3] == '0')
        ui->dINP4->setStyleSheet("background-color: red");
    else if (dInpArr[3] == '1')
        ui->dINP4->setStyleSheet("background-color: green");

    if (dInpArr[4] == '0')
        ui->dINP5->setStyleSheet("background-color: red");
    else if (dInpArr[4] == '1')
        ui->dINP5->setStyleSheet("background-color: green");

    if (dInpArr[5] == '0')
        ui->dINP6->setStyleSheet("background-color: red");
    else if (dInpArr[5] == '1')
        ui->dINP6->setStyleSheet("background-color: green");

    if (dInpArr[6] == '0')
        ui->dINP7->setStyleSheet("background-color: red");
    else if (dInpArr[6] == '1')
        ui->dINP7->setStyleSheet("background-color: green");

    aInpArrVal[0] = aInpArr[0] / 22.755555;
    ui->ai0->setText(QString::number(aInpArrVal[0],'f',1));

    //aInpArrVal[1] = aInpArr[1] / 22.755555;
    //ui->ai1->setText(QString::number(aInpArrVal[1],'f',1));
}

void MainWindow::transferData(){

    if (clientx->clientSocket.state() == QAbstractSocket::ConnectedState){

        clientx->datagram.clear();

        for (int i = 0; i < dOutSize; i++){
            clientx->datagram.append(dOutArr[i]);
        }

        for (int i = 0; i < aOutSize; i++){
            //cout << "ADC" << i <<": " << val << "...";  //DBG
            clientx->datagram.append( 'A' );
            char charVal[3];
            sprintf(charVal, "%d", aOutArr[i]);
            clientx->datagram.append( charVal );
            //clientx->datagram.append( 'Z' );
        }
        clientx->datagram.append( 'Z' );

        clientx->startTransfer();
    }
}

void MainWindow::on_pushButton_clicked(){

    dbThreadX->beginDate = ui->dateEdit_BEGIN->date().toString("dd/MM/yy");
    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    dbThreadX->beginTime = ui->timeEdit_BEGIN->time().toString();
    dbThreadX->endTime = ui->timeEdit_END->time().toString();
    //dbThreadX->verbose = true;
    dbThreadX->zeroStateLowThreshold = ui->thresholdEdit->text().toInt();
    dbThreadX->cmdAnalyzeAllZones = true;
    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();
}

void MainWindow::allZonesTable(){

    for (int i=0; i<zoneNumber+1; i++) {
        ui->tableAllZones->item(i, 0)->setText( QString::number(i) );
        ui->tableAllZones->item(i, 1)->setText( QString::number(statTotalActiveZones[i]) );
        ui->tableAllZones->item(i, 2)->setText( QDateTime::fromTime_t(statTotalActiveZonesDurations[i]).toUTC().toString("hh:mm:ss") );
        ui->tableAllZones->item(i, 3)->setText( QString::number(statTotalActiveZonesPercent[i], 'f', 1) );

        ui->tableAllZones->item(i, 3)->setFont(fontBold);
        ui->tableAllZones->item(i, 3)->setForeground(QColor::fromRgb(255,0,0));
    }

    ui->tableAllZones->item(zoneNumber+1, 0)->setText( "Toplam" );
    ui->tableAllZones->item(zoneNumber+1, 2)->setText( QDateTime::fromTime_t( totalTime ).toUTC().toString("hh:mm:ss") );

    ui->tableAllZones->item(zoneNumber+1, 0)->setFont(fontBold);
    ui->tableAllZones->item(zoneNumber+1, 2)->setFont(fontBold);
    ui->tableAllZones->item(zoneNumber+1, 2)->setForeground(QColor::fromRgb(255,0,0));

    ui->tableAllZones->item(zoneNumber+2, 0)->setText( "0 Aktif <Th" );
    ui->tableAllZones->item(zoneNumber+2, 1)->setText( QString::number(dbThreadX->zeroStateLowCount) );
    ui->tableAllZones->item(zoneNumber+2, 2)->setText( QDateTime::fromTime_t(dbThreadX->zeroStateLowTime).toUTC().toString("hh:mm:ss") );

    ui->tableAllZones->item(zoneNumber+3, 0)->setText( "0 Aktif >=Th" );
    ui->tableAllZones->item(zoneNumber+3, 1)->setText( QString::number(dbThreadX->zeroStateHighCount) );
    ui->tableAllZones->item(zoneNumber+3, 2)->setText( QDateTime::fromTime_t(dbThreadX->zeroStateHighTime).toUTC().toString("hh:mm:ss") );

    ui->tableAllZones->item(zoneNumber+3, 1)->setFont(fontBold);
    ui->tableAllZones->item(zoneNumber+3, 1)->setForeground(QColor::fromRgb(255,0,0));
    ui->tableAllZones->item(zoneNumber+3, 2)->setFont(fontBold);
    ui->tableAllZones->item(zoneNumber+3, 2)->setForeground(QColor::fromRgb(255,0,0));

    if (dbThreadX->delimiterEncountered){
        ui->textBrowser->append("Delimiter: " + dbThreadX->beginTimeDelimiter);
    }
    ui->textBrowser->append("Başlangıç: " + dbThreadX->queryBeginTime);
}

void MainWindow::on_saveAllZonesButton_clicked(){

    QString fileName = zoneAllFileName +
            ui->dateEdit_BEGIN->date().toString("yyMMdd") + "_" + ui->timeEdit_BEGIN->time().toString("hhmmss") + "-" +
            ui->dateEdit_END->date().toString("yyMMdd") + "_" + ui->timeEdit_END->time().toString("hhmmss") + ".csv";

    QFile file(fileName);

    QSqlRecord allZonesRecord;
    allZonesRecord = dbThreadX->qry.record();

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){

        if ( dbThreadX->qry.size() > 0 ) {

            QTextStream out(&file);

            int colNum = allZonesRecord.count();
            for (int i=0; i<colNum;i++)
                out << allZonesRecord.fieldName(i) << ",";
            out << "#" << endl;

            dbThreadX->qry.last();
            dbThreadX->stateList.last();
            int c = 0;
            int thresholdIndex = dbThreadX->thresholdIndexList.at(0);
            int x = 0;

            do {

                for (int i=0; i<colNum;i++)
                    out << dbThreadX->qry.value(i).toString() << ",";
                out << dbThreadX->stateList[c];

                if (c == thresholdIndex){
                    out << ",*";
                    x++;
                    if (x < dbThreadX->thresholdIndexList.count())
                        thresholdIndex = dbThreadX->thresholdIndexList.at(x);
                }
                out << endl;
                c++;

            } while (dbThreadX->qry.previous() && dbThreadX->qry.value(3).toString() != "*" );

            out << endl << " ,İstatistik" << endl << endl;

            out << " ,Aktif Bölge Top,Adet,Süre,Saat" << endl;

            for (int i=0; i<zoneNumber+1; i++) {
                out << " ," << i << "," << statTotalActiveZones[i] << "," << statTotalActiveZonesDurations[i] << ","
                    << QDateTime::fromTime_t( statTotalActiveZonesDurations[i] ).toUTC().toString("hh:mm:ss") << endl;
            }

            out << endl;
            out << "Threshold,Adet,Süre" << endl;
            out << "0 Aktif <Th," << dbThreadX->zeroStateLowCount << "," << QDateTime::fromTime_t( dbThreadX->zeroStateLowTime ).toUTC().toString("hh:mm:ss") << endl;
            out << "0 Aktif >=Th," << dbThreadX->zeroStateHighCount << "," << QDateTime::fromTime_t( dbThreadX->zeroStateHighTime ).toUTC().toString("hh:mm:ss") << endl;

            file.close();

            qDebug() << "file saved";
        }
    }
}

void MainWindow::on_zoneButton_clicked(){

    //currentZone = 6;
    if ( currentZone>=7 ) currentZone = 0;
    currentZone++;
    //if ( currentZone == 3 )   currentZone++;  // skip "balkon"
    zoneQuery(currentZone);
}

void MainWindow::zoneQuery(int zoneNumber){

    dbThreadX->beginDate = ui->dateEdit_BEGIN->date().toString("dd/MM/yy");
    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    dbThreadX->beginTime = ui->timeEdit_BEGIN->time().toString();
    dbThreadX->endTime = ui->timeEdit_END->time().toString();
    dbThreadX->verbose = false;

    dbThreadX->cmdAnalyzeZone = true;
    dbThreadX->currentZone = zoneNumber;
    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();
}

void MainWindow::zoneTable(){

    int rowNum = dbThreadX->currentZone - 1;
    float rate, total;

    if (dbThreadX->qry.size() > 0) {
        ui->tableZone->item(rowNum, 0)->setText( QString::number(dbThreadX->ONcount) );
        ui->tableZone->item(rowNum, 1)->setText( QString::number(dbThreadX->OFFcount) );
        ui->tableZone->item(rowNum, 2)->setText( QDateTime::fromTime_t( dbThreadX->ONtime ).toUTC().toString("hh:mm:ss") );
        ui->tableZone->item(rowNum, 3)->setText( QDateTime::fromTime_t( dbThreadX->OFFtime ).toUTC().toString("hh:mm:ss") );
        total = dbThreadX->ONtime + dbThreadX->OFFtime;
        rate = 0;
        if (total != 0)
            rate = 100.0 * dbThreadX->ONtime / total;
        ui->tableZone->item(rowNum, 4)->setText( QString::number(rate, 'f', 1) );
        ui->tableZone->item(rowNum, 4)->setFont(fontBold);
        ui->tableZone->item(rowNum, 4)->setForeground(QColor::fromRgb(255,0,0));

    } else {
        for (int c=0; c<5; c++)
            ui->tableZone->item(rowNum, c)->setText( "0" );
    }

    if (dbThreadX->delimiterEncountered){
        ui->textBrowser->append("Başlangıç-Delimiter: " + dbThreadX->beginTimeDelimiter);// + ", Bitiş: " + dbThreadX->endTime);
    }
}

void MainWindow::on_zoneSaveButton_clicked(){


    QString fileName = zoneFileNames[currentZone-1] +
            ui->dateEdit_BEGIN->date().toString("yyMMdd") + "_" + ui->timeEdit_BEGIN->time().toString("hhmmss") + "-" +
            ui->dateEdit_END->date().toString("yyMMdd") + "_" + ui->timeEdit_END->time().toString("hhmmss") + ".csv";

    QFile file(fileName);

    QSqlRecord record;
    record = dbThreadX->qry.record();

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){

        if ( dbThreadX->qry.size() > 0 ) {

            QTextStream out(&file);

            int total = dbThreadX->ONtime + dbThreadX->OFFtime;
            float rate = 0;
            if (total != 0)
                rate = 100.0 * dbThreadX->ONtime / total;

            out << "ON#,ON Süresi,OFF Süresi,OFF#,% ON" << endl;
            out << dbThreadX->ONcount << ","
                << QDateTime::fromTime_t( dbThreadX->ONtime ).toUTC().toString("hh:mm:ss") << ","
                << QDateTime::fromTime_t( dbThreadX->OFFtime ).toUTC().toString("hh:mm:ss") <<  ","
                << dbThreadX->OFFcount << ","
                << QString::number(rate,'f',1) << endl << endl;

            //---
            int colNum = record.count();
            for (int i=0; i<colNum;i++)
                out << record.fieldName(i) << ",";
            out << "diff" << endl;

            dbThreadX->qry.last();

            out << "X,,23:59:59," << dbThreadX->qry.value(3).toString() << endl;

            do {

                for (int i=0; i<colNum;i++)
                    out << dbThreadX->qry.value(i).toString() << ",";
                out << endl;
            } while (dbThreadX->qry.previous() && dbThreadX->qry.value(3).toString() != "*" );

            if (!dbThreadX->delimiterEncountered){
                out << "X,,00:00:00,";
                dbThreadX->qry.next();
                if (dbThreadX->qry.value(3) == 1)
                    out << "0" << endl;
                else
                    out << "1" << endl;
            } else {
                out << "X,," << dbThreadX->qry.value(2).toString() << ",";
                dbThreadX->qry.next();
                out << dbThreadX->qry.value(3).toString() << endl;
            }
            //---


            //---
            out << "-,-,-,-,-" << endl;
            dbThreadX->qry.last();
            QString prevTime = "23:59:59";

            //out << "X,Graph,23:59:59," << dbThreadX->qry.value(3).toString() << endl;

            do {

                out << ",,";
                out << prevTime << ","
                    << dbThreadX->qry.value(3).toString() << endl;

                out << ",,";
                out << dbThreadX->qry.value(2).toString() << ","
                    << dbThreadX->qry.value(3).toString() << endl;

                prevTime = dbThreadX->qry.value(2).toString();

            } while (dbThreadX->qry.previous() && dbThreadX->qry.value(3).toString() != "*" );

            if (!dbThreadX->delimiterEncountered){
                dbThreadX->qry.next();
                int lastState = 0;
                if (dbThreadX->qry.value(3) == 1)
                    lastState = 0;
                else
                    lastState = 1;
                out << "X,," << dbThreadX->qry.value(2).toString() << "," << lastState << endl;

                out << "X,,00:00:00," << lastState << endl;
            }
/*            else {
                dbThreadX->qry.previous();
                out << "X,," << dbThreadX->qry.value(2).toString() << "," << dbThreadX->qry.value(3).toString() << endl;
            }
*/
            //---

            file.close();

            qDebug() << "file saved";
        }
    }
}

void MainWindow::on_timeUpButton_clicked(){
    ui->timeEdit_END->setTime(QTime::fromString("23:59:59", "hh:mm:ss"));
}

void MainWindow::on_timeDownButton_clicked(){
    ui->timeEdit_BEGIN->setTime(QTime::fromString("00:00:00", "hh:mm:ss"));
/*
    QTime last = QTime::fromString("00:15:00", "hh:mm:ss");
    QTime first = QTime::fromString("23:44:00", "hh:mm:ss");
    qint64 diff = first.msecsTo(last) / 1000;
    qDebug() << last.toString() << " - " << first.toString() << " is " << diff;
*/
}

void MainWindow::on_report2DBButton_clicked(){

    dbThreadX->beginDate = ui->dateEdit_BEGIN->date().toString("dd/MM/yy");
    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    dbThreadX->beginTime = ui->timeEdit_BEGIN->time().toString();
    dbThreadX->endTime = ui->timeEdit_END->time().toString();
    //dbThreadX->verbose = true;
    dbThreadX->zeroStateLowThreshold = ui->thresholdEdit->text().toInt();
    dbThreadX->cmdSummaryReport = true;
    //dbThreadX->cmdAnalyzeAllZones = true;
    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();

}

void MainWindow::summaryResult(){

    for (int i=0; i<15; i++)
        for (int c=0; c<4; c++)
            ui->tableAllZones->item(i, c)->setText( " " );

    for (int i=0; i<zoneNumber+1; i++)
        ui->tableAllZones->item(i, 0)->setText( QString::number(i) );

    ui->tableAllZones->item(0, 3)->setText( QString::number(dbThreadX->summaryData.zone0_rate, 'f', 1) );
    ui->tableAllZones->item(1, 3)->setText( QString::number(dbThreadX->summaryData.zone1_rate, 'f', 1) );
    ui->tableAllZones->item(2, 3)->setText( QString::number(dbThreadX->summaryData.zone2_rate, 'f', 1) );
    ui->tableAllZones->item(3, 3)->setText( QString::number(dbThreadX->summaryData.zone3_rate, 'f', 1) );
    ui->tableAllZones->item(4, 3)->setText( QString::number(dbThreadX->summaryData.zone4_rate, 'f', 1) );
    ui->tableAllZones->item(5, 3)->setText( QString::number(dbThreadX->summaryData.zone5_rate, 'f', 1) );
    ui->tableAllZones->item(6, 3)->setText( QString::number(dbThreadX->summaryData.zone6_rate, 'f', 1) );
    ui->tableAllZones->item(7, 3)->setText( QString::number(dbThreadX->summaryData.zone7_rate, 'f', 1) );

    ui->tableAllZones->item(8, 0)->setText( "0 Aktif >=Th" );
    ui->tableAllZones->item(8, 1)->setText( QString::number(dbThreadX->summaryData.zone0_thr_count) );
    ui->tableAllZones->item(8, 2)->setText( dbThreadX->summaryData.zone0_thr_time );

    for (int i=0; i<7; i++)
        for (int c=0; c<5; c++)
            ui->tableZone->item(i, c)->setText( " " );

    ui->tableZone->item(0, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_oto, 'f', 1) );
    ui->tableZone->item(1, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_sln, 'f', 1) );
    ui->tableZone->item(2, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_blk, 'f', 1) );
    ui->tableZone->item(3, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_mut, 'f', 1) );
    ui->tableZone->item(4, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_eyo, 'f', 1) );
    ui->tableZone->item(5, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_cyo, 'f', 1) );
    ui->tableZone->item(6, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_yod, 'f', 1) );

    dbThreadX->cmdInsertToSummaryTable = true;
    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();

}

void MainWindow::addAxis(){

    for (int i=1; i <zoneNumber+1; i++){
        scene->addLine(0, sceneZoneStep*i-1, xMax, sceneZoneStep*i-1, penAxis);
    }
}

void MainWindow::clearGraph(){

    scene->clear();
    addAxis();
    ui->graphicsView->show();
}

void MainWindow::on_graphUpdateButton_clicked(){

    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    //-dbThreadX->endDate = QDate::currentDate().toString("dd/MM/yy");
    dbThreadX->beginDate = dbThreadX->endDate;
    dbThreadX->endTime = ui->timeEdit_END->time().toString();
    //-dbThreadX->endTime = QTime::currentTime().toString("hh:mm:ss");
    //dbThreadX->endTime = "00:59:59";

    QTime last, first;
    qint64 diff = 0;
    last = QTime::fromString(dbThreadX->endTime, "hh:mm:ss");
    first = QTime::fromString("00:00:00");
    diff = first.secsTo(last);

    if (diff < 3600)
        dbThreadX->beginTime = "00:00:00";
    else{
        first = last.addSecs(-3600);
    }

    dbThreadX->beginTime = first.toString("hh:mm:ss");

    qDebug() << dbThreadX->endDate << "," << dbThreadX->endTime << "," << dbThreadX->beginDate << "," << dbThreadX->beginTime;

    dbThreadX->verbose = true;
    dbThreadX->cmdGraphData = true;
    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();


}

void MainWindow::drawGraph(){

    clearGraph();
    addAxis();

    for (int i=1; i<zoneNumber+1; i++){

        int yRef0 = sceneZoneStep*i-1;
        int yRef1 = yRef0 - 20;
        int x1, x2, y;

        if (!dbThreadX->graphList[i].isEmpty()){
            for (int j=0; j<dbThreadX->graphList[i].count()-1; j++){

                x1 = dbThreadX->graphList[i].at(j).timeDiff / 3;
                x2 = dbThreadX->graphList[i].at(j+1).timeDiff / 3;

                if (dbThreadX->graphList[i].at(j+1).state == 0)
                    y = yRef0;
                else
                    y = yRef1;

                scene->addLine(x1, y, x2, y, penZone);
                scene->addLine(x2, yRef0, x2, yRef1, penZone);
            }
        }

    }

}
