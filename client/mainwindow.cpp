#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->connectB->setText( tr("CONNECT") );
	ui->statusL->setText( "" );
	ui->serverAddrBox->setText( app::conf.server );

	m_pControlSocket = new QTcpSocket( this );
	m_pTimer = new QTimer( this );
		m_pTimer->setInterval( 100 );

	setWindowTitle( "Tunnel Connector v" + app::conf.version );
	setWindowIcon( QIcon( "://index.ico" ) );
	setMinimumSize( 270, 220 );

	connect( ui->connectB, &QPushButton::clicked, this, &MainWindow::slot_connect );
	//connect( m_pControlSocket, &QTcpSocket::readyRead, this );
	connect( m_pControlSocket, &QTcpSocket::stateChanged, this, &MainWindow::slot_stateChange );
	connect( m_pTimer, &QTimer::timeout, this, &MainWindow::slot_timer );

	m_pTimer->start();
}

MainWindow::~MainWindow()
{
	if( m_pTimer->isActive() ) m_pTimer->stop();
	delete ui;
}

void MainWindow::slot_readyRead()
{
//	QByteArray buff;
//	while( m_pSPort->bytesAvailable() ) buff.append( m_pSPort->readAll() );

//	m_pHexViewer->appendData( buff );
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
			app::conf.server = ui->serverAddrBox->text();
			app::conf.settingsSave = true;
			m_disconnector = 10;
			myproto::Pkt pkt;
			pkt.
			sendData( myproto::buidPkt( pkt ) );
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

