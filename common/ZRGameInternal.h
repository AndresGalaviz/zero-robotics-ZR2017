#ifndef _ZR_GAME_INTERNAL_H_
#define _ZR_GAME_INTERNAL_H_
#pragma once

#include "Constants.h"
#include "ZeroRoboticsGameBaseImpl.hpp"
#include "comm.h"

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
		unsigned int 			concentration; 	// Bacteria concentration 
		unsigned int 			numDrills; 	   	// Number of times this cell has been drilled
		#ifndef ZR2D
		unsigned int			height;					// height of the cell for 3D games
		#endif
	};

	struct Base
	{
		unsigned int rotateTime;
	};

	struct World
	{
		Cell						grid[X_SIZE][Y_SIZE]; 								// grid of the world with indeces [0..(X_SIZE-1)],[0..(Y_SIZE-1)] 
		int							peakConcentration[2];
		unsigned int		numActiveGeysers;
		int							geyserLocations[MAX_NUM_GEYSERS][2]; 	// 10 geyser, 2D coordinates [0..(X_SIZE-1)],[0..(Y_SIZE-1)] of the grid where the geyser is at
		unsigned int		geyserTime[MAX_NUM_GEYSERS];					// 0 when OFF, otherwise gameTime() when it was turned on
	};
	
	struct PlayerInfo
	{
		float           zrState[12];
		state_vector    sphState;
		float 					fuelUsed;
		float 					userForces[6];											// stores userForces for download
		float 					score;															// current total score in the game
		short 					message;														// message to send to other satellite
		int							samples[MAX_SAMPLE_NUMBER][2]; 			// Space where samples are held: [OFF_GRID,OFF_GRID] = no sample; otherse square number [0..(X_SIZE-1)],[0..(Y_SIZE-1)] of sample
		unsigned int		total_samples;											// total number of samples picked-up
		bool						hasAnalyzer[NUM_ANALYZERS];					// true if analyzer held
		unsigned int		analyzerTime[NUM_ANALYZERS];				// time that the satellite has been over the analyzer
		bool						drillEnabled;
		bool						drillError;
		int							drillSquare[2];											// square where drill operation started
		float						drillInitAtt[3];										// the 2D (X & Y) pointing of the satellite at the start of a drill operation
		bool						sampleReady;												// turns true when a drill operation is complete and the sample can be picked-up
		int							lastDrill[2];												// the last completed drill, for communications with other sat to keep numDrill between both sats
		unsigned int		lastDrillTime;											// in case a player drills in the same square twice in a row
		float						geyserForce[2];											// the force if its being pushed by a geizer
		int							geyserSquare[2];										// the square where the geyser hit, so that its not repeated
		unsigned int		geyserTime;													// the time the geyser force started (it goes on for 5 seconds)
	};

	struct OtherInfo
	{
		float           zrState[12];
		state_vector    sphState;
		float 					score;												// latest known score of other player
		short 					message;											// message from other player
		bool 						hasAnalyzer[NUM_ANALYZERS];		// true if analyzer held
		unsigned int		total_samples;								// total number of samples picked-up
		int							lastDrill[2];												// the last completed drill, for communications with other sat to keep numDrill between both sats
		unsigned int		lastDrillTime;											// in case a player drills in the same square twice in a row
	};
	
	struct ChallengeInfo
	{	
		PlayerInfo 			me;
		OtherInfo 			other;
		World						world;
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

	bool enforceBoundaries(float forceTorque[6]);

	/**
	* Limits the direction in which the programmed SPHERE can travel and slows down the velocity if
	* it travels in the direction of the limit
	*/
	void limitDirection(state_vector ctrlState, float ctrlControl[6], unsigned int idx, float dir);

	/**
	* Send Grid initialized information
	*/
	#ifndef ISS_FINALS
	void sendInit(void);
	#endif
	
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

	#ifdef ZR2D
	void modify2DForceTorque(float forceTorque[6]);
	#endif

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
	
	/**
	 * Randomly sets the height of the surface for 3D games.
	 */
	#ifndef ZR2D
	void initializeTerrainHeight();
	#endif

	/**
	 * Copies the grid-height into a short array for communications 
	 */
	#ifndef ZR2D
	void fillInGridHeightData(short (&init_arr)[16], int i);
	#endif
	
	#ifdef ZR2D
	void fillInConcentrationData(short (&init_arr)[16],int i);
	#endif
	/**
	 * Checks if the satellite is currently at the square indicated by the grid point [0..(X_SIZE-1)],[0..(Y_SIZE-1)] 
	 */
	bool checkAtSquare(int gridX, int gridY);
	
	/**
	 * Updates the player score based on drill errors (add other "every second" penalties here)
	 */
	void updateScore();

	/**
	 * Turn off geysers that have been active for a set period of time
	 * and create forces if a satellite runs into an active geyser
	 */
	bool updateGeysers();

	/**
	 * Randomly activates a geyser at the given grid poing [0..(X_SIZE-1)],[0..(Y_SIZE-1)] based on numDrills at location
	 */
	void checkGeyser(int gridX, int gridY);

	/**
	 * Turn on a geyser at the corresponding grid point [0..(X_SIZE-1)],[0..(Y_SIZE-1)] with the given start time
	 * (created so that it can be called by the random geyser creation or by communications)
	 */
	void activateGeyser(int gridX, int gridY, unsigned int startTime);

	/**
	 * Check if an analyzer is being picked-up
	 */
	void updateAnalyzer();
	
	/**
	 * Check if a drill operation is complete and a sample is available
	 * or if a drill error occurs.
	 */
	void updateDrill();

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

#endif
