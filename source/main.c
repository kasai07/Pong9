#include "common.h"
#include "draw.h"
#include "fs.h"
#include "pong.h"
#include "screen.h"
#include "menu.h"



int main()
{
    screenInit();
	ClearScreenFull(true, true);
	DebugClear();
	

	Menu_Launcher();
	
	return 0;
}
