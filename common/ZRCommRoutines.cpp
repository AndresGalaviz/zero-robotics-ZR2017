
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

#ifndef ALLIANCE
	#define NUM_RX_ITEMS 3		// the 3 scoring items
#else
	#define NUM_RX_ITEMS 5		// in alliance also sends location of adapter ports
#endif

void ZeroRoboticsGameImpl::processRXData(default_rfm_packet packet)
{
	int man_num = ctrlManeuverNumGet();
	
	
	switch (packet[PKT_CM])
	{
		case COMM_CMD_DBG_SHORT_SIGNED:
			
			
		
			dbg_short_packet DebugVecShort;
			memcpy(DebugVecShort, &packet[PKT_DATA], sizeof(dbg_short_packet));
			GAME_TRACE(("Initializing world. | "));

			initializeWorld(DebugVecShort[0], DebugVecShort[1]);
			// // special item packets
			// if (DebugVecShort[15] == -32766)
			// {
			// 	int itemID = (int) DebugVecShort[1];
			// 	switch ((int) DebugVecShort[2])
			// 	{
			// 		case 0:		// held
			// 			// copy the attitude
			// 			if (challInfo.other.objectHeld != -1) {
			// 				for (int i = 0; i < 3; i++)
			// 				{
			// 					challInfo.item[itemID].zrState[i+6] = ((float) DebugVecShort[3+i]) / 10000.0f;
			// 				}
			// 				// set velocity to 0 while its held
			// 				for (int i = 3; i < 6; i++)
			// 				{
			// 					challInfo.item[itemID].zrState[i] = 0.0f;
			// 				}
			// 			}
			// 			break;
						
			// 		case 1:		// drop
			// 			// copy the attitude
			// 			for (int i = 0; i < 3; i++)
			// 			{
			// 				challInfo.item[itemID].zrState[i+6] = ((float) DebugVecShort[3+i]) / 10000.0f;
			// 			}
			// 			// determine inertia
			// 			challInfo.item[itemID].noInertia = (DebugVecShort[14] == 1);
			// 			challInfo.other.objectHeld = -1;
			// 			challInfo.other.lastObjectHeld = itemID;

			// 			if (challInfo.item[itemID].noInertia)
			// 			{
			// 				// set velocity to 0 if the item has no inertia (e.g. stopped at zone)
			// 				for (int i = 3; i < 6; i++)
			// 				{
			// 					challInfo.item[itemID].zrState[i] = 0.0f;
			// 				}
			// 			}
			// 			else
			// 			{
			// 				// copy the velocity for items with inertia
			// 				for (int i = 3; i < 6; i++)
			// 				{
			// 					challInfo.item[itemID].zrState[i] = ((float) DebugVecShort[3+i]) / 10000.0f;
			// 				}
			// 			}
			// 			break;

			// 		case 2: 		// collision
			// 			for (int i = 0; i < 3; i++)
			// 			{
			// 				challInfo.item[itemID].zrState[i+3] = ((float) DebugVecShort[i+3]) / 10000.0f;
			// 			}
			// 			challInfo.item[itemID].noInertia = false;
			// 			break;
					
			// 		case 3:			// reset
			// 			for (int i = 0; i < 3; i++)
			// 			{
			// 				challInfo.item[itemID].zrState[i+3] = 0.0f;
			// 				challInfo.item[itemID].zrState[i+6] = challInfo.item[itemID].initState[i+6];
			// 			}
			// 			break;
			// 	}
			// 	//GAME_TRACE(("RX Special Pkt %d - Item %d att: [%4.3f, %4.3f, %4.3f] |", DebugVecShort[2], itemID, challInfo.item[itemID].zrState[6], challInfo.item[itemID].zrState[7], challInfo.item[itemID].zrState[8]));
			// }
			// else if(man_num > 2)
			// {
			// 	#if (SPHERE_ID == SPHERE1)
			// 		int offset = 4;
			// 		int stop = 3;
			// 	#else
			// 		int offset = 0;
			// 		int stop = 4;
			// 	#endif

			// 	for(int i = 0; i < stop; i++)
			// 	{
			// 		for(int j = 1; j <= 3; j++)
			// 		{
			// 			challInfo.item[i+offset].zrState[j-1] = ((float) DebugVecShort[3*i+j]) / 10000.0f;
			// 		}
			// 	}
			// }

			// #ifndef ISS_FINALS
			// else
			// {
			// 	// initialization data for items
			// 	if (DebugVecShort[0] == 0)
			// 	{
			// 		#if (SPHERE_ID == SPHERE2)
			// 		// printf("Init DebugVecShort Received by Sphere 2: %d (should be 0), %d\n", DebugVecShort[0], man_num);
			// 		// make sure its one of our initialization packets, all other debugVecShort[0] are time * 10, so they are larger than 0
			// 		for(int i = 0; i < 4; i++)
			// 		{
			// 			for(int j = 1; j <= 3; j++)
			// 			{
			// 				if (i != 3)
			// 				{
			// 					challInfo.item[i*2].zrState[j-1] = DebugVecShort[i*3+j] / 10000.0f;
			// 					challInfo.item[i*2].initState[j-1] = challInfo.item[i*2].zrState[j-1];
			// 					challInfo.item[i*2+1].zrState[j-1] = -challInfo.item[i*2].zrState[j-1];
			// 					challInfo.item[i*2+1].initState[j-1] = challInfo.item[i*2+1].zrState[j-1];
			// 				}
			// 				else
			// 				{
			// 					challInfo.item[7].zrState[j-1] = DebugVecShort[i*3+j] / 10000.0f;
			// 					challInfo.item[7].initState[j-1] = challInfo.item[7].zrState[j-1];
			// 					challInfo.item[8].zrState[j-1] = -challInfo.item[7].zrState[j-1];
			// 					challInfo.item[8].initState[j-1] = challInfo.item[8].zrState[j-1];
			// 				}
			// 			}
			// 		}
			// 		challInfo.item[6].zrState[6] = 1.0f;
			// 		challInfo.item[6].initState[6] = challInfo.item[6].zrState[6];
			// 		#endif
			// 	}
			// }
			// #endif
			break;

		case COMM_CMD_DBG_SHORT_UNSIGNED:
			// dbg_ushort_packet DebugVecUShort;
			// memcpy(DebugVecUShort, &packet[PKT_DATA], sizeof(dbg_ushort_packet));
			// // printf("man_num %d\n",man_num);
			// // recieve initialization packet (control maneuver 1)

			// if (man_num > 2)
			// {
			// 	// new object picked up => set the "other.objectHeld" variable
			// 	if((short)DebugVecUShort[1] != -1)
			// 	{
			// 		challInfo.other.objectHeld = DebugVecUShort[1];
			// 		challInfo.item[DebugVecUShort[1]].held = true;
			// 	}

			// 	// last object held => an object was dropped, so drop it locally too
			// 	if((short)DebugVecUShort[2] != -1)
			// 	{
			// 		challInfo.item[DebugVecUShort[2]].held = false;
			// 		challInfo.other.objectHeld = DebugVecUShort[1];		// in case an item is picked up in the same loop iteration when one is dropped
			// 	}

			// 	challInfo.other.message = DebugVecUShort[10];
			// 	challInfo.other.hasReceiver = DebugVecUShort[11];
				
			// 	if ((challInfo.other.hasReceiver == 7) || (challInfo.other.hasReceiver == 15))
			// 		challInfo.item[7].held = true;
					
			// 	if ((challInfo.other.hasReceiver == 8) || (challInfo.other.hasReceiver == 15))
			// 		challInfo.item[8].held = true;
					
			// } else {
			// 	#if (SPHERE_ID == SPHERE2)
			// 	// printf("Init DebugVecShort Received by Sphere 2: %d (should be 0), %d\n", DebugVecShort[0], man_num);
			// 	// make sure its one of our initialization packets, all other debugVecShort[0] are time * 10, so they are larger than 0

			// 	for(int i = 0; i < 3; i++) {
			// 	  	for (int j = 3; j <= 5; j++) {
			// 	  		challInfo.item[i*2].zrState[j+3] = (short) DebugVecUShort[3*i+j] / 10000.0f;
			// 	  		challInfo.item[i*2].initState[j+3] = challInfo.item[i*2].zrState[j+3];
			// 	  		challInfo.item[i*2+1].zrState[j+3] = -challInfo.item[i*2].zrState[j+3];
			// 			challInfo.item[i*2+1].initState[j+3] = challInfo.item[i*2+1].zrState[j+3];
			// 		}
			// 	}
			// 	#endif
			// }
			break;

		case COMM_CMD_DBG_FLOAT:
			// dbg_float_packet DebugVecFloat;
			// memcpy(DebugVecFloat, &packet[PKT_DATA], sizeof(dbg_float_packet));
			
			// #ifndef ISS_FINALS
			// if(man_num < 3) {
			// 	#if (SPHERE_ID == SPHERE2)
			// 	//printf("Init DebugVecFloat Received by Sphere 2: %f (should be -1), %d\n", DebugVecFloat[0], man_num);
			// 	memcpy(challInfo.other.zone.center, DebugVecFloat+1, 3*sizeof(float));
			// 	memcpy(challInfo.me.zone.center, DebugVecFloat+4, 3*sizeof(float));
			// 	#endif
			// }
			// #endif
			
			// challInfo.other.score = DebugVecFloat[1];
			//challInfo.other.fuel = DebugVecFloat[INDEX];
			break;
	}
}

