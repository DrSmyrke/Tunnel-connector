#include "global.h"
#include "myfunctions.h"

#include <QIODevice>
#include <QDateTime>
#include <QSettings>

namespace app {
	Config conf;
	
	void loadSettings()
	{
		QSettings settings( "MySoft", app::conf.appName );

		auto addr = settings.value( "SERVER/port", "0.0.0.0").toString();
		app::conf.server.port = settings.value( "SERVER/port", app::conf.server.port ).toUInt();
		app::conf.server.ip.setAddress( addr );
	}

	void saveSettings()
	{
		QSettings settings( "MySoft", app::conf.appName );
		settings.clear();

		settings.setValue( "SERVER/ip",		app::conf.server.ip.toString() );
		settings.setValue( "SERVER/port",	app::conf.server.port );
	}

	bool parsArgs(int argc, char *argv[])
	{
		bool ret = true;
		for(int i=0;i<argc;i++){
			if(QString(argv[i]).indexOf("-")==0){
				if(QString(argv[i]) == "--help" or QString(argv[1]) == "-h"){
					printf("Usage: %s [OPTIONS]\n"
							"  -l <FILE>    log file\n"
							"  -c <FILE>    config file\n"
							"  -v    Verbose output\n"
							"\n", argv[0]);
					ret = false;
				}
				if(QString(argv[i]) == "-l") app::conf.logFile = QString(argv[++i]);
				//if(QString(argv[i]) == "-c") app::conf.confFile = QString(argv[++i]);
				if(QString(argv[i]) == "-v") app::conf.verbose = true;
			//}else{
			//	bool ok = false;
			//	QString(argv[i]).toInt(&ok,10);
			//	if(ok) app::conf.port = QString(argv[i]).toInt();
			}
		}
		return ret;
	}

	void setLog(const uint8_t logLevel, const QString &mess)
	{
		if(app::conf.logLevel < logLevel or app::conf.logLevel == 0) return;

		QDateTime dt = QDateTime::currentDateTime();
		QString str = dt.toString("yyyy.MM.dd [hh:mm:ss] ") + mess + "\n";

		if( app::conf.verbose ){
			printf( "%s", str.toUtf8().data() );
			fflush( stdout );
		}

		if( app::conf.logFile.isEmpty() ) return;
		QFile f;
		f.setFileName( app::conf.logFile );
		if( f.open( QIODevice::Append ) ){
			f.write( str.toUtf8() );
			f.close();
		}
	}

	void loadUsers()
	{
		if( app::conf.usersFile.isEmpty() ){
			app::setLog( 2, QString("LOAD USER FILE [%1] ... ERROR not defined").arg( app::conf.usersFile ) );
			return;
		}

		if( !mf::checkFile( app::conf.usersFile ) ){
			app::setLog( 2, QString("LOAD USER FILE [%1] ... ERROR not found").arg( app::conf.usersFile ) );
			return;
		}

		app::setLog( 3, QString("LOAD USER FILE [%1] ...").arg( app::conf.usersFile ) );

		app::conf.users.clear();

		QSettings users( app::conf.usersFile, QSettings::IniFormat );

		for( auto group:users.childGroups() ){
			app::setLog( 4, QString("   FOUND USER [%1]").arg( group ) );
			users.beginGroup( group );

			User user;

			user.login				= group;
			//user.group				= app::getUserGroupFromName( users.value( "group", "" ).toString() );
			user.pass				= users.value( "password", "" ).toString();
			user.maxConnections		= users.value( "maxConnections", user.maxConnections ).toUInt();
			user.bytesMax			= users.value( "bytesMax", user.bytesMax ).toUInt();
			auto accessList			= users.value( "accessList", "*" ).toString().split(",");
			auto blockList			= users.value( "blockList", "*" ).toString().split(",");

			//app::updateListFromList( accessList, user.accessList );
			//app::updateListFromList( blockList, user.blockList );

			app::conf.users.push_back( user );

			users.endGroup();
		}
	}

	void saveUsers()
	{
		if( app::conf.usersFile.isEmpty() ){
			app::setLog( 2, QString("SAVE USER FILE [%1] ... ERROR not defined").arg( app::conf.usersFile ) );
			return;
		}

		app::setLog( 3, QString("SAVE USER FILE [%1] ...").arg( app::conf.usersFile ) );

		QSettings users( app::conf.usersFile, QSettings::IniFormat );
		users.clear();

		for( auto user:app::conf.users ){
			users.beginGroup( user.login );

			//users.setValue( "group", app::getUserGroupNameFromID( user.group ) );
			users.setValue( "password", user.pass );
			users.setValue( "maxConnections", user.maxConnections );
			QStringList accessList;
			QStringList blockList;
			//app::updateListFromList( user.accessList, accessList );
			//app::updateListFromList( user.blockList, blockList );
			users.setValue( "accessList", accessList.join(",") );
			users.setValue( "blockList", blockList.join(",") );
			users.setValue( "bytesMax", user.bytesMax );

			users.endGroup();
		}

		app::conf.usersSave = false;
	}


	
}
