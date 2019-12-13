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
	QUrl getTarget(){ return m_target; }
	void setTarget(const QUrl &url){ m_target = url; }
public slots:
	void slot_incommingData(const QByteArray &data);
signals:
	void signal_newData(const QByteArray data);
private slots:
	void slot_tcp_readyRead();
	void slot_udp_readyRead();
private:
	QString m_state;
	QTcpServer* m_pTcpServer;
	QUdpSocket* m_pUdpServer;
	QByteArray m_inBuffer;
	QTcpSocket* m_pTcpClient;
	QUrl m_target;

	void sendData(const QByteArray &data);
};

#endif // LOCALSERVER_H
