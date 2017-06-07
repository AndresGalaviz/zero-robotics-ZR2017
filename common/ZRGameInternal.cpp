//ZRHS2017
#include <string.h>
#include <math.h>

#include "ZRGame.h"
#include "ZRGameInternal.h"
#include "ZR_API_internal.h"
#include "ZR_API.h"
#include "Constants.h"

//SPHERES C includes
extern "C" {
#include "commands.h"
#include "system.h"
#include "math_matrix.h"
#include "gsp.h"
#include "control.h"
#include "find_state_error.h"
#include "housekeeping_internal.h"
}

/**
 * This file is used to define the C++ implementation of both the public and the
 * private game API functions.  Public functions will be prefixed with ZeroRoboticsGame::
 * while private implementation functions will be prefixed with ZeroRoboticsGameImpl::
 */

 /**
  * Use these vectors to choose the satellites state when the user is given control.
  */
#ifdef ZR2D
	#if SPHERE_ID==SPHERE1
	state_vector initStateZR = {0.0f, 0.15f,0.0f, 0.0f,0.0f,0.0f,  0.0000f, 1.0000f,0.0000f,0.0000f, 0.0f,0.0f,0.0f};
	#else
	state_vector initStateZR = {0.0f,-0.15f,0.0f, 0.0f,0.0f,0.0f, -1.0000f, 0.0000f,0.0000f,0.0000f, 0.0f,0.0f,0.0f};
	#endif
#else
	#if SPHERE_ID==SPHERE1
	state_vector initStateZR = {0.0f, 0.15f,0.0f, 0.0f,0.0f,0.0f,  0.7071f,-0.7071f,0.0000f, 0.0000f, 0.0f,0.0f,0.0f};
	#else
	state_vector initStateZR = {0.0f,-0.15f,0.0f, 0.0f,0.0f,0.0f,  0.0000f, 0.0000f,0.7071f,-0.7071f, 0.0f,0.0f,0.0f};
	#endif
#endif

// variables sent from the simulation configuration
extern "C" {
	int zoneX_cm = 0;
	int zoneY_cm = 0;
	int zoneZ_cm = 0;
}

//======= Singleton Instance of Game and Implementation=====//
ZeroRoboticsGameImpl &ZeroRoboticsGameImpl::instance() {
	///Singleton instance of the game implementation
	static ZeroRoboticsGameImpl gameImplInstance(ZeroRoboticsAPIImpl::instance());
	return gameImplInstance;
	}
ZeroRoboticsGame &ZeroRoboticsGame::instance() {
	static ZeroRoboticsGame gameInstance(ZeroRoboticsGameImpl::instance(), ZeroRoboticsAPIImpl::instance());
	return gameInstance;
}

//======== Constructors ========//

//Constructor for ZeroRobotics game with initializers for the ZR API.  Do not modify.
ZeroRoboticsGame::ZeroRoboticsGame(ZeroRoboticsGameImpl &impl, ZeroRoboticsAPIImpl &apiImpl):
pimpl(impl), apiImpl(apiImpl)
{
	impl.game = this;
}

ZeroRoboticsGameImpl::ZeroRoboticsGameImpl(ZeroRoboticsAPIImpl &apiImpl) : apiImpl(apiImpl) {}

//Internal accessor for game implementation
ZeroRoboticsGameImpl *getGameImpl()
{
	return &ZeroRoboticsGameImpl::instance();
}

//======== GAME IMPLEMENTATION METHODS ========//

//Add methods here for the public game

void ZeroRoboticsGameImpl::init()
{
	srand(time(NULL));	

	/*
	*
	Game Initialization setup.
	*/
	//printf("***************************RUNNING INIT***************************");
	apiImpl.gameTime = GAME_TIME;
	memset(&challInfo,0,sizeof(ChallengeInfo));
	// general clear of challInfo to 0 = false = empty

	challInfo.specialTime = 188;
	// update the state data on challInfo
	updateStates();

	challInfo.me.fuelUsed = 0.0f;
	challInfo.me.score = START_SCORE;
	challInfo.me.hasAnalyzer = 0;
	challInfo.me.drillEnabled = false;
	challInfo.me.incorrectDrill = false;
	challInfo.other.score = START_SCORE;
	challInfo.other.hasAnalyzer = 0;
	challInfo.world.numActiveGeysers = 0;
	
	
	#if SPHERE_ID==SPHERE1
	// initialize world scenario
	initializeWorld();
	#endif

	// send information to ground and sat2 at the end of init
	sendDebug();
	
}

void ZeroRoboticsGameImpl::updateStates() {
	apiImpl.api->getMyZRState(challInfo.me.zrState);
	apiImpl.api->getOtherZRState(challInfo.other.zrState);

	apiImpl.api->getMySphState(challInfo.me.sphState);
	apiImpl.api->getOtherSphState(challInfo.other.sphState);

	challInfo.currentTime = apiImpl.api->getTime();
}