void ZeroRoboticsGameImpl::sendDebug() {
  dbg_short_packet  DebugVecShort; // short[16]
  dbg_ushort_packet DebugVecUShort; // ushort[16] Zone Error, Points per second, receiver, and game type
  dbg_float_packet  DebugVecFloat; // float[8] Fuel and Score 
  unsigned int tstep;

  // send debug after estimator convergence
  #ifndef ISS_FINALS


  //fflush(stdout);
  if (ctrlManeuverNumGet() < 3)
  {
  sendInit();
  }
  #endif
  
//   if (ctrlManeuverNumGet() > 1)
//   {
  
//   // normal game packages
//   tstep = apiImpl.api->getTime();
	
//   if (!tstep) return; // only send starting with time 1, since time 0 can cause problems with initialization

//   // initialize all packets to 0
//   memset(DebugVecShort,  0, sizeof(dbg_short_packet));
//   memset(DebugVecUShort, 0, sizeof(dbg_ushort_packet));
//   memset(DebugVecFloat,  0, sizeof(dbg_float_packet));

//   // debug short: item locations
//   DebugVecShort[0]  = (short)(tstep*10); //Timestamp

//   #if (SPHERE_ID == SPHERE1)
//   int offset = 0;
//   int stop = 4;
//   #else
//   int offset = 4;
//   int stop = 3;
//   #endif

//   for (int i = 0; i < stop; i++) {
//     for(int j = 1; j <= 3; j++) {
//       DebugVecShort[3*i+j] = (short)(10000 * challInfo.item[i+offset].zrState[j-1]);
//     }
//   }



//   // unsigned short debug packet: status of game variables
//   DebugVecUShort[0] = (unsigned short)(tstep*10); //Timestamp



//   DebugVecUShort[8] = (unsigned short) challInfo.me.zone.numSPSheld;
//   DebugVecUShort[10] = (unsigned short) challInfo.me.message;
//   DebugVecUShort[11] = (unsigned short) challInfo.me.hasReceiver;

//   //Float debug packet: score, fuel, forces
//   DebugVecFloat[0] = (float)tstep;
//   DebugVecFloat[1] = (float) (game->getScore());
//   DebugVecFloat[2] = game->getFuelRemaining() / ((double)PROP_ALLOWED_SECONDS);
//   memcpy(&DebugVecFloat[5], challInfo.me.userForces, 3*sizeof(float)); //Forces for reference

//   //Send packets to other SPHERES/ground/sim; do not modify below this line
//   commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_SIGNED, (unsigned char *) DebugVecShort,0);
//   commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_FLOAT, (unsigned char *) DebugVecFloat,0);
//   commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_UNSIGNED, (unsigned char *) DebugVecUShort,0);

// 	//commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_SIGNED, (unsigned char*) DebugVecItem, 0);
//   #ifdef ZRSIMULATION
//   apiImpl.ZRUserDbgVec[0] = (float)tstep;
//   commSendPacket(COMM_CHANNEL_STL, GROUND, sysIdentityGet(), COMM_CMD_DBG_ZRUSER, (unsigned char *) apiImpl.ZRUserDbgVec,0);
//   #endif
//   }
}

