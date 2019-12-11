#include "localserver.h"

LocalServer::LocalServer(QObject *parent) : QObject(parent)
{
	app::setLog( 0, QString("LOCAL SERVER CREATING ...").arg(app::conf.version) );

	m_pTcpServer = new QTcpServer( this );

	connect( m_pTcpServer, &QTcpServer::newConnection, this, [this](){
		QTcpSocket* client = m_pTcpServer->nextPendingConnection();
		connect( client, &QTcpSocket::readyRead, this, &LocalServer::slot_tcp_readyRead );
		connect( client, &QTcpSocket::disconnected, client, &QTcpSocket::deleteLater );
	} );
}

LocalServer::~LocalServer()
{
	app::setLog( 0, "LOCAL SERVER DIE..." );
	emit signal_stopAll();
	stop();
}

bool LocalServer::run()
{
	if( !m_pTcpServer->listen( QHostAddress::AnyIPv4 ) ){
		app::setLog( 0, QString("SERVER [ NOT ACTIVATED ] PORT: [%1] %2").arg( m_pTcpServer->serverPort() ).arg( m_pTcpServer->errorString() ) );
		return false;
	}

	app::setLog( 0, QString("SERVER [ ACTIVATED ] PORT: [%1]").arg( m_pTcpServer->serverPort() ) );

	m_state = QString( tr( "Listen at %1 port" ) ).arg( m_pTcpServer->serverPort() );

	return true;
}

void LocalServer::stop()
{
	app::setLog( 0, "LOCAL SERVER STOPPING..." );
	app::saveSettings();
	m_pTcpServer->close();
}

void LocalServer::slot_tcp_readyRead()
{
	QTcpSocket* pSocket = (QTcpSocket*)sender();
	QByteArray buf;
	while( pSocket->bytesAvailable() ){
		buf.append( pSocket->read(1024) );
		emit signal_newData( buf );
		buf.clear();
	}
}