bool ZeroRoboticsGameImpl::update(float forceTorqueOut[6]) {
	//keep as is
	// empty the userForces array
	memset(challInfo.me.userForces,0,sizeof(float)*6);

	// declare a static finalResult variable
	// updateGameOverManuever will change it
	static unsigned char finalResult = 0;
	unsigned char useForces = false;

	// maneuver 3 = game mode
	// maneuver 202 = end game (get final score)
	// maneuver 204 = terminate

	switch (ctrlManeuverNumGet())
	{
		case 3:
		useForces = updateGameModeManeuver(forceTorqueOut); //our game
		break;
		//------End main game code------//
		case 202: // Endgame scenario
		useForces = updateGameOverManeuver(forceTorqueOut, finalResult); //should not have to change
		break;

		case 203: // terminate test after determining final score
		{
		ctrlTestTerminate(finalResult);
		}
		break;
    }

	//Store user forces for sending to ground
	memcpy(challInfo.me.userForces, forceTorqueOut, sizeof(float)*6);

	#ifdef ZR2D
	memset(&challInfo.me.userForces[2], 0, sizeof(float)*3);
	#endif

	//Send Telemetry
	//HERE FOR DEBUG
	sendDebug();

	return useForces;

}

#ifdef ZR2D
void ZeroRoboticsGameImpl::modify2DForceTorque(float forceTorque[6])
{
	state_vector myState, targetState;
	apiImpl.api->getMySphState(myState);
	memcpy(targetState, myState, sizeof(state_vector));		
	
	targetState[POS_Z] = 0.0f;
	targetState[VEL_Z] = 0.0f;
	targetState[RATE_X] = 0.0f;
	targetState[RATE_Y] = 0.0f;
	
	// only care about X & Y rotations, make sure always points "down" otherwise
	targetState[QUAT_3] = 0.0f;
	targetState[QUAT_4] = 0.0f;
	mathVecNormalize(&targetState[QUAT_1], 4);
	
	state_vector error;
	findStateError(error, myState, targetState);

	float newForces[6] = {0};
	
	ctrlPositionPD(KPpositionPD, KDpositionPD, error, newForces);
	ctrlAttitudeNLPIDwie(KPattitudePID, KIattitudePID, KDattitudePID, 
						 KPattitudePID, KIattitudePID, KDattitudePID, 
						 KPattitudePID, KIattitudePID, KDattitudePID, 
						 1.0f, error, newForces);
	forceTorque[FORCE_Z] = newForces[FORCE_Z];
	forceTorque[TORQUE_X] = newForces[TORQUE_X];
	forceTorque[TORQUE_Y] = newForces[TORQUE_Y];
}
#endif

void ZeroRoboticsGameImpl::sphereCollision(float* sphereState, float* otherState, float* forceTorqueOut) {
	float otherVFromSphereFrame[3];
	mathVecSubtract(otherVFromSphereFrame, &sphereState[VEL_X], &otherState[VEL_X], 3);
	float pointLoss = COLLISION_MULT * SPHERE_MASS * mathVecMagnitude(otherVFromSphereFrame, 3); // positive number
	challInfo.me.score -= pointLoss;
	#ifdef ALLIANCE
	for(int i = 0; i < 3; i++) {
		forceTorqueOut[i] += otherState[i + VEL_X] * SPHERE_MASS / SPHERE_MASS;	
		otherState[VEL_X + i] = sphereState[i + VEL_X] * SPHERE_MASS / SPHERE_MASS;
	}
	#endif
	//printf("SPHERE %d lost %f points for being reckless.\n",SPHERE_ID,pointLoss);
	//printf("SPHERE %d's score is now %f\n",SPHERE_ID,challInfo.me.score);
}

