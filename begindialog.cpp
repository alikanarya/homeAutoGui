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
}

begindialog::~begindialog(){
    delete ui;
}

bool begindialog::readSettings(){

    if (QFile::exists(INIFILENAME)){

        clientAddress1 = settings->value("clientAddress1", _CLIENT_ADR).toString();
        clientAddress2 = settings->value("clientAddress2", _CLIENT_ADR).toString();
        clientAddress3 = settings->value("clientAddress3", _CLIENT_ADR).toString();
        clientAddress4 = settings->value("clientAddress4", _CLIENT_ADR).toString();
        clientAddress5 = settings->value("clientAddress5", _CLIENT_ADR).toString();
        clientPort = settings->value("clientPort", _CLIENT_PORT).toInt();
        dbName = settings->value("dbName", _DB_NAME).toString();
        dbUser = settings->value("dbUser", _DB_USER).toString();
        dbPass = settings->value("dbPass", _DB_PASS).toString();

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
