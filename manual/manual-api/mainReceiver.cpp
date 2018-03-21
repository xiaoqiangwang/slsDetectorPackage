/**
 \file mainReceiver.cpp

This file is an example of how to implement the slsReceiverUsers class
You can compile it linking it to the slsReceiver library

g++ mainReceiver.cpp -L lib -lSlsReceiver -L/usr/lib64/ -L lib2 -lzmq  -pthread -lrt -lm -lstdc++

where,

lib is the location of lSlsReceiver.so

lib2 is the location of the libzmq.a.
[ libzmq.a is required only when using data call backs and enabling data streaming from receiver to client.
It is linked in manual/manual-api from slsReceiverSoftware/include ]

 */

#include "sls_receiver_defs.h"
#include "slsReceiverUsers.h"

#include <iostream>
#include <string.h>
#include <signal.h>	//SIGINT
#include <cstdlib>		//system
//#include "utilities.h"
//#include "logger.h"
#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait
#include <string>
#include <unistd.h> 	//usleep
#include <errno.h>
#include <syscall.h>	//tid
using namespace std;


/** Define Colors to print data call back in different colors for different recievers */
#define PRINT_IN_COLOR(c,f, ...) 	printf ("\033[%dm" f RESET, 30 + c+1, ##__VA_ARGS__)


/** Variable is true to continue running, set to false upon interrupt */
bool keeprunning;

/**
 * Control+C Interrupt Handler
 * Sets the variable keeprunning to false, to let all the processes know to exit properly
 */
void sigInterruptHandler(int p){
	keeprunning = false;
}

/**
 * prints usage of this example program
 */
void printHelp() {
	cprintf(RESET, "Usage:\n"
			"./detReceiver [start_tcp_port] [num_receivers] [1 for call back, 0 for none]\n\n");
	exit(EXIT_FAILURE);
}

/**
 * Start Acquisition Call back
 * slsReceiver writes data if file write enabled.
 * Users get data to write using call back if registerCallBackRawDataReady is registered.
 * @param filepath file path
 * @param filename file name
 * @param fileindex file index
 * @param datasize data size in bytes
 * @param p pointer to object
 * \returns ignored
 */
int StartAcq(char* filepath, char* filename, uint64_t fileindex, uint32_t datasize, void*p){
	cprintf(BLUE, "#### StartAcq:  filepath:%s  filename:%s fileindex:%llu  datasize:%u ####\n",
			filepath, filename, fileindex, datasize);

	cprintf(BLUE, "--StartAcq: returning 0\n");
	return 0;
}

/**
 * Acquisition Finished Call back
 * @param frames Number of frames caught
 * @param p pointer to object
 */
void AcquisitionFinished(uint64_t frames, void*p){
	cprintf(BLUE, "#### AcquisitionFinished: frames:%llu ####\n",frames);
}

/**
 * Get Receiver Data Call back
 * Prints in different colors(for each receiver process) the different headers for each image call back.
 * @param frameNumber frame number
 * @param expLength real time exposure length (in 100ns) or sub frame number (Eiger 32 bit mode only)
 * @param packetNumber number of packets caught for this frame
 * @param bunchId bunch id from beamline
 * @param timestamp time stamp  in 10MHz clock (not implemented for most)
 * @param modId module id	(not implemented for most)
 * @param xCoord x coordinates (detector id in 1D)
 * @param yCoord y coordinates (not implemented)
 * @param zCoord z coordinates (not implemented)
 * @param debug debug values if any
 * @param roundRNumber (not implemented)
 * @param detType detector type see :: detectorType
 * @param version version of standard header (structure format)
 * @param datapointer pointer to data
 * @param datasize data size in bytes. To write/stream a smaller size of processed data, change this value (only smaller value is allowed).
 * @param p pointer to object
 */
void GetData(uint64_t frameNumber, uint32_t expLength, uint32_t packetNumber, uint64_t bunchId, uint64_t timestamp,
		uint16_t modId, uint16_t xCoord, uint16_t yCoord, uint16_t zCoord, uint32_t debug, uint16_t roundRNumber, uint8_t detType, uint8_t version,
		char* datapointer, uint32_t* datasize, void* p){

	PRINT_IN_COLOR (modId?modId:xCoord,
			"#### %d GetData: ####\n"
			"frameNumber: %llu\t\texpLength: %u\t\tpacketNumber: %u\t\tbunchId: %llu\t\ttimestamp: %llu\t\tmodId: %u\t\t"
			"xCoord: %u\t\tyCoord: %u\t\tzCoord: %u\t\tdebug: %u\t\troundRNumber: %u\t\tdetType: %u\t\t"
			"version: %u\t\tfirstbytedata: 0x%x\t\tdatsize: %u\n\n",
			xCoord, frameNumber, expLength, packetNumber, bunchId, timestamp, modId,
			xCoord, yCoord, zCoord, debug, roundRNumber, detType, version,
			((uint8_t)(*((uint8_t*)(datapointer)))), *datasize);
	*datasize = 26000;
}