bool ZeroRoboticsGameImpl::updateGameModeManeuver(float forceTorqueOut[6]) {
	float collisionInfo[2];

	// get state-of-health of other satellite (mainly for end of test)
	comm_payload_soh soh_partner;
	#if (SPHERE_ID == SPHERE1)
	commBackgroundSOHGet(SPHERE2, &soh_partner);
	#else
	commBackgroundSOHGet(SPHERE1, &soh_partner);
	#endif

	// determine current speed of satellite, use to determine collision avoidance motion
	float speed = mathVecMagnitude(&challInfo.me.sphState[3],3);

	unsigned char useForces = false;

	// Run user's "loop" function.  apiImpl is used because we are accessing
	// a private part of the API containing a pointer to the user's API
	//OLD LOOP PLACEMENT **********************************************************8

	updateStates();

	//before user code runs stuff
	apiImpl.zruser->loop();
	
	turnOffOldGeysers();
	subtractScore();
	//after user code runs stuff
	
	// TODO: Update state of the things that changed and need to be synced between SPHERES

	useForces = useForces || apiImpl.getForceTorque(forceTorqueOut);
	prop_time tent_times;
	#ifdef ZRFLATFLOOR
	ctrlMixWLoc(&tent_times, forceTorqueOut, challInfo.me.sphState, 10, 40.0f, FORCE_FRAME_INERTIAL);
	#else
	ctrlMixWLoc(&tent_times, forceTorqueOut, challInfo.me.sphState, 10, 20.0f, FORCE_FRAME_INERTIAL);
	#endif

	float tent_time_sum = 0;
	for (int i = 0; i < 12; i++) {
		tent_time_sum += tent_times.off_time[i]-tent_times.on_time[i];
	}

	// Check for fuel or energy out at this point
	if(game->getFuelRemaining() <= 0.0f)
	{
		memset(forceTorqueOut,0,6*sizeof(float));
		// anything the user has commanded is ignored, only firings would be from out-of-bounds or collision avoidance
		useForces = false;
		// assume there will be no firings
	}

	#ifndef ZR2D
	//checks for collisions between spheres
	if(dist3d(challInfo.me.zrState, challInfo.other.zrState) < SPHERE_RADIUS * 2) {
		if(!challInfo.me.sphereCollisionActive) {
			GAME_TRACE(("SPHERE %d collided with its opponent\n",SPHERE_ID));
			sphereCollision(challInfo.me.zrState, challInfo.other.zrState, forceTorqueOut);
		}
		challInfo.me.sphereCollisionActive = true;
	} else {
		challInfo.me.sphereCollisionActive = false;
	}	

	#endif

	//Enforce boundaries of the game
	if (enforceBoundaries(forceTorqueOut))
	{
		useForces = true;
	}

	//Collision avoidance
	//check to see that its speed is not neglible.
	if (speed > 0.01f) {
		challInfo.me.collisionActive = ctrlAvoidWithOverride(challInfo.me.sphState,&challInfo.other.sphState,1,forceTorqueOut,collisionInfo,3);
	}

	//------Thruster control------//
	// Run standard mixer to get fuel consumption
	prop_time times;
	ctrlMixWLoc(&times, forceTorqueOut, challInfo.me.sphState, 10, 20.0f, FORCE_FRAME_INERTIAL);

	//Tally fuel and energy used for firing thrusters
	for (int i=0; i<12; i++)
	{
		float thrusterTime = times.off_time[i]-times.on_time[i];
		if (thrusterTime > 300.0f) {
			//printf("thrust: %f", thrusterTime);
		}
		challInfo.me.fuelUsed += thrusterTime / 1000.0f;

		//Protect against NAN or INF hacking
		if (challInfo.me.fuelUsed < -0.0f)
		{
			//printf("SPHERE %i: %f, %f, %f \n", SPHERE_ID, challInfo.me.zrState[0], challInfo.me.zrState[0], challInfo.me.zrState[0]);
			DEBUG(("WARNING: invalid forces/torques applied to satellite at t=%d. Fuel will be set to 0. Check your code for errors.\n", challInfo.currentTime));
			//printf("WARNING: invalid forces/torques applied to satellite %d at t=%d. Fuel will be set to 0. Check your code for errors.\n",SPHERE_ID,challInfo.currentTime);
			challInfo.me.fuelUsed = PROP_ALLOWED_SECONDS;
		}
	}
	#ifdef ZR2D
	modify2DForceTorque(forceTorqueOut);
	useForces = true;
	#endif

	//------Game end conditions------//
	if (challInfo.currentTime >= (MAX_GAME_TIME))
		ctrlManeuverNumSet(202);

	return useForces;
}

