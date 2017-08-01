#ifndef _ZR_GAME_INTERNAL_H_
#define _ZR_GAME_INTERNAL_H_
#pragma once

#include "Constants.h"
#include "ZeroRoboticsGameBaseImpl.hpp"
#include "comm.h"
#include <bitset>         // std::bitset

///Forward declaration of ZR Game
class ZeroRoboticsGame;

/**
 * This class defines the hidden game implementation. Any internal
 * utility functions or game ru//les functsions hould be put here.  You may also
 * override any of the standard GSP functions by implementing
 * them here.  The functions init(), update(), and sendDebug() are required.
 */

class ZeroRoboticsGameImpl : public ZeroRoboticsGameBaseImpl<ZeroRoboticsGameImpl>
{
public:
	/**
	* (Required) Runs initialization fo the ZR game at the start of a test. Use this function
	* to reset all member variables.
	*/
	struct Cell
	{
		int 			concentration; // Bacteria concentration 
		int 			numDrills; 	   // Number of times this cell has been drilled
		#ifdef ZR3D
		int				height;
		#endif
		Cell() { concentration = 0; numDrills = 0;};
	};

	struct Samples
	{
		std::bitset<MAX_SAMPLE_NUMBER> 	samplesHeld;
		std::bitset<MAX_SAMPLE_NUMBER> 	samplesAnalyzed;
		float			sampleConcentrations[MAX_SAMPLE_NUMBER];
		Samples() {}
	};

	struct Base
	{
		int rotateTime;
	};

	struct World
	{
		Cell				grid[Y_SIZE][XZ_SIZE]; // YX arrangement
		float				analyzer1Coords[3];
		float 				analyzer2Coords[3];
		int 				peakConcentration[2];
		int					numActiveGeysers;
		float 				geyserLocations[10][2]; // 10 geyser, 2D coordinates
		int					geyserActiveTime[10];
	};

	
	struct PlayerInfo
	{
		float           zrState[12];
		state_vector    sphState;
		float 			fuelUsed;
		bool 			collisionActive;		// true if collision avoidance is activated
		float 			userForces[6];			// stores userForces for download
		float 			score;					// current total score in the game QUESTION: Should this be float?
		short 			message;				// message to send to other satellite
		Samples			samples; /// Space where samples are held
		float 			lastAttTarget[4];
		float 			lastPosTarget[4];
		bool			collisionsActive[NUM_SCORE_ITEMS];
		bool 			sphereCollisionActive;
		int				hasAnalyzer;		 	// 0 if no Analyzer object held. ItemID if one is held.
		bool			drillEnabled;
		bool			incorrectDrill;

	};

	struct OtherInfo
	{
		float           zrState[12];
		state_vector    sphState;
		float 			score;					// latest known score of other player
		short 			message;				// message from other player
		int 			hasAnalyzer;		 	// 0 if no analyzer held. ItemID if one is held.
	};
	
	struct ChallengeInfo
	{	
		int 				currentTime;
		int 				random;
		PlayerInfo 			me;
		OtherInfo 			other;
		int 				specialTime;				//counter for comm routiines
		World				world;
	};

 	ChallengeInfo challInfo;

 	void init(void);

/**
	* (Required) Runs an update of the game rules and calls the user function loop(). The return
	* value for this function indicates if the forceTorqueOut vector should be mixed into
	* thruster firings.  The function should return 0 if the user does not activate any
	* ZR API movement commands or implements their own thruster firing logic.
	*
	* Here we update player's scores
	*
	* /param forceTorqueOut forces and torques to be mixed into thruster firings
	* /return return 1 if forceTorqueOut should be mixed into thruster firings and 0 otherwise
	*/

	bool update(float forceTorqueOut[6]);

	bool updateGameModeManeuver(float forceTorqueOut[6]);

	bool updateGameOverManeuver(float forceTorqueOut[6], unsigned char& finalScore);

	void updateStates();

	void sphereCollision(float* sphereState, float* otherState, float* forceTorqueOut);
	/**
	* (Required) Called on every gspControl control cycle.  It should be used to send debug
	* and telemetry information to the ground.
	*/
	bool enforceBoundaries(float forceTorque[6]);

	/**
	* Limits the direction in which the programmed SPHERE can travel and slows down the velocity if
	* it travels in the direction of the limit
	*/
	void limitDirection(state_vector ctrlState, float ctrlControl[6], unsigned int idx, float dir);

	/**
	* Send Grid initialized information
	*/
	void sendInit(void);
	void modifyInit(short (&init_arr)[16],int i); //Adjust Init array

	/**
	* (Required) Called on every gspControl control cycle, It should be used to send debug
	* and telemetry information to the ground.
	*/
	void sendDebug(void);


	/**
	* (Required) Processes SPHERES telemetry data
	*/
	void processRXData(default_rfm_packet packet);

	/**
	* (Required) Initial state where the satellite is initialized.
	*/
	static const state_vector initStateZR;

	void  breakTie();

	#ifdef ZR2D
	void modify2DForceTorque(float forceTorque[6]);
	#endif

	//Edits forceTorqueOut to resolve the SPHERE's movement
	void resolveCollision(float zrState[12], float forceTorqueOut[6]);
	float dist3d(float* pos1, float* pos2) const;

	float 		randomizeStartingLocs(int itemID, int coord);
	bool		isFacingPos(float position[]);
	bool    	isFacingOther();

	/****************GRID FUNCTIONS***************************/

	/**
	 * Initializes the World with random concentration variables unless a valid grid location is given
	 * \param concentrationX Location in the X axis of the peak concentration(This is mirrored depending on SPHEREID)
	 * \param concentrationY Location in the Y axis of the peak concentration(This is mirrored depending on SPHEREID)
	 */
	void initializeWorld(int concentrationX = -1, int concentrationY = -1);
	#ifdef ZR3D
	void initializeTerrainHeight();
	#endif
	
	void subtractScore();
	/**
	 * Turn off geysers that have been active for a set period of time
	 */
	void turnOffOldGeysers();

	/*********************Zone functions**********************/

	/**
	* Retrieves singleton instance of the game implementation
	* \return The game implementation singleton
	*/
	static ZeroRoboticsGameImpl &instance();

	/**
	* Constructor for binding an API implementation
	*/
	ZeroRoboticsGameImpl(ZeroRoboticsAPIImpl &apiImpl);

	///Reference to ZR API instance
	ZeroRoboticsAPIImpl &apiImpl;

	///Back pointer to the game instance
	ZeroRoboticsGame *game;

};

ZeroRoboticsGameImpl *getGameImpl();

void overrideTarget2D(float target[13]);

// void overrideTarget(float target[13]);
void swingTwistDecomposition(float q[4], float q_sw[4], float q_tw[4], float sw_vec[3]);
void quatToPositive(float q[4]);

#endif
