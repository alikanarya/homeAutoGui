#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "checkclient.h"
#include "dbthread.h"
#include "begindialog.h"

#include <iostream>

using namespace std;

Server *serverx;
Client *clientx;
checkClient *checkClientX;
dbThread *dbThreadX;
extern begindialog *bd;

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

    //bd->prmArray[0].cycleTime = 20;
    //qDebug() << bd->prmArray[0].cycleTime;

    ui->tableAllZones->setRowCount(15);
    ui->tableAllZones->verticalHeader()->setDefaultSectionSize(20);
    for (int i=0; i<15; i++)
        for (int c=0; c<4; c++){
            ui->tableAllZones->setItem(i, c, new QTableWidgetItem( " " ) );
            ui->tableAllZones->item(i, c)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }

    for (int i=0; i<7; i++)
        for (int c=0; c<6; c++){
            ui->tableZone->setItem(i, c, new QTableWidgetItem( " " ) );
            ui->tableZone->item(i, c)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            if (c==4 || c==5)
                ui->tableZone->item(i, c)->setForeground(QColor::fromRgb(255,0,0));
            if (c==5)
                ui->tableZone->item(i, c)->setFont(fontBold);
        }



    zoneTableHeader = QStringList() << "ON #" << "OFF #" << "ON t" << "OFF t" << "% ON" << "LOAD";

    serverx = new Server();
    serverx->serverPort = serverPort;
    serverx->init();
    clientx = new Client();
    checkClientX = new checkClient();

    //settings = new QSettings(INIFILENAME, QSettings::IniFormat);
    //readSettings();
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

    connect(serverx, SIGNAL(mainIO()),this, SLOT(displayInputs()));
    connect(serverx, SIGNAL(remote1IO()),this, SLOT(displayInputsR1()));
    connect(serverx, SIGNAL(readFinished()),this, SLOT(_debug()));
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
    connect(dbThreadX, SIGNAL(tempOutDone()), this, SLOT(updateTempGUI()));
    connect(dbThreadX, SIGNAL(tempOutAvgDone()), this, SLOT(avgTempGUI()));
    connect(dbThreadX, SIGNAL(analyzeZonesDone()), this, SLOT(drawGraphZones()));
    connect(dbThreadX, SIGNAL(singleZoneDone()), this, SLOT(drawSingleZone()));
    connect(dbThreadX, SIGNAL(ngConsumptionDone(float)), this, SLOT(updateNgConsumptionValue(float)));
    connect(dbThreadX, SIGNAL(ngConsumptionGraph()), this, SLOT(ngMeterGraph()));

    QTimer *timerTemp = new QTimer();
    QObject::connect(timerTemp, SIGNAL(timeout()), this, SLOT(updateTemp()));
    timerTemp->start(600000);

    dbThreadX->cmdConnect = true;
    dbThreadX->start();

    // init deviation trend
    scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);

    penAxisSolid.setColor(Qt::gray);
    penAxisSolid.setWidth(1);

    penAxisDash.setColor(Qt::gray);
    penAxisDash.setWidth(1);
    penAxisDash.setStyle(Qt::DashLine);

    penZone.setColor(Qt::red);
    penZone.setWidth(1);

    penZoneBlue.setColor(Qt::blue);
    penZoneBlue.setWidth(1);

    penZoneGreen.setColor(Qt::green);
    penZoneGreen.setWidth(1);

    penZoneEst.setColor(Qt::darkRed);
    penZoneEst.setWidth(1);

    sceneRect = ui->graphicsView->geometry();
    sceneZoneStep = sceneRect.height() / zoneNumber;
    sceneWidth = sceneRect.width() - 2;
    xMax = sceneWidth - 1;
    sceneHeight = sceneRect.height() - 2;
    yMax = sceneHeight - 1;

    //qDebug() << sceneWidth << "," << sceneHeight << "," << xMax << "," << yMax;
    addAxis();

    timeLabels[0] = ui->labelT0;
    timeLabels[1] = ui->labelT1;
    timeLabels[2] = ui->labelT2;
    timeLabels[3] = ui->labelT3;
    timeLabels[4] = ui->labelT4;
    timeLabels[5] = ui->labelT5;
    timeLabels[6] = ui->labelT6;
    timeLabels[7] = ui->labelT7;
    timeLabels[8] = ui->labelT8;
    timeLabels[9] = ui->labelT9;
    timeLabels[10] = ui->labelT10;
    timeLabels[11] = ui->labelT11;
    timeLabels[12] = ui->labelT12;

    yLabels[0] = ui->labelY0;
    yLabels[1] = ui->labelY1;
    yLabels[2] = ui->labelY2;
    yLabels[3] = ui->labelY3;
    yLabels[4] = ui->labelY4;
    yLabels[5] = ui->labelY5;
    yLabels[6] = ui->labelY6;

    prmTables[0] = ui->tab1Table;
    prmTables[1] = ui->tab2Table;
    prmTables[2] = ui->tab3Table;
    prmTables[3] = ui->tab4Table;
    prmTables[4] = ui->tab5Table;
    prmTables[5] = ui->tab6Table;
    prmTables[6] = ui->tab7Table;

    for (int i=0; i<7; i++){
        prmTables[i]->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        for (int c=0; c<11; c++){
            prmTables[i]->setItem(c, 0, new QTableWidgetItem( " " ) );
            prmTables[i]->item(c, 0)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }

        prmTables[i]->item(0, 0)->setText( QString::number(bd->prmArray[i].propBandWidth) );
        prmTables[i]->item(1, 0)->setText( QString::number(bd->prmArray[i].cycleTime) );
        prmTables[i]->item(2, 0)->setText( QString::number(bd->prmArray[i].propBandPos) );
        prmTables[i]->item(3, 0)->setText( QString::number(bd->prmArray[i].normalSet, 'f', 1) );
        prmTables[i]->item(4, 0)->setText( QString::number(bd->prmArray[i].reducedSet, 'f', 1) );
        prmTables[i]->item(5, 0)->setText( bd->prmArray[i].P1NormalModeTime );
        prmTables[i]->item(6, 0)->setText( bd->prmArray[i].P1ReducedModeTime );
        prmTables[i]->item(7, 0)->setText( bd->prmArray[i].P2NormalModeTime );
        prmTables[i]->item(8, 0)->setText( bd->prmArray[i].P2ReducedModeTime );
        prmTables[i]->item(9, 0)->setText( bd->prmArray[i].P3NormalModeTime );
        prmTables[i]->item(10, 0)->setText( bd->prmArray[i].P3ReducedModeTime );
    }
    ui->paramTabs->setCurrentIndex(0);

    meterTable = ui->tab8Table;

    pic = new QLabel(this);
    pic->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    //pic->setAlignment(Qt::AlignBottom | Qt::AlignRight);
    pic->setGeometry(560,60,400,300);
    pic->setScaledContents(true);

    layout = new QVBoxLayout();
    layout->addWidget(pic);
    //setLayout(layout);
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

    updateTemp();
    calcAvgTemp();
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

