#include "server.h"

Server::Server(QObject *parent) : QTcpServer(parent)
{
	app::setLog( 0, QString("SERVER CREATING v%1 ...").arg(app::conf.version) );
}

Server::~Server()
{
	app::setLog( 0, "SERVER DIE..." );
	emit signal_stopAll();
	stop();
}

bool Server::run()
{
	if(!this->listen( QHostAddress::AnyIPv4, app::conf.port )){
		app::setLog( 0, QString("SERVER [ NOT ACTIVATED ] PORT: [%1] %2").arg( app::conf.port ).arg( this->errorString() ) );
		return false;
	}

	app::setLog( 0, QString("SERVER [ ACTIVATED ] PORT: [%1]").arg( app::conf.port ) );

	return true;
}

void Server::stop()
{
	app::setLog( 0, "SERVER STOPPING..." );
	app::saveSettings();
	this->close();
}

void Server::incomingConnection(qintptr socketDescriptor)
{
	ServerClient* client = new ServerClient(socketDescriptor);
	//QThread* thread = new QThread();
	//client->moveToThread(thread);
//	connect(thread,&QThread::started,client,&Client::slot_start);
//	connect(client,&Client::signal_finished,thread,&QThread::quit);
	connect( this, &Server::signal_stopAll, client, &ServerClient::slot_stop );
	connect( client, &ServerClient::signal_finished, client, &ServerClient::deleteLater );
//	connect(thread,&QThread::finished,thread,&QThread::deleteLater);
//	thread->start();
	client->run();
}



ServerClient::ServerClient(qintptr descriptor, QObject *parent) : QTcpSocket(parent)
{
	if( !this->setSocketDescriptor( descriptor ) ) slot_stop();

	m_pTarget = new QTcpSocket();

	connect( this, &QTcpSocket::readyRead, this, &ServerClient::slot_readyRead);
	connect( m_pTarget, &QTcpSocket::readyRead, this, &ServerClient::slot_targetReadyRead);
	connect( this, &QTcpSocket::disconnected, this, [this](){
		//app::setLog(6,QString("Client::Client disconnected"));
		slot_stop();
	});
	connect( m_pTarget, &QTcpSocket::disconnected, this, [this](){
		//app::setLog(6,QString("Client::Target disconnected"));
		slot_stop();
	});
}

bool ServerClient::run()
{

	return true;
}

void ServerClient::stop()
{

}

void ServerClient::slot_stop()
{
	emit signal_finished();
}

void ServerClient::slot_targetReadyRead()
{
	while( m_pTarget->bytesAvailable() ){
		QByteArray buff;
		buff.append( m_pTarget->read( 1024 ) );
		sendToClient( buff );
		//if( m_auth ) app::addBytesInTraffic( m_pTarget->peerAddress().toString(), m_userLogin, buff.size() );
	}

	//if(buff.size() > 0) app::setLog(3,QString("buff.size() > 0 !!! [%1]").arg( QString( buff ) ));
}

void ServerClient::slot_readyRead()
{
	while( this->bytesAvailable() ){
		m_rxBuff.append( this->read(1024) );
		//if( m_pTarget->isOpen() && m_tunnel ){
		//	sendToTarget( buff );
		//	buff.clear();
		//}
	}

	qDebug()<<m_rxBuff.toHex();


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

	myproto::parsData( pkt );

	switch (pkt.head.channel) {
		case myproto::Channel::comunication:	parsPktCommunication( pkt );	break;
	}

	// Если данные еще есть, выводим
	if( m_rxBuff.size() > 0 ) qDebug()<<m_rxBuff.toHex();
}

void ServerClient::sendToClient(const QByteArray &data)
{
	if( data.size() == 0 ) return;
	if( this->state() == QAbstractSocket::ConnectingState ) this->waitForConnected(300);
	if( this->state() == QAbstractSocket::UnconnectedState ) return;
	this->write(data);
	this->waitForBytesWritten(100);
	//app::setLog(5,QString("ServerClient::sendToClient %1 bytes [%2]").arg(data.size()).arg(QString(data)));
	//app::setLog(6,QString("ServerClient::sendToClient [%3]").arg(QString(data.toHex())));
}

void ServerClient::sendToTarget(const QByteArray &data)
{
	if( data.size() == 0 ) return;
	if( m_pTarget->state() == QAbstractSocket::ConnectingState ) m_pTarget->waitForConnected(300);
	if( m_pTarget->state() == QAbstractSocket::UnconnectedState ) return;
	m_pTarget->write( data );
	m_pTarget->waitForBytesWritten(100);
	//app::setLog(5,QString("ServerClient::sendToTarget %1 bytes [%2]").arg(data.size()).arg(QString(data)));
	//app::setLog(6,QString("ServerClient::sendToTarget [%2]").arg(QString(data.toHex())));
}

void ServerClient::parsPktCommunication(const myproto::Pkt &pkt)
{
	QByteArray ba;
	switch (pkt.head.type) {
		case myproto::PktType::hello:
			app::setLog(3,QString("ServerClient::parsPktCommunication client version [%1]").arg(QString(ba)));
			ba = myproto::findData( pkt, myproto::DataType::version );
			if( ba != app::conf.version.toUtf8() ){
				m_pkt.rawData.clear();
				m_pkt.head.channel = pkt.head.channel;
				m_pkt.head.type = myproto::PktType::bye;
				myproto::addData( m_pkt.rawData, myproto::DataType::version, app::conf.version.toUtf8() );
				sendToClient( myproto::buidPkt( pkt ) );

				this->close();
			}else{
				m_pkt.rawData.clear();
				m_pkt.head.channel = pkt.head.channel;
				m_pkt.head.type = pkt.head.type;
				myproto::addData( m_pkt.rawData, myproto::DataType::version, app::conf.version.toUtf8() );
				sendToClient( myproto::buidPkt( pkt ) );
			}
		break;
		default: break;
	}
}
