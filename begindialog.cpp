#include "begindialog.h"
#include "ui_begindialog.h"


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
    ui->comboBox->addItem(clientAddress);
}

begindialog::~begindialog(){
    delete ui;
}

bool begindialog::readSettings(){

    if (QFile::exists(INIFILENAME)){

        clientAddress = settings->value("clientAddress", _CLIENT_ADR).toString();
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
    this->close();
}
