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
extern int statTotalActiveZonesPercent[];

extern QString tableNames[];

class dbThread: public QThread{

    Q_OBJECT

public:

    dbThread();
    ~dbThread();

    void stop();
    void connectToDB();
    void analyzeAllZones();
    void analyzeZone();

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

};

#endif // DBTHREAD_H
