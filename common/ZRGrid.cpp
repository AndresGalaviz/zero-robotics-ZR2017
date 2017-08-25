#include <stdio.h>
#include <math.h>
#include "ZRGame.h"
#include "ZRGameInternal.h"
#include "Constants.h"

// TODO: Verify crash into surface

/*****************HIDDEN FUNCTIONS*****************/

void ZeroRoboticsGameImpl::initializeWorld(int concentrationX, int concentrationY)
{
	int peakConcentration2[2];

	#ifndef ISS_FINALS
		// SPHERE1 initialization (if arguments are not sent, they are initialized to -1 in the header)
		if(concentrationX == -1 || concentrationY == -1)
		{
			// Peak Concentrations can be anywhere except keep out:
			//  - 2 grid sizes from the center
			//  - one from edges 
			// i.e. x=[3..6] and y = [3..8]
			challInfo.world.peakConcentration[0] = rand()%4+3;
			if(rand()%2==1)
				challInfo.world.peakConcentration[0]*=-1;
			challInfo.world.peakConcentration[1] = rand()%6+3;
			if(rand()%2==1)
				challInfo.world.peakConcentration[1]*=-1;
		}
		else
	#endif
	{
		// For when sphere 1 sends initialization data to sphere 2 or for ISS Finals
		challInfo.world.peakConcentration[0] =  concentrationX;
		challInfo.world.peakConcentration[1] =  concentrationY;
	}
	
	peakConcentration2[0] = -1*challInfo.world.peakConcentration[0];
	peakConcentration2[1] = -1*challInfo.world.peakConcentration[1];

	// Available concentrations
	int concentrations[] = {HIGH_CONCENTRATION, MED_CONCENTRATION, LOW_CONCENTRATION, MIN_CONCENTRATION};

	// Initialize grid
	for(int j = -GRID_Y_SIDE; j < GRID_Y_SIDE; j++)
	{
		for(int i = -GRID_X_SIDE; i < GRID_X_SIDE; i++)
		{
			// World.peakConcentration needs to be [...-2,-1,1,2...] (no zero),while access to the grid is [...,-2,-1,0,1,2,...]
			int difX1 = (challInfo.world.peakConcentration[0] - i);
			int difY1 = (challInfo.world.peakConcentration[1] - j);
			if (challInfo.world.peakConcentration[0] > 0) difX1--;
			if (challInfo.world.peakConcentration[1] > 0) difY1--; //to deal with the edge case of row/column 0

			int difX2 = (peakConcentration2[0] - i);
			int difY2 = (peakConcentration2[1] - j);
			if (peakConcentration2[0] > 0) difX2--;
			if (peakConcentration2[1] > 0) difY2--; //same as above

			int dist1 = roundf(sqrt(difX1*difX1+difY1*difY1));
			int dist2 = roundf(sqrt(difX2*difX2+difY2*difY2));

			// Takes into consideration the longest distance and assigns concentration accordingly 
			if (dist1 < dist2)
				challInfo.world.grid[i + GRID_X_SIDE][j + GRID_Y_SIDE].concentration = dist1 > 3 ? MIN_CONCENTRATION : concentrations[dist1];
			else 
				challInfo.world.grid[i + GRID_X_SIDE][j + GRID_Y_SIDE].concentration = dist2 > 3 ? MIN_CONCENTRATION : concentrations[dist2];
		}
	}
  
	GAME_TRACE(("Playing field initialized. Peak concentrations @ [%d, %d] & [%d, %d].",
								challInfo.world.peakConcentration[0], challInfo.world.peakConcentration[1],
								peakConcentration2[0], peakConcentration2[1]));
	
	#ifdef SHOW_DEBUG_GRID
		printf("peak concentration 1 %d %d \n",challInfo.world.peakConcentration[0],challInfo.world.peakConcentration[1]);
		printf("peak concentration 2 %d %d \n",peakConcentration2[0],peakConcentration2[1]);

		for(int j = -GRID_Y_SIDE; j < GRID_Y_SIDE; j++)
		{
			for(int i = -GRID_X_SIDE; i < GRID_X_SIDE; i++) 
				printf("%*d ", 3, challInfo.world.grid[i + GRID_X_SIDE][j + GRID_Y_SIDE].concentration);
			printf("\n");
		}

		GAME_TRACE(("                      |-8765432112345678+|"));
		for(int j = 0; j < Y_SIZE; j++)
		{
			int idx = (j >= GRID_Y_SIDE) ? (j - GRID_Y_SIDE + 1) : (j - GRID_Y_SIDE);
			GAME_TRACE(("grid row: %d |%d| col: |%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x|", j, idx,
			  challInfo.world.grid[0][j].concentration,
				challInfo.world.grid[1][j].concentration,
			  challInfo.world.grid[2][j].concentration,
				challInfo.world.grid[3][j].concentration,
				challInfo.world.grid[4][j].concentration,
				challInfo.world.grid[5][j].concentration,
				challInfo.world.grid[6][j].concentration,
				challInfo.world.grid[7][j].concentration,
				challInfo.world.grid[8][j].concentration,
				challInfo.world.grid[9][j].concentration,
				challInfo.world.grid[10][j].concentration,
				challInfo.world.grid[11][j].concentration,
				challInfo.world.grid[12][j].concentration,
				challInfo.world.grid[13][j].concentration,
				challInfo.world.grid[14][j].concentration,
				challInfo.world.grid[15][j].concentration));
		}
	#endif
}