void MainWindow::displayInputsR1(){
    /*
    QString message = "Central Heating:" + (dInpArr_R1_bool[4] ? QString("ON") : QString("OFF")) + "  Hot Water:" + (dInpArr_R1_bool[5] ? "ON" : "OFF") + "  Flame:" + (dInpArr_R1_bool[6] ? "ON" : "OFF");
    message += "  Boiler temperature is " + QString::number(aInpArr_R1[0]/100.0, 'f', 1) + " C";
    */
    if (dInpArr_R1_bool[1] || dInpArr_R1_bool[2] || dInpArr_R1_bool[3]) {
        QString message = rs1Hour + ":" + rs1Min + ":" + rs1Sec + " ";
        message += "OpenThermResponseStatus:" + (dInpArr_R1_bool[1] ? QString("not initialized ") : QString(" ")) + (dInpArr_R1_bool[2] ? "Invalid response " : " ") + (dInpArr_R1_bool[3] ? "Response timeout " : " ");
        ui->textBrowser->append(message);
    }

    if (dInpArr_R1_bool[4]) ui->boilerCH->setStyleSheet("background-color: green");
    else ui->boilerCH->setStyleSheet("background-color: red");

    if (dInpArr_R1_bool[5]) ui->boilerDHW->setStyleSheet("background-color: green");
    else ui->boilerDHW->setStyleSheet("background-color: red");

    if (dInpArr_R1_bool[6]) ui->boilerFlame->setStyleSheet("background-color: green");
    else ui->boilerFlame->setStyleSheet("background-color: red");

    ui->boilerTemp->setText(QString::number(aInpArr_R1[0]/100.0,'f',1) + " / " + QString::number(aInpArr_R1[2]) + " C"); // Boiler Temp / Set Temp

    QString wifiqual = (aInpArr_R1[1] != 999) ? QString::number(aInpArr_R1[1]) : "---";
    ui->remst1WiFi->setText("WiFi Qual %" + wifiqual);
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

    //currentZone = 1;
    if ( currentZone>=7 ) currentZone = 0;
    currentZone++;
    //  if ( currentZone == 3 )   currentZone++;  // skip "balkon"
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

        zoneTableHeader[5] = "LOAD";
        ui->tableZone->setHorizontalHeaderLabels(zoneTableHeader);

        ui->tableZone->item(rowNum, 0)->setText( QString::number(dbThreadX->ONcount) );
        ui->tableZone->item(rowNum, 1)->setText( QString::number(dbThreadX->OFFcount) );
        ui->tableZone->item(rowNum, 2)->setText( QDateTime::fromTime_t( dbThreadX->ONtime ).toUTC().toString("hh:mm:ss") );
//        ui->tableZone->item(rowNum, 3)->setText( QDateTime::fromTime_t( dbThreadX->OFFtime ).toUTC().toString("d:hh:mm:ss") );
        ui->tableZone->item(rowNum, 3)->setText( getTimeStr( dbThreadX->OFFtime ) );
        total = dbThreadX->ONtime + dbThreadX->OFFtime;
        rate = 0;
        if (total != 0)
            rate = 100.0 * dbThreadX->ONtime / total;
        ui->tableZone->item(rowNum, 4)->setText( QString::number(rate, 'f', 1) );
        //ui->tableZone->item(rowNum, 4)->setFont(fontBold);
        //ui->tableZone->item(rowNum, 4)->setForeground(QColor::fromRgb(255,0,0));

        ui->tableZone->item(rowNum, 5)->setText( QString::number(rate * loadFactors[currentZone - 1]/13, 'f', 2) );
        //ui->tableZone->item(rowNum, 5)->setFont(fontBold);
        //ui->tableZone->item(rowNum, 5)->setForeground(QColor::fromRgb(255,0,0));

        // message window
        ui->textBrowser->append(tableNames[currentZone]);

        int colNum = dbThreadX->qry.record().count();

        QString temp = "End: ";
        dbThreadX->qry.last();
        for( int c=0; c<colNum; c++ )
            temp += dbThreadX->qry.value(c).toString() + " ";
        ui->textBrowser->append(temp);

        temp = "Bgn: ";
        dbThreadX->qry.first();
        for( int c=0; c<colNum; c++ )
            temp += dbThreadX->qry.value(c).toString() + " ";
        ui->textBrowser->append(temp);

    } else {
        for (int c=0; c<6; c++)
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

void MainWindow::on_timeNow1Button_clicked(){
    ui->timeEdit_END->setTime(QTime::currentTime());
}

void MainWindow::on_timeNow2Button_clicked(){
    ui->timeEdit_BEGIN->setTime(QTime::currentTime());
}

void MainWindow::on_yesterdayButton_clicked(){
    ui->timeEdit_BEGIN->setTime(ui->timeEdit_END->time());
    ui->dateEdit_BEGIN->setDate(ui->dateEdit_END->date().addDays(-1));
}

void MainWindow::on_report2DBButton_clicked(){

    dbThreadX->beginDate = ui->dateEdit_BEGIN->date().toString("dd/MM/yy");
    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    dbThreadX->beginTime = ui->timeEdit_BEGIN->time().toString();
    dbThreadX->endTime = ui->timeEdit_END->time().toString();

    dbThreadX->verbose = false;
    dbThreadX->zeroStateLowThreshold = ui->thresholdEdit->text().toInt();
    dbThreadX->cmdSummaryReport = true;

    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    reportToDB = ui->checkBoxDB->isChecked();
    //qDebug() << reportToDB;
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

    float totalLoad = dbThreadX->summaryData.on_rate_oto * loadFactors[0] + dbThreadX->summaryData.on_rate_sln * loadFactors[1] + dbThreadX->summaryData.on_rate_blk * loadFactors[2] +
            dbThreadX->summaryData.on_rate_mut * loadFactors[3] + dbThreadX->summaryData.on_rate_eyo * loadFactors[4] + dbThreadX->summaryData.on_rate_cyo * loadFactors[5] + dbThreadX->summaryData.on_rate_yod * loadFactors[6];

    totalLoad /= 13;
    zoneTableHeader[5] = "L:" + QString::number(totalLoad, 'f', 2);
    ui->tableZone->setHorizontalHeaderLabels(zoneTableHeader);

    ui->tableZone->item(0, 0)->setText( QString::number(dbThreadX->summaryData.on_count_oto) );
    ui->tableZone->item(1, 0)->setText( QString::number(dbThreadX->summaryData.on_count_sln) );
    ui->tableZone->item(2, 0)->setText( QString::number(dbThreadX->summaryData.on_count_blk) );
    ui->tableZone->item(3, 0)->setText( QString::number(dbThreadX->summaryData.on_count_mut) );
    ui->tableZone->item(4, 0)->setText( QString::number(dbThreadX->summaryData.on_count_eyo) );
    ui->tableZone->item(5, 0)->setText( QString::number(dbThreadX->summaryData.on_count_cyo) );
    ui->tableZone->item(6, 0)->setText( QString::number(dbThreadX->summaryData.on_count_yod) );

    ui->tableZone->item(0, 2)->setText( QDateTime::fromTime_t( dbThreadX->summaryData.on_time_oto ).toUTC().toString("hh:mm:ss") );
    ui->tableZone->item(1, 2)->setText( QDateTime::fromTime_t( dbThreadX->summaryData.on_time_sln ).toUTC().toString("hh:mm:ss") );
    ui->tableZone->item(2, 2)->setText( QDateTime::fromTime_t( dbThreadX->summaryData.on_time_blk ).toUTC().toString("hh:mm:ss") );
    ui->tableZone->item(3, 2)->setText( QDateTime::fromTime_t( dbThreadX->summaryData.on_time_mut ).toUTC().toString("hh:mm:ss") );
    ui->tableZone->item(4, 2)->setText( QDateTime::fromTime_t( dbThreadX->summaryData.on_time_eyo ).toUTC().toString("hh:mm:ss") );
    ui->tableZone->item(5, 2)->setText( QDateTime::fromTime_t( dbThreadX->summaryData.on_time_cyo ).toUTC().toString("hh:mm:ss") );
    ui->tableZone->item(6, 2)->setText( QDateTime::fromTime_t( dbThreadX->summaryData.on_time_yod ).toUTC().toString("hh:mm:ss") );

    ui->tableZone->item(0, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_oto, 'f', 1) );
    ui->tableZone->item(1, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_sln, 'f', 1) );
    ui->tableZone->item(2, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_blk, 'f', 1) );
    ui->tableZone->item(3, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_mut, 'f', 1) );
    ui->tableZone->item(4, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_eyo, 'f', 1) );
    ui->tableZone->item(5, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_cyo, 'f', 1) );
    ui->tableZone->item(6, 4)->setText( QString::number(dbThreadX->summaryData.on_rate_yod, 'f', 1) );

    ui->tableZone->item(0, 5)->setText( QString::number(dbThreadX->summaryData.on_rate_oto * loadFactors[0]/13, 'f', 2) );
    ui->tableZone->item(1, 5)->setText( QString::number(dbThreadX->summaryData.on_rate_sln * loadFactors[1]/13, 'f', 2) );
    ui->tableZone->item(2, 5)->setText( QString::number(dbThreadX->summaryData.on_rate_blk * loadFactors[2]/13, 'f', 2) );
    ui->tableZone->item(3, 5)->setText( QString::number(dbThreadX->summaryData.on_rate_mut * loadFactors[3]/13, 'f', 2) );
    ui->tableZone->item(4, 5)->setText( QString::number(dbThreadX->summaryData.on_rate_eyo * loadFactors[4]/13, 'f', 2) );
    ui->tableZone->item(5, 5)->setText( QString::number(dbThreadX->summaryData.on_rate_cyo * loadFactors[5]/13, 'f', 2) );
    ui->tableZone->item(6, 5)->setText( QString::number(dbThreadX->summaryData.on_rate_yod * loadFactors[6]/13, 'f', 2) );

    if (reportToDB) {
        dbThreadX->cmdInsertToSummaryTable = true;
        progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
        dbThreadX->start();
    }

    clearGraphZones();
    calcAvgTemp();
    on_ngMeterButton_clicked();
}

