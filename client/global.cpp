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

		app::conf.server		= settings.value( "SERVER/server", app::conf.server ).toString();
		app::conf.user.login	= settings.value( "USER/login", app::conf.user.login ).toUInt();
	}

	void saveSettings()
	{
		QSettings settings( "MySoft", app::conf.appName );
		settings.clear();

		settings.setValue( "SERVER/server",	app::conf.server );
		settings.setValue( "USER/login",	app::conf.user.login );
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
}