bool ZeroRoboticsGameImpl::checkAtSquare(int gridX, int gridY)
{
	float gridXcenter = ((float)(gridX-GRID_X_SIDE)*CELL_SIZE) + (CELL_SIZE/2.0f);
	float diffX = fabs(gridXcenter - challInfo.me.sphState[0]);
	bool X_OK = diffX < (CELL_SIZE/2.0f);
	
	#ifdef SHOW_DEBUG
		if (!X_OK) GAME_TRACE(("checkAtSquare [%d,%d] X : Center X = %4.3f; diffX = %4.3f", gridX, gridY, gridXcenter, diffX));
	#endif
	
	float gridYcenter = ((float)(gridY-GRID_Y_SIDE)*CELL_SIZE) + (CELL_SIZE/2.0f);
	float diffY = fabs(gridYcenter - challInfo.me.sphState[1]);
  bool Y_OK = diffY < (CELL_SIZE/2.0f);

	#ifdef SHOW_DEBUG
		if (!Y_OK) GAME_TRACE(("checkAtSquare [%d,%d] Y : Center Y = %4.3f; diffY = %4.3f", gridX, gridY, gridYcenter, diffY));
	#endif
	
	return X_OK && Y_OK;
}

#ifndef ZR2D
void ZeroRoboticsGameImpl::initializeTerrainHeight()
{
	#ifndef ISS_FINALS
		srand (time(NULL));
		
		for(int i = 0; i < X_SIZE; i++)
		{
			for(int j = 0; j < Y_SIZE; j++)
			{
				challInfo.world.grid[i][j].height = rand()%4+1;
			}
		}
	#else
		// assign fixed values
		unsigned int ISS_HEIGHTS[X_SIZE/8][Y_SIZE] = { {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2},
																									 {1, 2} };
		for(int j = 0; j < Y_SIZE; j++)
		{
			for(int i = 0; i < (X_SIZE/8); i++)
			{
				challInfo.world.grid[i][j].height = (ISS_HEIGHTS[i][j] >> 28) & 0xF;
				challInfo.world.grid[i][j].height = (ISS_HEIGHTS[i][j] >> 24) & 0xF;
				challInfo.world.grid[i][j].height = (ISS_HEIGHTS[i][j] >> 20) & 0xF;
				challInfo.world.grid[i][j].height = (ISS_HEIGHTS[i][j] >> 16) & 0xF;
				challInfo.world.grid[i][j].height = (ISS_HEIGHTS[i][j] >> 12)  0xF;
				challInfo.world.grid[i][j].height = (ISS_HEIGHTS[i][j] >>  8) & 0xF;
				challInfo.world.grid[i][j].height = (ISS_HEIGHTS[i][j] >>  4) & 0xF;
				challInfo.world.grid[i][j].height = (ISS_HEIGHTS[i][j])       & 0xF;
			}
		}
	#endif

	printf("\n Sphere 1 grid height: \n");
	for(int j = -GRID_Y_SIDE; j < GRID_Y_SIDE; j++)
	{
		for(int i = -GRID_X_SIDE; i < GRID_X_SIDE; i++)
			printf("%*d ", 3, challInfo.world.grid[i + GRID_X_SIDE][j + GRID_Y_SIDE].height);
		printf("\n");
	}
}
#endif

/***********USER FUNCTIONS*************/
unsigned int ZeroRoboticsGame::getDrills(float pos[2]) 
{
	int posX = GRID_X(pos[0]);
	int posY = GRID_Y(pos[1]);
	return pimpl.challInfo.world.grid[posX][posY].numDrills;
}

unsigned int ZeroRoboticsGame::getDrills(int grid[2]) 
{
	int posX = grid[0] + GRID_X_SIDE;
	int posY = grid[1] + GRID_Y_SIDE;
	return pimpl.challInfo.world.grid[posX][posY].numDrills;
}
