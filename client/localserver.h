#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QUrl>

#include "global.h"

class LocalServer : public QObject
{
	Q_OBJECT
public:
	explicit LocalServer(QObject *parent = nullptr);
	~LocalServer();
	bool run();
	void stop();
	QString getState() const { return m_state; }
public slots:
	void slot_incommingData(const QByteArray &data);
signals:
	void signal_newData(const QByteArray data);
private slots:
	void slot_tcp_readyRead();
private:
	QString m_state;
	QTcpServer* m_pTcpServer;
	QByteArray m_inBuffer;
	QTcpSocket* m_pTcpClient;

	void sendData(const QByteArray &data);
};

#endif // LOCALSERVER_H
