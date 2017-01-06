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


class dbThread: public QThread{

    Q_OBJECT

public:

    dbThread();
    ~dbThread();

    void stop();
    void connectToDB();
    void analyzeAllZones();

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    QSqlQuery qry;
    QList<int> totalActiveZoneList;

    QString beginDate;
    QString endDate;
    QString beginTime;
    QString endTime;

    bool verbose = false;
    bool cmdConnect = false;
    bool cmdAnalyzeAllZones = false;

public slots:


protected:

    void run();

private:

    volatile bool stopped;

signals:

    void connected();
    void unconnected();
    void allZonesProcessed();

};

#endif // DBTHREAD_H
