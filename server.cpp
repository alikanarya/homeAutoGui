#include "server.h"
#include "client.h"

#include <iostream>
#include <unistd.h>

using namespace std;

extern QString MSG_SERVER_INIT;
extern int dInpSize;
extern int dInpSizeUsed;
extern char dInpArr[];
extern int dOutSize;
extern const int dOutSizeUsed;
extern char dOutReadArr[];
extern int aInpArr[];
extern const int aInpSizeUsed;
extern int dataBufferSizeMax;
extern int dataBufferSizeMin;

// remote station 1
extern const int dInpSize_R1;
extern const int dOutSize_R1;
extern const int aInpSize_R1;
extern const int aOutSize_R1;
extern const int dataBufferSizeMax_R1;
extern const int dataBufferSizeMin_R1;
extern char dInpArr_R1[];
extern bool dInpArr_R1_bool[];
extern int aInpArr_R1[];
extern QString rs1Hour;
extern QString rs1Min;
extern QString rs1Sec;

Server::Server(QObject* parent): QObject(parent){
}

void Server::init(){
    connect(&server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    server.listen(QHostAddress::Any, serverPort);
    cout << MSG_SERVER_INIT.toUtf8().constData() << serverPort << endl;
}

Server::~Server(){

    server.close();
}

void Server::acceptConnection(){

    //cout <<"acceptConnection" << endl;    //DBG
    client = server.nextPendingConnection();
    connect(client, SIGNAL(readyRead()), this, SLOT(startRead()));
}

void Server::startRead(){

    datagram.clear();
    //while (client->waitForReadyRead(300)) {
    //cout << client->bytesAvailable() << "--";
    while (client->bytesAvailable() > 0) {
        datagram.append(client->readAll());
        client->flush();
    }

    emit readFinished();

    //}

    switch (datagram.at(0)) {
        case 'A' : frameRemote1IO(); break;     // remote station 1
        default: frameMainIO(); break;          // main station
    }
    //client->close();
}

void Server::frameMainIO(){

    bool validData = !datagram.isEmpty() && datagram.size() >= dataBufferSizeMin && datagram.size() <= dataBufferSizeMax && datagram.at(datagram.size()-1) == 'Z';

    // DI (&& DO) buffer check
    if (validData) {
        for (int k=0; k<dInpSizeUsed; k++) {   // +dOutSizeUsed
            if ( !QChar(datagram.at(k)).isDigit() ) {
                validData = false;
                cout << datagram.at(k);
            }
        }
    }

    if (validData) {

        emit dataValid();

        for (int i = 0; i < dInpSizeUsed; i++) {
            dInpArr[i] = datagram.data()[i];
        }

        //for (int i = 0; i < dOutSize; i++) { dOutReadArr[i] = datagram.data()[i + dInpSize]; }

        int pos = dInpSizeUsed;         //+dOutSizeUsed;
        char ch = datagram.at(pos);
        //cout << ch << endl;       //DBG
        int j = 0, x = 0;

        while (ch != 'Z') {
            //x = 0;
            if (ch == 'A') {
                char temp[16];
                j = -1;
                do {
                    pos++;
                    ch = datagram.at(pos);
                    //cout << " " << ch;
                    if (ch == 'Z') break;
                    if (ch == 'A') break;
                    j++;
                    temp[j] = ch;

                } while ( ch != 'Z' );

                temp [j+1] = '\0';
                if ( x < aInpSizeUsed ) {
                    aInpArr[x] = atoi(temp);
                    //cout << " " << aInpArr[x];
                } else
                    break;
                x++;
            }
        }
        //cout << endl;
        emit this->mainIO();
    } else {
        if (!datagram.isEmpty()) {
            cout << "invalid data-main: " << datagram.data() << endl;      //DBG
            emit dataInValid();
        }
    }

}

void Server::frameRemote1IO(){
    // A h h m m s s  1  1  1  1  1  1  1  A  x  A  x  A  x  Z \0
    // 0 1 2 3 4 5 6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21
    bool validData = !datagram.isEmpty() && datagram.size() >= dataBufferSizeMin_R1 && datagram.size() <= dataBufferSizeMax_R1;

    if (datagram.size() >=2 )
        validData = (datagram.at(datagram.size()-2) == 'Z');
    else
        validData = false;

    // DI (&& DO) buffer check
    if (validData) {
        for (int k=1; k<=dInpSize_R1; k++) {   // +dOutSizeUsed
            if ( !QChar(datagram.at(k+7)).isDigit() ) {
                validData = false;
                cout << datagram.at(k);
            }
        }
    }

    if (validData) {

        emit dataValid();

        char tempChArr[3];
        tempChArr[0] = datagram.at(1); tempChArr[1] = datagram.at(2); tempChArr[2] = '\0';
        rs1Hour=  QString::fromUtf8(tempChArr);
        tempChArr[0] = datagram.at(3); tempChArr[1] = datagram.at(4); tempChArr[2] = '\0';
        rs1Min=  QString::fromUtf8(tempChArr);
        tempChArr[0] = datagram.at(5); tempChArr[1] = datagram.at(6); tempChArr[2] = '\0';
        rs1Sec=  QString::fromUtf8(tempChArr);
        //cout << rs1Hour.toUtf8().constData() << "-" << rs1Min.toUtf8().constData() << "-" << rs1Sec.toUtf8().constData() << endl;

        for (int i = 0; i < dInpSize_R1; i++) {
            dInpArr_R1[i] = datagram.data()[i+7];
            dInpArr_R1_bool[i] = (dInpArr_R1[i] == '0') ? false : true;
        }

        //for (int i = 0; i < dOutSize; i++) { dOutReadArr[i] = datagram.data()[i + dInpSize]; }

        int pos = dInpSize_R1+7;         //+dOutSizeUsed;
        char ch = datagram.at(pos);
        //cout << ch << endl;       //DBG
        int j = 0, x = 0;

        while (ch != 'Z') {
            //x = 0;
            if (ch == 'A') {
                char temp[16];
                j = -1;
                do {
                    pos++;
                    ch = datagram.at(pos);
                    //cout << " " << ch;
                    if (ch == 'Z') break;
                    if (ch == 'A') break;
                    j++;
                    temp[j] = ch;

                } while ( ch != 'Z' );

                temp [j+1] = '\0';
                if ( x < aInpSize_R1 ) {
                    aInpArr_R1[x] = atoi(temp);
                    //cout << " " << aInpArr[x];
                } else
                    break;
                x++;
            }
        }
        //cout << endl;
        emit this->remote1IO();
    } else {
        if (!datagram.isEmpty()) {
            cout << "invalid data-remote1: " << datagram.size() << datagram.data() << endl;      //DBG
            emit dataInValid();
        }
    }

}

/* old fn
void Server::startRead(){
    datagram.clear();
    while (client->bytesAvailable())
        datagram.append(client->readAll());
    //qDebug() << datagram.data();// << endl;      //DBG

    for (int i = 0; i < dInpSizeUsed; i++) { dInpArr[i] = datagram.data()[i]; }

    //for (int i = 0; i < dOutSize; i++) { dOutReadArr[i] = datagram.data()[i + dInpSize]; }

    int pos = dInpSizeUsed;//+dOutSize;
    char ch = datagram.at(pos);
    //cout << ch << endl;      //DBG

    int j = 0, x = 0;
    while (ch != 'Z') {
        //x = 0;
        if (ch == 'A') {
            char temp[16];
            j = -1;
            do {
                pos++;
                ch = datagram.at(pos);
                //cout << " " << ch;
                if (ch == 'Z') break;
                if (ch == 'A') break;
                j++;
                temp[j] = ch;

            } while ( ch != 'Z' );

            temp [j+1] = '\0';
            aInpArr[x] = atoi(temp);
            //cout << " " << aInpArr[i];
            x++;
        }
    }
    //cout << endl;
    emit this->readFinished();
    //client->close();
}
*/
