/* slsReceiver */

#include "logger.h"
#include "Receiver.h"
#include "sls_detector_defs.h"
#include "container_utils.h"

#include <csignal>	//SIGINT
#include <cstdlib>		//system
#include <cstring>
#include <iostream>
#include <sys/types.h>	//wait
#include <sys/wait.h>	//wait
#include <syscall.h>
#include <unistd.h> 	//usleep
#include <memory>
#include <semaphore.h>

sem_t semaphore;

void sigInterruptHandler(int p){
	sem_post(&semaphore);
}


int main(int argc, char *argv[]) {

	sem_init(&semaphore,1,0);

	FILE_LOG(logINFOBLUE) << "Created [ Tid: " << syscall(SYS_gettid) << " ]";

	// Catch signal SIGINT to close files and call destructors properly
	struct sigaction sa;
	sa.sa_flags=0;							// no flags
	sa.sa_handler=sigInterruptHandler;		// handler function
	sigemptyset(&sa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGINT, &sa, nullptr) == -1) {
		FILE_LOG(logERROR) << "Could not set handler function for SIGINT";
	}


	// if socket crash, ignores SISPIPE, prevents global signal handler
	// subsequent read/write to socket gives error - must handle locally
	struct sigaction asa;
	asa.sa_flags=0;							// no flags
	asa.sa_handler=SIG_IGN;					// handler function
	sigemptyset(&asa.sa_mask);				// dont block additional signals during invocation of handler
	if (sigaction(SIGPIPE, &asa, nullptr) == -1) {
		FILE_LOG(logERROR) << "Could not set handler function for SIGPIPE";
	}

	std::unique_ptr<Receiver> receiver = nullptr;
	try {
		receiver = sls::make_unique<Receiver>(argc, argv);
	} catch (...) {
		FILE_LOG(logINFOBLUE) << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
		throw;
	}

	FILE_LOG(logINFO) << "[ Press \'Ctrl+c\' to exit ]";
	sem_wait(&semaphore);
	sem_destroy(&semaphore);
	FILE_LOG(logINFOBLUE) << "Exiting [ Tid: " << syscall(SYS_gettid) << " ]";
	FILE_LOG(logINFO) << "Exiting Receiver";
	return 0;
}

