#include <CppUTest/CommandLineTestRunner.h>
#include <cstdlib>
#include <ctime>

int main( int argc, char **argv )
{
	srand( time( nullptr ) );
	return RUN_ALL_TESTS( argc, argv );
}

// EOF
// /////////////////////////////////////////////////////////////////////////////
