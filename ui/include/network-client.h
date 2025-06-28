#pragma once

#include <QObject>
#include <QTcpSocket>
#include "target.h"

class NetworkClient : public QObject {
    Q_OBJECT
public:
    explicit NetworkClient(QObject* parent = nullptr);
    void connectToServer(const QString& host, quint16 port);
    void sendCommand(const QByteArray& cmd);

signals:
    void newFrame(const std::vector<Target>& targets);
    void errorOccured(const QString& msg);

private slots:
    void onConnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError);

private:
    QTcpSocket* _socket;
    QByteArray  _buffer;
};