//happens after a collision, item drop, when a item is being held, when an item is being reset
// void ZeroRoboticsGameImpl::sendSpecial(int caseType, int itemID) {
//   dbg_short_packet 	DebugVecSpecial; //short[16]

//   memset(DebugVecSpecial,  0, sizeof(dbg_short_packet));

//   int tstep = apiImpl.api->getTime();
//   DebugVecSpecial[0] = (short) (challInfo.specialTime*10);

//   if (caseType == 4) {
//   	DebugVecSpecial[15] = -32767;
//   	commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_SIGNED, (unsigned char *) DebugVecSpecial,0); 
//   	return;
//   }

//   if (challInfo.item[itemID].noInertia) {
//   	DebugVecSpecial[14] = 1;
//   } else {
//   	DebugVecSpecial[14] = 0;
//   }

//   DebugVecSpecial[13] = (short)(tstep*10);
//   DebugVecSpecial[15] = -32766;

//   //caseType: 0 = held item, 1 = dropped item, 2 = collision, 3 = reset, 4 = nothing
//   DebugVecSpecial[2] = caseType;

//   switch (caseType) {
//   	case 0:
// 		case 3:
//   		for (int i = 0; i < 3; i++) {
//   			DebugVecSpecial[3+i] = (short) (challInfo.item[itemID].zrState[i+6] * 10000);
//   		}
//   		break;
//   	case 1:
//   		for (int i = 0; i < 3; i++) {
// 				DebugVecSpecial[3+i] = (short) (challInfo.item[itemID].zrState[i+6] * 10000);
//   		}
// 			for (int i = 3; i < 6; i++) {
// 				DebugVecSpecial[3+i] = (short) (challInfo.item[itemID].zrState[i] * 10000);
// 			}
//   		break;
//   	case 2: 
//   		for (int i = 0; i < 3; i++) {
//   			DebugVecSpecial[3+i] = (short) (challInfo.item[itemID].zrState[i+3] * 10000);
//   		}
//   		break;
//   }

