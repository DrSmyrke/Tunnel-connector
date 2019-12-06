#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myfunctions.h"

#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_auth(false)
{
	m_pControlSocket = new QTcpSocket( this );
	m_pTimer = new QTimer( this );
		m_pTimer->setInterval( 100 );
	m_model = new MyList();

	ui->setupUi(this);
	ui->connectB->setText( tr("CONNECT") );
	ui->statusL->setText( "" );
	ui->serverAddrBox->setText( app::conf.server );
	ui->loginBox->setText( app::conf.user.login );
	ui->tableView->setModel( m_model );
	ui->tableView->setEditTriggers( QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked );

	setWindowTitle( "Tunnel Connector v" + app::conf.version );
	setWindowIcon( QIcon( "://index.ico" ) );
	setMinimumSize( 270, 220 );

	connect( ui->connectB, &QPushButton::clicked, this, &MainWindow::slot_connect );
	connect( m_pControlSocket, &QTcpSocket::readyRead, this, &MainWindow::slot_readyRead );
	connect( m_pControlSocket, &QTcpSocket::stateChanged, this, &MainWindow::slot_stateChange );
	connect( m_pTimer, &QTimer::timeout, this, &MainWindow::slot_timer );
	connect( ui->addAddressB, &QPushButton::clicked, this, &MainWindow::slot_addAddress );
	connect( ui->tableView, &QTableView::doubleClicked, this, [this](const QModelIndex &index){
		if( !index.isValid() ) return;
		qDebug()<<index.data().toString();
	} );

	m_pTimer->start();
}

MainWindow::~MainWindow()
{
	m_model->deleteLater();
	if( m_pTimer->isActive() ) m_pTimer->stop();
	delete ui;
}

void MainWindow::slot_readyRead()
{
	while( m_pControlSocket->bytesAvailable() ){
		m_rxBuff.append( m_pControlSocket->read(1024) );
		//if( m_pTarget->isOpen() && m_tunnel ){
		//	sendToTarget( buff );
		//	buff.clear();
		//}
	}

	app::setLog(3,QString("MainWindow::slot_readyRead [%2]").arg(QString(m_rxBuff.toHex())));


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

void MainWindow::slot_connect()
{
	if( m_pControlSocket->state() != QAbstractSocket::UnconnectedState){
		m_pControlSocket->close();
		return;
	}

	uint16_t port = app::conf.port;

	auto str = ui->serverAddrBox->text();
	auto tmp = str.split(":");
	if( tmp.size() == 2 ){
		port = tmp[1].toUShort();
	}

	m_pControlSocket->connectToHost( tmp[0], port );
}

void MainWindow::slot_stateChange(const QAbstractSocket::SocketState socketState)
{
	switch (socketState) {
		case QAbstractSocket::UnconnectedState:
			ui->statusL->setText( setColorText( tr("Disconnected") ) );
			ui->connectB->setText( tr("CONNECT") );
			ui->connectB->setEnabled( true );
		break;
		case QAbstractSocket::ConnectingState:
			ui->statusL->setText( setColorText( tr("Connecting ..."), StatusState::processing ) );
			ui->connectB->setEnabled( false );
		break;
		case QAbstractSocket::ConnectedState:
			ui->statusL->setText( setColorText( tr("Connected"), StatusState::normal ) );
			ui->connectB->setText( tr("DISCONNECT") );
			ui->connectB->setEnabled( true );
			app::conf.user.login = ui->loginBox->text();
			app::conf.user.pass = app::conf.user.login + ":>:" + ui->passwordBox->text();
			app::conf.user.pass = mf::md5( app::conf.user.pass.toUtf8() );
			app::conf.server = ui->serverAddrBox->text();
			app::conf.settingsSave = true;
			m_disconnector = 10;
			sendInit();
		break;
		default: break;
	}
}

void MainWindow::slot_timer()
{
	if( m_disconnector > 0 ){
		m_disconnector--;
		if( m_disconnector == 0 ) m_pControlSocket->close();
	}
	if( app::conf.settingsSave ) app::saveSettings();
}

void MainWindow::slot_addAddress()
{
	QUrl url;
	url.setUrl( ui->addAddressBox->text() );
	if( !url.isValid() || url.port() < 1 ) return;

	m_model->addTarget( url );
	ui->tableView->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
}

void MainWindow::sendData(const QByteArray &data)
{
	if( data.size() == 0 ) return;
	if( m_pControlSocket->state() == QAbstractSocket::ConnectingState ) m_pControlSocket->waitForConnected(300);
	if( m_pControlSocket->state() == QAbstractSocket::UnconnectedState ) return;
	m_pControlSocket->write( data );
	m_pControlSocket->waitForBytesWritten(100);
	//app::setLog(5,QString("MainWindow::sendData %1 bytes [%2]").arg(data.size()).arg(QString(data)));
	//app::setLog(6,QString("MainWindow::sendData [%2]").arg(QString(data.toHex())));
}

QString MainWindow::setColorText(const QString &text, const uint8_t state)
{
	QString res = "<span style=\"color:";
	switch (state) {
		case StatusState::normal:		res += "green";		break;
		case StatusState::processing:	res += "orange";	break;
		case StatusState::warning:		res += "yellow";	break;
		case StatusState::error:		res += "red";		break;
		default:						res += "gray";		break;
	}
	res += ";\">" + text + "</span>";
	return res;
}

void MainWindow::sendInit()
{
	myproto::Pkt pkt;
	pkt.head.channel = myproto::Channel::comunication;
	pkt.head.type = myproto::PktType::hello;
	myproto::addData( pkt.rawData, myproto::DataType::version, app::conf.version.toUtf8() );
	sendData( myproto::buidPkt( pkt ) );
}

void MainWindow::parsPktCommunication(const myproto::Pkt &pkt)
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

void MainWindow::parsPktAuth(const myproto::Pkt &pkt)
{
	QByteArray ba;
	switch (pkt.head.type) {
		case myproto::PktType::hello:
			ba = myproto::findData( pkt, myproto::DataType::text );
			app::setLog(3,QString("MainWindow::parsPktAuth server hello [%1]").arg(QString(ba)));
			if( ba != "hello" ){
				m_pControlSocket->close();
			}else{
				m_disconnector = 0;
			}
		break;
	}
}