void MainWindow::addAxis(){

    for (int i=1; i <zoneNumber+1; i++){
        scene->addLine(0, sceneZoneStep*i-1, xMax, sceneZoneStep*i-1, penAxisSolid);
    }

    for (int i=1; i <12; i++){

        if (i % 2 == 0)
            scene->addLine(100*i-1, 0, 100*i-1, yMax, penAxisSolid);
        else
            scene->addLine(100*i-1, 0, 100*i-1, yMax, penAxisDash);
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

    //qDebug() << dbThreadX->endDate << "," << dbThreadX->endTime << "," << dbThreadX->beginDate << "," << dbThreadX->beginTime;

    dbThreadX->verbose = false;
    dbThreadX->cmdGraphData = true;
    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();
}

void MainWindow::drawGraph(){

    clearGraph();

    ui->textBrowser->append("-----");
    ui->labelBeginDate->setText( "---" );
    ui->labelEndDate->setText( ui->dateEdit_END->text());

    graphScale = 3;

    for (int i=0; i<zoneNumber; i++){

        int yRef0 = sceneZoneStep*(i+1) - 1;
        int yRef1 = yRef0 - 20;
        int x1, x2, y, t1, t2, diff;

        if (!dbThreadX->graphList[i].isEmpty()){

            QString temp = tableNames[i+1];

            for (int j=0; j<dbThreadX->graphList[i].count()-1; j++){

                t1 = dbThreadX->graphList[i].at(j).timeDiff;
                x1 = t1 / graphScale;
                t2 = dbThreadX->graphList[i].at(j+1).timeDiff;
                x2 = t2 / graphScale;

                if (dbThreadX->graphList[i].at(j+1).state == 0)
                    y = yRef0;
                else
                    y = yRef1;

                scene->addLine(x1, y, x2, y, penZone);
                scene->addLine(x2, yRef0, x2, yRef1, penZone);

                diff = t2 - t1;
                if (diff < 3600)
                    temp += " ," + QDateTime::fromTime_t(t2-t1).toUTC().toString("mm:ss");
                else
                    temp += " ," + QDateTime::fromTime_t(t2-t1).toUTC().toString("hh:mm:ss");
            }
            ui->textBrowser->append(temp);
        }
    }

    QTime last, first;
    last = QTime::fromString(dbThreadX->endTime, "hh:mm:ss");

    timeLabels[0]->setText(ui->timeEdit_END->time().toString("hh:mm"));

    for (int i=1; i<13; i++){
        first = last.addSecs(-300*i);
        timeLabels[i]->setText(first.toString("hh:mm"));
    }

}

void MainWindow::on_forwardLargeButton_clicked(){

    QString now = ui->timeEdit_END->time().toString();
    QTime nowT = QTime::fromString(now, "hh:mm:ss");
    QTime nextT = nowT.addSecs(3600);
    QDate nowD =  ui->dateEdit_END->date();

    ui->timeEdit_END->setTime(nextT);

    if (nowT.hour() == 23){
        QDate nextD = nowD.addDays(1);
        ui->dateEdit_END->setDate(nextD);
        ui->dateEdit_BEGIN->setDate(nextD);
    }

    on_graphUpdateButton_clicked();
}

void MainWindow::on_forwardSmallButton_clicked(){

    QString now = ui->timeEdit_END->time().toString();
    QTime nowT = QTime::fromString(now, "hh:mm:ss");
    QTime nextT = nowT.addSecs(300);
    QDate nowD =  ui->dateEdit_END->date();

    ui->timeEdit_END->setTime(nextT);

    if (nowT.hour() == 23){
        QDate nextD = nowD.addDays(1);
        ui->dateEdit_END->setDate(nextD);
        ui->dateEdit_BEGIN->setDate(nextD);
    }

    on_graphUpdateButton_clicked();
}

void MainWindow::on_backwardLargeButton_clicked(){

    QString now = ui->timeEdit_END->time().toString();
    QTime nowT = QTime::fromString(now, "hh:mm:ss");
    QTime nextT = nowT.addSecs(-3600);
    QDate nowD =  ui->dateEdit_END->date();

    ui->timeEdit_END->setTime(nextT);

    if (nowT.hour() == 0){
        QDate nextD = nowD.addDays(-1);
        ui->dateEdit_END->setDate(nextD);
        ui->dateEdit_BEGIN->setDate(nextD);
    }

    on_graphUpdateButton_clicked();
}

void MainWindow::on_backwardSmallButton_clicked(){

    QString now = ui->timeEdit_END->time().toString();
    QTime nowT = QTime::fromString(now, "hh:mm:ss");
    QTime nextT = nowT.addSecs(-300);
    QDate nowD =  ui->dateEdit_END->date();

    ui->timeEdit_END->setTime(nextT);

    if (nowT.hour() == 0){
        QDate nextD = nowD.addDays(-1);
        ui->dateEdit_END->setDate(nextD);
        ui->dateEdit_BEGIN->setDate(nextD);
    }

    on_graphUpdateButton_clicked();
}

void MainWindow::updateTemp(){

    dbThreadX->cmdTempData = true;
    progress->setWindowTitle("Sıcaklık Alınıyor");
    dbThreadX->start();
}

void MainWindow::updateTempGUI(){

    ui->outTemp->setText( QString::number(dbThreadX->tempOut, 'f', 1) + " °C");//+"<sub>"+dbThreadX->tempTime+"</sub><sup>"+dbThreadX->tempDate+"</sup>" );
    ui->outTempDate->setText( dbThreadX->tempDate );
    ui->outTempTime->setText( dbThreadX->tempTime );
}

void MainWindow::calcAvgTemp(){

    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    dbThreadX->beginDate = ui->dateEdit_BEGIN->date().toString("dd/MM/yy");
//    dbThreadX->beginDate = ui->dateEdit_END->date().addDays(-1).toString("dd/MM/yy");
    dbThreadX->endTime = ui->timeEdit_END->time().toString();
    dbThreadX->beginTime = ui->timeEdit_BEGIN->time().toString();
//    dbThreadX->beginTime = dbThreadX->endTime;

    dbThreadX->verbose = false;
    dbThreadX->cmdAvgTempData = true;

    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();
}

void MainWindow::avgTempGUI(){

    ui->avgTemp->setText( QString::number(dbThreadX->tempMin, 'f', 1) + " // " + QString::number(dbThreadX->tempAvg, 'f', 1) + " // " + QString::number(dbThreadX->tempMax, 'f', 1) + " °C" );
    ui->avgTemp2->setText( "T(24h)  min: <font color=\"blue\">" + QString::number(dbThreadX->tempMin, 'f', 1) + "</font>  avg: <font color=\"red\">" + QString::number(dbThreadX->tempAvg, 'f', 1) + "</font>  max: <font color=\"blue\">" + QString::number(dbThreadX->tempMax, 'f', 1) + " °C</font>" );

    if (tempSave)
        saveTempData();

    if ( dbThreadX->qry.size() > 0 ) {

        if (!drawGraphZonesFlag) {
            QTime last, first;
            last = QTime::fromString(dbThreadX->endTime, "hh:mm:ss");

            timeLabels[0]->setText(ui->timeEdit_END->time().toString("hh:mm"));

            for (int i=1; i<13; i++){
                first = last.addSecs(-7200*i);
                timeLabels[i]->setText(first.toString("hh:mm"));
            }

            ui->labelBeginDate->setText( ui->dateEdit_END->date().addDays(-1).toString("dd.MM.yyyy"));
            ui->labelEndDate->setText( ui->dateEdit_END->text());
            graphScale = 72;
        }

        int x1, x2, y1, y2;

        float range = (dbThreadX->tempMax - dbThreadX->tempMin);
        float ySpan = range * 1.1;
        float yScale = scene->height() / ySpan;
        int min = yScale * range * 0.05;

        float y = 0;
        for (int i=0; i<7; i++){
            y = (i+1)*ySpan/7 + dbThreadX->tempMin - range*0.05;
            yLabels[i]->setText(QString::number(y, 'f', 1));
        }

        QString temp = fontAttrRed + "TMP"+ fontAttrEnd + ".....";

        int timeStep = 7200*graphScale/72;
        //qDebug() << "timestep: " << timeStep << " count: " << count << " diff: " << dbThreadX->tempList[ 0 ].timeDiff;

        int count = 1;

        for (int i=dbThreadX->tempList.size()-1; i>0; i--){

            x1= dbThreadX->tempList[i].timeDiff / graphScale;
            x2= dbThreadX->tempList[i-1].timeDiff / graphScale;

            y1 = sceneHeight - (dbThreadX->tempList[i].value - dbThreadX->tempMin) * yScale - min;
            y2 = sceneHeight - (dbThreadX->tempList[i-1].value - dbThreadX->tempMin) * yScale - min;

            scene->addLine(x1, y1, x2, y2, penZone);
            scene->addEllipse(x1-2, y1-2, 4, 4, penZone);
            scene->addEllipse(x2-2, y2-2, 4, 4, penZone);

            if (dbThreadX->tempList[i].timeDiff >= count*timeStep){
                count++;
                temp += " ," + fontAttrRed + QTime::fromString(dbThreadX->endTime, "hh:mm:ss").addSecs(-1*dbThreadX->tempList[i].timeDiff).toString("hh:mm") + fontAttrEnd + " " + QString::number(dbThreadX->tempList[i].value, 'f', 1);
            } else {
                temp += " ," + QString::number(dbThreadX->tempList[i].value, 'f', 1);
            }
        }

        temp += " ," + QString::number(dbThreadX->tempList[0].value, 'f', 1);
        ui->textBrowser->append(temp);

        //drawGraphZonesFlag = false;   //**********
    }
}

void MainWindow::ngMeterGraph()
{
    if ( dbThreadX->qry.size() > 0 ) {

        if (!drawGraphZonesFlag) {
            QTime last, first;
            last = QTime::fromString(dbThreadX->endTime, "hh:mm:ss");

            timeLabels[0]->setText(ui->timeEdit_END->time().toString("hh:mm"));

            for (int i=1; i<13; i++){
                first = last.addSecs(-7200*i);
                timeLabels[i]->setText(first.toString("hh:mm"));
            }

            ui->labelBeginDate->setText( ui->dateEdit_END->date().addDays(-1).toString("dd.MM.yyyy"));
            ui->labelEndDate->setText( ui->dateEdit_END->text());
        }

        graphScale = 72;
        int x1, x2, y1, y2;

        float minValue = dbThreadX->ngMeterList.first().value;
        float range = (dbThreadX->ngMeterList.last().value - minValue);
        float ySpan = range * 1.1;
        float yScale = sceneHeight / ySpan; //scene->height()
        int min = yScale * range * 0.05;
        //qDebug() << "minValue: " << minValue << " range: " << range << " ySpan: " << ySpan << " yScale: " << yScale;

        float y = 0;
        for (int i=0; i<7; i++){
            y = (i+1)*ySpan/7 + 0 - range*0.05;
            yLabels[i]->setText(QString::number(y, 'f', 1));
        }

        QString temp = fontAttrRed + "NG"+ fontAttrEnd + ".....";

        int timeStep = 7200*graphScale/72;
        //qDebug() << "timestep: " << timeStep << " count: " << count << " diff: " << dbThreadX->tempList[ 0 ].timeDiff;

        int count = 1;

        int size = dbThreadX->ngMeterList.size();
        //if (dbThreadX->ngMeterTableListAllDay)  size--;

        for (int i=size-1; i>0; i--){

            x1= dbThreadX->ngMeterList[i].timeDiff / graphScale;
            x2= dbThreadX->ngMeterList[i-1].timeDiff / graphScale;

            y1 = sceneHeight - (dbThreadX->ngMeterList[i].value - minValue) * yScale - min;
            y2 = sceneHeight - (dbThreadX->ngMeterList[i-1].value - minValue) * yScale - min;

            scene->addLine(x1, y1, x2, y2, penZoneGreen);
            scene->addEllipse(x1-2, y1-2, 4, 4, penZoneEst);
            scene->addEllipse(x2-2, y2-2, 4, 4, penZoneEst);

            if (dbThreadX->ngMeterList[i].timeDiff >= count*timeStep){
                count++;
                temp += " ," + fontAttrRed + QTime::fromString(dbThreadX->endTime, "hh:mm:ss").addSecs(-1*dbThreadX->ngMeterList[i].timeDiff).toString("hh:mm") + fontAttrEnd + " " + QString::number(dbThreadX->ngMeterList[i].value, 'f', 1);
            } else {
                temp += " ," + QString::number(dbThreadX->ngMeterList[i].value, 'f', 1);
            }
        }

        temp += " ," + QString::number(dbThreadX->ngMeterList[0].value, 'f', 1);
        ui->textBrowser->append(temp);

        drawGraphZonesFlag = true;
        //-------------------------------------------------------------------------------
        meterTable->setRowCount(dbThreadX->ngMeterTableList.size());
        meterTable->setColumnCount(6);
        meterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        //meterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        for (int y=0; y<size; y++){
            for (int x=0; x<dbThreadX->meterDataSize; x++){
                meterTable->setItem(y, x, new QTableWidgetItem( " " ) );
                meterTable->item(y, x)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            }

            meterTable->item(y, 0)->setText( dbThreadX->ngMeterTableList[y].date );
            meterTable->item(y, 1)->setText( dbThreadX->ngMeterTableList[y].time );
            meterTable->item(y, 2)->setText( QString::number(dbThreadX->ngMeterTableList[y].value, 'f', 1) );
            meterTable->item(y, 3)->setText( dbThreadX->ngMeterTableList[y].note );
            meterTable->item(y, 4)->setText( QString::number(dbThreadX->ngMeterTableList[y].ocr, 'f', 1) );

            QTableWidgetItem *item = new QTableWidgetItem(" ");
            item->setCheckState(Qt::Unchecked);
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            meterTable->setItem(y, dbThreadX->meterDataSize-1, item );
        }

        ui->paramTabs->setCurrentIndex(7);

        QString mainPath = "//"+clientAddress;
        fileOpenDir.setPath(mainPath+"/ngmeter-data/"+ui->dateEdit_BEGIN->date().toString("yyyy-MM/dd/"));

        filesInDirList = fileOpenDir.entryList(fileFilters, QDir::Files);
        if (filesInDirList.isEmpty()) {
            mainPath = "D:/Engineering/Repository Data/meter";
            fileOpenDir.setPath(mainPath+"/ngmeter-data/"+ui->dateEdit_BEGIN->date().toString("yyyy-MM/dd/"));
            filesInDirList = fileOpenDir.entryList(fileFilters, QDir::Files);
        }

        //qDebug() << fileOpenDir;
        imageFile.load(fileOpenDir.path()+"/"+filesInDirList.at(0));
        pic->setPixmap( QPixmap::fromImage( imageFile ).scaled( imageFile.width(), imageFile.height(), Qt::KeepAspectRatio));
        ui->meterValue->setText(QString::number(dbThreadX->ngMeterTableList[0].value, 'f', 1));
    }
}

