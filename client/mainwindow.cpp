#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	m_pPortLabel = new QLabel(this);
	m_pPortError = new QLabel(this);
	m_pHexViewer = new HexViewer(this);
		m_pHexViewer->setBlockSize( 8 );

	m_pSPort = new QSerialPort(this);
		m_pSPort->setBaudRate( 9600 );
		m_pSPort->setDataBits( QSerialPort::DataBits::Data8 );
		m_pSPort->setParity( QSerialPort::Parity::NoParity );
		m_pSPort->setStopBits( QSerialPort::StopBits::OneStop );
		m_pSPort->setFlowControl( QSerialPort::FlowControl::NoFlowControl );

	setWindowTitle( "Serial Terminal v" + app::conf.version );
	setWindowIcon( QIcon( "://index.ico" ) );
	setMinimumSize( 640, 480 );

	rescanPorts();

	m_pSPort->setPortName( ui->portBox->currentText() );
	m_pPortLabel->setText( ui->portBox->currentText() );
	ui->connectB->setText( tr( "OPEN" ) );
	ui->statusbar->addWidget( m_pPortLabel );
	ui->statusbar->addWidget( m_pPortError );
	ui->scrollArea->setWidget( m_pHexViewer );

	connect( ui->connectB, &QPushButton::clicked, this, [this](){
		if( !m_pSPort->isOpen() ){
			if( m_pSPort->open( QIODevice::ReadWrite ) ){
				ui->connectB->setText( tr( "CLOSE" ) );
			}else{
				m_pPortError->setText( m_pSPort->errorString() );
			}
		}else{
			m_pSPort->close();
			if( !m_pSPort->isOpen() ) ui->connectB->setText( tr( "OPEN" ) );
		}
	} );

	connect( m_pSPort, &QSerialPort::readyRead, this, &MainWindow::slot_readyRead );
	connect( ui->portBox, &QComboBox::currentTextChanged, this, [this](const QString &portName){
		m_pSPort->setPortName( portName );
		m_pPortLabel->setText( portName );
	} );
	connect( ui->messLine, &QLineEdit::returnPressed, this, &MainWindow::slot_sendMess );
	connect( ui->messLine, &QLineEdit::textChanged, this,  &MainWindow::slot_textChanged );
	connect( ui->sendB, &QPushButton::clicked, this, &MainWindow::slot_sendMess );
	connect( ui->inputModeHex, &QRadioButton::clicked, this, [this](){
		ui->messLine->clear();
		hexReMask(1);
		ui->messLine->setFocus();
	} );
	connect( ui->inputModeAscii, &QRadioButton::clicked, this, [this](){
		ui->messLine->clear();
		ui->messLine->setInputMask("");
		ui->messLine->setFocus();
	} );
	connect( m_pHexViewer, &HexViewer::signal_customContextMenu, this, [this](const QPoint pos){
		QMenu* menu = new QMenu(this);
		QAction* clearA = new QAction(tr("Clear"), this);
		connect(clearA, &QAction::triggered, this, [this](){
			m_pHexViewer->clearData();
		});
		menu->addAction( clearA );
		menu->popup( ui->scrollArea->viewport()->mapToGlobal( pos ) );
	} );

	ui->messLine->setFocus();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::slot_readyRead()
{
	QByteArray buff;
	while( m_pSPort->bytesAvailable() ) buff.append( m_pSPort->readAll() );

	m_pHexViewer->appendData( buff );
}

void MainWindow::slot_sendMess()
{
	QByteArray data;
	auto text = ui->messLine->text();
	data.append( text );
	sendData( data );

	//m_pHexViewer->appendData( data );
}

void MainWindow::slot_textChanged(const QString &text)
{
	if( ui->inputModeHex->isChecked() ){
		uint8_t num = text.length() / 3;
		hexReMask( num + 1 );
	}
}

void MainWindow::rescanPorts()
{
	ui->portBox->clear();
	for( auto portInfo:QSerialPortInfo::availablePorts() ){
		ui->portBox->addItem( portInfo.portName() );
	}

	/*
#ifdef __linux__

#elif _WIN32
	for( uint8_t i = 1; i < 250; i++ ){
		auto str = QString("COM%1").arg( i );
		if( checkPort( str ) ) ui->portBox->addItem( str );
	}
#endif
	*/
}

bool MainWindow::checkPort(const QString &port)
{
	bool res = false;

	if( m_pSPort->isOpen() ) m_pSPort->close();

	m_pSPort->setPortName( port );

	if( m_pSPort->open( QIODevice::ReadOnly ) ){
		m_pSPort->close();
		res = true;
	}

	return res;
}

void MainWindow::sendData(const QByteArray &data)
{
	if( !m_pSPort->isOpen() ) return;
	if( data.size() == 0 ) return;
	m_pSPort->write( data );
}

void MainWindow::hexReMask(uint8_t num)
{
	QString mask = ">";
	for( uint8_t i = 0; i < num; i++ ) mask += "HH ";
	mask += ";_";
	if( ui->messLine->inputMask() != mask ){
		ui->messLine->setInputMask(mask);
		ui->messLine->setCursorPosition( ui->messLine->text().length() - 1 );
	}
}

