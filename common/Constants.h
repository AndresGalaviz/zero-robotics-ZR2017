/************************
*						*
*						*
*						*
*		Constants		*
*		6/21/16			*
*						*
*						*
*						*
************************/

/**
 * \file Constants.h
 *
 * A list of constants used in the ZR program.
 */

#ifndef		Constants_H
#define		Constants_H
#pragma	once

extern "C" {
	#include "math_matrix.h"
}

#define		ZR2D

#define		ZR3D
#undef		ALLIANCE
#undef  	ISS_FINALS

#define 	SHOW_GAME_TRACE // If this is undef, then the GAME_TRACEs won't show up.

/**
 * \var int GAME_TIME
 *
 * The time at game start.
 */
#define		GAME_TIME			0

/*****************************************************************************************
****                                SPHERE PROPERTIES                                 ****
*****************************************************************************************/

#define		SPHERE_MAX_VEL				0.05f		// Measured in meters per second
#define		SPHERE_RADIUS				0.11f		// Measured in meters
#define 	SPHERE_MASS					4.08f		// Measured in kilograms
/**
 * \var int VEL_X
 *
 * The index for the beginning of the velocity array inside of ZRState.
 */
#define		VEL_X					3

/**
 * \var int MAX_GAME_TIME
 *
 * Length of the whole game in seconds.
 */
#define		MAX_GAME_TIME         	180

/**
 * \var float MAX_DOCKING_ANGLE
 *
 * Cosine of the angle at which blocks may be docked.
 */
#define		MAX_DOCKING_ANGLE			0.968912f // ~cosf(0.25f)


#ifdef ALLIANCE
#define		NUM_ITEMS				9			//!< The number of items at the start of the game
#define		NUM_RECEIVER			2			//!< The number of items at the start of the game that are receivers
#else
#define		NUM_ITEMS 				6
#define 	NUM_RECEIVER	 		0
#endif

#define		NUM_SCORE_ITEMS			(NUM_ITEMS-NUM_RECEIVER)

#ifdef ZR2D

#elif defined ZR3D

#else

#endif

#define		MAX_FACING_ANGLE	0.968912f

//Scoring Constants
#define		START_SCORE				0.0f
#define		COLLISION_MULT			5.0f 		//!< Multiply by collision speed to find amount of points lost in a collission
#define 	BAD_DOCK_POINTS			0.3f 		//!< Points lost for docking incorrectly
#define		BAD_DRILL				5
#define		MIN_CONCENTRATION		1
#define		LOW_CONCENTRATION		3
#define		MED_CONCENTRATION		6
#define		HIGH_CONCENTRATION		10


//Fuel management variables
#define		MIN_FUEL(a, b) ((a < b) ? b : a)		// Prevents fuel from being less than 0
#define		MAX_FUEL(c, d) ((c < d) ? c : d)		// Prevents fuel from being greater than PROP_ALLOWED_SECONDS
#define		PROP_ALLOWED_SECONDS	60.0f 							//!< Time in thruster seconds allowed per user. Full tank _500 seconds.
#define		OUT_OF_BOUNDS_PENALTY		0.02f * PROP_ALLOWED_SECONDS	//!< Fuel penalty per second for being out of bounds.

//Zone Constraints
#define		ZONE_RADIUS			0.1f
#define		OOBgain				10.0f 				//Used to slow down the satellite.
// random x,y,z coordinates for center

//World Constraints		subject to change
#define		WORLD_pX		0.64f			//!< Highest X coordinate in bounds.
#define		WORLD_pY		0.80f			//!< Highest Y coordinate in bounds.
#define		WORLD_pZ		0.64f			//!< Highest Z coordinate in bounds.
#define		WORLD_nX		-WORLD_pX		//!< Lowest X coordinate in bounds.
#define		WORLD_nY		-WORLD_pY		//!< Lowest Y coordinate in bounds.
#define		WORLD_nZ		-WORLD_pZ		//!< Lowest Z coordinate in bounds.

// Grid constants
#define GRID_X_SIDE		8
#define GRID_Y_SIDE		10
#define GRID_Z_SIDE		8
#define XZ_SIZE			16
#define Y_SIZE			20
#define BASE_SIDE_SIZE	2
#define CELL_SIZE		0.08
#define GEYSER_ACTIVE_TIME	10

#ifdef ZR3D
#define MAX_GEYSER_HEIGHT		4
#endif

// Player constants
#define MAX_SAMPLE_NUMBER	3

//#define		ITEM_EDGE		(SPHERE_RADIUS + LARGE_ITEM_RADIUS)

const float limits[3] = {WORLD_pX, WORLD_pY, WORLD_pZ};		//!< The limits of the interaction zone.

#endif
