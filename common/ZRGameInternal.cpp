//ZRHS2017
#include <string.h>
#include <math.h>
#include <time.h>

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
#if SPHERE_ID==SPHERE1
state_vector initStateZR = { 0.2f, 0.2f,0.0f, 0.0f,0.0f,0.0f,  0.0000f, 1.0000f,0.0000f,0.0000f, 0.0f,0.0f,0.0f};
#else
state_vector initStateZR = {-0.2f,-0.2f,0.0f, 0.0f,0.0f,0.0f, -1.0000f, 0.0000f,0.0000f,0.0000f, 0.0f,0.0f,0.0f};
#endif

// variables sent from the simulation configuration
extern "C" {
	int zoneX = 0;
	int zoneY = 0;
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
	memset(&challInfo,0,sizeof(ChallengeInfo));															// general clear of challInfo to 0 = false = empty
	for (int i=0; i < MAX_SAMPLE_NUMBER; i++)
	{
		challInfo.me.samples[i][0] = OFF_GRID;
		challInfo.me.samples[i][1] = OFF_GRID;
	}
	updateStates();																													// update the state data on challInfo

	#ifndef ISS_FINALS
		#if SPHERE_ID==SPHERE1
			// allow users to set zone locations in simulation
			if ( (zoneX > -7) && (zoneX < 7) && ((zoneX < -2) || (zoneX > 2)) &&
					 (zoneY > -9) && (zoneY < 9) && ((zoneY < -2) || (zoneY > 2)) )
			{
				GAME_TRACE(("USER defined BO-Zone @ grid [%d, %d]", zoneX, zoneY));
				initializeWorld(zoneX, zoneY);
			}
			else
				initializeWorld();								// initialize world with random max zone locations
			
			#ifndef ZR2D
				initializeTerrainHeight();
			#endif
			// send information to ground and sat2 at the end of init
			sendInit();
		#endif
	#else
		// set fixed terrain for both satellites, not just sat 1
		initializeWorld(zoneX_ISS, zoneY_ISS);
		initializeTerrainHeight();
	#endif
}

void ZeroRoboticsGameImpl::updateStates() {
	apiImpl.api->getMyZRState(challInfo.me.zrState);
	apiImpl.api->getOtherZRState(challInfo.other.zrState);

	apiImpl.api->getMySphState(challInfo.me.sphState);
	apiImpl.api->getOtherSphState(challInfo.other.sphState);
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

	//Send Telemetry
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

bool ZeroRoboticsGameImpl::updateGameModeManeuver(float forceTorqueOut[6])
{
	float collisionInfo[2];
	float geyserForces[6];
	memset(geyserForces,0.0f,sizeof(geyserForces));
	
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

	// GAME: before user code runs:
	updateStates();
	updateAnalyzer();

	// GAME: user code:
	apiImpl.zruser->loop();
	
	// calculate forces requested by the user, before overriding with game and API functions
	useForces = apiImpl.getForceTorque(forceTorqueOut);			// get the requested forces from the user code

	// if no fuel remaining at all, zero out all user requested forces
	if(game->getFuelRemaining() <= 0.0f)
	{
		memset(forceTorqueOut,0,6*sizeof(float));		// anything the user has commanded is ignored, only firings would be from out-of-bounds or collision avoidance
		useForces = false;													// assume there will be no firings
	}

	// GAME: after user code runs:
	
	// update geysers and if the sat is hit, override forces
	if (updateGeysers())
	{
		// for some reason I can't send forceTorqueOut as an argument,
		// plus we need to keep memory of original push-out direction,
		// so this function overwrites with the direction and updateGeysers keeps track of time
		#ifdef ZR2D
			forceTorqueOut[0] = challInfo.me.geyserForce[0];
			forceTorqueOut[1] = challInfo.me.geyserForce[1];
		#else
			forceTorqueOut[2] = -GEYSER_FORCE;
		#endif
		useForces = true;
	}
	updateDrill();
	updateScore();

	//Enforce boundaries of the game
	useForces = useForces || (enforceBoundaries(forceTorqueOut));

	//Collision avoidance when speed is not negligible
	if (speed > 0.01f) {
		useForces = useForces || ctrlAvoidWithOverride(challInfo.me.sphState,&challInfo.other.sphState,1,forceTorqueOut,collisionInfo,3);
	}

	//------Thruster control------//
	// Run standard mixer to get fuel consumption based on user, game, and boundaries/collision avoidance requested forces
	prop_time times;
	ctrlMixWLoc(&times, forceTorqueOut, challInfo.me.sphState, 10, 20.0f, FORCE_FRAME_INERTIAL);

	//Tally fuel and energy used for firing thrusters
	for (int i=0; i<12; i++)
	{
		float thrusterTime = times.off_time[i]-times.on_time[i];
		challInfo.me.fuelUsed += thrusterTime / 1000.0f;

		//Protect against NAN or INF hacking
		if (challInfo.me.fuelUsed < 0.0f)
		{
			DEBUG(("WARNING: invalid forces/torques applied to satellite at. Fuel will be set to 0. Check your code for errors.\n"));
			challInfo.me.fuelUsed = PROP_ALLOWED_SECONDS;
		}
	}

	#ifdef ZR2D
	modify2DForceTorque(forceTorqueOut);
	useForces = true;
	#endif

	//------Game end conditions------//
	if (apiImpl.api->getTime() >= (MAX_GAME_TIME))
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

	// break ties by:
	//  - most number of samples picked up
	//  - one closest to the Base at end of game
	if (myScore == otherScore)
	{
		if (challInfo.me.total_samples > challInfo.other.total_samples)
		{
			challInfo.me.score += 0.01f;
			GAME_TRACE (("Tie Breaker (%12.10f = %12.10f) MORE SAMPLES, new score = %12.10f | ", myScore, otherScore, challInfo.me.score));
		}
		else if (challInfo.me.total_samples == challInfo.other.total_samples)
		{
			challInfo.me.score  -= (0.01f * sqrtf((challInfo.me.zrState[0]*challInfo.me.zrState[0])+(challInfo.me.zrState[1]*challInfo.me.zrState[1])+(challInfo.me.zrState[2]*challInfo.me.zrState[2])));
			GAME_TRACE (("Tie Breaker (%12.10f = %12.10f) CLOSEST TO BASE, new score = %12.10f | ", myScore, otherScore, challInfo.me.score));
		}
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
	}

	// terminate after we find the final result & timeout (4s)
	if (finalResult && (ctrlManeuverTimeGet() > 4000))
	{
		GAME_TRACE (("GAME ENDED! Final Score Float: %10.8f / Integer: %d | Test Result %d", myScore, result, finalResult));
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

/*************************Score functions***********************/

void ZeroRoboticsGameImpl::updateScore(){
	// TODO: Other things that might make the user lose points
	if (challInfo.me.drillError)
	{
		GAME_TRACE(("Score PENALTY: Drill Error! (-%3.2f points)", SCORE_DRILL_PENALTY));
		challInfo.me.score -= SCORE_DRILL_PENALTY;
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

void ZeroRoboticsGame::sendMessage(unsigned char inputMsg)
{
	pimpl.challInfo.me.message = (short)inputMsg;
}

unsigned char ZeroRoboticsGame::receiveMessage()
{
	return (unsigned char)pimpl.challInfo.other.message;
}
