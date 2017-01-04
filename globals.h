#ifndef GLOBALS_H
#define GLOBALS_H

#include "server.h"
#include "client.h"

#define INIFILENAME         "settings.ini"
#define _CLIENT_ADR         "localhost"
#define _CLIENT_PORT        8888
#define _DB_NAME            "homeAutoDB"
#define _DB_USER            "ali"
#define _DB_PASS            "reyhan"

QString MSG_SERVER_INIT = "Server is listening...";
QString MSG_CLIENT_CONN = "Client connected";
QString MSG_HI = "Hi by Windows";
QString MSG_CONN_YES = "Connected";
QString MSG_CONN_NO = "DisConnected";

extern const int dInpSize = 8;
extern const int dOutSize = 4;
extern const int aInpSize = 7;
extern const int aOutSize = 4;

char dInpArr[dInpSize+1];
int dInpGpioMap[dInpSize] = {45, 44, 26, 47, 46, 27, 65, 61};

int dInpNum = 7;

char dOutArr[dOutSize+1];
char dOutReadArr[dOutSize+1];
int dOutGpioMap[dOutSize] = {66, 67, 69, 68};

int aInpArr[aInpSize];
float aInpArrVal[aInpSize];

int aOutArr[aOutSize];

QString clientAddress = "";
quint16 clientPort = 0;
QString dbName;
QString dbUser;
QString dbPass;

//QString clientAddress = "192.168.2.232"; // BBB ***
//QString clientAddress = "192.168.1.235"; // BBB ***
//QString clientAddress = "192.168.1.124"; // BBBak

//QString clientAddress = "192.168.2.241"; // DebianVB
//QString clientAddress = "192.168.2.222"; // Windows

#endif // GLOBALS_H
