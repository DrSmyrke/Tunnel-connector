#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include "global.h"
#include "myproto.h"
#include "mylist.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	struct StatusState
	{
		enum{
			normal = 1,
			processing,
			warning,
			error
		};
	};
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
private slots:
	void slot_readyRead();
	void slot_connect();
	void slot_stateChange(const QAbstractSocket::SocketState socketState);
	void slot_timer();
	void slot_addAddress();
private:
	Ui::MainWindow *ui;
	QTcpSocket* m_pControlSocket;
	QTimer* m_pTimer;
	uint8_t m_disconnector;
	QByteArray m_rxBuff;
	myproto::Pkt m_pkt;
	bool m_auth;
	MyList* m_model;

	void sendData(const QByteArray &data);
	QString setColorText(const QString &text, const uint8_t state = 0);
	void sendInit();
	void parsPktCommunication(const myproto::Pkt &pkt);
	void parsPktAuth(const myproto::Pkt &pkt);
};
#endif // MAINWINDOW_H
