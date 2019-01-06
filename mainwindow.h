#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QDebug>
#include <QProgressDialog>
//#include <QtGui>
//#include <QPixmap>
#include <QGraphicsScene>
#include <QTableWidget>

namespace Ui {
class MainWindow;
}

class ThreadProgressDialog;

class MainWindow : public QMainWindow{

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool readSettings();
    void zoneQuery(int zoneNumber);

    void addAxis();
    void clearGraph();
    void addAxisZones();
    void clearGraphZones();
    void saveTempData();
    QString getTimeStr(int val);
    void calcBandValues(int zone);
    float calcEstValueNormal(int zone, int inp);
    float calcEstValueReduced(int zone, int inp);
    void drawRoomEstimation();

    QSettings *settings;                // settings: to read/write ini file
    ThreadProgressDialog *progress;

    QFont fontBold;

    QGraphicsScene *scene;
    QPen penAxisSolid;
    QPen penAxisDash;
    QPen penZone;
    QPen penZoneBlue;
    QPen penZoneGreen;
    QPen penZoneEst;
    QRect sceneRect;
    int sceneWidth, sceneHeight, xMax, yMax;
    float graphScale = 3;

    int sceneZoneStep;

    QLabel *timeLabels[13];
    QLabel *yLabels[7];
    QStringList zoneTableHeader;
    bool drawGraphZonesFlag = false;

    QString fontAttrRed = "<font color=\"red\">";
    QString fontAttrEnd = "</font>";

    int estimatedZone = 0;
    QTableWidget *prmTables[7];

    struct graphData {
        int timeDiff;
        int state;
    };

private:

    Ui::MainWindow *ui;
    QUdpSocket *udpSocket;
    bool reportToDB = false;
    bool tempSave = false;

private slots:

    void ConnectedToServer();
    void NotConnectedToServer();
    void connectedToDB();
    void unconnectedToDB();

    void displayInputs();
    void displayInputsR1(); // for remote station 1
    void transferData();
    void allZonesTable();
    void zoneTable();
    void summaryResult();
    void drawGraph();
    void drawGraphZones();
    void drawSingleZone();
    void updateTemp();
    void updateTempGUI();
    void calcAvgTemp();
    void avgTempGUI();

    void on_pushButton_clicked();
    void on_saveAllZonesButton_clicked();
    void on_zoneButton_clicked();
    void on_zoneSaveButton_clicked();
    void on_timeUpButton_clicked();
    void on_timeDownButton_clicked();
    void on_report2DBButton_clicked();
    void on_graphUpdateButton_clicked();
    void on_forwardLargeButton_clicked();
    void on_forwardSmallButton_clicked();
    void on_backwardLargeButton_clicked();
    void on_backwardSmallButton_clicked();
    void on_timeNow1Button_clicked();
    void on_timeNow2Button_clicked();

    void on_saveTempData_clicked();
    void on_yesterdayButton_clicked();
    void on_zoneGraphButton_clicked();
    void on_dateEdit_END_dateChanged(const QDate &date);
    void on_zoneEstimateButton_clicked();
    void on_comboBox_currentIndexChanged(int index);

    void _debug();

    void on_testButton_clicked();

signals:

    void connectTo();
    void sendData();
};




class ThreadProgressDialog : public QProgressDialog {

    Q_OBJECT

public:

    explicit ThreadProgressDialog(QWidget* parent = 0) : QProgressDialog(parent), m_timer(new QTimer(this)) {

        setRange(0, 0);
        setCancelButton(0);
//        setWindowFlags(this->windowFlags() & ~Qt::WindowCloseButtonHint);
//        setWindowFlags(Qt:: Dialog | Qt:: FramelessWindowHint | Qt:: WindowTitleHint | Qt:: CustomizeWindowHint);
        setWindowFlags(Qt:: Dialog | Qt:: WindowTitleHint | Qt:: CustomizeWindowHint);
        setWindowModality(Qt::WindowModal);
        m_timer->setInterval(1000);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(advance()));
    }

public slots:

    void threadStarted() {
        m_timer->start();
        show();
    }

    void threadFinished() {
        m_timer->stop();
        hide();
    }

private:

    QTimer *m_timer;

private slots:

    void advance() {
        setValue(value() + 1);
    }

};

#endif // MAINWINDOW_H
