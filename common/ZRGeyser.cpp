#include <math.h>
#include "ZRGame.h"
#include "ZRGameInternal.h"
#include "Constants.h"

bool ZeroRoboticsGameImpl::updateGeysers()
{
	for(int i = 0; i < 10; i++)
	{
		if (challInfo.world.geyserTime[i] != 0)	// if the geyser is active, check it
		{
			if ((challInfo.world.geyserTime[i] + GEYSER_ACTIVE_TIME) > apiImpl.api->getTime())	// if the geyser is currently on
			{
				// check if the satellite is nearby in the 2D grid and has not been hit by this geyser in the past GEYSER_PUSH_TIME
				if ((checkAtSquare(challInfo.world.geyserLocations[i][0], challInfo.world.geyserLocations[i][1])) &&
				    (challInfo.me.geyserSquare[0] != challInfo.world.geyserLocations[i][0]) &&
						(challInfo.me.geyserSquare[1] != challInfo.world.geyserLocations[i][1]) )
				{
					#ifdef ZR2D
						// in 2D the satellite is pushed back in the opposite direction it entered the geyser
						float dir[2];
						dir[0] = challInfo.me.zrState[VEL_X];		// current velocity into the geyser
						dir[1] = challInfo.me.zrState[VEL_Y];
						mathVecNormalize(dir, 2);								// normalize the direction vector
						challInfo.me.geyserForce[0] = -GEYSER_FORCE*dir[0];				// override the force request
						challInfo.me.geyserForce[1] = -GEYSER_FORCE*dir[1];
						challInfo.me.geyserTime = GEYSER_PUSH_TIME;
						challInfo.me.geyserSquare[0] = challInfo.world.geyserLocations[i][0];
						challInfo.me.geyserSquare[1] = challInfo.world.geyserLocations[i][1];
						GAME_TRACE(("Satellite entered GEYSER, being pushed towards [%4.3f, %4.3f]!", challInfo.me.geyserForce[0], challInfo.me.geyserForce[1]));
					#else
						// in 3D the satellite is pushed in the -Z direction directly, so only write time and square location
						challInfo.me.geyserTime = GEYSER_PUSH_TIME;
						challInfo.me.geyserSquare[0] = challInfo.world.geyserLocations[i][0];
						challInfo.me.geyserSquare[1] = challInfo.world.geyserLocations[i][1];
						GAME_TRACE(("Satellite entered GEYSER, being pushed to -Z!"));
					#endif
				}
			}
			else if ((challInfo.world.geyserTime[i] + GEYSER_ACTIVE_TIME) == apiImpl.api->getTime())
			{
				GAME_TRACE(("Geyser %d @ [%d,%d] shut OFF", i, challInfo.world.geyserLocations[i][0], challInfo.world.geyserLocations[i][1]));
				challInfo.world.geyserTime[i] = 0;
				challInfo.world.geyserLocations[i][0] = OFF_GRID;
				challInfo.world.geyserLocations[i][1] = OFF_GRID;
			}
		}
	}

	// impart forces if they are active
	if (challInfo.me.geyserTime > 0)
	{
		challInfo.me.geyserTime --;
		
		if (challInfo.me.geyserTime == 0)
		{
			challInfo.me.geyserForce[0] = 0.0f;
			challInfo.me.geyserForce[1] = 0.0f;
			challInfo.me.geyserSquare[0] = OFF_GRID;
			challInfo.me.geyserSquare[1] = OFF_GRID;
			GAME_TRACE(("Geyser push-out stopped."));
		}
	}
	return (challInfo.me.geyserTime > 0);
}

void ZeroRoboticsGameImpl::checkGeyser(int gridX, int gridY)
{
	float numDrills = challInfo.world.grid[gridX][gridY].numDrills;
	if (((float) rand() / (RAND_MAX)) < ((float)numDrills*numDrills*numDrills)/64.0f)
	{
		activateGeyser(gridX, gridY, (unsigned int) apiImpl.api->getTime());

		// check if geyser activated near base, then its a point penalty!
		if ((gridX > 5) && (gridX < 10) && (gridY > 7) && (gridY < 12))
		{
			GAME_TRACE(("Geyser AT BASE STATION, PENALTY of -%3.2f points!", SCORE_GEYSER_PENALTY));
			challInfo.me.score -= SCORE_GEYSER_PENALTY;
		}
	}
}

void ZeroRoboticsGameImpl::activateGeyser(int gridX, int gridY, unsigned int startTime)
{
	// first check that this geyser does not exist already
	for (int i=0; i < MAX_NUM_GEYSERS; i++)
	{
		if ((challInfo.world.geyserTime[i] == startTime) && 
		    (challInfo.world.geyserLocations[i][0] == gridX) &&
			  (challInfo.world.geyserLocations[i][1] == gridY) )
			return;		// do nothing, this geyser is already active
	}
	
	// a geyser needs to be activated, activate it if there is an open geyser slot (less than MAX_NUM_GEYSERS are active)
	for(int i = 0; i < MAX_NUM_GEYSERS; i++)
	{
		if (challInfo.world.geyserTime[i] == 0)		// geyserTime == 0 means that geyser spot is available
		{
			challInfo.world.geyserTime[i]         = startTime;
			challInfo.world.geyserLocations[i][0] = gridX;
			challInfo.world.geyserLocations[i][1] = gridY;
			GAME_TRACE(("Geyser [%d] ACTIVATED at grid location [%d, %d] with start time %d!", i, gridX, gridY, startTime));
			
			return;
		}
	}
	GAME_TRACE(("Geyser NOT activated: max number of geysers are active!"));
}

unsigned int ZeroRoboticsGame::getNumGeysers()
{
	return pimpl.challInfo.world.numActiveGeysers;
}

void ZeroRoboticsGame::getGeyserLocations(int grid[10][2])
{
	memcpy(&grid, &pimpl.challInfo.world.geyserLocations, sizeof(grid)); 
}

bool ZeroRoboticsGame::isGeyserHere(float pos[2])
{
	int posX = GRID_X(pos[0]);
	int posY = GRID_Y(pos[1]);
	
	for(int i =0; i < 10; i++)
	{
		if ((pimpl.challInfo.world.geyserLocations[i][0] == posX) && (pimpl.challInfo.world.geyserLocations[i][1] == posY))
				return true;
	}
	return false;
}

bool ZeroRoboticsGame::isGeyserHere(int grid[2])
{
	int posX = grid[0] + GRID_X_SIDE;
	int posY = grid[1] + GRID_Y_SIDE;
	
	for(int i =0; i < 10; i++)
	{
		if ((pimpl.challInfo.world.geyserLocations[i][0] == posX) && (pimpl.challInfo.world.geyserLocations[i][1] == posY))
				return true;
	}
	return false;
}
