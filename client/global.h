#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QDir>
#include <QHostAddress>

struct Host{
	QHostAddress ip;
	uint16_t port						= 0;
};

struct User{
	QString login;
	QString pass;
	uint32_t lastLoginTimestamp			= 0;
	uint32_t maxConnections				= 37;
	std::vector<Host> accessList;
	std::vector<Host> blockList;
	uint32_t inBytes					= 0;
	uint32_t outBytes					= 0;
	uint32_t bytesMax					= 536870912;
};

struct Config{
	bool verbose						= false;
	uint8_t logLevel					= 3;
#ifdef __linux__
	QString logFile						= "/tmp/tunnelConnector.log";
	QString usersFile					= "/etc/DrSmyrke/TunnelConnector/users.list";
#elif _WIN32
	QString logFile						= QDir::homePath() + "/tunnelConnector.log";
	QString usersFile					= QDir::homePath() + "/TunnelConnector/users.list";
#endif
	QString appName						= "tunnelConnector";
	QString version						= "0.1";
	uint16_t port						= 7301;
	uint16_t controlPort				= 7302;
	bool settingsSave					= false;
	bool usersSave						= false;
	std::vector<User> users;
	QByteArray realmString				= "TunnelConnector";
	Host server;
};

namespace app {
	extern Config conf;

	void loadSettings();
	void saveSettings();
	bool parsArgs(int argc, char *argv[]);
	void setLog(const uint8_t logLevel, const QString &mess);

	void loadUsers();
	void saveUsers();
}

#endif // GLOBAL_H
