#ifndef MYLIST_H
#define MYLIST_H

#include <QObject>
#include <QAbstractTableModel>
#include <QTcpServer>
#include <QUdpSocket>
#include <QUrl>

#include "global.h"

class MyList : public QAbstractTableModel
{
public:
	MyList();
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool addTarget(const QUrl &url);
public slots:
	void slotUpdate();
private:
	std::vector<QTcpServer*> m_TcpServers;
	std::vector<QUdpSocket*> m_UdpServers;
	std::vector<ProxyData> m_data;
};

#endif // MYLIST_H
