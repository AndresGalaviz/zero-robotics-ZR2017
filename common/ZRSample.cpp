#include <string.h>
#include <math.h>
#include "ZRGame.h"
#include "ZRGameInternal.h"
#include <stdlib.h>
#include "Constants.h"

/*****************HIDDEN FUNCTIONS*****************/

/***********USER FUNCTIONS*************/

int ZeroRoboticsGame::getNumSamplesHeld()
{
    return pimpl.challInfo.me.samples.samplesHeld.count();
}

void ZeroRoboticsGame::getSamplesHeld(bool samples[MAX_SAMPLE_NUMBER])
{
    for(int i = MAX_SAMPLE_NUMBER - 1; i >= 0; i--) {
        samples[i] = pimpl.challInfo.me.samples.samplesHeld[MAX_SAMPLE_NUMBER - i - 1];
    }
}

void ZeroRoboticsGame::getConcentrations(float concentrations[MAX_SAMPLE_NUMBER])
{
    for(int i = 0; i < MAX_SAMPLE_NUMBER; i++) {
        if(pimpl.challInfo.me.samples.samplesAnalyzed[MAX_SAMPLE_NUMBER - i - 1]) {
            concentrations[i] = pimpl.challInfo.me.samples.sampleConcentrations[i];
        } else if(pimpl.challInfo.me.samples.samplesHeld[MAX_SAMPLE_NUMBER - i - 1]) {
            concentrations[i] = -2;
        } else {
            concentrations[i] = -1;
        }
    }
}

int ZeroRoboticsGame::pickupSample()
{
    // TODO: Add conditions other than drill being enabled(Sample is available?)
    if(pimpl.challInfo.me.drillEnabled) {
        for(int i = 0; i < MAX_SAMPLE_NUMBER; i++) {
            if(!pimpl.challInfo.me.samples.samplesHeld[MAX_SAMPLE_NUMBER - i - 1]) {
                // Player discrete location
                int playerX = pimpl.challInfo.me.zrState[0]/CELL_SIZE;
                int playerY = pimpl.challInfo.me.zrState[1]/CELL_SIZE;
            
                pimpl.challInfo.me.samples.samplesHeld[MAX_SAMPLE_NUMBER - i - 1] = 1;
                pimpl.challInfo.me.samples.samplesAnalyzed[MAX_SAMPLE_NUMBER - i - 1] = 0;
                pimpl.challInfo.me.samples.sampleConcentrations[i] = pimpl.challInfo.world.grid[playerY][playerX].concentration;
                GAME_TRACE(("[%d]samplesPicked:%d,1|", challInfo.currentTime, MAX_SAMPLE_NUMBER - i - 1));
                return i;
            }
        }
        return -1;
    }
    return -2;
}

bool ZeroRoboticsGame::discardSample(int sampleID)
{
    if(sampleID < 0 || sampleID > MAX_SAMPLE_NUMBER ||
       !pimpl.challInfo.me.samples.samplesHeld[MAX_SAMPLE_NUMBER - sampleID - 1]) {
        // Return false for incorrect sample number
        return false;
    }
    pimpl.challInfo.me.samples.samplesHeld[MAX_SAMPLE_NUMBER - sampleID - 1] = 0;
    GAME_TRACE(("[%d]samplesPicked:%d,0|", challInfo.currentTime, MAX_SAMPLE_NUMBER - sampleID - 1));

    return true;
}

// TODO: Conditions(3D): Knob on right face, speed < 0.01m/s, w < small rad/s
bool ZeroRoboticsGame::deliverSample(int sampleID)
{
    float playerPosition[] = {pimpl.challInfo.me.zrState[0], pimpl.challInfo.me.zrState[1], pimpl.challInfo.me.zrState[2]};
    if(atBaseStation(playerPosition) && discardSample(sampleID)) {
        pimpl.challInfo.me.samples.samplesAnalyzed[MAX_SAMPLE_NUMBER - sampleID - 1] = 1;
        // Concentration points + 2 points for delivering
        pimpl.challInfo.me.score += .5*pimpl.challInfo.me.samples.sampleConcentrations[sampleID] + 2; 
        GAME_TRACE(("[%d]samplesPicked:%d,0|", challInfo.currentTime, MAX_SAMPLE_NUMBER - sampleID - 1));
        GAME_TRACE(("[%d]samplesAnalyzed:%d,1|", challInfo.currentTime, MAX_SAMPLE_NUMBER - sampleID - 1));
        GAME_TRACE(("[%d]sampleConcentrations:%d,%f|", 
                    challInfo.currentTime, MAX_SAMPLE_NUMBER - sampleID - 1, pimpl.challInfo.me.samples.sampleConcentrations[sampleID]));
        return true;
    }
    return false;
}
