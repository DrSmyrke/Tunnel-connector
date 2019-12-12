#include "localserver.h"

LocalServer::LocalServer(QObject *parent) : QObject(parent)
{
	app::setLog( 0, QString("LOCAL SERVER CREATING ...") );

	m_pTcpServer = new QTcpServer( this );
	m_pTcpClient = new QTcpSocket( this );

	connect( m_pTcpClient, &QTcpSocket::readyRead, this, &LocalServer::slot_tcp_readyRead );
	connect( m_pTcpServer, &QTcpServer::newConnection, this, [this](){
		if( m_pTcpClient->state() != QAbstractSocket::UnconnectedState ){
			m_pTcpServer->nextPendingConnection()->close();
			return;
		}
		m_pTcpClient->setSocketDescriptor( m_pTcpServer->nextPendingConnection()->socketDescriptor() );
		app::setLog(3,QString("LocalServer::newConnection"));
		sendData( m_inBuffer );
	} );
}

LocalServer::~LocalServer()
{
	stop();
	app::setLog( 0, "LOCAL SERVER DIE..." );
}

bool LocalServer::run()
{
	if( !m_pTcpServer->listen( QHostAddress::AnyIPv4 ) ){
		app::setLog( 0, QString("LOCAL SERVER [ NOT ACTIVATED ] PORT: [%1] %2").arg( m_pTcpServer->serverPort() ).arg( m_pTcpServer->errorString() ) );
		return false;
	}

	app::setLog( 0, QString("LOCAL SERVER [ ACTIVATED ] PORT: [%1]").arg( m_pTcpServer->serverPort() ) );

	m_state = QString( tr( " Listen at %1 port" ) ).arg( m_pTcpServer->serverPort() );

	return true;
}

void LocalServer::stop()
{
	app::setLog( 0, "LOCAL SERVER STOPPING..." );
	m_state.clear();
	m_pTcpServer->close();
	m_inBuffer.clear();
}

void LocalServer::slot_incommingData(const QByteArray &data)
{
	app::setLog(3,QString("LocalServer::slot_incommingData [%1] %2").arg(QString(data.toHex())).arg( data.size() ));
	if( m_pTcpClient->state() == QAbstractSocket::UnconnectedState ){
		m_inBuffer.append( data );
	}else{
		sendData( data );
	}
}

void LocalServer::slot_tcp_readyRead()
{
	while( m_pTcpClient->bytesAvailable() ){
		QByteArray buf = m_pTcpClient->read(1024);
		app::setLog(3,QString("LocalServer::slot_tcp_readyRead [%1]").arg(QString(buf)));
		emit signal_newData( buf );
	}
}

void LocalServer::sendData(const QByteArray &data)
{
	if( data.size() == 0 ) return;
	if( m_pTcpClient->state() == QAbstractSocket::ConnectingState ) m_pTcpClient->waitForConnected(300);
	if( m_pTcpClient->state() == QAbstractSocket::UnconnectedState ) return;
	m_pTcpClient->write( data );
	m_pTcpClient->waitForBytesWritten(100);
	app::setLog(3,QString("LocalServer::sendData %1 bytes [%2]").arg(data.size()).arg(QString(data)));
	app::setLog(6,QString("LocalServer::sendData [%1]").arg(QString(data.toHex())));
}
