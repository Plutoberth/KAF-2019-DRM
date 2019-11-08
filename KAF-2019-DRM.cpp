#include <iostream>
#include "windows.h"


#define ExitDebug() if (IsDebuggerPresent()) {exit(1337);}

int main()
{
	ExitDebug();
	std::cout << "Test!" << std::endl;
	char _ = getchar();   
}
