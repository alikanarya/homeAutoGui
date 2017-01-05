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


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){

    ui->setupUi(this);

    ui->dateEdit_BEGIN->setDate(QDate::currentDate());
    ui->dateEdit_END->setDate(QDate::currentDate());

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
    connect (dbThreadX, SIGNAL(started()), progress, SLOT(threadStarted()));
    connect (dbThreadX, SIGNAL(finished()), progress, SLOT(threadFinished()));
    //progress->setWindowModality(Qt::WindowModal);

    dbThreadX->cmdConnect = true;
    dbThreadX->start();

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

    aInpArrVal[1] = aInpArr[1] / 22.755555;
    ui->ai1->setText(QString::number(aInpArrVal[1],'f',1));
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

    dbThreadX-> beginDate = ui->dateEdit_BEGIN->date().toString("dd/MM/yy");
    dbThreadX->endDate = ui->dateEdit_END->date().toString("dd/MM/yy");
    dbThreadX->beginTime = ui->timeEdit_BEGIN->time().toString();
    dbThreadX->endTime = ui->timeEdit_END->time().toString();
    dbThreadX->verbose = true;
    dbThreadX->cmdAnalyzeAllZones = true;
    dbThreadX->start();
}

/*
QTime last = QTime::fromString(timeList.at(0), "hh:mm:ss");
QTime first = QTime::fromString(timeList.at(1), "hh:mm:ss");
qint64 diff = first.msecsTo(last) / 1000;
qDebug() << last.toString() << " - " << first.toString() << " is " << diff;
*/
