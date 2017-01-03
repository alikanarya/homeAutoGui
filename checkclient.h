#ifndef CHECKCLIENT_H
#define CHECKCLIENT_H

#include <QThread>

class checkClient : public QThread {

    Q_OBJECT

public:

    checkClient();
    ~checkClient();

    void stop();

public slots:

    void connect();

protected:

    void run();

private:

    volatile bool stopped;

signals:

    void Connected();
    void notConnected();

};

#endif // CHECKCLIENT_H