void MainWindow::on_saveTempData_clicked(){

    clearGraphZones();
    tempSave = ui->checkBoxTempSave->isChecked();
    calcAvgTemp();
}

void MainWindow::saveTempData(){

    QString fileName = "temp--" +
            //ui->dateEdit_BEGIN->date().toString("yyMMdd") + "_" + ui->timeEdit_BEGIN->time().toString("hhmmss") + "-" +
            ui->dateEdit_END->date().toString("yyMMdd") + "_" + ui->timeEdit_END->time().toString("hhmmss") + ".csv";

    QFile file(fileName);

    QSqlRecord record;
    record = dbThreadX->qry.record();

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){

        if ( dbThreadX->qry.size() > 0 ) {

            //int colNum = record.count();

            QTextStream out(&file);
            out << "Date;Time;#;Temp" << endl;
            dbThreadX->qry.last();

            do {

                out << dbThreadX->qry.value(1).toString() << ";";
                out << dbThreadX->qry.value(2).toString() << ";";
                out << dbThreadX->qry.value(0).toString() << ";";
                out << dbThreadX->qry.value(3).toString().replace(".", ",");
                //for (int i=0; i<colNum;i++) out << dbThreadX->qry.value(i).toString() << ",";
                out << endl;
            } while (dbThreadX->qry.previous() && dbThreadX->qry.value(3).toString() != "-99" );

            file.close();

            qDebug() << "file saved";
        }
    }

    //tempSave = false;
}

