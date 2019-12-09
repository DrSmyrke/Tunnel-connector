#include "connector.h"

Connector::Connector(const QUrl &url, QObject *parent)
	: QObject(parent)
	, m_target(url)
{
	m_pSocket = new QTcpSocket( this );
	m_pClientSocket = new QTcpSocket( this );
	m_pTcpServer = new QTcpServer( this );

	connect( m_pSocket, &QTcpSocket::readyRead, this, &Connector::slot_readyRead );
	connect( m_pTcpServer, &QTcpServer::newConnection, this, [this](){
		if( m_pClientSocket->state() != QAbstractSocket::UnconnectedState ) return;
		m_pClientSocket = m_pTcpServer->nextPendingConnection();
		connect( m_pClientSocket, &QTcpSocket::readyRead, this, &Connector::slot_clientReadyRead );
	} );
	connect( m_pSocket, &QTcpSocket::stateChanged, this, &Connector::slot_stateChange );
}

bool Connector::init()
{
	if( m_pSocket->state() != QAbstractSocket::UnconnectedState){
		m_pSocket->close();
		if( m_pTcpServer->isListening() ) m_pTcpServer->close();
		return false;
	}

	uint16_t port = app::conf.port;

	auto tmp = app::conf.server.split(":");

	if( tmp.size() == 2 ){
		port = tmp[1].toUShort();
	}

	app::setLog(3,QString("Connector::init [%1:%2]").arg( app::conf.server ).arg( port ));

	m_pSocket->connectToHost( app::conf.server, port );

	return true;
}

void Connector::slot_stop()
{
	emit signal_finished(this);
}

void Connector::slot_stateChange(const QAbstractSocket::SocketState socketState)
{
	switch (socketState) {
		case QAbstractSocket::UnconnectedState:
			m_state = StatusConnectState::disconnected;
		break;
		case QAbstractSocket::ConnectingState:
			m_state = StatusConnectState::processing;
		break;
		case QAbstractSocket::ConnectedState:
			m_state = StatusConnectState::normal;
			sendInit();
		break;
		default: break;
	}
}

void Connector::slot_readyRead()
{
	while( m_pSocket->bytesAvailable() ){
		m_rxBuff.append( m_pSocket->read(1024) );
	}

	app::setLog(3,QString("Connector::slot_readyRead [%1]").arg(QString(m_rxBuff.toHex())));


	myproto::Pkt pkt = myproto::parsPkt( m_rxBuff );

	if( pkt.next ) return;
	if( pkt.error ){
		//TODO: send eoor
		return;
	}
	if( pkt.retry ){
		slot_readyRead();
		return;
	}

	if( pkt.head.channel == myproto::Channel::auth ){
		myproto::parsData( pkt, app::conf.user.pass.toUtf8() );
	}else{
		myproto::parsData( pkt );
	}

	switch (pkt.head.channel) {
		case myproto::Channel::comunication:	parsPktCommunication( pkt );	break;
		case myproto::Channel::auth:			parsPktAuth( pkt );				break;
	}

	// Если данные еще есть, выводим
	if( m_rxBuff.size() > 0 ) qDebug()<<m_rxBuff.toHex();
}

void Connector::slot_clientReadyRead()
{
	//QTcpSocket* pSocket = (QTcpSocket*)sender();
	QByteArray buf;
	while( m_pClientSocket->bytesAvailable() ){
		m_pkt.rawData.clear();
		buf.append( m_pClientSocket->read(1024) );
		m_pkt.head.channel = myproto::Channel::auth;
		m_pkt.head.type = myproto::PktType::data;
		myproto::addData( m_pkt.rawData, myproto::DataType::data, buf );
		sendData( myproto::buidPkt( m_pkt, app::conf.user.pass.toUtf8() ) );
		app::setLog(3,QString("Connector::slot_clientReadyRead [%1] %2").arg(QString(buf)).arg( buf.size() ));
	}
}

