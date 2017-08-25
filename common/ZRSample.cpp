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
	int num_samples = 0;
	for (int i=0; i < MAX_SAMPLE_NUMBER; i++)
		if (pimpl.challInfo.me.samples[i][0] != OFF_GRID) num_samples++;
  return num_samples;
}

void ZeroRoboticsGame::getSamplesHeld(bool samples[MAX_SAMPLE_NUMBER])
{
    for(int i=0; i < MAX_SAMPLE_NUMBER; i++)
		{
        samples[i] = (pimpl.challInfo.me.samples[i][0] != OFF_GRID);
    }
}

void ZeroRoboticsGame::getConcentrations(float concentrations[MAX_SAMPLE_NUMBER])
{
	for (int i = 0; i < MAX_SAMPLE_NUMBER; i++)
	{
		// must have the analyzer in order to get concentrations if not dropping at Base Station (dropSample returns the conc when delivering at Base)
		if (pimpl.challInfo.me.samples[i][0] == OFF_GRID)
			concentrations[i] = -2.0f;		// sample not held
		else if (pimpl.challInfo.me.hasAnalyzer[0] || pimpl.challInfo.me.hasAnalyzer[1])
			concentrations[i] = ((float) pimpl.challInfo.world.grid[pimpl.challInfo.me.samples[i][0]][pimpl.challInfo.me.samples[i][1]].concentration)/10.0f;
		else
			concentrations[i] = -1.0f;		// sample not yet analyzed
	}
}

bool ZeroRoboticsGame::checkSample()
{
	return pimpl.challInfo.me.sampleReady;
}

int ZeroRoboticsGame::pickupSample()
{
	// attempt to pick-up a sample
	
	// sample is not ready for pickup
	if (!pimpl.challInfo.me.sampleReady)
	{
		GAME_TRACE(("Sample pickup ERROR : a sample is not ready for pickup."));
		return -2;
	}
	
	// the satellite should remain within the original square it started drilling at
	if (!pimpl.checkAtSquare(pimpl.challInfo.me.drillSquare[0], pimpl.challInfo.me.drillSquare[1]))
	{
		GAME_TRACE(("Sample pickup ERROR : satellite moved out of original sample square, go back to square to complete pick up!"));
		return -2;
	}
	
	// check there is space
	if ((pimpl.challInfo.me.samples[0][0] != OFF_GRID) &&
			(pimpl.challInfo.me.samples[1][0] != OFF_GRID) &&
			(pimpl.challInfo.me.samples[2][0] != OFF_GRID))
	{
		GAME_TRACE(("Sample pickup ERROR : sample spaces are full."));
		return -1;
	}

	// find the first space for the sample and pick it up
	for (int i = 0; i < MAX_SAMPLE_NUMBER; i++)
	{
		if (pimpl.challInfo.me.samples[i][0] == OFF_GRID)
		{
			pimpl.challInfo.me.samples[i][0] = pimpl.challInfo.me.drillSquare[0];		// copy square coordinates where drill took place
			pimpl.challInfo.me.samples[i][1] = pimpl.challInfo.me.drillSquare[1];
			
			pimpl.challInfo.me.total_samples ++;
			
			// get points for picking up sample
			float add_score = 0.0f;
			unsigned int numDrills = pimpl.challInfo.world.grid[pimpl.challInfo.me.drillSquare[0]][pimpl.challInfo.me.drillSquare[1]].numDrills;
			if (numDrills < 4) add_score = numDrills;
			pimpl.challInfo.me.score += add_score;
			
			GAME_TRACE(("Sample pickup SUCCESS! Slot %d filled with sample at [%d, %d], %d times square drilled, gained %3.2f points (%d samples picked up so far).", i, pimpl.challInfo.me.samples[i][0], pimpl.challInfo.me.samples[i][1], numDrills, add_score, pimpl.challInfo.me.total_samples));

			// reset drill information - can keep drilling same square if they want, but need to complete new rotation
			pimpl.challInfo.me.sampleReady = false;
			pimpl.challInfo.me.drillInitAtt[0] = pimpl.challInfo.me.zrState[ZR_ATT_X];	// copy attitude at time of pick-up
			pimpl.challInfo.me.drillInitAtt[1] = pimpl.challInfo.me.zrState[ZR_ATT_Y];
			mathVecNormalize(pimpl.challInfo.me.drillInitAtt, 2);												// normalize 2D pointing vector
			
			// Geysers appear at the time of pick-up based on the probability (numDrills^3 / 64)
			pimpl.checkGeyser(pimpl.challInfo.me.samples[i][0], pimpl.challInfo.me.samples[i][1]);
			return i;
		}
	}
	
	// ERROR if we reach here
	GAME_TRACE(("Sample pickup ERROR : UNKNOWN, should never reach here!"));
	return -3;
}

// TODO: Conditions(3D): Knob on right face, speed < 0.01m/s, w < small rad/s
float ZeroRoboticsGame::dropSample(int sampleID)
{
	float conc = 0.0f;

	// check that we have a sample
	if (pimpl.challInfo.me.samples[sampleID][0] == OFF_GRID)
	{
		GAME_TRACE(("Drop sample ERROR : that sample space is not currently full."));
		return conc;
	}

	// if we have a sample, and at base station, deliver and gain points
	if (atBaseStation())
	{
		// Concentration points + 2 points for delivering
		float conc = ((float) pimpl.challInfo.world.grid[pimpl.challInfo.me.samples[sampleID][0]][pimpl.challInfo.me.samples[sampleID][1]].concentration)/10.0f;		// get concentration
		float add_points = 5.0f*conc + 2.0f;
		pimpl.challInfo.me.score += add_points;
		GAME_TRACE(("Base Station: successful delivery! Sample from [%d, %d] had concentration of %3.2f. Points gained: %3.2f.",
			pimpl.challInfo.me.samples[sampleID][0], pimpl.challInfo.me.samples[sampleID][1], conc, add_points));
	}
	else
		GAME_TRACE(("Drop sample: sample discarded."));
	
	// erase sample from memory
	pimpl.challInfo.me.samples[sampleID][0] = OFF_GRID;
	pimpl.challInfo.me.samples[sampleID][1] = OFF_GRID;
	
	return conc;
}