/**
 * Example of main program using the slsReceiverUsers class
 *
 * - Defines in file for:
 *  	- Default Number of receivers is 1
 *  	- Default Start TCP port is 1954
 */
int main(int argc, char *argv[]) {

	/**	- set default values */
	int numReceivers = 1;
	int startTCPPort = 1954;
	int withCallback = 0;
	keeprunning = true;

	/**	- get number of receivers and start tcp port from command line arguments */
	if ( (argc != 4) || (!sscanf(argv[1],"%d", &startTCPPort)) || (!sscanf(argv[2],"%d", &numReceivers)) || (!sscanf(argv[3],"%d", &withCallback)) )
		printHelp();
	cprintf(BLUE,"Parent Process Created [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
	cprintf(RESET, "Number of Receivers: %d\n", numReceivers);
	cprintf(RESET, "Start TCP Port: %d\n", startTCPPort);
	cprintf(RESET, "Callback Enable: %d\n", withCallback);



	/** - Catch signal SIGINT to close files and call destructors properly */
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
	sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		cprintf(RED, "Could not set handler function for SIGINT\n");
	}

	/** - Ignore SIG_PIPE, prevents global signal handler, handle locally,
	   instead of a server crashing due to client crash when writing, it just gives error */
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=SIG_IGN;					// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGPIPE, &asa, NULL) == -1) {
		cprintf(RED, "Could not set handler function for SIGPIPE\n");
	}


	/** - loop over number of receivers */
	for (int i = 0; i < numReceivers; ++i) {

		/**	- fork process to create child process */
		pid_t pid = fork();

		/**	- if fork failed, raise SIGINT and properly destroy all child processes */
		if (pid < 0) {
			cprintf(RED,"fork() failed. Killing all the receiver objects\n");
			raise(SIGINT);
		}

			/**	- if child process */
		else if (pid == 0) {
			cprintf(BLUE,"Child process %d [ Tid: %ld ]\n", i, (long)syscall(SYS_gettid));

			char temp[10];
			sprintf(temp,"%d",startTCPPort + i);
			char* args[] = {(char*)"ignored", (char*)"--rx_tcpport", temp};
			int ret = slsReceiverDefs::OK;
			/**	-  create slsReceiverUsers object with appropriate arguments */
			slsReceiverUsers *receiver = new slsReceiverUsers(3, args, ret);
			if(ret==slsReceiverDefs::FAIL){
				delete receiver;
				exit(EXIT_FAILURE);
			}


			/**	- register callbacks. remember to set file write enable to 0 (using the client)
		  if we should not write files and you will write data using the callbacks */
			if (withCallback) {

				/** - Call back for start acquisition */
				cprintf(BLUE, "Registering 	StartAcq()\n");
				receiver->registerCallBackStartAcquisition(StartAcq, NULL);

				/** - Call back for acquisition finished */
				cprintf(BLUE, "Registering 	AcquisitionFinished()\n");
				receiver->registerCallBackAcquisitionFinished(AcquisitionFinished, NULL);

				/* 	- Call back for raw data */
				cprintf(BLUE, "Registering     GetData() \n");
				receiver->registerCallBackRawDataReady(GetData,NULL);
			}



			/**	- start tcp server thread */
			if (receiver->start() == slsReceiverDefs::FAIL){
				delete receiver;
				cprintf(BLUE,"Exiting Child Process [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
				exit(EXIT_FAILURE);
			}

			/**	- as long as keeprunning is true (changes with Ctrl+C) */
			while(keeprunning)
				pause();
			/**	- interrupt caught, delete slsReceiverUsers object and exit */
			delete receiver;
			cprintf(BLUE,"Exiting Child Process [ Tid: %ld ]\n", (long)syscall(SYS_gettid));
			exit(EXIT_SUCCESS);
			break;
		}
	}

	/** - Parent process ignores SIGINT (exits only when all child process exits) */
	sa.sa_flags=0;							// no flags
	sa.sa_handler=SIG_IGN;					// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		cprintf(RED, "Could not set handler function for SIGINT\n");
	}


	/** - Print Ready and Instructions how to exit */
	cout << "Ready ... " << endl;
	cprintf(RESET, "\n[ Press \'Ctrl+c\' to exit ]\n");

	/** - Parent process waits for all child processes to exit */
	for(;;) {
		pid_t childPid = waitpid (-1, NULL, 0);

		// no child closed
		if (childPid == -1) {
			if (errno == ECHILD) {
				cprintf(GREEN,"All Child Processes have been closed\n");
				break;
			} else {
				cprintf(RED, "Unexpected error from waitpid(): (%s)\n",strerror(errno));
				break;
			}
		}

		//child closed
		cprintf(BLUE,"Exiting Child Process [ Tid: %ld ]\n", (long int) childPid);
	}

	cout << "Goodbye!" << endl;
	return 0;
}

