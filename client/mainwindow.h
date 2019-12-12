#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "global.h"
#include "connector.h"
#include "localserver.h"

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
	void slot_timer();
private:
	Ui::MainWindow *ui;
	QTimer* m_pTimer;
	Connector* m_pConnector;
	LocalServer* m_pServer;

	QString setColorText(const uint8_t state = StatusConnectState::disconnected) const;
};
#endif // MAINWINDOW_H
