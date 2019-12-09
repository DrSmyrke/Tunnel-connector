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
	m_model = new MyList();

	ui->setupUi(this);

	setWindowTitle( "Tunnel Connector v" + app::conf.version );
	setWindowIcon( QIcon( "://index.ico" ) );
	setMinimumSize( 270, 220 );

	connect( m_pTimer, &QTimer::timeout, this, &MainWindow::slot_timer );
	connect( ui->addAddressB, &QPushButton::clicked, this, &MainWindow::slot_addAddress );
	connect( ui->tableView, &QTableView::doubleClicked, this, [this](const QModelIndex &index){
		if( !index.isValid() ) return;
		if( index.column() == 1 ) m_model->requestConnect( index.row() );
	} );

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

	ui->serverAddrBox->setText( app::conf.server );
	ui->loginBox->setText( app::conf.user.login );
	ui->tableView->setModel( m_model );
	ui->tableView->setEditTriggers( QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked );

	m_pTimer->start();
}

MainWindow::~MainWindow()
{
	m_model->deleteLater();
	if( m_pTimer->isActive() ) m_pTimer->stop();
	delete ui;
}

void MainWindow::slot_timer()
{
	if( app::conf.settingsSave ) app::saveSettings();

	ui->tableView->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
}

void MainWindow::slot_addAddress()
{
	QUrl url;
	url.setUrl( ui->addAddressBox->text() );
	if( !url.isValid() || url.port() < 1 ) return;

	if( m_model->addTarget( url ) ) ui->addAddressBox->clear();
}
