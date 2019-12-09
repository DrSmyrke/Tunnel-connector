#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "global.h"
#include "mylist.h"

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
	void slot_addAddress();
private:
	Ui::MainWindow *ui;
	QTimer* m_pTimer;
	MyList* m_model;
};
#endif // MAINWINDOW_H
