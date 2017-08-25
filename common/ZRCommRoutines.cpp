
//Basic SPHERES communications

/*
COMM ROUTINES OVERVIEW FOR 2016

During initialization, SPHERE1 sends the position of one of each of large,
medium, small, and adapter items as shorts. SPHERE2 reflects these to
generate its own copy of every item's initial position. SPHERE1 also sends the
attitude of large, medium, and small items as unsigned shorts, which SPHERE2
stores. SPHERE2 does not send any item data during initialization.

During the game, both spheres continuously send item positions to each other.
SPHERE1 sends items 0-2 and SPHERE2 sends items 3-5. Items 6-7 are adapter
items and never move. Whenever a sphere picks up or drops an item, it sends an
unsigned short with that item's ID. An item only changes velocity when a sphere
drops it, or it collides with a sphere or another item. Since the spheres
communicate when a drop occurs, and collisions do not occur until items are
moving, each sphere can determine each item's velocity on its own. Therefore,
item velocity is never sent during the game. Item attitude also never changes
until a sphere picks up and drops an item, and the visualization already knows
to rotate a held item to keep it "docked" with the sphere. Therefore, item
attitude also never needs to be sent between spheres.
*/

#include "ZRGame.h"
#include <stdio.h>

//SPHERES C includes
extern "C" {
#include "spheres_types.h"
#include "comm.h"
}


void ZeroRoboticsGameImpl::processRXData(default_rfm_packet packet){
	int man_num = ctrlManeuverNumGet();

	switch (packet[PKT_CM]){
		case COMM_CMD_DBG_SHORT_SIGNED:

			// copy data into a local variable
			dbg_short_packet DebugVecShort;
			memcpy(DebugVecShort, &packet[PKT_DATA], sizeof(dbg_short_packet));

			//intializing world for sphere 2
			if (man_num < 3)
			{ 
				if(DebugVecShort[1]==99)
				{
					#ifdef SHOW_DEBUG
						GAME_TRACE(("Initializing world from Sat 1 data [%d, %d].", DebugVecShort[2], DebugVecShort[3]));
					#endif
					initializeWorld(DebugVecShort[2], DebugVecShort[3]);
				}
				else
				{
					#ifndef ZR2D
						int row = DebugVecShort[1]; //ranges from 0 to XZ_SIZE (16)
						int col = 0;
						for(int i = 2;i<12;i++)
						{
							challInfo.world.grid[row][col].height =  (DebugVecShort[i]>>8)& 0xFF;
							challInfo.world.grid[row][col+1].height = DebugVecShort[i]& 0xFF;
							col+=2;
						}
						if (row==15)
						{
							printf("\n Sphere 2 received grid height: \n ");
							for(int j = -GRID_Y_SIDE; j < GRID_Y_SIDE; j++)
							{
								for(int i = -GRID_X_SIDE; i < GRID_X_SIDE; i++) 
									printf("%*d ", 3, challInfo.world.grid[i + GRID_X_SIDE][j + GRID_Y_SIDE].height);
								printf("\n");
							}
						}
					#endif
				}
			}
			else
			{
				// get geyser information from other satellite, and actiate it
				for (int i=0; i < MAX_NUM_GEYSERS; i++) // geyser locations, upper nibble = X, lower nibble = Y
				{
					unsigned int startTime = (unsigned int) ((DebugVecShort[i+1] >> 8) & 0xFF);
					int geyserX   = (int) ((DebugVecShort[i+1] >> 4) & 0xF);
					int geyserY   = (int) ((DebugVecShort[i+1])      & 0xF);
					// activate geysers that are not too old (should be off by now anyway)
					if ((startTime != 0) && ((startTime + GEYSER_ACTIVE_TIME/2) > apiImpl.api->getTime()))
						activateGeyser(geyserX, geyserY, startTime);
				}
				// last drill time, to update numDrills if needed
				int lastDrillTime = (int) (DebugVecShort[11] & 0xFF00) >> 8;
				int lastDrillX    = (int) (DebugVecShort[11] & 0x00F0) >> 4;
				int lastDrillY    = (int) (DebugVecShort[11] & 0x000F);
				if ((lastDrillTime != challInfo.other.lastDrillTime) ||
				    (lastDrillX != challInfo.other.lastDrill[0]) ||
						(lastDrillX != challInfo.other.lastDrill[1]))
				{
					challInfo.other.lastDrillTime = lastDrillTime;
					challInfo.other.lastDrill[0] = lastDrillX;
					challInfo.other.lastDrill[0] = lastDrillY;
					challInfo.world.grid[lastDrillX][lastDrillY].numDrills ++;
				}
			}
			break;

		case COMM_CMD_DBG_SHORT_UNSIGNED:
			dbg_ushort_packet DebugVecUShort;
			memcpy(DebugVecUShort, &packet[PKT_DATA], sizeof(dbg_ushort_packet));
			challInfo.other.total_samples  = DebugVecUShort[7];
			challInfo.other.hasAnalyzer[0] = DebugVecUShort[8];
			challInfo.other.hasAnalyzer[1] = DebugVecUShort[9];
			challInfo.other.message        = DebugVecUShort[10];
			break;

		case COMM_CMD_DBG_FLOAT:
			dbg_float_packet DebugVecFloat;
			memcpy(DebugVecFloat, &packet[PKT_DATA], sizeof(dbg_float_packet));
			challInfo.other.score = DebugVecFloat[1];
			break;
	}
}

