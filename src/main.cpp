#include "application.h"
#include "include.h"

int main(int argc, char *argv[])
{
    Application a(argc, argv);
	a.setApplicationName(kExecName) ;
	a.setApplicationVersion(kVersion) ;

	if ( !a.checkOpt() ) {
		a.print_usage();
		return 1 ;
	}

	return a.execute() ;
}
