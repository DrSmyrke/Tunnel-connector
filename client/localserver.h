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
signals:
	void signal_stopAll();
	void signal_newData(const QByteArray);
private slots:
	void slot_tcp_readyRead();
private:
	QString m_state;
	QTcpServer* m_pTcpServer;
};

#endif // LOCALSERVER_H
