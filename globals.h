#ifndef GLOBALS_H
#define GLOBALS_H

#include "server.h"
#include "client.h"

using namespace std;

#define INIFILENAME         "settings.ini"
#define _CLIENT_ADR         "localhost"
#define _CLIENT_PORT        8889
#define _SERVER_PORT        8888
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

bool debugIncomingData = false;
extern const int dInpSize = 8;
int dInpSizeUsed = 7;
extern const int dOutSize = 4;
extern const int dOutSizeUsed = 0;
extern const int aInpSize = 7;
extern const int aInpSizeUsed = 2;
extern const int aOutSize = 4;
extern const int dataBufferSizeMax = 19;   // dInpSize+dOutSizeUsed+aInpSizeUsed*5{A+4digit}+1{Z}
extern const int dataBufferSizeMin = 13;   // dInpSize+dOutSizeUsed+aInpSizeUsed*2{A+1digit}+1{Z}

// remote station 1
extern const int dInpSize_R1 = 7;
extern const int dOutSize_R1 = 0;
extern const int aInpSize_R1 = 3; // Boiler Temp, Wifi Qual, Boiler Set Temp
extern const int aOutSize_R1 = 0;
extern const int dataBufferSizeMax_R1 = 31;     // 1{A}+Time{hhmmss)+dInpSize+dOutSize+aInpSize*5{A+4digit}+1{Z}
extern const int dataBufferSizeMin_R1 = 22;     // 1{A}+Time{hhmmss)+dInpSize+dOutSize+aInpSize*2{A+1digit}+1{Z}
QString rs1Hour =  "";
QString rs1Min = "";
QString rs1Sec = "";

char dInpArr[dInpSize+1];
int dInpGpioMap[dInpSize] = {45, 44, 26, 47, 46, 27, 65, 61};

extern const int zoneNumber = 7;

char dOutArr[dOutSize+1];
char dOutReadArr[dOutSize+1];
int dOutGpioMap[dOutSize] = {66, 67, 69, 68};

int aInpArr[aInpSize];
float aInpArrVal[aInpSize];

int aOutArr[aOutSize];

char dInpArr_R1[dInpSize_R1+1];
bool dInpArr_R1_bool[dInpSize_R1];
int aInpArr_R1[aInpSize_R1];

QString clientAddress = "";
quint16 clientPort = 0;
int serverPort = 0;
QString dbName;
QString dbUser;
QString dbPass;

int statTotalActiveZones[zoneNumber+1];
int statTotalActiveZonesDurations[zoneNumber+1];
int totalTime;
float statTotalActiveZonesPercent[zoneNumber+1];

QString zoneFileNames[7] = {"z1_oto--", "z2_sln--", "z3_blk--", "z4_mut--", "z5_eyo--", "z6_cyo--", "z7_yod--"};
QString zoneAllFileName = "zoneAll--";
QString zonesFileName = "zones.csv";
QString tableNames[8] = {"zones", "oto", "sln", "blk", "mut", "eyo", "cyo", "yod"};
QString tableSummary = "summary";
QString tableNgValue = "gas_reading";
QString tblSummaryFiels[19] =
    {"index", "date",
     "0_zone_rate", "1_zone_rate", "2_zone_rate", "3_zone_rate", "4_zone_rate", "5_zone_rate", "6_zone_rate", "7_zone_rate",
     "0_zone_thr_count", "0_zone_thr_time",
     "on_rate_oto", "on_rate_sln", "on_rate_blk", "on_rate_mut", "on_rate_eyo", "on_rate_cyo", "on_rate_yod" };

int loadFactors[7] = { 1, 3, 1, 2, 2, 2, 2};

//QString clientAddress = "192.168.2.232"; // BBB ***
//QString clientAddress = "192.168.1.235"; // BBB ***
//QString clientAddress = "192.168.1.124"; // BBBak
//QString clientAddress = "192.168.2.241"; // DebianVB
//QString clientAddress = "192.168.2.222"; // Windows


#endif // GLOBALS_H
