#include "mylist.h"

MyList::MyList()
{

}

int MyList::rowCount(const QModelIndex &parent) const
{
	return m_data.size();
}

int MyList::columnCount(const QModelIndex &parent) const
{
	return 2;
}

QVariant MyList::data(const QModelIndex &index, int role) const
{
	QVariant data;
	if( role == Qt::DisplayRole ){
		auto elem = m_data.at( index.row() );
		if( index.column() == 0 ) data.setValue( elem.target.toString() );
		if( index.column() == 1 ) data.setValue( elem.status );
	}
	return data;
}

bool MyList::addTarget(const QUrl &url)
{
	this->beginInsertRows( QModelIndex(), m_data.size(), m_data.size() + 1 );
	bool res = false;
	bool find = false;
	for( auto elem:m_data ){
		if( elem.target == url ){
			find = true;
			break;
		}
	}

	if( !find ){
		app::setLog(3,QString("MyList::addTarget [%1:%2][%3]").arg( url.host() ).arg( url.port() ).arg( url.scheme() ));

		uint16_t id;
		for( id = 1; id < 65535; id++ ){
			find = false;
			for( auto elem:m_data ){
				if( elem.id == id ){
					find = true;
					break;
				}
			}
			if( !find ) break;
		}

		if( !find ){
			ProxyData data;
			data.target = url;
			data.id = id;
			data.status = "disconnected";

			m_data.push_back( data );
			res = true;
		}
	}

	this->endInsertRows();

	return res;
}

void MyList::slotUpdate()
{
	emit(dataChanged(QModelIndex(), QModelIndex()));
}
