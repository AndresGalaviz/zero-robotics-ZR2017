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