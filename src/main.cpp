#include "capplication.h"
#include "include.h"

int main(int argc, char *argv[])
{
	CApplication a(argc, argv);
	a.setApplicationName(kExecName) ;
	a.setApplicationVersion(kVersion) ;

	if ( !a.checkOpt() ) {
		a.print_usage();
		return 1 ;
	}

	return a.execute() ;
}
