<<<<<<< HEAD
#include "ZRGame.h"
#include <string.h>
#include "ZRGameInternal.h"
#include <math.h>
#define zr_rand() rand()
// TODO: Ports might take processing time?
// TODO: Implement a SPHERES-like base with the characteristics discussed in the past meeting (Fri 19)

bool ZeroRoboticsGame::atBaseStation()
{
	// check that all requirements to be at the base station are met
	
	// must be within 0.15cm of center of work area
	float dist = mathVecMagnitude(&pimpl.challInfo.me.sphState[0],3);
	if (dist > BASE_RADIUS)
	{
		GAME_TRACE(("Base Station: too far from Base Station."));
		return false;
	}
	
	// must have a speed under 0.01m/s
	float speed = mathVecMagnitude(&pimpl.challInfo.me.sphState[3],3);
	if (speed > MAX_BASE_SPEED)
	{
		GAME_TRACE(("Base Station: moving too fast."));
		return false;
	}
	
	float rot_rate = mathVecMagnitude(&pimpl.challInfo.me.sphState[RATE_X],3);
	if (rot_rate > MAX_BASE_RATE)
	{
		GAME_TRACE(("Base Station: rotating too fast."));
		return false;
	}
	
	#ifndef ZR2D
		// must be pointing correctly
	#endif
	
	return true;
}
=======
#include "ZRGame.h"
#include <string.h>
#include "ZRGameInternal.h"
#include <math.h>
#define zr_rand() rand()
// TODO: Ports might take processing time?
// TODO: Implement a SPHERES-like base with the characteristics discussed in the past meeting (Fri 19)

//has to be over base station pointing down with velocity under .01 m/s CHECK: 3d vs 2d
//for 3d have to check for 'correct height' (not yet specified in game manual)

//modify score when docking to base station with samples
// void ZeroRoboticsGame::SampleScoring(){
//     samples = pimpl.challInfo.me.samples;
//     concentrations = samples.sampleConcentrations;
//     for (int i = 0; i < samples.samplesHeld; i++){
//         //access concentration at for each sample, remove from sampleHeld
//         //and add to sampleAnalyzed
//         pimpl.challInfo.me.score += concentrations[i]
//         pimpl.challInfo.me.samplesHeld[i] = 0
//         pimpl.challInfo.me.samplesAnalyzed[i] = 1
//     }

// }
>>>>>>> 84b88435f6813f0dcde9e18f4496ed50ce0a9a5f
