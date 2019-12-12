#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>
#include <QTcpSocket>
#include <QUrl>

#include "myproto.h"
#include "global.h"

class Connector : public QObject
{
	Q_OBJECT
public:
	explicit Connector(QObject *parent = nullptr);
	QUrl getTarget(){ return m_target; }
	void setTarget(const QUrl &url){ m_target = url; }
	bool init();
	void stop();
	uint8_t getState() const { return m_state; }
public slots:
	void slot_incomingData(const QByteArray &data);
signals:
	void signal_finished(Connector* connector);
	void signal_stateChanged(const uint8_t state);
	void signal_newData(const QByteArray &data);
	void signal_accessGaranted();
private slots:
	void slot_stateChange(const QAbstractSocket::SocketState socketState);
	void slot_readyRead();
private:
	QUrl m_target;
	QTcpSocket* m_pSocket;
	QByteArray m_rxBuff;
	myproto::Pkt m_pkt;
	uint8_t m_state;

	void openTunnel();
	void sendData(const QByteArray &data);
	void parsPktCommunication(const myproto::Pkt &pkt);
	void parsPktAuth(const myproto::Pkt &pkt);
};

#endif // CONNECTOR_H
