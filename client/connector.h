#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QUrl>

#include "myproto.h"
#include "global.h"

class Connector : public QObject
{
	Q_OBJECT
public:
	explicit Connector(const QUrl &url, QObject *parent = nullptr);
	QUrl getTarget(){ return m_target; }
	void setTarget(const QUrl &url){ m_target = url; }
	uint8_t getState(){ return m_state; }
	uint16_t getLocalPort(){ return m_localPort; }
	bool init();
public slots:
	void slot_stop();
signals:
	void signal_finished(Connector* connector);
private slots:
	void slot_stateChange(const QAbstractSocket::SocketState socketState);
	void slot_readyRead();
	void slot_clientReadyRead();
private:
	QUrl m_target;
	QTcpSocket* m_pSocket;
	QByteArray m_rxBuff;
	uint8_t m_state;
	myproto::Pkt m_pkt;
	QTcpServer* m_pTcpServer;
	QUdpSocket* m_pUdpServer;
	QTcpSocket* m_pClientSocket;
	uint16_t m_localPort;

	void sendData(const QByteArray &data);
	void sendClientData(const QByteArray &data);
	void sendInit();
	void parsPktCommunication(const myproto::Pkt &pkt);
	void parsPktAuth(const myproto::Pkt &pkt);
};

#endif // CONNECTOR_H
