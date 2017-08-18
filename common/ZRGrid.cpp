#include <string.h>
#include <math.h>
#include "ZRGame.h"
#include "ZRGameInternal.h"
#include <stdlib.h>
#include <stdio.h>
#include "Constants.h"
#include <time.h>
// TODO: Verify crash into surface

/*****************HIDDEN FUNCTIONS*****************/

void ZeroRoboticsGameImpl::initializeWorld(int concentrationX, int concentrationY) {
        if(concentrationX == -1 || concentrationY == -1) {
                // Random is slightly biased toward low numbers however in this range is not significant
                //Peak Concentration 1 can be anywhere except keep out 2 grid sizes from the center and one from edges (i.e. x=[3..7] and y = [3..9])
                challInfo.world.peakConcentration1[0] = rand()%5+3;
                if(rand()%2==1)
                    challInfo.world.peakConcentration1[0]*=-1;
                challInfo.world.peakConcentration1[1] = rand()%7+3;
                if(rand()%2==1)
                    challInfo.world.peakConcentration1[1]*=-1;
                //Peak Concentration 2 is mirrored from peak concentration 1 
                challInfo.world.peakConcentration2[0] = -1*challInfo.world.peakConcentration1[0];
                challInfo.world.peakConcentration2[1] = -1*challInfo.world.peakConcentration1[1];
        }
    else { //For when sphere 1 sends initialization data to sphere 2 
        challInfo.world.peakConcentration1[0] = concentrationX;
        challInfo.world.peakConcentration1[1] = concentrationY;
        challInfo.world.peakConcentration2[0] = -concentrationX;
        challInfo.world.peakConcentration2[1] = -concentrationY;
    }
    //analyzer1 is at x = .3 y = .48 z = .36
    challInfo.world.analyzer1Coords[0] = .3;
    challInfo.world.analyzer1Coords[1] = .48;
    challInfo.world.analyzer2Coords[0] = -.3;
    challInfo.world.analyzer2Coords[1] = -.48;
    #ifdef ZR3D
        challInfo.world.analyzer1Coords[2] = -0.36; 
        challInfo.world.analyzer2Coords[2] = -0.36;
    #else 
        challInfo.world.analyzer1Coords[2] = 0;
        challInfo.world.analyzer2Coords[2] = 0;
    #endif

    // Available concentrations
    int concentrations[] = {HIGH_CONCENTRATION, MED_CONCENTRATION, LOW_CONCENTRATION, MIN_CONCENTRATION};
    // Initialize grid, only traverse one side
    for(int i = -GRID_Y_SIDE; i < GRID_Y_SIDE; i++) {
		for(int j = -GRID_X_SIDE; j < GRID_X_SIDE; j++) {
            //World.peakConcentration needs to be [...-2,-1,1,2...] (no zero),while access to the grid is [...,-2,-1,0,1,2,...]
            int difX1 = (challInfo.world.peakConcentration1[0] - j);
            int difY1 = (challInfo.world.peakConcentration1[1] - i);
            if(challInfo.world.peakConcentration1[0]>0) difX1--;
            if(challInfo.world.peakConcentration1[1]>0) difY1--; //to deal with the edge case of row/column 0

            int difX2 = (challInfo.world.peakConcentration2[0] - j);
            int difY2 = (challInfo.world.peakConcentration2[1] - i);
            if(challInfo.world.peakConcentration2[0]>0) difX2--;
            if(challInfo.world.peakConcentration2[1]>0) difY2--; //same as above

            int dist1 = round(sqrt(difX1*difX1+difY1*difY1));
            int dist2 = round(sqrt(difX2*difX2+difY2*difY2));

        // Takes into consideration the longest distance and assigns concentration accordingly 
            if(dist1<dist2) 
                challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].concentration = dist1 > 3 ? MIN_CONCENTRATION : concentrations[dist1];
            else 
                challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].concentration = dist2 > 3 ? MIN_CONCENTRATION : concentrations[dist2];
        }
	}
    
    #ifdef SHOW_GAME_TRACE
    printf("peak concentration 1 %d %d \n",challInfo.world.peakConcentration1[0],challInfo.world.peakConcentration1[1]);
    printf("peak concentration 2 %d %d \n",challInfo.world.peakConcentration2[0],challInfo.world.peakConcentration2[1]);

        for(int i = -GRID_Y_SIDE; i < GRID_Y_SIDE; i++) {
            for(int j = -GRID_X_SIDE; j < GRID_X_SIDE; j++) 
                printf("%*d ", 3, challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].concentration);
            printf("\n");
        }
    #endif

    GAME_TRACE(("peakConcentration1:%d,%d", challInfo.world.peakConcentration1[0], challInfo.world.peakConcentration1[1]));
    GAME_TRACE(("peakConcentration2: %d ,%d",challInfo.world.peakConcentration2[0],challInfo.world.peakConcentration2[1]));
    for(int i = 0;i<Y_SIZE;i++){
        GAME_TRACE(("grid row: %d col: %x%x%x%x%x%x%x%x%x%x%x%x%x%x%x%x" , i, challInfo.world.grid[i][0].concentration,challInfo.world.grid[i][1].concentration,
challInfo.world.grid[i][2].concentration,challInfo.world.grid[i][3].concentration,challInfo.world.grid[i][4].concentration,challInfo.world.grid[i][5].concentration,challInfo.world.grid[i][6].concentration,challInfo.world.grid[i][7].concentration,challInfo.world.grid[i][8].concentration,           challInfo.world.grid[i][9].concentration,challInfo.world.grid[i][10].concentration,challInfo.world.grid[i][11].concentration,challInfo.world.grid[i][12].concentration,challInfo.world.grid[i][13].concentration,challInfo.world.grid[i][14].concentration,challInfo.world.grid[i][15].concentration));}
}