QString MainWindow::getTimeStr(int val){

    int d = val / 86400;
    val = val - d*86400;
    int h = val / 3600;
    val = val - h*3600;
    int m = val / 60;
    val = val - m*60;
    int s = val;
    h += d*24;

    char timeInfo[10];
    sprintf (timeInfo, "%02d:%02d:%02d", h, m, s);

    QString time(timeInfo);

    return time;
}

void MainWindow::on_zoneGraphButton_clicked(){

    dbThreadX->beginDate = ui->dateEdit_BEGIN->date().toString("dd/MM/yy");
    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    dbThreadX->beginTime = ui->timeEdit_BEGIN->time().toString();
    dbThreadX->endTime = ui->timeEdit_END->time().toString();
    dbThreadX->verbose = false;

    QDateTime endDT;
    endDT.setDate(ui->dateEdit_END->date());
    endDT.setTime(ui->timeEdit_END->time());

    QDateTime beginDT;
    beginDT.setDate(ui->dateEdit_BEGIN->date());
    beginDT.setTime(ui->timeEdit_BEGIN->time());

    int timeDifference = beginDT.secsTo(endDT);

    if (timeDifference != 0)
        graphScale = 72.0 * timeDifference / 86400;
    else
        graphScale = 72;

    //qDebug() << beginDT.secsTo(endDT);

    ui->textBrowser->clear();
    dbThreadX->cmdAnalyzeZones = true;
    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();

}

void MainWindow::addAxisZones(){

    for (int i=1; i <zoneNumber+1; i++){
        scene->addLine(0, sceneZoneStep*i-1, xMax, sceneZoneStep*i-1, penAxisSolid);
    }

    for (int i=1; i <24; i++){
        scene->addLine(50*i-1, 0, 50*i-1, yMax, penAxisDash);
    }
}

void MainWindow::clearGraphZones(){

    scene->clear();
    addAxisZones();
    ui->graphicsView->show();
}

void MainWindow::drawGraphZones(){

    clearGraphZones();

    ui->textBrowser->append("-----");
    ui->labelBeginDate->setText( ui->dateEdit_BEGIN->text());
    ui->labelEndDate->setText( ui->dateEdit_END->text());

    for (int i=0; i<zoneNumber; i++){

        int yRef0 = sceneZoneStep*(i+1) - 1;
        int yRef1 = yRef0 - 20;
        int x1, x2, y, t1, t2, diff;

        QString fontStart = "";
        QString fonEnd = "";

        if (!dbThreadX->graphList[i].isEmpty()){

            QString temp = fontAttrRed + tableNames[i+1].toUpper() + fontAttrEnd + ".....";

            for (int j=0; j<dbThreadX->graphList[i].count()-1; j++){

                t1 = dbThreadX->graphList[i].at(j).timeDiff;
                x1 = t1 / graphScale;
                t2 = dbThreadX->graphList[i].at(j+1).timeDiff;
                x2 = t2 / graphScale;

                if (dbThreadX->graphList[i].at(j+1).state == 0){
                    y = yRef0;
                    fontStart = "";
                    fonEnd = "";
                } else {
                    y = yRef1;
                    fontStart = fontAttrRed;
                    fonEnd = fontAttrEnd;
                }

                scene->addLine(x1, y, x2, y, penZoneBlue);
                scene->addLine(x2, yRef0, x2, yRef1, penZoneBlue);

                diff = t2 - t1;
                if (diff < 3600)
                    temp += " ," + fontStart + QDateTime::fromTime_t(t2-t1).toUTC().toString("mm:ss") + fonEnd;
                else
                    temp += " ," + fontStart + QDateTime::fromTime_t(t2-t1).toUTC().toString("hh:mm:ss") + fonEnd;
            }
            ui->textBrowser->append(temp);
            //ui->textBrowser->append("...");

        }
    }

    QTime last, first;
    last = QTime::fromString(dbThreadX->endTime, "hh:mm:ss");

    timeLabels[0]->setText(ui->timeEdit_END->time().toString("hh:mm"));

    for (int i=1; i<13; i++){
        int backSecs = -7200*i*graphScale/72;
        first = last.addSecs(backSecs);
        timeLabels[i]->setText(first.toString("hh:mm"));
    }
    drawGraphZonesFlag = true;
    calcAvgTemp();

}

