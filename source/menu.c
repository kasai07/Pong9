#include "menu.h"
#include "draw.h"
#include "hid.h"
#include "fs.h"
#include "pong.h"
#include "timer.h"

u32 Menu_Launcher()
{
 struct pongGlobals myPongGlobals;
	//Flag for restarting the entire game.
	myPongGlobals.restart = 1;
	//scale of game
	myPongGlobals.scale=1;
	//Default locations for paddles and ball location and movement dx/dy
	myPongGlobals.p1X_default=40*myPongGlobals.scale;
	myPongGlobals.p2X_default=340*myPongGlobals.scale;
	myPongGlobals.ballX_default=200*myPongGlobals.scale;
	myPongGlobals.p1Y_default=150*myPongGlobals.scale;
	myPongGlobals.p2Y_default=150*myPongGlobals.scale;
	myPongGlobals.ballY_default=120*myPongGlobals.scale;
	//Sizes of objects
	myPongGlobals.p1X_size=20*myPongGlobals.scale;
	myPongGlobals.p1Y_size=60*myPongGlobals.scale;
	myPongGlobals.ballX_size=8*myPongGlobals.scale;
	myPongGlobals.ballY_size=8*myPongGlobals.scale;
	myPongGlobals.p2X_size=20*myPongGlobals.scale;
	myPongGlobals.p2Y_size=60*myPongGlobals.scale;
	//Boundry of play area (screen)
	myPongGlobals.xMinBoundry=1*myPongGlobals.scale;
	myPongGlobals.xMaxBoundry=400*myPongGlobals.scale;
	myPongGlobals.yMinBoundry=1*myPongGlobals.scale;
	myPongGlobals.yMaxBoundry=240*myPongGlobals.scale;
	
	
	myPongGlobals.score1X=20;
	myPongGlobals.score2X=300;
	
	myPongGlobals.gamewin1X=20;
	myPongGlobals.gamewin2X=300;
	
	myPongGlobals.gamewin1Y=130;
	myPongGlobals.gamewin2Y=130;
	
	
	myPongGlobals.score1Y=110;
	myPongGlobals.score2Y=110;
	
	//Keep track of score
	myPongGlobals.score1 = 0;
	myPongGlobals.score2 = 0;
	
	myPongGlobals.gamewin1 = 0;
	myPongGlobals.gamewin2 = 0;
	
	myPongGlobals.scoreWin = 9;
	//Game engine globals
	myPongGlobals.direction = 1;
	myPongGlobals.count = 0;
	//Used for collision
	myPongGlobals.flag = 0;
	//Flag to determine if p1 should be rendered along with p1's movement direction
	myPongGlobals.renderP1Flag = 0;
	//Flag to determine if p2 should be rendered along with p2's movement direction
	myPongGlobals.renderP2Flag = 0;
	//Flags for render states
	myPongGlobals.renderResetFlag = 0;
	myPongGlobals.renderBallFlag = 0;
	myPongGlobals.renderWinFlag = 0;
	myPongGlobals.renderScoreFlag = 0;
	/****************************>            VPAD Loop            <****************************/
	
	while (1)
	{
		
		u32 pad_state = Input();
		//To exit the game
		if (pad_state & BUTTON_POWER)
		{
			PowerOff();
		} else if (pad_state & BUTTON_HOME) {
			Reboot();
        }
		//speed ball
		TimeOut(6);
		
		//If the game has been restarted, reset the game (we do this one time in the beginning to set everything up)
		if (myPongGlobals.restart == 1)
		{
			reset(&myPongGlobals);
			myPongGlobals.restart = 0;
		}		
		//Set old positions.
		updatePosition(&myPongGlobals);
		//Update location of player1 and 2 paddles
		p1Move(&myPongGlobals);
		p2Move(&myPongGlobals);

		//Update location of the ball
		moveBall(&myPongGlobals);
		//Check if their are any collisions between the ball and the paddles.
		checkCollision(&myPongGlobals);
		//Render the scene
		myPongGlobals.renderBallFlag = 1;
		render(&myPongGlobals);

		//Increment the counter (used for physicals calcuations)
		myPongGlobals.count+=1;

		
		
	}
	
}