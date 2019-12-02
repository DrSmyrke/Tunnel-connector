#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "global.h"

class Server : public QTcpServer
{
	Q_OBJECT
public:
	explicit Server(QObject *parent = nullptr);
	~Server();
	bool run();
	void stop();
protected:
	void incomingConnection(qintptr socketDescriptor);
signals:
	void signal_stopAll();
};



class ServerClient : public QTcpSocket
{
	Q_OBJECT
public:
	explicit ServerClient(qintptr descriptor, QObject *parent = nullptr);
	bool run();
	void stop();
signals:
	void signal_finished();
public slots:
	void slot_stop();
private slots:
	void slot_targetReadyRead();
	void slot_readyRead();
private:
	QTcpSocket* m_pTarget;

	void sendToClient(const QByteArray &data);
	void sendToTarget(const QByteArray &data);
};

#endif // SERVER_H