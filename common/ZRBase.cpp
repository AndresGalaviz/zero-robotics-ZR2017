#include "ZRGame.h"
#include <string.h>
#include "ZRGameInternal.h"
#include <math.h>
#define zr_rand() rand()
// TODO: Ports might take processing time?
// TODO: Implement a SPHERES-like base with the characteristics discussed in the past meeting (Fri 19)

//modify score when docking to base station with samples
void ZeroRoboticsGame::SampleScoring(){
    samples = pimpl.challInfo.me.samples;
    concentrations = samples.sampleConcentrations;
    //find existing game score
    for (int i = 0; a < samples.samplesHeld; i++){
        //access concentration at for each sample, remove from sampleHeld
        //and add to sampleAnalyzed
    }

}