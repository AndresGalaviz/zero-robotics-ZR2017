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

void ZeroRoboticsGameImpl::initializeWorld(int concentrationX, int concentrationY) 
{
    if(concentrationX == -1 || concentrationY == -1) {
        // Repeat until not in base zone
        do {
            // Random is slightly biased toward low numbers however in this range is not significant
            challInfo.world.peakConcentration[0] = -GRID_X_SIDE + 2 + (rand() % (int)(2*(GRID_X_SIDE - 2)));
            challInfo.world.peakConcentration[1] = (rand() % (int)(GRID_Y_SIDE - 2));
        } while(std::abs(challInfo.world.peakConcentration[0]) < (BASE_SIDE_SIZE + 1));

        // Analyzer is the middle of cells [x, y, z] = 0, 12, 0
        challInfo.world.analyzerCoords[0] = CELL_SIZE/2;
        challInfo.world.analyzerCoords[1] = GRID_Y_SIDE/2;
        challInfo.world.analyzerCoords[2] = 0;

    } else {
        challInfo.world.peakConcentration[0] = -1*concentrationX - 1;
        challInfo.world.peakConcentration[1] = -1*concentrationY - 1;
        // Analyzer is on pos [x, y, z] = 15, 3, 0
        challInfo.world.analyzerCoords[0] = -CELL_SIZE/2;
        challInfo.world.analyzerCoords[1] = -GRID_Y_SIDE/2;
        challInfo.world.analyzerCoords[2] = 0;
    }
    GAME_TRACE(("[%d]|peakConcentration:%d,%d|", challInfo.currentTime, challInfo.world.peakConcentration[0], challInfo.world.peakConcentration[1]));
    GAME_TRACE(("[%d]|analyzerCoords:%d,%d,%d|", 
                challInfo.currentTime, challInfo.world.analyzerCoords[0], challInfo.world.analyzerCoords[1], challInfo.world.analyzerCoords[2]));
    // Available concentrations
    int concentrations[] = {HIGH_CONCENTRATION, MED_CONCENTRATION, LOW_CONCENTRATION, MIN_CONCENTRATION};
    // Initialize grid, only traverse one side
    for(int i = -GRID_Y_SIDE; i < GRID_Y_SIDE; i++) {
		for(int j = -GRID_X_SIDE; j < GRID_X_SIDE; j++) {
            int difX = std::abs(challInfo.world.peakConcentration[0] - j);
            int difY = std::abs(challInfo.world.peakConcentration[1] - i);
            // Takes into consideration the longest distance and assigns concentration accordingly if new cell
            if(challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].concentration == 0) {
                if(difX > difY) {
                    challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].concentration = difX > 3 ? concentrations[3] : concentrations[difX];
                } else {
                    challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].concentration = difY > 3 ? concentrations[3] : concentrations[difY];
                }
            }

            if(challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].concentration != 1) {
                // Mirror high concentrations
                challInfo.world.grid[GRID_Y_SIDE - i -1][GRID_X_SIDE - j - 1].concentration = challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].concentration;
            }
            #ifdef ZR3D
            challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].height = (rand() % (int)(MAX_GEYSER_HEIGHT));
            #endif
        }
	}
    
    #ifdef SHOW_GAME_TRACE
    for(int i = -GRID_Y_SIDE; i < GRID_Y_SIDE; i++) {
        for(int j = -GRID_X_SIDE; j < GRID_X_SIDE; j++) {
            printf("%*d ", 3, challInfo.world.grid[i + GRID_Y_SIDE][j + GRID_X_SIDE].concentration);
        }
        printf("\n");
    }
    #endif

    #ifdef ZR3D
        challInfo.world.analyzerCoords[2] = 0.36; // Analyzer is in the middle of the 4th square above ground
    #endif
}


void ZeroRoboticsGameImpl::initializeTerrainHeight(){
    srand (time(NULL));    
    #ifdef ZR3D
        for(int i = 0;i<Y_SIZE;i++){
            for(int j = 0;j<XZ_SIZE;j++){
                    challInfo.world.grid[i][j].height = rand()%4+1;
            }
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
                GAME_TRACE(("[%d]geyserLocations:%d,%d,0|", challInfo.currentTime, geyserLocations[i][0], geyserLocations[i][1]));
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

