#ifndef GLOBALS_H
#define GLOBALS_H

#include "server.h"
#include "client.h"

using namespace std;

#define INIFILENAME         "settings.ini"
#define _CLIENT_ADR         "localhost"
#define _CLIENT_PORT        8888
#define _DB_NAME            "homeAutoDB"
#define _DB_USER            "ali"
#define _DB_PASS            "reyhan"

QString MSG_SERVER_INIT = "Server is listening...";
QString MSG_CLIENT_CONN = "Client connected";
QString MSG_HI = "Hi by Windows";
QString MSG_BBB_CON_YES = "BBB+";
QString MSG_BBB_CON_NO = "BBB-";
QString MSG_DB_CON_YES = "DB+";
QString MSG_DB_CON_NO = "DB-";

extern const int dInpSize = 8;
extern const int dOutSize = 4;
extern const int aInpSize = 7;
extern const int aOutSize = 4;

char dInpArr[dInpSize+1];
int dInpGpioMap[dInpSize] = {45, 44, 26, 47, 46, 27, 65, 61};

int dInpNum = 7;
extern const int zoneNumber = 7;

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

int statTotalActiveZones[zoneNumber+1];
int statTotalActiveZonesDurations[zoneNumber+1];

QString zoneFileNames[8] = {"zones.csv", "z1_oto.csv", "z2_sln.csv", "z3_blk.csv", "z4_mut.csv", "z5_eyo.csv", "z6_cyo.csv", "z7_yod.csv"};

//QString clientAddress = "192.168.2.232"; // BBB ***
//QString clientAddress = "192.168.1.235"; // BBB ***
//QString clientAddress = "192.168.1.124"; // BBBak
//QString clientAddress = "192.168.2.241"; // DebianVB
//QString clientAddress = "192.168.2.222"; // Windows

#endif // GLOBALS_H