void MainWindow::drawSingleZone(){

    zoneTable();

    clearGraphZones();

    ui->textBrowser->append("-----");
    ui->labelBeginDate->setText( ui->dateEdit_BEGIN->text());
    ui->labelEndDate->setText( ui->dateEdit_END->text());


    int yRef0 = sceneZoneStep*(estimatedZone+1) - 1;
    int yRef1 = yRef0 - 20;
    int x1, x2, y, t1, t2, diff;

    QString fontStart = "";
    QString fonEnd = "";

    if (!dbThreadX->graphList[estimatedZone].isEmpty()){

        QString temp = fontAttrRed + tableNames[estimatedZone+1].toUpper() + fontAttrEnd + ".....";

        for (int j=0; j<dbThreadX->graphList[estimatedZone].count()-1; j++){

            t1 = dbThreadX->graphList[estimatedZone].at(j).timeDiff;
            x1 = t1 / graphScale;
            t2 = dbThreadX->graphList[estimatedZone].at(j+1).timeDiff;
            x2 = t2 / graphScale;

            if (dbThreadX->graphList[estimatedZone].at(j+1).state == 0){
                y = yRef0;
                fontStart = "";
                fonEnd = "";
            } else {
                y = yRef1;
                fontStart = fontAttrRed;
                fonEnd = fontAttrEnd;
            }

            scene->addLine(x1, y, x2, y, penZoneBlue);
            scene->addLine(x2, yRef0, x2, yRef1, penZoneBlue);

            diff = t2 - t1;
            if (diff < 3600)
                temp += " ," + fontStart + QDateTime::fromTime_t(t2-t1).toUTC().toString("mm:ss") + fonEnd;
            else
                temp += " ," + fontStart + QDateTime::fromTime_t(t2-t1).toUTC().toString("hh:mm:ss") + fonEnd;
        }
        ui->textBrowser->append(temp);
        //ui->textBrowser->append("...");

    }

    QTime last, first;
    last = QTime::fromString(dbThreadX->endTime, "hh:mm:ss");

    timeLabels[0]->setText(ui->timeEdit_END->time().toString("hh:mm"));

    for (int i=1; i<13; i++){
        int backSecs = -7200*i*graphScale/72;
        first = last.addSecs(backSecs);
        timeLabels[i]->setText(first.toString("hh:mm"));
    }
    //drawGraphZonesFlag = true;

    drawRoomEstimation();
}

void MainWindow::drawRoomEstimation(){

    ui->textBrowser->append("-----");

    int yRef0;// = sceneZoneStep*(estimatedZone+2) - 1;
    int yRef1;// = yRef0 - 20;
    int x1, x2, y;

    QTime initial = QTime::fromString(ui->timeEdit_END->text(), "hh:mm:ss");
    QTime end = QTime::fromString("23:59:59", "hh:mm:ss");
    qint64 offset = initial.secsTo(end);
    qint64 P1NormalModeTimeX = -1;
    qint64 P1ReducedModeTimeX = -1;
    qint64 P2NormalModeTimeX = -1;
    qint64 P2ReducedModeTimeX = -1;
    qint64 P3NormalModeTimeX = -1;
    qint64 P3ReducedModeTimeX = -1;

    if ( bd->prmArray[estimatedZone].P1NormalModeTime != "--:--:--" ){
        P1NormalModeTimeX = QTime::fromString(bd->prmArray[estimatedZone].P1NormalModeTime, "hh:mm:ss").secsTo( end ) - offset;
        P1ReducedModeTimeX = QTime::fromString(bd->prmArray[estimatedZone].P1ReducedModeTime, "hh:mm:ss").secsTo( end ) - offset;
    }

    if ( bd->prmArray[estimatedZone].P2NormalModeTime != "--:--:--" ){
        P2NormalModeTimeX = QTime::fromString(bd->prmArray[estimatedZone].P2NormalModeTime, "hh:mm:ss").secsTo( end ) - offset;
        P2ReducedModeTimeX = QTime::fromString(bd->prmArray[estimatedZone].P2ReducedModeTime, "hh:mm:ss").secsTo( end ) - offset;
    }

    if ( bd->prmArray[estimatedZone].P3NormalModeTime != "--:--:--" ){
        P3NormalModeTimeX = QTime::fromString(bd->prmArray[estimatedZone].P3NormalModeTime, "hh:mm:ss").secsTo( end ) - offset;
        P3ReducedModeTimeX = QTime::fromString(bd->prmArray[estimatedZone].P3ReducedModeTime, "hh:mm:ss").secsTo( end ) - offset;
    }
    /*
    qDebug() << offset;
    qDebug() << P1NormalModeTimeX << " " << P1ReducedModeTimeX;
    qDebug() << P2NormalModeTimeX << " " << P2ReducedModeTimeX;
    qDebug() << P3NormalModeTimeX << " " << P3ReducedModeTimeX;
    */

    float high = bd->prmArray[estimatedZone].normalHigh;
    float low = bd->prmArray[estimatedZone].reducedLow;

    float range = ( high - low );
    float z = 7 - range;
    float ySpan = range + z;// * 1.17;
    float yScale = sceneHeight / ySpan;
    float min = z / 2;
    //qDebug() << ":: " << range << " " << z << " " << ySpan << " " << yScale << " " << scene->height();

    float ys = 0;
    for (int i=0; i<7; i++){
        ys = (i+1)*ySpan/7 + low - min;// - range*0.05;
        yLabels[i]->setText(QString::number(ys, 'f', 1));
    }


    QList<graphData> refList;

    graphData start;
    bool startFlag = false;
    int endState = 0;

    if (P3ReducedModeTimeX >= 0){
        if (!startFlag){
            start.state = 0;
            start.timeDiff = 0;
            refList.append(start);
            startFlag = true;
        }
        graphData x;
        x.timeDiff = P3ReducedModeTimeX;
        x.state = 0;
        refList.append(x);
        endState = 1;
    }
    if (P3NormalModeTimeX >= 0){
        if (!startFlag){
            start.state = 1;
            start.timeDiff = 0;
            refList.append(start);
            startFlag = true;
        }
        graphData x;
        x.timeDiff = P3NormalModeTimeX;
        x.state = 1;
        refList.append(x);
        endState = 0;
    }
    if (P2ReducedModeTimeX >= 0){
        if (!startFlag){
            start.state = 0;
            start.timeDiff = 0;
            refList.append(start);
            startFlag = true;
        }
        graphData x;
        x.timeDiff = P2ReducedModeTimeX;
        x.state = 0;
        refList.append(x);
        endState = 1;
    }
    if (P2NormalModeTimeX >= 0){
        if (!startFlag){
            start.state = 1;
            start.timeDiff = 0;
            refList.append(start);
            startFlag = true;
        }
        graphData x;
        x.timeDiff = P2NormalModeTimeX;
        x.state = 1;
        refList.append(x);
        endState = 0;
    }
    if (P1ReducedModeTimeX >= 0){
        if (!startFlag){
            start.state = 0;
            start.timeDiff = 0;
            refList.append(start);
            startFlag = true;
        }
        graphData x;
        x.timeDiff = P1ReducedModeTimeX;
        x.state = 0;
        refList.append(x);
        endState = 1;
    }
    if (P1NormalModeTimeX >= 0){
        if (!startFlag){
            start.state = 1;
            start.timeDiff = 0;
            refList.append(start);
            startFlag = true;
        }
        graphData x;
        x.timeDiff = P1NormalModeTimeX;
        x.state = 1;
        refList.append(x);
        endState = 0;
    }

    if (refList.isEmpty()) {
        graphData x;
        x.timeDiff = 0;
        x.state = 0;
        refList.append(x);
        endState = 0;
    }

    yRef0 = sceneHeight - (bd->prmArray[estimatedZone].reducedSet - low) * yScale - min * yScale;
    yRef1 = sceneHeight - (bd->prmArray[estimatedZone].normalSet - low) * yScale - min * yScale;

    for (int j=0; j<refList.count()-1; j++){

        //qDebug() << refList[j].timeDiff;
        x1 = refList.at(j).timeDiff / graphScale;
        x2 = refList.at(j+1).timeDiff / graphScale;

        if (refList.at(j+1).state == 0){
            y = yRef0;
        } else {
            y = yRef1;
        }

        scene->addLine(x1, y, x2, y, penZone);
        scene->addLine(x2, yRef0, x2, yRef1, penZone);
    }

    if (refList.count() == 1) {
        scene->addLine(0, yRef0, scene->width(), yRef0, penZone);
    }

    //qDebug() << refList.last().timeDiff;

    if (endState == 0)
        y = yRef0;
    else
        y = yRef1;

    scene->addLine(x2, y, scene->width(), y, penZone);

//------------------
    int y1, t1, t2, diff;
    float val1;
    QString temp = "Estimated Temps.....";

    QStringList vals;
    for (int j=dbThreadX->graphList[estimatedZone].count()-1; j>0; j--){

        t1 = dbThreadX->graphList[estimatedZone].at(j).timeDiff;
        x1 = t1 / graphScale;
        t2 = dbThreadX->graphList[estimatedZone].at(j-1).timeDiff;
        x2 = t2 / graphScale;
        diff = t1 - t2;

        if (diff >=180) {

            if (dbThreadX->graphList[estimatedZone].at(j).state == 0){
            } else {

                val1 = 0;

                for (int j=0; j<refList.count()-1; j++){

                    if (t1 >= refList[j].timeDiff && t1 <= refList[j+1].timeDiff){
                        if (refList[j+1].state == 1)
                            val1 = calcEstValueNormal(estimatedZone, diff);
                        else
                            val1 = calcEstValueReduced(estimatedZone, diff);
                        break;
                    }
                }

                if (val1 == 0) {
                    if (refList.last().state == 1)
                        val1 = calcEstValueReduced(estimatedZone, diff);
                    else
                        val1 = calcEstValueNormal(estimatedZone, diff);
                }


                //val1 = calcEstValueNormal(estimatedZone, diff);
                y1 = sceneHeight - (val1 - low) * yScale - min * yScale;
                //qDebug() << t1 << " " << val1 << " " << low << " " << yScale << " " << min;
                //qDebug() << t1 << " " << dbThreadX->graphList[estimatedZone].at(j-1).timeDiff << " " << diff << " " << val1 << " " << y1;
                scene->addEllipse(x1-2, y1-2, 4, 4, penZoneEst);
                vals.append(QString::number(val1, 'f', 1));
            }
        }

//        scene->addLine(x1, y, x2, y, penZoneBlue);
//        scene->addLine(x2, yRef0, x2, yRef1, penZoneBlue);
    }

    for (int i=vals.count()-1; i>=0; i--){
        temp += vals.at(i) + " ,";
    }
    ui->textBrowser->append(temp);

}