void ZeroRoboticsGameImpl::sendDebug()
{
  dbg_short_packet  DebugVecShort; 	// short[16]	geyser locations, last completed drill (if any)
  dbg_ushort_packet DebugVecUShort; // ushort[16] has analyzer, total samples (for ties)
  dbg_float_packet  DebugVecFloat; 	// float[8] 	Fuel and Score 
  unsigned int tstep;

	// send debug only send data if we are in game mode (after init)
  if (ctrlManeuverNumGet() < 3) return;
	  
  // normal game packages
	tstep = apiImpl.api->getTime();
	if (!tstep) return; // only send starting with time 1, since time 0 can cause problems with initialization

	// initialize all packets to 0
	memset(DebugVecShort,  0, sizeof(dbg_short_packet));
	memset(DebugVecUShort, 0, sizeof(dbg_ushort_packet));
	memset(DebugVecFloat,  0, sizeof(dbg_float_packet));

	// debug short
	DebugVecShort[0]  = (short)(tstep*10); //Timestamp
	for (int i=0; i < MAX_NUM_GEYSERS; i++) // geyser locations, upper nibble = X, lower nibble = Y
		DebugVecShort[i+1] = ((challInfo.world.geyserTime[i] << 8) & 0xFF00) + ((challInfo.world.geyserLocations[i][0] << 4) & 0x00F0) + (challInfo.world.geyserLocations[i][1] & 0x000F);
	DebugVecShort[11] = (challInfo.me.lastDrillTime << 8) + (((challInfo.me.lastDrill[0] & 0xF) << 4) | (challInfo.me.lastDrill[1] & 0xF));

	// unsigned short debug packet: status of game variables
	DebugVecUShort[0] = (unsigned short)(tstep*10); //Timestamp
	DebugVecUShort[7] = (unsigned short) challInfo.me.total_samples;
	DebugVecUShort[8] = (unsigned short) challInfo.me.hasAnalyzer[0];
	DebugVecUShort[9] = (unsigned short) challInfo.me.hasAnalyzer[1];
	DebugVecUShort[10] = (unsigned short) challInfo.me.message;

	//Float debug packet: score, fuel, forces
	DebugVecFloat[0] = (float)tstep;
	DebugVecFloat[1] = (float) (game->getScore());
	DebugVecFloat[2] = game->getFuelRemaining() / ((double)PROP_ALLOWED_SECONDS);
	memcpy(&DebugVecFloat[5], challInfo.me.userForces, 3*sizeof(float)); //Forces for reference

	//Send packets to other SPHERES/ground/sim; do not modify below this line
	commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_SIGNED, (unsigned char *) DebugVecShort,0);
	commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_FLOAT, (unsigned char *) DebugVecFloat,0);
	commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_UNSIGNED, (unsigned char *) DebugVecUShort,0);

  #ifdef ZRSIMULATION
	  apiImpl.ZRUserDbgVec[0] = (float)tstep;
	  commSendPacket(COMM_CHANNEL_STL, GROUND, sysIdentityGet(), COMM_CMD_DBG_ZRUSER, (unsigned char *) apiImpl.ZRUserDbgVec,0);
  #endif
}

#ifndef ISS_FINALS
void ZeroRoboticsGameImpl::sendInit()
{
	#if (SPHERE_ID == SPHERE1)
		dbg_short_packet DebugVecShort;

		// initialize packets to 0
		memset(DebugVecShort,  0, sizeof(dbg_short_packet)); 

		DebugVecShort[0] = 0;
		DebugVecShort[1] = 99;
		DebugVecShort[2] = (short) challInfo.world.peakConcentration[0];
		DebugVecShort[3] = (short) challInfo.world.peakConcentration[1];

		#ifdef ALLIANCE
			DebugVecShort[4] = (unsigned short) 3;
		#elif defined ZR3D
			DebugVecShort[4] = (unsigned short) 2;
		#elif defined ZR2D
			DebugVecShort[4] = (unsigned short) 1;
		#else
			DebugVecShort[4] = (unsigned short) 0;
		#endif

		commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_SIGNED, (unsigned char *) DebugVecShort,0);

		#ifndef ZR2D
			for(int i = 0;i<X_SIZE;i++){	
				fillInGridHeightData(DebugVecShort,i);
				commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_SIGNED, (unsigned char *) DebugVecShort,0);
			}
		#endif

		#ifdef SHOW_DEBUG
			GAME_TRACE(("World initialization sent to Sat 2."));
		#endif
	#endif
}
#endif

#ifndef ZR2D
void ZeroRoboticsGameImpl::fillInGridHeightData(short (&init_arr)[16], int i)
{
		init_arr[0]=(200+i)*10;
		init_arr[1]=i;//what row we're sending data about from 0 to 15 
		int counter = 2; //used for iterating through init_arr 
		for(int j = 0;j<Y_SIZE;j+=2){
			init_arr[counter]=(short)((char)challInfo.world.grid[i][j].height <<8)+((char)challInfo.world.grid[i][j+1].height ); //storing two different bytes of data in a single short 
			counter++;
		}
}
#endif
