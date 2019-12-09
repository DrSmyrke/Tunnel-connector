#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QDir>
#include <QHostAddress>
#include <QUrl>

struct StatusConnectState
{
	enum{
		normal = 1,
		processing,
		warning,
		error,
		disconnected
	};
};

struct Host{
	QHostAddress ip;
	uint16_t port						= 0;
};

struct ProxyData{
	QUrl target;
	uint16_t localPort					= 0;
	bool connected						= false;
	uint8_t id							= 0;
	uint16_t socketDescriptor			= 0;
	QString status;
};

struct User{
	QString login;
	QString pass;
	uint32_t lastLoginTimestamp			= 0;
	uint32_t maxConnections				= 37;
	uint32_t inBytes					= 0;
	uint32_t outBytes					= 0;
	uint32_t bytesMax					= 536870912;
};

struct Config{
	bool verbose						= false;
	uint8_t logLevel					= 3;
#ifdef __linux__
	QString logFile						= "/tmp/tunnelConnector.log";
#elif _WIN32
	QString logFile						= QDir::homePath() + "/tunnelConnector.log";
#endif
	QString appName						= "tunnelConnector";
	QString version						= "0.1";
	uint16_t port						= 7301;
	bool settingsSave					= false;
	User user;
	QByteArray realmString				= "TunnelConnector";
	QString server;
	std::vector<QUrl> connections;
};

namespace app {
	extern Config conf;

	void loadSettings();
	void saveSettings();
	bool parsArgs(int argc, char *argv[]);
	void setLog(const uint8_t logLevel, const QString &mess);
}

#endif // GLOBAL_H
