#ifndef DBTHREAD_H
#define DBTHREAD_H

#include <QThread>
#include <QtSql>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>


extern QString clientAddress;
extern QString dbName;
extern QString dbUser;
extern QString dbPass;
extern const int zoneNumber;
extern int statTotalActiveZones[];
extern int statTotalActiveZonesDurations[];
extern int totalTime;
extern float statTotalActiveZonesPercent[];

extern QString tableNames[];
extern QString tableSummary;
extern QString tblSummaryFiels[];

class dbThread: public QThread{

    Q_OBJECT

public:

    dbThread();
    ~dbThread();

    void stop();
    void connectToDB();
    void analyzeAllZones();
    void analyzeZone();
    void summaryReport();
    void insertToSummaryTable();
    void analyzeZonesForGraph();
    void getGraphData();
    void getTemperature();
    void getAvgTemperature();
    void analyzeZones();
    void ngConsumption();

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    QSqlQuery qry;
    //QList<int> totalActiveZoneList;
    QList<int> timeDiffList;
    QList<int> timeDiffListOn;
    QList<int> timeDiffListOff;
    QList<int> stateList;
    QList<int> thresholdIndexList;

    QString beginDate;
    QString endDate;
    QString beginTime;
    QString endTime;
    QString beginTimeDelimiter;
    QString endTimeDelimiter;
    QString queryBeginTime;

    bool delimiterEncountered;
    int currentZone = 0;
    int ONtime;
    int OFFtime;
    int ONcount;
    int OFFcount;
    int zeroStateLowThreshold = 0;
    int zeroStateLowTime = 0;
    int zeroStateLowCount = 0;
    int zeroStateHighTime = 0;
    int zeroStateHighCount = 0;

    bool verbose = false;
    bool cmdConnect = false;
    bool cmdAnalyzeAllZones = false;
    bool cmdAnalyzeZone = false;
    bool cmdSummaryReport = false;
    bool cmdInsertToSummaryTable = false;
    bool cmdGraphData = false;
    bool cmdTempData = false;
    bool cmdAvgTempData = false;
    bool cmdAnalyzeZones = false;
    bool cmdSingleZone = false;
    bool cmdNgConsumption = false;

    struct summaryTable {
        QString date;
        float zone0_rate;
        float zone1_rate;
        float zone2_rate;
        float zone3_rate;
        float zone4_rate;
        float zone5_rate;
        float zone6_rate;
        float zone7_rate;
        int zone0_thr_count;
        QString zone0_thr_time;
        float on_rate_oto;
        float on_rate_sln;
        float on_rate_blk;
        float on_rate_mut;
        float on_rate_eyo;
        float on_rate_cyo;
        float on_rate_yod;
        int on_count_oto;
        int on_count_sln;
        int on_count_blk;
        int on_count_mut;
        int on_count_eyo;
        int on_count_cyo;
        int on_count_yod;
        int on_time_oto;
        int on_time_sln;
        int on_time_blk;
        int on_time_mut;
        int on_time_eyo;
        int on_time_cyo;
        int on_time_yod;
        float tempAvg;
        float tempMin;
        float tempMax;
        float ngMeter;
    } summaryData;

    struct graphData {
        int timeDiff;
        int state;
    };

    struct tempData {
        int timeDiff;
        float value;
    };

    QList<graphData> graphList[7];

    float tempOut = -99;
    float tempAvg = -99;
    float tempMin = -99;
    float tempMax = 99;
    QString tempDate="", tempTime="";

    QList<tempData> tempList;
    QList<tempData> ngMeterList;

public slots:


protected:

    void run();

private:

    volatile bool stopped;

signals:

    void connected();
    void unconnected();
    void allZonesProcessed();
    void zoneProcessed();
    void summaryReportProcess();
    void summaryReportDone();
    void graphDataDone();
    void tempOutDone();
    void tempOutAvgDone();
    void analyzeZonesDone();
    void singleZoneDone();
    void ngConsumptionDone(float);
};

#endif // DBTHREAD_H