void ZeroRoboticsGameImpl::initializeTerrainHeight(){
    srand (time(NULL));    
    #ifdef ZR3D
        for(int i = 0;i<Y_SIZE;i++){
            for(int j = 0;j<XZ_SIZE;j++){
                    challInfo.world.grid[i][j].height = rand()%4+1;
            }
        }
        printf("\n Sphere 1 grid height: \n");
        for(int i = -GRID_Y_SIDE; i < GRID_Y_SIDE; i++) {
            for(int j = -GRID_X_SIDE; j < GRID_X_SIDE; j++) 
                printf("%*d ", 3, challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].height);
            printf("\n");
        }

    #endif
}


void ZeroRoboticsGameImpl::turnOffOldGeysers()
{
    for(int i = 0; i < 10; i++) {
        if (challInfo.world.geyserActiveTime[i] >= 0) {
            if(++challInfo.world.geyserActiveTime[i] >= GEYSER_ACTIVE_TIME) {
                challInfo.world.geyserActiveTime[i] = -1;
                challInfo.world.geyserLocations[i][0] = -1;
                challInfo.world.geyserLocations[i][1] = -1;
                // Game trace: X,Y,0 -> Location for turning off geyser
             //   GAME_TRACE(("[%d]geyserLocations:%d,%d,0|", challInfo.currentTime, challInfo.world.geyserLocations[i][0], challInfo.world.geyserLocations[i][1]));
            }
        }
    }
}

/***********USER FUNCTIONS*************/
int ZeroRoboticsGame::getDrills(float pos[3]) 
{
    int posX = static_cast<int>(pos[1]/CELL_SIZE);
    int posY = static_cast<int>(pos[0]/CELL_SIZE);
    return pimpl.challInfo.world.grid[posY + GRID_Y_SIDE][posX + GRID_X_SIDE].numDrills;
}

int ZeroRoboticsGame::getNumGeysers()
{
    return pimpl.challInfo.world.numActiveGeysers;
}

void ZeroRoboticsGame::getGeyserLocations(float locations[10][2])
{
    memcpy(&locations, &pimpl.challInfo.world.geyserLocations, sizeof(locations)); 
}

bool ZeroRoboticsGame::isGeyserHere(float pos[3])
{
    int posX = static_cast<int>(pos[0]/CELL_SIZE);
    int posY = static_cast<int>(pos[1]/CELL_SIZE);
    int posZ = static_cast<int>(pos[2]/CELL_SIZE);
    // QUESTION: Are geysers of a defined or variable height? (Here implemented for defined height of 12 units)
    for(int i =0; i < 10; i++) {
        int geyserX = static_cast<int>(pimpl.challInfo.world.geyserLocations[i][0]/CELL_SIZE);
        int geyserY = static_cast<int>(pimpl.challInfo.world.geyserLocations[i][1]/CELL_SIZE);

        if(geyserX == posX && geyserY == posY && posZ <= 4) {
            return true;
        }
    }
    return false;
}

bool ZeroRoboticsGame::atBaseStation(float pos[3])
{
    int posX = std::abs(static_cast<int>(pos[0]/CELL_SIZE));
    int posY = std::abs(static_cast<int>(pos[1]/CELL_SIZE));
    int posZ = std::abs(static_cast<int>(pos[2]/CELL_SIZE));
    return (posX <= BASE_SIDE_SIZE && posY <= BASE_SIDE_SIZE && posZ <= BASE_SIDE_SIZE);
}