bool ZeroRoboticsGameImpl::updateGameOverManeuver(float forceTorqueOut[6], unsigned char& finalResult) {
	unsigned char useForces = false;
	int result;

	float collisionInfo[2];

	// get state-of-health of other satellite (mainly for end of test)
	comm_payload_soh soh_partner;
	#if (SPHERE_ID == SPHERE1)
	commBackgroundSOHGet(SPHERE2, &soh_partner);
	#else
	commBackgroundSOHGet(SPHERE1, &soh_partner);
	#endif

	float myScore;
	float otherScore;

	myScore = game->getScore();
	otherScore = game->getOtherScore();

	useForces = ctrlAvoidWithOverride(challInfo.me.sphState, &challInfo.other.sphState, 1, forceTorqueOut, collisionInfo, 3) > 0;
	useForces = useForces || enforceBoundaries(forceTorqueOut);

	// break ties by making a tiny change based on the random state
	if (myScore == otherScore)
	{
		challInfo.me.score  -= (0.01f * sqrtf((challInfo.me.zrState[0]*challInfo.me.zrState[0])+(challInfo.me.zrState[1]*challInfo.me.zrState[1])+(challInfo.me.zrState[2]*challInfo.me.zrState[2])));
		GAME_TRACE (("Tie Breaker (%12.10f = %12.10f), new score = %12.10f | ", myScore, otherScore, challInfo.me.score));
	}

	// get final score
	if ((soh_partner.last_test_result || (soh_partner.maneuver_number > 200) || (NUM_SPHERES == 1)))
	{
		result = 10*floorf(myScore);
		if (result < 10) result = 10;
		if (result > 220) result = 220;
		if (myScore > otherScore)
		result += 10;
		finalResult = (result + apiImpl.getTeamId() + 1);
		useForces = false;
	}

	// terminate after we find the final result & timeout (4s)
	if (finalResult && (ctrlManeuverTimeGet() > 4000))
	{
		GAME_TRACE (("GAME ENDED! Final Score Float: %10.8f / Integer: %d | Test Result %d | ", myScore, result, finalResult));
		//printf("Sphere %d score is: %f \n",SPHERE_ID,challInfo.me.score);
		ctrlManeuverTerminate();
	}
	return useForces;
}

bool ZeroRoboticsGameImpl::enforceBoundaries(float forceTorqueOut[6])
{
	state_vector sphState;
	apiImpl.api->getMySphState(sphState);
	bool retValue = false;

	//Run velocity controller to stop motion out of volume
	//Zero out any controls that are leading the user out of the volume
	for (int i=0; i<3; i++)
	{
		if (sphState[POS_X+i] > limits[i])
		{
			limitDirection(sphState, forceTorqueOut, i, 1.0f);
			retValue = true;
		}
		else if (sphState[POS_X+i] < -limits[i])
		{
			limitDirection(sphState, forceTorqueOut, i, -1.0f);
			retValue = true;
		}
	}
	return retValue;
}

void ZeroRoboticsGameImpl::limitDirection(state_vector ctrlState, float ctrlControl[6], unsigned int idx, float dir)
{
// if command is to move out further in same direction as limit
	if ((ctrlControl[idx]*dir >= 0.0f) || (ctrlState[VEL_X+idx]*dir >= 0.005f))
	{
		ctrlControl[idx] = -1.0f * OOBgain * ctrlState[VEL_X+idx];  // slow down to get 0 velocity (ignore position)
	}
}

/******************Collision Avoidance Functions************************/
void ZeroRoboticsGameImpl::resolveCollision(float zrState[12], float forceTorqueOut[6])
{
	for (int i=0; i<3; i++)
	{
		if (zrState[i] > 0)
		limitDirection(zrState, forceTorqueOut, i, -1.0f);
		else if (zrState[i] < 0)
		limitDirection(zrState, forceTorqueOut, i, 1.0f);
	}
}

float ZeroRoboticsGameImpl::dist3d(float* pos1, float* pos2) const
{
	float tmpVec[3];
	mathVecSubtract(tmpVec,static_cast<float*>(pos1),static_cast<float*>(pos2),3);
	return mathVecMagnitude(tmpVec,3);
}

/*************************Score functions***********************/

void ZeroRoboticsGameImpl::subtractScore() {
    // TODO: Other things that might make the user lose points
    if(challInfo.me.incorrectDrill) {
        challInfo.me.score -= 1;
    }
}

float ZeroRoboticsGame::getScore()
{
	return pimpl.challInfo.me.score;
}

float ZeroRoboticsGame::getOtherScore()
{
	return pimpl.challInfo.other.score;
}

//======== PUBLIC API METHODS ========//
//------General Functions------//

float ZeroRoboticsGame::getFuelRemaining()
{
	ZeroRoboticsGameImpl::ChallengeInfo& challInfo = pimpl.challInfo;
	float total = PROP_ALLOWED_SECONDS-challInfo.me.fuelUsed;
	total = MIN_FUEL(total, 0.0f); //~Prevents fuel from being less than 0.0f
	total = MAX_FUEL(total, PROP_ALLOWED_SECONDS); //~Prevents fuel from being greater than MAX fuel in tank
	return total;
}

int ZeroRoboticsGame::getCurrentTime()
{
	return pimpl.challInfo.currentTime;
}

void ZeroRoboticsGame::sendMessage(unsigned char inputMsg)
{
	pimpl.challInfo.me.message = (short)inputMsg;
}

unsigned char ZeroRoboticsGame::receiveMessage()
{
	return (unsigned char)pimpl.challInfo.other.message;
}

//TODO: Dock function for SPHERES

/************USER DOCKING FUNCTIONS**************/