void MainWindow::on_dateEdit_END_dateChanged(const QDate &date){
    if (date < ui->dateEdit_BEGIN->date())
        ui->dateEdit_BEGIN->setDate(date);

}

void MainWindow::on_zoneEstimateButton_clicked(){
    //estimatedZone = ui->comboBox->currentIndex();
    //qDebug() << estimatedZone;
    bd->prmArray[estimatedZone].propBandWidth = prmTables[estimatedZone]->item(0,0)->text().toInt();
    bd->prmArray[estimatedZone].cycleTime = prmTables[estimatedZone]->item(1,0)->text().toInt();
    bd->prmArray[estimatedZone].propBandPos = prmTables[estimatedZone]->item(2,0)->text().toInt();
    bd->prmArray[estimatedZone].normalSet = prmTables[estimatedZone]->item(3,0)->text().toFloat();
    bd->prmArray[estimatedZone].reducedSet = prmTables[estimatedZone]->item(4,0)->text().toFloat();
    bd->prmArray[estimatedZone].P1NormalModeTime = prmTables[estimatedZone]->item(5,0)->text();
    bd->prmArray[estimatedZone].P1ReducedModeTime = prmTables[estimatedZone]->item(6,0)->text();
    bd->prmArray[estimatedZone].P2NormalModeTime = prmTables[estimatedZone]->item(7,0)->text();
    bd->prmArray[estimatedZone].P2ReducedModeTime = prmTables[estimatedZone]->item(8,0)->text();
    bd->prmArray[estimatedZone].P3NormalModeTime = prmTables[estimatedZone]->item(9,0)->text();
    bd->prmArray[estimatedZone].P3ReducedModeTime = prmTables[estimatedZone]->item(10,0)->text();

    calcBandValues(estimatedZone);
    //int x = QTime::fromString("00:00:00","hh:mm:ss").secsTo( QTime::fromString(prmTables[estimatedZone]->item(10,0)->text(),"hh:mm:ss") );
    //qDebug() << calcEstValueNormal(estimatedZone, x) << " <N  R> " << calcEstValueReduced(estimatedZone, x);

    dbThreadX->beginDate = ui->dateEdit_BEGIN->date().toString("dd/MM/yy");
    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    dbThreadX->beginTime = ui->timeEdit_BEGIN->time().toString();
    dbThreadX->endTime = ui->timeEdit_END->time().toString();
    dbThreadX->verbose = false;

    QDateTime endDT;
    endDT.setDate(ui->dateEdit_END->date());
    endDT.setTime(ui->timeEdit_END->time());

    QDateTime beginDT;
    beginDT.setDate(ui->dateEdit_BEGIN->date());
    beginDT.setTime(ui->timeEdit_BEGIN->time());

    int timeDifference = beginDT.secsTo(endDT);

    if (timeDifference != 0)
        graphScale = 72.0 * timeDifference / 86400;
    else
        graphScale = 72;

    //qDebug() << beginDT.secsTo(endDT);

    //ui->textBrowser->clear();
    dbThreadX->cmdSingleZone = true;
    currentZone = estimatedZone+1;
    dbThreadX->currentZone = currentZone;
    dbThreadX->graphList[estimatedZone].clear();
    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();

}

void MainWindow::calcBandValues(int zone){
    bd->prmArray[zone].normalLow = bd->prmArray[zone].normalSet - (100 - bd->prmArray[zone].propBandPos)*bd->prmArray[zone].propBandWidth / 100.0;
    bd->prmArray[zone].normalHigh = bd->prmArray[zone].normalSet + bd->prmArray[zone].propBandPos*bd->prmArray[zone].propBandWidth / 100.0;
    bd->prmArray[zone].reducedLow = bd->prmArray[zone].reducedSet - (100 - bd->prmArray[zone].propBandPos)*bd->prmArray[zone].propBandWidth / 100.0;
    bd->prmArray[zone].reducedHigh = bd->prmArray[zone].reducedSet + bd->prmArray[zone].propBandPos*bd->prmArray[zone].propBandWidth / 100.0;
    //qDebug() << "NL: " << bd->prmArray[zone].normalLow << "NH: " << bd->prmArray[zone].normalHigh << "RL: " << bd->prmArray[zone].reducedLow << "RH: " << bd->prmArray[zone].reducedHigh;
    ui->textBrowser->append( "Normal Low: "+QString::number(bd->prmArray[zone].normalLow,'f',1)+" Normal High: "+QString::number(bd->prmArray[zone].normalHigh,'f',1)+" Reduced Low: "+QString::number(bd->prmArray[zone].reducedLow,'f',1)+" Reduced High: "+QString::number(bd->prmArray[zone].reducedHigh,'f',1));
}

