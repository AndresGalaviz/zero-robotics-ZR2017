/*
 * gsp.c
 *
 * SPHERES Guest Scientist Program custom source code template.
 *
 * MIT Space Systems Laboratory
 * SPHERES Guest Scientist Program
 * http://ssl.mit.edu/spheres/
 * 
 * Copyright 2005 Massachusetts Institute of Technology
 */


/*----------------------------------------------------------------------------*/
/*                         Do not modify this section.                        */
/*----------------------------------------------------------------------------*/
#include "comm.h"
#include "commands.h"
#include "control.h"
#include "gsp.h"
#include "gsp_task.h"
#include "gsp_internal.h"
#include "pads.h"
#include "prop.h"
#include "spheres_constants.h"
#include "spheres_physical_parameters.h"
#include "spheres_types.h"
#include "std_includes.h"
#include "system.h"
#include "util_memory.h"

/*----------------------------------------------------------------------------*/
/*                     Modify as desired below this point.                    */
/*----------------------------------------------------------------------------*/
#include "gsutil_checkout.h"

#include "ZRGameInternal.h"
#include "ZRGame.h"

#define CHECKOUT_TESTNUM_OFFSET			(000)
#define ZERROBOTICS_TESTNUM_OFFSET		(100)

                                 // 1   2    3    4    5    6    7    8    9   10   11   12,   13,  14,  15,  16,  17
const unsigned int refTestNumber[]={101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 199};

static int testclass;
typedef enum _test_class {invalid_test, checkout_tests, zerorobotics_tests} test_class;

static unsigned int testnum = 0;

static ZeroRoboticsGameImpl &gameImplRef = ZeroRoboticsGameImpl::instance();

test_class get_test_class(unsigned int test_num)
{
	if ((test_num > CHECKOUT_TESTNUM_OFFSET) && (test_num <= ZERROBOTICS_TESTNUM_OFFSET))
		return checkout_tests;

	if ((test_num > ZERROBOTICS_TESTNUM_OFFSET))
		return zerorobotics_tests;
		
	return invalid_test;
}


void gspIdentitySet()
{
   // set the logical identifier (SPHERE#) for this vehicle
   sysIdentitySet(SPHERE_ID);
}


void gspInitProgram()
{
   // set the unique program identifier (to be assigned by MIT)
   sysProgramIDSet(9998);

   // set up communications TDMA frames
   commTdmaStandardInit(COMM_CHANNEL_STL, sysIdentityGet(), NUM_SPHERES);
   commTdmaStandardInit(COMM_CHANNEL_STS, sysIdentityGet(), NUM_SPHERES);

   // enable communications channels
   commTdmaEnable(COMM_CHANNEL_STL);
   commTdmaEnable(COMM_CHANNEL_STS);   
   
   // allocate storage space for IMU samples
   padsInertialAllocateBuffers(50);

   // inform system of highest beacon number in use
   padsInitializeFPGA(NUM_BEACONS);
}

void gspInitTest(unsigned int test_number)
{
	// make a zero-offset index
	unsigned int index;
	
	index = test_number-1;
	
	testclass = get_test_class(refTestNumber[index]);
	testnum = refTestNumber[index] % 100;

	switch (testclass)
	{
		case checkout_tests:
			gspInitTest_Checkout(testnum);
			break;
		case zerorobotics_tests:
			gameImplRef.gspInitTest(test_number);
			break;
		default:
			ctrlTestTerminate(TEST_RESULT_UNKNOWN_TEST);
			break;
	}
}

void gspInitTask()
{
}


void gspPadsInertial(IMU_sample *accel, IMU_sample *gyro, unsigned int num_samples)
{
}


void gspPadsGlobal(unsigned int beacon, beacon_measurement_matrix measurements)
{
}


void gspTaskRun(unsigned int gsp_task_trigger, unsigned int extra_data)
{
	switch (testclass)
	{
		case checkout_tests:
			gspTaskRun_Checkout(gsp_task_trigger,extra_data);
			break;
	}

}


void gspControl(unsigned int test_number, unsigned int test_time, unsigned int maneuver_number, unsigned int maneuver_time)
{
	switch (testclass)
	{
		case checkout_tests:
			gspControl_Checkout(testnum, test_time, maneuver_number, maneuver_time); 
			break;
		case zerorobotics_tests:
			gameImplRef.gspControl(test_number,test_time,maneuver_number,maneuver_time);
			break;
	}
}


void gspProcessRXData(default_rfm_packet packet)
{
	//USED TO AVOID CHECKS IN MUDFLAP
	#ifdef ZRSIMULATION
		default_rfm_packet localPacket;
		unchecked_memcpy(localPacket,packet,37);
	#else
		#define localPacket packet
	#endif
	switch(testclass)
	{
		case zerorobotics_tests:
			gameImplRef.gspProcessRXData(packet);
		break;
	
	}
	#ifndef ZRSIMULATION
	#undef localPacket
	#endif
}
