#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QLabel>
#include "global.h"
#include "hexviewer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
private slots:
	void slot_readyRead();
	void slot_sendMess();
	void slot_textChanged(const QString &text);
private:
	Ui::MainWindow *ui;
	QSerialPort* m_pSPort;
	QLabel* m_pPortLabel;
	QLabel* m_pPortError;
	HexViewer* m_pHexViewer;

	void rescanPorts();
	bool checkPort(const QString &port);
	void sendData(const QByteArray &data);
	void hexReMask(uint8_t num = 1);
};
#endif // MAINWINDOW_H
