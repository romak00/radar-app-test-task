#include "network-client.h"
#include <QDataStream>
#include <QImage>
#include <QBuffer>

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent), _socket(new QTcpSocket(this))
{
    connect(_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
        this, &NetworkClient::onError);
}

void NetworkClient::connectToServer(const QString& host, quint16 port) {
    _socket->connectToHost(host, port);
}

void NetworkClient::onConnected() {
    
}

void NetworkClient::sendCommand(const QByteArray& cmd) {
    if (_socket->state() == QAbstractSocket::ConnectedState) {
        _socket->write(cmd);
    }
}

void NetworkClient::onError(QAbstractSocket::SocketError) {
    emit errorOccured(_socket->errorString());
}

void NetworkClient::onReadyRead() {
    _buffer.append(_socket->readAll());

    QBuffer buf(&_buffer);
    buf.open(QIODevice::ReadOnly);
    QDataStream in(&buf);
    in.setByteOrder(QDataStream::LittleEndian);

    const quint32 MAGIC = 0xABCDEF01;
    const int TRAIL_SIZE = 3;
    const qint64 PER_TGT_HDR = 4 + 6 * 8 + 1;
    const qint64 PER_TGT_PT = TRAIL_SIZE * 2 * 8;
    const qint64 PER_TGT_SIZE = PER_TGT_HDR + PER_TGT_PT;

    while (true) {
        if (buf.bytesAvailable() < int(sizeof(quint32) + sizeof(quint16))) {
            break;
        }

        in.startTransaction();
        quint32 magic;  in >> magic;
        if (magic != MAGIC) {
            _buffer.remove(0, 1);
            buf.close();
            buf.setData(_buffer);
            buf.open(QIODevice::ReadOnly);
            in.setDevice(&buf);
            continue;
        }

        quint16 count; in >> count;
        qint64 need = count * PER_TGT_SIZE;
        if (buf.bytesAvailable() < need) {
            in.rollbackTransaction();
            break;
        }

        std::vector<Target> targets;
        targets.reserve(count);
        for (int i = 0; i < count; ++i) {
            int id; double dist, ang, dir;
            double r, g, b; uint8_t ts;
            in >> id >> dist >> ang >> dir;
            in >> r >> g >> b;
            in >> ts;
            std::vector<Eigen::Vector2d> trail;
            trail.reserve(ts);
            for (int j = 0; j < ts; ++j) {
                double x, y; in >> x >> y;
                trail.emplace_back(x, y);
            }
            targets.emplace_back(id, dist, ang, dir, Eigen::Vector3d(r, g, b), trail);
        }

        in.commitTransaction();

        emit newFrame(targets);

        qint64 consumed = buf.pos();
        _buffer.remove(0, consumed);

        buf.close();
        buf.setData(_buffer);
        buf.open(QIODevice::ReadOnly);
        in.setDevice(&buf);
    }
}