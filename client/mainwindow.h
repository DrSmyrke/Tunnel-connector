#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTimer>
#include "global.h"
#include "myproto.h"

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
private:
	Ui::MainWindow *ui;
	QTcpSocket* m_pControlSocket;
	QTimer* m_pTimer;
	uint8_t m_disconnector;

	void sendData(const QByteArray &data);
	QString setColorText(const QString &text, const uint8_t state = 0);
};
#endif // MAINWINDOW_H
