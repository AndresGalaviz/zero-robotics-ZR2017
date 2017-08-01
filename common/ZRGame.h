#ifndef _ZR_GAME_H_
#define _ZR_GAME_H_
#pragma once

extern "C" {
  #include "pads.h"
}

#include "Constants.h"
#include "spheres_constants.h"
#include "ZR_API.h"
#include "ZRGameInternal.h"

/** \file ZRGame.h
 * Contains documentation of functions specific to the player side of the game.
 * Use this documentation to learn about using the API functions available in the challenge.
 * General API operations for Zero Robotics are available under ZR_API.h.
 *
 * Access members of this file by using the syntax "game.functionname(inputs);"
 */

//Forward declaration of the game implementation (not visible)
class ZeroRoboticsGameImpl;

//Forward declaration of API implementation
class ZeroRoboticsAPIImpl;

/** \class ZeroRoboticsGame
 *
 * The class of the game object that you will use. Contains publicly available member functions.
 */
class ZeroRoboticsGame {

 public:

  /*!
   * Retrieves the singleton instance of the game API.  Users are not allowed to construct a game instance, so the
   * API must be retrieved through this interface.
   *
   * \return singleton of the game API
   */
  static ZeroRoboticsGame &instance();


  // DECLARATIONS FOR PUBLIC METHODS AND VARIALBES AVAILABLE TO USERS GO HERE

  /*!
   * Tells the player how much fuel remains.
   *
   * \return float indicating how many seconds of fuel remain.
   */
  float getFuelRemaining();
  
  /*!
   * Send a value from 0-255 to the other satellite.
   * 
   * \param inputMsg Unsigned Char to be sent to other satellite.
   */
  void sendMessage(unsigned char inputMsg);
  
  /*!
   * Receive value from 0-255 from other satellite.
   *
   * \return An unsigned char containing a value from 0-255.
   */
  unsigned char receiveMessage();
  

  /**
   * Checks if the satellite is facing the correct side of an item.
   *
   * \param itemID the item to be checked
   */
  bool isFacingCorrectItemSide(int itemID);
  
  /**
   * Returns the player's current score.
   * 
   * \return Player satellite score.
   */
  float getScore();     //returns me.score
  
  /**
   * Returns the opponent's current score.
   */
  float getOtherScore();    //returns other.score

   /**
    * Returns the time.
    */
  int getCurrentTime();

  // Constructor for the game.  The provided references should be singleton instances.
  ZeroRoboticsGame(ZeroRoboticsGameImpl &impl, ZeroRoboticsAPIImpl &apiImpl) ;

  /*************GRID USER FUNCTIONS***************/

  /**
   * Used to obtain the number of times drilling has occurred in the square that contains pos
   * \param pos float array length 3 (x,y,z)
   * \return Number of drill times
   */
  int getDrills(float pos[3]);

  /**
   * Used to obtain number of active geysers
   * \return Number of active geysers
   */
  int getNumGeysers();

  /**
   * Used to obtain list of known geysers.
   * \param locations Where the locations are updated. List of 10 geysers with location or invalid location.
   */
  void getGeyserLocations(float locations[10][2]);

  /**
   * Used for finding if geyser is present at current location
   * \param position float array length 3 (x,y,z). Location passed by the user.
   * \return True if geyser is present. False otherwise.
   */
  bool isGeyserHere(float position[3]);

  /**
   * Used to find if the station is correctly located above the base station and can drop a sample for points.
   * \param pos float array length 3 (x,y,z), 
   * \return True if the satellite is correctly located above the base station. False otherwise.
   */
  bool atBaseStation(float pos[3]);

  /**************BACTERIA SAMPLE USER FUNCTIONS***********/

  /**
   * Used to obtain the number of samples currently being carried.
   * \return Number of held samples
   */
  int getNumSamplesHeld();

  /**
   * Used to obtain bool list of samples held where each index corresponds to a sample number 
   * \param samples Value at the index is True if the sample has been collected or False if it is empty.
   */
  void getSamplesHeld(bool samples[MAX_SAMPLE_NUMBER]);

  /**
   * Used to obtain a list of concentrations of each of the sample entries that have been analyzed.
   * Where the array index value is the sample number.
   * \param pos float array length 3 (x,y,z). Concentrations updated as: -1 if the sample has not been analyzed. -2 if the sample is empty.
   */
  void getConcentrations(float concentrations[MAX_SAMPLE_NUMBER]);

  /**
   * Attempts to collect Sample. Must be called while drilling.
   * \return -1 if sample space is full, -2 if incorrect Drilling procedure
   */
  int pickupSample();

  /**
   *  Tries to deliver the sample specified by the ID if the sample is held and player is at base station
   *  \param sampleID ID of the sample that is being delivered
   *  \return True if sample successfully delivered. False otherwise.
   */
  bool deliverSample(int sampleID);

  /**
   *  Discards the sample specified by the ID if the sample is held.
   *  \param sampleID ID of the sample that is being delivered
   *  \return True if sample successfully discarded. False otherwise.
   */
  bool discardSample(int sampleID);

  /**************DRILL USER FUNCTIONS********************/

  /**
   * Starts drilling at current position of satellite. 
   * \return True if started successfully (if drilling incorrectly, lose x points per second drill is on)
   */
  bool startDrill();

  /**
   * Stops drilling.
   */
  bool stopDrill();

  /*************ANALYZER USER FUNCTIONS******************/

  /**
   * Takes analyzerID as either 0 or 1, defines pos as the vector of the position of that analyzer
   * \param pos float array length 3 (x,y,z), where position is returned
   * \param analyzerID The ID of the analyzer you want to obtain
   */
  void getAnalyzerPos(float pos[3], int analyzerID);

  /**
   * Used to see if an analyzer is held
   * \return AnalyzerID if the SPHERES has the Sample Analyzer Item 0 otherwise
   */
  int hasAnalyzer();
  void obtainAnalyzer(int analyzerID);

private:
  // REQUIRED: reference to the game implementation, do not delete!
  ZeroRoboticsGameImpl &pimpl;
  ZeroRoboticsAPIImpl &apiImpl;
};


#endif