float MainWindow::calcEstValueNormal(int zone, int inp){
    float rate = inp / (bd->prmArray[zone].cycleTime * 60.0);
    return ( bd->prmArray[zone].normalLow+(1-rate)*bd->prmArray[zone].propBandWidth);
}

float MainWindow::calcEstValueReduced(int zone, int inp){
    float rate = inp / (bd->prmArray[zone].cycleTime * 60.0);
    return ( bd->prmArray[zone].reducedLow+(1-rate)*bd->prmArray[zone].propBandWidth);
}

void MainWindow::on_comboBox_currentIndexChanged(int index){
    estimatedZone = index;
    //qDebug() << estimatedZone;
    ui->paramTabs->setCurrentIndex(index);
}

void MainWindow::_debug(){
    if (debugIncomingData)
        ui->textBrowser->append(serverx->datagram.data());

}

void MainWindow::on_testButton_clicked(){
    cout << "....s" << endl;

    if (clientx->clientSocket.state() == QAbstractSocket::ConnectedState){
        clientx->datagram.clear();
        clientx->datagram.append( ui->testEdit->text() );
        clientx->datagram.append( '\n' );
        cout << clientx->datagram.data() << endl;
        clientx->startTransfer();
    }
}

void MainWindow::on_ngMeterButton_clicked()
{
    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    dbThreadX->beginDate = ui->dateEdit_BEGIN->date().toString("dd/MM/yy");
    dbThreadX->endTime = ui->timeEdit_END->time().toString();
    dbThreadX->beginTime = ui->timeEdit_BEGIN->time().toString();
    dbThreadX->verbose = false;
    dbThreadX->cmdNgConsumption = true;

    progress->setWindowTitle("Sorgu Sonucu Bekleniyor");
    dbThreadX->start();

}

void MainWindow::updateNgConsumptionValue(float val)
{
    ui->ngMeterButton->setText(QString::number(val,'f',1)+" m3");
}

void MainWindow::on_tab8Table_itemSelectionChanged()
{
    //qDebug() << meterTable->currentRow();
    QDateTime date;
    date.setDate( QDate::fromString( dbThreadX->ngMeterTableList[meterTable->currentRow()].date, "dd/MM/yy") );
    date.setTime( QTime::fromString( dbThreadX->ngMeterTableList[meterTable->currentRow()].time, "hh:mm:ss") );

    QString mainPath = "//"+clientAddress;
    fileOpenDir.setPath(mainPath+"/ngmeter-data/"+ui->dateEdit_BEGIN->date().toString("yyyy-MM/dd/"));
    //qDebug() << fileOpenDir;

    filesInDirList = fileOpenDir.entryList(fileFilters, QDir::Files);
    if (filesInDirList.isEmpty()) {
        mainPath = "D:/Engineering/Repository Data/meter";
        fileOpenDir.setPath(mainPath+"/ngmeter-data/"+ui->dateEdit_BEGIN->date().toString("yyyy-MM/dd/"));
        filesInDirList = fileOpenDir.entryList(fileFilters, QDir::Files);
    }

    QString filePath= mainPath+"/ngmeter-data/20"+date.date().toString("yy-MM/dd/20")+date.date().toString("yyMMdd_")+date.time().toString("hhmmss.jpeg");
    //qDebug() << filePath;

    //imageFile.load(fileOpenDir.path()+"/"+filesInDirList.at(meterTable->currentRow()));
    imageFile.load(filePath);
    pic->setPixmap( QPixmap::fromImage( imageFile ).scaled( imageFile.width(), imageFile.height(), Qt::KeepAspectRatio));
    ui->meterValue->setText(QString::number(dbThreadX->ngMeterTableList[meterTable->currentRow()].value, 'f', 1));
    //ui->meterValue->raise();
}

void MainWindow::on_updateNgMeterData_clicked()
{
    if (filesInDirList.size() > 0) {
        dbThreadX->ngMeterUpdateList.clear();
        for (int i=0; i<filesInDirList.size(); i++) {
            if (meterTable->item(i, 5)->checkState() == Qt::Checked) {
                //qDebug() << meterTable->item(i, 2)->text();
                dbThreadX->appendMeterUpdateList(dbThreadX->ngMeterTableList[i].index, meterTable->item(i, 2)->text().toFloat());
            }
        }

        dbThreadX->cmdUpdateMeterValue = true;
        dbThreadX->start();
    }
}

void MainWindow::on_clearScene_clicked()
{
    clearGraph();
}

void MainWindow::on_checkNgMeterData_clicked()
{
    meterTable->setColumnCount(8);
    meterTable->setHorizontalHeaderLabels({"date","time","value","note","ocr","upd","diff","Tdiff"});
    int size = dbThreadX->ngMeterList.size();
    //if (dbThreadX->ngMeterTableListAllDay)  size--;

    QList<bool> issueList;
    issueList.append(false);

    for (int y=0; y<size; y++){

        float diff = 0;
        int diffTime = 0;

        if (y>0) {

            QDateTime currentDT;
            currentDT.setDate( QDate::fromString(dbThreadX->ngMeterTableList[y].date, "dd/MM/yy") );
            currentDT.setTime( QTime::fromString(dbThreadX->ngMeterTableList[y].time, "hh:mm:ss") );

            int yy = y;
            bool cont = true;
            do {
                yy--;
                if (!issueList.at(yy)) {
                    QDateTime prevDT;
                    prevDT.setDate( QDate::fromString(dbThreadX->ngMeterTableList[yy].date, "dd/MM/yy") );
                    prevDT.setTime( QTime::fromString(dbThreadX->ngMeterTableList[yy].time, "hh:mm:ss") );

                    diffTime = prevDT.secsTo(currentDT);
                    diff = dbThreadX->ngMeterTableList[y].ocr - dbThreadX->ngMeterTableList[yy].ocr;
                    cont = false;
                }
            } while (y > 0 && cont);
        }

        meterTable->setItem(y, 6, new QTableWidgetItem( QString::number(diff, 'f', 1) ) );
        meterTable->item(y, 6)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        meterTable->setItem(y, 7, new QTableWidgetItem( QString::number(diffTime) ) );
        meterTable->item(y, 7)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        float n = dbThreadX->ngMeterTableList[y].ocr;
        if (n==0)   n=1;
        int digitNo = qFloor(log10(n))+1;

        if ( diff < 0 || diff > (1 * diffTime/3600.0) ) {
            if (digitNo>4) {
                dbThreadX->ngMeterTableList[y].ocr = n - qFloor(n/10000) * 10000;
                meterTable->item(y, 2)->setText( QString::number(dbThreadX->ngMeterTableList[y].ocr, 'f', 1) );
            }
            issueList.append(true);
            meterTable->item(y, 3)->setText( "error" );
            //meterTable->item(y, 6)->setBackground(QBrush(QColor(255, 0, 0)));
            //meterTable->item(y, 6)->setForeground(QBrush(QColor(255, 0, 0)));
        } else {
            if (y!=0) issueList.append(false);
        }

    }


}
