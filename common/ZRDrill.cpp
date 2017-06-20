#include <string.h>
#include <math.h>
#include "ZRGame.h"
#include "ZRGameInternal.h"
#include <stdlib.h>
#include "Constants.h"

/*****************HIDDEN FUNCTIONS*****************/

/***********USER FUNCTIONS*************/
// TODO: What happens when you run into de ground? 
// TODO: Tank has to be pointing down, position +4cm from surface, speed < 1cm/s. Penalyzed if conditions not met
bool ZeroRoboticsGame::startDrill() {
    // QUESTION: For 2D you cannot move with drill on?

    int playerX = pimpl.challInfo.me.zrState[0]/CELL_SIZE;
    int playerY = pimpl.challInfo.me.zrState[1]/CELL_SIZE;
    int playerZ = pimpl.challInfo.me.zrState[2]/CELL_SIZE;
    if(playerZ > -7) {
        // (3D)TODO: Other conditions for incorrect drilling procedure (speed, orientation, etc)
        pimpl.challInfo.me.incorrectDrill = true;
        return false;
    }
    // Points for drilling but only until 3rd time
    if(++pimpl.challInfo.world.grid[playerY][playerX].numDrills <= 3) {
        pimpl.challInfo.me.score += pimpl.challInfo.world.grid[playerY][playerX].numDrills;
    }
    
    // Geyser appearance probability
    if(((float) rand() / (RAND_MAX)) < ((float)pimpl.challInfo.world.grid[playerY][playerX].numDrills)/64.0) {
        for(int i = 0; i < 10; i++) {
            if (pimpl.challInfo.world.geyserActiveTime[i] == -1) {
                // No more than 10 geysers should be active at any time
                pimpl.challInfo.world.geyserActiveTime[i] == 0;
                pimpl.challInfo.world.geyserLocations[i][0] = playerX;
                pimpl.challInfo.world.geyserLocations[i][1] = playerY;
                pimpl.challInfo.me.samples.samplesHeld.reset(); // Drop all samples
                GAME_TRACE(("[%d]geyserLocations:%d,%d,1|", challInfo.currentTime, geyserLocations[i][0], geyserLocations[i][1]));
                // TODO: Add code for "stunning" satellite in 2D and flying it up in 3D
                return false;
            }
        }
    }

    GAME_TRACE(("[%d]drillEnabled:1|", challInfo.currentTime));
    pimpl.challInfo.me.drillEnabled = true;
    return true;
}

bool ZeroRoboticsGame::stopDrill() {
    GAME_TRACE(("[%d]drillEnabled:0|", challInfo.currentTime));
    pimpl.challInfo.me.drillEnabled = false;
    return true;
}
