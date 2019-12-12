#include "server.h"
#include "myfunctions.h"

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
	if( !this->listen( QHostAddress::AnyIPv4, app::conf.port ) ){
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
	//app::setLog(6,QString("ServerClient::new client [%1:%2]").arg( this->peerAddress().toString() ).arg( this->peerPort() ) );

	m_pTargetTcp = new QTcpSocket();

	connect( this, &QTcpSocket::readyRead, this, &ServerClient::slot_readyRead);
	connect( m_pTargetTcp, &QTcpSocket::readyRead, this, &ServerClient::slot_targetReadyRead);
	connect( this, &QTcpSocket::disconnected, this, [this](){
		//app::setLog(6,QString("ServerClient::Client disconnected"));
		slot_stop();
	});
	connect( m_pTargetTcp, &QTcpSocket::disconnected, this, [this](){
		//app::setLog(6,QString("ServerClient::Target disconnected"));
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
	while( m_pTargetTcp->bytesAvailable() ){
		m_pkt.rawData.clear();
		m_pkt.head.channel = myproto::Channel::auth;
		m_pkt.head.type = myproto::PktType::data;
		QByteArray buf = m_pTargetTcp->read( 1024 );
		m_pkt.rawData.append( buf );
		sendToClient( myproto::buidPkt( m_pkt, m_user.pass.toUtf8() ) );
		//if( m_auth ) app::addBytesInTraffic( m_pTarget->peerAddress().toString(), m_userLogin, buff.size() );
	}

	//if(buff.size() > 0) app::setLog(3,QString("buff.size() > 0 !!! [%1]").arg( QString( buff ) ));
}

void ServerClient::slot_readyRead()
{
	while( this->bytesAvailable() ){
		m_rxBuff.append( this->read(1024) );
	}

	//app::setLog(6,QString("ServerClient::slot_readyRead [%1]").arg(QString(m_rxBuff.toHex())));

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
		if( pkt.head.type == myproto::PktType::data ){
			mf::XOR( pkt.rawData, m_user.pass.toUtf8() );
		}else{
			myproto::parsData( pkt, m_user.pass.toUtf8() );
		}
	}else{
		myproto::parsData( pkt );
	}

	switch (pkt.head.channel) {
		case myproto::Channel::comunication:	parsPktCommunication( pkt );	break;
		case myproto::Channel::auth:			parsPktAuth( pkt );				break;
	}

	// Если данные еще есть, выводим
	if( m_rxBuff.size() > 0 ){
		slot_readyRead();
		//qDebug()<<m_rxBuff.toHex();
	}
}

void ServerClient::sendToClient(const QByteArray &data)
{
	if( data.size() == 0 ) return;
	if( this->state() == QAbstractSocket::ConnectingState ) this->waitForConnected(300);
	if( this->state() == QAbstractSocket::UnconnectedState ) return;
	this->write(data);
	this->waitForBytesWritten(100);
	//app::setLog(5,QString("ServerClient::sendToClient %1 bytes [%2]").arg(data.size()).arg(QString(data)));
	//app::setLog(6,QString("ServerClient::sendToClient [%1]").arg(QString(data.toHex())));
}

void ServerClient::sendToTarget(const QByteArray &data)
{
	if( data.size() == 0 ) return;
	if( m_pTargetTcp->state() == QAbstractSocket::ConnectingState ) m_pTargetTcp->waitForConnected(300);
	if( m_pTargetTcp->state() == QAbstractSocket::UnconnectedState ) return;
	m_pTargetTcp->write( data );
	m_pTargetTcp->waitForBytesWritten(100);
	//app::setLog(5,QString("ServerClient::sendToTarget %1 bytes [%2]").arg(data.size()).arg(QString(data)));
	//app::setLog(6,QString("ServerClient::sendToTarget [%2]").arg(QString(data.toHex())));
}

void ServerClient::parsPktAuth(const myproto::Pkt &pkt)
{
	QByteArray ba;
	QByteArray url;
	QByteArray id;
	QByteArray boolean;
	QUrl targetUrl;

	switch (pkt.head.type) {
		case myproto::PktType::hello:
			ba = myproto::findData( pkt, myproto::DataType::text );
			//app::setLog(4,QString("ServerClient::parsPktAuth client hello [%1]").arg(QString(ba)));
			if( ba != "hello" ){
				sendBye();
			}else{
				m_pkt.rawData.clear();
				m_pkt.head.channel = myproto::Channel::auth;
				m_pkt.head.type = myproto::PktType::hello;
				myproto::addData( m_pkt.rawData, myproto::DataType::text, ba );
				sendToClient( myproto::buidPkt( m_pkt, m_user.pass.toUtf8() ) );
			}
		break;
		case myproto::PktType::request:
			m_pkt.rawData.clear();
			url = myproto::findData( pkt, myproto::DataType::url );
			id = myproto::findData( pkt, myproto::DataType::id );
			boolean = myproto::findData( pkt, myproto::DataType::boolean );
			//app::setLog(4,QString("ServerClient::parsPktAuth client request url [%1] %2 %3").arg(QString(url)).arg(QString(id)).arg(QString(boolean)));
			targetUrl.setUrl( url );
			m_pkt.head.channel = myproto::Channel::auth;
			m_pkt.head.type = myproto::PktType::response;
			myproto::addData( m_pkt.rawData, myproto::DataType::id, id );
			//TODO: Реализовать запрет коннекта без разрешения
			if( targetUrl.isValid() ){
				if( addConnect( targetUrl ) ){
					myproto::addData( m_pkt.rawData, myproto::DataType::boolean, "1" );
				}else{
					myproto::addData( m_pkt.rawData, myproto::DataType::boolean, "0" );
				}
			}else{
				myproto::addData( m_pkt.rawData, myproto::DataType::boolean, "0" );
			}
			sendToClient( myproto::buidPkt( m_pkt, m_user.pass.toUtf8() ) );
		break;
		case myproto::PktType::data:
			//ba = myproto::findData( pkt, myproto::DataType::data );
			//app::setLog(4,QString("ServerClient::parsPktAuth client data [%1] %2 bytes").arg(QString(pkt.rawData)).arg( pkt.rawData.size() ));
			sendToTarget( pkt.rawData );
		break;
	}
}

void ServerClient::sendBye()
{
	m_pkt.rawData.clear();
	m_pkt.head.channel = myproto::Channel::comunication;
	m_pkt.head.type = myproto::PktType::bye;
	myproto::addData( m_pkt.rawData, myproto::DataType::version, app::conf.version.toUtf8() );
	sendToClient( myproto::buidPkt( m_pkt ) );
	this->close();
}

bool ServerClient::addConnect(const QUrl &url)
{
	if( url.scheme().toLower() == "tcp" ){
		m_pTargetTcp->connectToHost( url.host(), url.port() );
		return true;
	}
	return false;
}

void ServerClient::parsPktCommunication(const myproto::Pkt &pkt)
{
	QByteArray ba;
	switch (pkt.head.type) {
		case myproto::PktType::hello:
			ba = myproto::findData( pkt, myproto::DataType::version );
			//app::setLog(3,QString("ServerClient::parsPktCommunication client version [%1]").arg(QString(ba)));
			if( ba != app::conf.version.toUtf8() ){
				sendBye();
			}else{
				m_pkt.rawData.clear();
				m_pkt.head.channel = pkt.head.channel;
				m_pkt.head.type = pkt.head.type;
				myproto::addData( m_pkt.rawData, myproto::DataType::version, app::conf.version.toUtf8() );
				sendToClient( myproto::buidPkt( m_pkt ) );
			}
		break;
		case myproto::PktType::hello2:
			ba = myproto::findData( pkt, myproto::DataType::login );
			//app::setLog(3,QString("ServerClient::parsPktCommunication client login [%1]").arg(QString(ba)));

			for( auto user:app::conf.users ){
				if( user.login.toUtf8() == ba ){
					m_user = user;
					break;
				}
			}

			m_pkt.rawData.clear();
			m_pkt.head.channel = pkt.head.channel;
			m_pkt.head.type = pkt.head.type;
			sendToClient( myproto::buidPkt( m_pkt ) );
		break;
		default: break;
	}
}
