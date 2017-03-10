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
        prmArray[0].propBandWidth = settings->value("propBandWidth_Z1", 0).toInt();
        prmArray[0].cycleTime = settings->value("cycleTime_Z1", 0).toInt();
        prmArray[0].propBandPos = settings->value("propBandPos_Z1", 0).toInt();
        prmArray[0].normalSet = settings->value("normalSet_Z1", 0).toFloat();
        prmArray[0].reducedSet = settings->value("reducedSet_Z1", 0).toFloat();
        prmArray[0].P1NormalModeTime = settings->value("P1NormalModeTime_Z1", "--:--:--").toString();
        prmArray[0].P1ReducedModeTime = settings->value("P1ReducedModeTime_Z1", "--:--:--").toString();
        prmArray[0].P2NormalModeTime = settings->value("P2NormalModeTime_Z1", "--:--:--").toString();
        prmArray[0].P2ReducedModeTime = settings->value("P2ReducedModeTime_Z1", "--:--:--").toString();
        prmArray[0].P3NormalModeTime = settings->value("P3NormalModeTime_Z1", "--:--:--").toString();
        prmArray[0].P3ReducedModeTime = settings->value("P3ReducedModeTime_Z1", "--:--:--").toString();

        prmArray[1].propBandWidth = settings->value("propBandWidth_Z2", 0).toInt();
        prmArray[1].cycleTime = settings->value("cycleTime_Z2", 0).toInt();
        prmArray[1].propBandPos = settings->value("propBandPos_Z2", 0).toInt();
        prmArray[1].normalSet = settings->value("normalSet_Z2", 0).toFloat();
        prmArray[1].reducedSet = settings->value("reducedSet_Z2", 0).toFloat();
        prmArray[1].P1NormalModeTime = settings->value("P1NormalModeTime_Z2", "--:--:--").toString();
        prmArray[1].P1ReducedModeTime = settings->value("P1ReducedModeTime_Z2", "--:--:--").toString();
        prmArray[1].P2NormalModeTime = settings->value("P2NormalModeTime_Z2", "--:--:--").toString();
        prmArray[1].P2ReducedModeTime = settings->value("P2ReducedModeTime_Z2", "--:--:--").toString();
        prmArray[1].P3NormalModeTime = settings->value("P3NormalModeTime_Z2", "--:--:--").toString();
        prmArray[1].P3ReducedModeTime = settings->value("P3ReducedModeTime_Z2", "--:--:--").toString();

        prmArray[2].propBandWidth = settings->value("propBandWidth_Z3", 0).toInt();
        prmArray[2].cycleTime = settings->value("cycleTime_Z3", 0).toInt();
        prmArray[2].propBandPos = settings->value("propBandPos_Z3", 0).toInt();
        prmArray[2].normalSet = settings->value("normalSet_Z3", 0).toFloat();
        prmArray[2].reducedSet = settings->value("reducedSet_Z3", 0).toFloat();
        prmArray[2].P1NormalModeTime = settings->value("P1NormalModeTime_Z3", "--:--:--").toString();
        prmArray[2].P1ReducedModeTime = settings->value("P1ReducedModeTime_Z3", "--:--:--").toString();
        prmArray[2].P2NormalModeTime = settings->value("P2NormalModeTime_Z3", "--:--:--").toString();
        prmArray[2].P2ReducedModeTime = settings->value("P2ReducedModeTime_Z3", "--:--:--").toString();
        prmArray[2].P3NormalModeTime = settings->value("P3NormalModeTime_Z3", "--:--:--").toString();
        prmArray[2].P3ReducedModeTime = settings->value("P3ReducedModeTime_Z3", "--:--:--").toString();

        prmArray[3].propBandWidth = settings->value("propBandWidth_Z4", 0).toInt();
        prmArray[3].cycleTime = settings->value("cycleTime_Z4", 0).toInt();
        prmArray[3].propBandPos = settings->value("propBandPos_Z4", 0).toInt();
        prmArray[3].normalSet = settings->value("normalSet_Z4", 0).toFloat();
        prmArray[3].reducedSet = settings->value("reducedSet_Z4", 0).toFloat();
        prmArray[3].P1NormalModeTime = settings->value("P1NormalModeTime_Z4", "--:--:--").toString();
        prmArray[3].P1ReducedModeTime = settings->value("P1ReducedModeTime_Z4", "--:--:--").toString();
        prmArray[3].P2NormalModeTime = settings->value("P2NormalModeTime_Z4", "--:--:--").toString();
        prmArray[3].P2ReducedModeTime = settings->value("P2ReducedModeTime_Z4", "--:--:--").toString();
        prmArray[3].P3NormalModeTime = settings->value("P3NormalModeTime_Z4", "--:--:--").toString();
        prmArray[3].P3ReducedModeTime = settings->value("P3ReducedModeTime_Z4", "--:--:--").toString();

        prmArray[4].propBandWidth = settings->value("propBandWidth_Z5", 0).toInt();
        prmArray[4].cycleTime = settings->value("cycleTime_Z5", 0).toInt();
        prmArray[4].propBandPos = settings->value("propBandPos_Z5", 0).toInt();
        prmArray[4].normalSet = settings->value("normalSet_Z5", 0).toFloat();
        prmArray[4].reducedSet = settings->value("reducedSet_Z5", 0).toFloat();
        prmArray[4].P1NormalModeTime = settings->value("P1NormalModeTime_Z5", "--:--:--").toString();
        prmArray[4].P1ReducedModeTime = settings->value("P1ReducedModeTime_Z5", "--:--:--").toString();
        prmArray[4].P2NormalModeTime = settings->value("P2NormalModeTime_Z5", "--:--:--").toString();
        prmArray[4].P2ReducedModeTime = settings->value("P2ReducedModeTime_Z5", "--:--:--").toString();
        prmArray[4].P3NormalModeTime = settings->value("P3NormalModeTime_Z5", "--:--:--").toString();
        prmArray[4].P3ReducedModeTime = settings->value("P3ReducedModeTime_Z5", "--:--:--").toString();

        prmArray[5].propBandWidth = settings->value("propBandWidth_Z6", 0).toInt();
        prmArray[5].cycleTime = settings->value("cycleTime_Z6", 0).toInt();
        prmArray[5].propBandPos = settings->value("propBandPos_Z6", 0).toInt();
        prmArray[5].normalSet = settings->value("normalSet_Z6", 0).toFloat();
        prmArray[5].reducedSet = settings->value("reducedSet_Z6", 0).toFloat();
        prmArray[5].P1NormalModeTime = settings->value("P1NormalModeTime_Z6", "--:--:--").toString();
        prmArray[5].P1ReducedModeTime = settings->value("P1ReducedModeTime_Z6", "--:--:--").toString();
        prmArray[5].P2NormalModeTime = settings->value("P2NormalModeTime_Z6", "--:--:--").toString();
        prmArray[5].P2ReducedModeTime = settings->value("P2ReducedModeTime_Z6", "--:--:--").toString();
        prmArray[5].P3NormalModeTime = settings->value("P3NormalModeTime_Z6", "--:--:--").toString();
        prmArray[5].P3ReducedModeTime = settings->value("P3ReducedModeTime_Z6", "--:--:--").toString();

        prmArray[6].propBandWidth = settings->value("propBandWidth_Z7", 0).toInt();
        prmArray[6].cycleTime = settings->value("cycleTime_Z7", 0).toInt();
        prmArray[6].propBandPos = settings->value("propBandPos_Z7", 0).toInt();
        prmArray[6].normalSet = settings->value("normalSet_Z7", 0).toFloat();
        prmArray[6].reducedSet = settings->value("reducedSet_Z7", 0).toFloat();
        prmArray[6].P1NormalModeTime = settings->value("P1NormalModeTime_Z7", "--:--:--").toString();
        prmArray[6].P1ReducedModeTime = settings->value("P1ReducedModeTime_Z7", "--:--:--").toString();
        prmArray[6].P2NormalModeTime = settings->value("P2NormalModeTime_Z7", "--:--:--").toString();
        prmArray[6].P2ReducedModeTime = settings->value("P2ReducedModeTime_Z7", "--:--:--").toString();
        prmArray[6].P3NormalModeTime = settings->value("P3NormalModeTime_Z7", "--:--:--").toString();
        prmArray[6].P3ReducedModeTime = settings->value("P3ReducedModeTime_Z7", "--:--:--").toString();
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
