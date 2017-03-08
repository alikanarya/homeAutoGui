#include "begindialog.h"
#include "ui_begindialog.h"

extern bool appState;

begindialog::begindialog(QWidget *parent) : QDialog(parent), ui(new Ui::begindialog){
/*
    // only title & close button on title bar
    Qt::WindowFlags flags = 0;
    flags |= Qt::Dialog;
    flags |= Qt::WindowSystemMenuHint;
    flags |= Qt::WindowTitleHint;
    this->setWindowFlags(flags);
    this->setAttribute(Qt::WA_DeleteOnClose, true);
*/
    settings = new QSettings(INIFILENAME, QSettings::IniFormat);
    readSettings();

    ui->setupUi(this);
    ui->comboBox->addItem(clientAddress1);
    ui->comboBox->addItem(clientAddress2);
    ui->comboBox->addItem(clientAddress3);
    ui->comboBox->addItem(clientAddress4);
    ui->comboBox->addItem(clientAddress5);
    ui->comboBox->setCurrentIndex(defaultAddr);
}

begindialog::~begindialog(){
    delete ui;
}

bool begindialog::readSettings(){

    if (QFile::exists(INIFILENAME)){

        defaultAddr = settings->value("defaultAddr", _DEFAULT_ADR).toInt();
        clientAddress1 = settings->value("clientAddress0", _CLIENT_ADR).toString();
        clientAddress2 = settings->value("clientAddress1", _CLIENT_ADR).toString();
        clientAddress3 = settings->value("clientAddress2", _CLIENT_ADR).toString();
        clientAddress4 = settings->value("clientAddress3", _CLIENT_ADR).toString();
        clientAddress5 = settings->value("clientAddress4", _CLIENT_ADR).toString();
        clientPort = settings->value("clientPort", _CLIENT_PORT).toInt();
        dbName = settings->value("dbName", _DB_NAME).toString();
        dbUser = settings->value("dbUser", _DB_USER).toString();
        dbPass = settings->value("dbPass", _DB_PASS).toString();
        prmArray[0].propBandWidth = settings->value("propBandWidth_Z1", 5).toInt();
        prmArray[0].cycleTime = settings->value("cycleTime_Z1", 21).toInt();
        prmArray[0].propBandPos = settings->value("propBandPos_Z1", 50).toInt();
        prmArray[0].P1NormalModeTime = settings->value("P1NormalModeTime_Z1", "--:--:--").toString();
        prmArray[0].P1ReducedModeTime = settings->value("P1ReducedModeTime_Z1", "--:--:--").toString();
        prmArray[0].P2NormalModeTime = settings->value("P2NormalModeTime_Z1", "--:--:--").toString();
        prmArray[0].P2ReducedModeTime = settings->value("P2ReducedModeTime_Z1", "--:--:--").toString();
        prmArray[0].P3NormalModeTime = settings->value("P3NormalModeTime_Z1", "--:--:--").toString();
        prmArray[0].P3ReducedModeTime = settings->value("P3ReducedModeTime_Z1", "--:--:--").toString();

        //cout << clientAddress.toUtf8().constData() << endl;
        return true;

    } else {
        qDebug() << "ini file not found" << endl;
        return false;
    }
}

void begindialog::on_okButton_clicked(){
    state = true;
    appState = true;
    clientAddress = ui->comboBox->currentText();
    this->close();
}
