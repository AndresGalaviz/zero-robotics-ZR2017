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

#undef		ZR3D
#undef		ALLIANCE
#undef  	ISS_FINALS

#define 	SHOW_GAME_TRACE 	// If this is undef, then the GAME_TRACEs won't show up.
#define		SHOW_DEBUG				// show full debug game_trace's, not just the ones that remain in the game

/*****************************************************************************************
****                                SPHERE PROPERTIES                                 ****
*****************************************************************************************/

#define		SPHERE_MAX_VEL			0.05f		// Measured in meters per second
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
//Scoring Constants
#define		SCORE_DRILL_PENALTY		0.25f
#define		SCORE_GEYSER_PENALTY	10.0f
#define		MIN_CONCENTRATION		1
#define		LOW_CONCENTRATION		3
#define		MED_CONCENTRATION		6
#define		HIGH_CONCENTRATION	10

//Drilling Requirements
#define		MAX_DRILL_SPEED						0.01f
#define		MIN_DRILL_ROTATION				0.968912f // ~cosf(0.25 rad ~= 14.3 degrees)
#define		MAX_DRILL_POINT_ERROR			0.195090f // ~sinf(pi/16 rad = 11.25 degrees)
#define		MAX_DRILL_INIT_RATE				0.04f // rad/sec

//Geyser variables
#define	MAX_NUM_GEYSERS							10
#define GEYSER_ACTIVE_TIME					30
#define	GEYSER_MIN_DISTANCE					SPHERE_RADIUS
#define	GEYSER_FORCE								0.2f
#define GEYSER_PUSH_TIME						5

//Fuel management variables
#define		MIN_FUEL(a, b) ((a < b) ? b : a)		// Prevents fuel from being less than 0
#define		MAX_FUEL(c, d) ((c < d) ? c : d)		// Prevents fuel from being greater than PROP_ALLOWED_SECONDS
#define		PROP_ALLOWED_SECONDS		60.0f 			//!< Time in thruster seconds allowed per user. Full tank _500 seconds.
#define		OUT_OF_BOUNDS_PENALTY		0.02f * PROP_ALLOWED_SECONDS	//!< Fuel penalty per second for being out of bounds.

//Zone Constraints
#define		ZONE_RADIUS			0.1f
#define		OOBgain					10.0f 				//Used to slow down the satellite.
// random x,y,z coordinates for center

//World Constraints		subject to change
#define		WORLD_pX		0.64f			//!< Highest X coordinate in bounds.
#define		WORLD_pY		0.80f			//!< Highest Y coordinate in bounds.
#define		WORLD_pZ		0.64f			//!< Highest Z coordinate in bounds.
#define		WORLD_nX		-WORLD_pX		//!< Lowest X coordinate in bounds.
#define		WORLD_nY		-WORLD_pY		//!< Lowest Y coordinate in bounds.
#define		WORLD_nZ		-WORLD_pZ		//!< Lowest Z coordinate in bounds.

// Grid constants
#define GRID_X_SIDE					8
#define GRID_Y_SIDE					10
#define GRID_Z_SIDE					8
#define X_SIZE							16
#define Y_SIZE							20
#define Z_SIZE							16
#define CELL_SIZE						0.08f
#define	GRID_X(posX)				(static_cast<int>(floor(posX/CELL_SIZE))+GRID_X_SIDE)
#define	GRID_Y(posY)				(static_cast<int>(floor(posY/CELL_SIZE))+GRID_Y_SIDE)
#define	GRID_Z(posZ)				(static_cast<int>(floor(posZ/CELL_SIZE))+GRID_Z_SIDE)
#define	OFF_GRID						-100

// base constants
#define BASE_RADIUS					0.15f
#define	MAX_BASE_SPEED			0.01f
#define MAX_BASE_RATE				0.04f

// analzers constants
#define NUM_ANALYZERS					2
#define	MAX_ANALYZER_SPEED		0.01f

// Player constants
#define MAX_SAMPLE_NUMBER		3

const float limits[3] = {WORLD_pX, WORLD_pY, WORLD_pZ};		//!< The limits of the interaction zone.

#endif