//   DebugVecSpecial[1] = itemID;
//   challInfo.specialTime++;
//   commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_SIGNED, (unsigned char *) DebugVecSpecial,0);

// 	//GAME_TRACE(("TX Special Pkt %d - Item %d att: [%4.3f, %4.3f, %4.3f] |", caseType, itemID, challInfo.item[itemID].zrState[6], challInfo.item[itemID].zrState[7], challInfo.item[itemID].zrState[8]));

// }

void ZeroRoboticsGameImpl::sendInit()
{
	#ifndef ISS_FINALS
  	#if (SPHERE_ID == SPHERE1)
    dbg_short_packet DebugVecShort;
    dbg_ushort_packet DebugVecUShort;
    dbg_float_packet DebugVecFloat;
	
	GAME_TRACE(("Sending world initialization|"));
	// initialize packets to 0
	memset(DebugVecShort,  0, sizeof(dbg_short_packet)); // float[16]
	memset(DebugVecUShort,  0, sizeof(dbg_ushort_packet));
	memset(DebugVecFloat,  0, sizeof(dbg_float_packet));

	DebugVecShort[0] = (short) challInfo.world.peakConcentration[0];
	DebugVecShort[1] = (short) challInfo.world.peakConcentration[1];

	DebugVecUShort[0] = (unsigned short) 0;

	#ifdef ALLIANCE
	DebugVecUShort[2] = (unsigned short) 3;
	#elif defined ZR3D
	DebugVecUShort[2] = (unsigned short) 2;
	#elif defined ZR2D
	DebugVecUShort[2] = (unsigned short) 1;
	#else
	DebugVecUShort[2] = (unsigned short) 0;
	#endif


	commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_SIGNED, (unsigned char *) DebugVecShort,0);
	commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_UNSIGNED, (unsigned char *) DebugVecUShort,0);
	commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_FLOAT, (unsigned char *) DebugVecFloat,0);
	#endif
	/*
	#if (SPHERE_ID == SPHERE2)
	dbg_short_packet DebugVecShort;
	DebugVecShort[0] = 0;

	commSendPacket(COMM_CHANNEL_STL, BROADCAST, 0, COMM_CMD_DBG_SHORT_SIGNED, (unsigned char *) DebugVecShort,0);
	#endif
	*/
	#endif
}