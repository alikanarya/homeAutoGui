#ifndef BEGINDIALOG_H
#define BEGINDIALOG_H

#include <QDialog>
#include <QtCore>

namespace Ui {
    class begindialog;
}

#define INIFILENAME         "settings.ini"
#define _CLIENT_ADR         "localhost"
#define _DEFAULT_ADR        0
#define _CLIENT_PORT        8888
#define _DB_NAME            "homeAutoDB"
#define _DB_USER            "ali"
#define _DB_PASS            "reyhan"

extern QString clientAddress;
extern quint16 clientPort;
extern QString dbName;
extern QString dbUser;
extern QString dbPass;



class begindialog : public QDialog {
    Q_OBJECT

public:

    explicit begindialog(QWidget *parent = 0);
    ~begindialog();
    bool readSettings();

    QSettings *settings;                // settings: to read/write ini file

    QString clientAddress1 = "";
    QString clientAddress2 = "";
    QString clientAddress3 = "";
    QString clientAddress4 = "";
    QString clientAddress5 = "";
    int defaultAddr = 0;

    bool state = false;

    struct controllerPrms {
        int propBandWidth = 0;
        int cycleTime = 0;
        int propBandPos = 0;
        QString P1NormalModeTime = "--:--:--";
        QString P1ReducedModeTime = "--:--:--";
        QString P2NormalModeTime = "--:--:--";
        QString P2ReducedModeTime = "--:--:--";
        QString P3NormalModeTime = "--:--:--";
        QString P3ReducedModeTime = "--:--:--";
    };

    controllerPrms prmArray[7];

private slots:
    void on_okButton_clicked();

private:
    Ui::begindialog *ui;
};

#endif // BEGINDIALOG_H