void Connector::sendData(const QByteArray &data)
{
	if( data.size() == 0 ) return;
	if( m_pSocket->state() == QAbstractSocket::ConnectingState ) m_pSocket->waitForConnected(300);
	if( m_pSocket->state() == QAbstractSocket::UnconnectedState ) return;
	m_pSocket->write( data );
	m_pSocket->waitForBytesWritten(100);
	//app::setLog(5,QString("Connector::sendData %1 bytes [%2]").arg(data.size()).arg(QString(data)));
	app::setLog(3,QString("Connector::sendData [%1]").arg(QString(data.toHex())));
}

void Connector::sendClientData(const QByteArray &data)
{
	if( data.size() == 0 ) return;
	if( m_pClientSocket->state() == QAbstractSocket::ConnectingState ) m_pClientSocket->waitForConnected(300);
	if( m_pClientSocket->state() == QAbstractSocket::UnconnectedState ) return;
	m_pClientSocket->write( data );
	m_pClientSocket->waitForBytesWritten(100);
	//app::setLog(5,QString("Connector::sendData %1 bytes [%2]").arg(data.size()).arg(QString(data)));
	//app::setLog(6,QString("Connector::sendData [%2]").arg(QString(data.toHex())));
}

void Connector::sendInit()
{
	myproto::Pkt pkt;
	pkt.head.channel = myproto::Channel::comunication;
	pkt.head.type = myproto::PktType::hello;
	myproto::addData( pkt.rawData, myproto::DataType::version, app::conf.version.toUtf8() );
	sendData( myproto::buidPkt( pkt ) );
}

void Connector::parsPktCommunication(const myproto::Pkt &pkt)
{
	QByteArray ba;
	switch (pkt.head.type) {
		case myproto::PktType::hello:
			ba = myproto::findData( pkt, myproto::DataType::version );
			app::setLog(3,QString("MainWindow::parsPktCommunication server version [%1]").arg(QString(ba)));
			if( ba == app::conf.version.toUtf8() ){
				m_pkt.rawData.clear();
				m_pkt.head.channel = pkt.head.channel;
				m_pkt.head.type = myproto::PktType::hello2;
				myproto::addData( m_pkt.rawData, myproto::DataType::login, app::conf.user.login.toUtf8() );
				sendData( myproto::buidPkt( m_pkt ) );
			}
		break;
		case myproto::PktType::hello2:
			m_pkt.rawData.clear();
			m_pkt.head.channel = myproto::Channel::auth;
			m_pkt.head.type = myproto::PktType::hello;
			myproto::addData( m_pkt.rawData, myproto::DataType::text, "hello" );
			sendData( myproto::buidPkt( m_pkt, app::conf.user.pass.toUtf8() ) );
		break;
		default: break;
	}
}

void Connector::parsPktAuth(const myproto::Pkt &pkt)
{
	QByteArray ba;
	switch (pkt.head.type) {
		case myproto::PktType::hello:
			ba = myproto::findData( pkt, myproto::DataType::text );
			app::setLog(3,QString("Connector::parsPktAuth server hello [%1]").arg(QString(ba)));
			if( ba != "hello" ){
				m_pSocket->close();
			}else{
				m_pkt.rawData.clear();
				m_pkt.head.channel = myproto::Channel::auth;
				m_pkt.head.type = myproto::PktType::request;
				myproto::addData( m_pkt.rawData, myproto::DataType::url, m_target.toString().toUtf8() );
				myproto::addData( m_pkt.rawData, myproto::DataType::boolean, "1" );
				sendData( myproto::buidPkt( m_pkt, app::conf.user.pass.toUtf8() ) );
			}
		break;
		case myproto::PktType::response:
			ba = myproto::findData( pkt, myproto::DataType::boolean );
			app::setLog(3,QString("Connector::parsPktAuth server response [%1]").arg(QString(ba)));
			if( ba.toUShort() != 1 ){
				m_pSocket->close();
			}else{
				if( m_target.scheme().toLower() == "tcp" ){
					if( m_pTcpServer->listen( QHostAddress::LocalHost ) ){
						m_localPort = m_pTcpServer->serverPort();
					}
				}
			}
		break;
		case myproto::PktType::data:
			ba = myproto::findData( pkt, myproto::DataType::data );
			app::setLog(3,QString("Connector::parsPktAuth server data [%1]").arg(QString(ba)));
			sendClientData( ba );
		break;
	}
}
