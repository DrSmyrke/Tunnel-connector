#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myfunctions.h"

#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	m_pTimer = new QTimer( this );
		m_pTimer->setInterval( 100 );
	m_pConnector = new Connector( this );
	m_pServer = new LocalServer( this );

	ui->setupUi(this);

	setWindowTitle( "Tunnel Connector v" + app::conf.version );
	setWindowIcon( QIcon( "://index.ico" ) );
	setMinimumSize( 270, 190 );

	connect( m_pTimer, &QTimer::timeout, this, &MainWindow::slot_timer );
	connect( ui->serverAddrBox, &QLineEdit::editingFinished, this, [this](){
		app::conf.server =  ui->serverAddrBox->text();
		app::conf.settingsSave = true;
	} );
	connect( ui->loginBox, &QLineEdit::editingFinished, this, [this](){
		app::conf.user.login = ui->loginBox->text();
		app::conf.settingsSave = true;
	} );
	connect( ui->passwordBox, &QLineEdit::editingFinished, this, [this](){
		app::conf.user.pass = app::conf.user.login + ":>:" + ui->passwordBox->text();
		app::conf.user.pass = mf::md5( app::conf.user.pass.toUtf8() );
		app::conf.settingsSave = true;
	} );
	connect( ui->addAddressBox, &QLineEdit::editingFinished, this, [this](){
		QUrl url;
		url.setUrl( ui->addAddressBox->text() );
		if( !url.isValid() || url.port() < 1 ){
			ui->addAddressBox->setText( "tcp://" );
		}else{
			m_pConnector->setTarget( url );
		}
	} );
	connect( m_pConnector, &Connector::signal_stateChanged, this, [this](const uint8_t state){
		ui->statusL->setText( setColorText( state ) );
	} );
	connect( ui->connectB, &QPushButton::clicked, this, [this](){
		m_pConnector->init();
	} );
	connect( m_pConnector, &Connector::signal_accessGaranted, this, [this](){
		m_pServer->run();
		ui->statusL->setText( setColorText( m_pConnector->getState() ) );
	} );
	connect( m_pConnector, &Connector::signal_newData, m_pServer, &LocalServer::slot_incommingData );
	connect( m_pServer, &LocalServer::signal_newData, m_pConnector, &Connector::slot_incomingData );

	ui->connectB->setText( tr("CONNECT") );
	ui->serverAddrBox->setText( app::conf.server );
	ui->loginBox->setText( app::conf.user.login );
	ui->addAddressBox->setText( "tcp://" );
	ui->addAddressBox->setFocus();
	ui->statusL->clear();

	m_pTimer->start();
}

MainWindow::~MainWindow()
{
	m_pConnector->stop();
	if( m_pTimer->isActive() ) m_pTimer->stop();
	delete ui;
}

void MainWindow::slot_timer()
{
	if( app::conf.settingsSave ) app::saveSettings();
}

QString MainWindow::setColorText(const uint8_t state) const
{
	QString text;
	QString res = "<span style=\"color:";
	switch (state) {
		case StatusConnectState::normal:
			res += "green";
			text = tr("Connected");
			ui->connectB->setText( tr("DISCONNECT") );
			ui->connectB->setEnabled( true );
		break;
		case StatusConnectState::processing:
			res += "orange";
			text = tr("Connecting...");
			ui->connectB->setEnabled( false );
		break;
		case StatusConnectState::warning:
			res += "yellow";
			text = tr("Warning");
			ui->connectB->setEnabled( true );
		break;
		case StatusConnectState::error:
			res += "red";
			text = m_pConnector->getStateString();
			if( text.isEmpty() ) text = tr("Unknown error");
			ui->connectB->setEnabled( true );
		break;
		case StatusConnectState::disconnected:
			res += "gray";
			text = tr("Disconnected");
			ui->connectB->setText( tr("CONNECT") );
			ui->connectB->setEnabled( true );
			m_pServer->stop();
		break;
		default:
			res += "gray";
			text = tr("Unknown");
			ui->connectB->setEnabled( true );
		break;
	}

	text += m_pServer->getState();

	res += ";\">" + text + "</span>";

	return res;
}
