#include "slsDetectorCommand.h"
#include "multiSlsDetector.h"
#include "string_utils.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <iomanip>
// using namespace std;

/*! \mainpage Introduction

This program is intended to control the SLS detectors via command line interface.
This is the only way to access all possible functionality of the detectors, however it is often recommendable to avoid changing the most advanced settings, rather leaving the task to configuration files, as when using the GUI or the API provided.

The command line interface consists in four main functions:

- \b sls_detector_acquire to acquire data from the detector
- \b sls_detector_put to set detector parameters
- \b sls_detector_get to retrieve detector parameters
- \b sls_detector_help to get help concerning the text commands
Additionally the program slsReceiver should be started on the machine expected to receive the data from the detector.


If you need control a single detector, the use of the command line interface does not need any additional arguments.

For commands addressing a single controller of your detector, the command  cmd should be called with the index i of the controller:


<b>sls_detector_clnt i:cmd</b>


where \b sls_detector_clnt is the text client (put, get, acquire, help).

In case more than one detector is configured on the control PC, the command  cmd should be called with their respective index  j:


<b>sls_detector_clnt j-cmd</b>


where \b sls_detector_clnt is the text client (put, get, acquire, help).

To address a specific controller i of detector j use:

<b>sls_detector_clnt j-i:cmd</b>


To use different shared memory segements for different detectors on the same
client pc, one can use environment variable <b>SLSDETNAME</b> set to any string to
different strings to make the shared memory segments unique. One can then use
the same multi detector id for both detectors as they have a different shared memory names.

For additional questions concerning the indexing of the detector, please refer to the SLS Detectors FAQ documentation.

The commands are sudivided into different pages depending on their functionalities:
 - \ref acquisition "Acquisition": commands to start/stop the acquisition and retrieve data
 - \ref config "Configuration": commands to configure the detector
 - \ref timing "Timing": commands to configure the detector timing
 - \ref data "Data postprocessing": commands to process the data
 - \ref settings "Settings": commands to define detector settings/threshold.
 - \ref output "Output": commands to define output file destination and format
 - \ref network "Network": commands to setup the network between client, detector and receiver
 - \ref receiver "Receiver": commands to configure the receiver
 - \ref prototype "Prototype (Chip Test Board / Moench)": commands specific for the chiptest board or moench
 - \ref test "Developer": commands to be used only for software debugging. Avoid using them!
 
 */

slsDetectorCommand::slsDetectorCommand(multiSlsDetector *det) {

    myDet = det;

    int i = 0;

    cmd = std::string("none");

    /*! \page test Developer
    Commands to be used only for software debugging. Avoid using them!
    - \b test returns an error
	 */

    descrToFuncMap[i].m_pFuncName = "test"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdUnderDevelopment;
    ++i;

    /*! \page test
   - <b>help</b> Returns a list of possible commands.
	 */
    descrToFuncMap[i].m_pFuncName = "help"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdHelp;
    ++i;

    /*! \page test
   - <b>exitserver</b> Shuts down all the detector servers. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "exitserver"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /*! \page test
   - <b>exitreceiver</b> Shuts down all the receivers. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "exitreceiver"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /*! \page test
   - <b>execcommand</b> Executes a command on the detector server. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "execcommand"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /*! \page test
   - <b>rx_execcommand</b> Executes a command on the receiver server. Don't use it!!!!
	 */
    descrToFuncMap[i].m_pFuncName = "rx_execcommand"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdExitServer;
    ++i;

    /*! \page test
   - <b>flippeddatay [i]</b> enables/disables data being flipped across y axis. 1 enables, 0 disables. Not implemented.
	 */
    descrToFuncMap[i].m_pFuncName = "flippeddatay"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /* digital test and debugging */

    /*! \page test
   - <b>digibittest:[i]</b> performs digital test of the module i. Returns 0 if succeeded, otherwise error mask. Gotthard only. Only put!
	 */
    descrToFuncMap[i].m_pFuncName = "digibittest"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDigiTest;
    ++i;

    /*! \page test
   - <b>bustest</b> performs test of the bus interface between FPGA and embedded Linux system. Can last up to a few minutes. Cannot set! Jungfrau only. Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "bustest"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDigiTest;
    ++i;

    /*! \page test
   - <b>firmwaretest</b> performs the firmware test.  Cannot set! Jungfrau only. Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "firmwaretest"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDigiTest;
    ++i;

    /*! \page test
   - <b>reg [addr] [val]</b> ??? writes to an register \c addr with \c value in hexadecimal format.
	 */
    descrToFuncMap[i].m_pFuncName = "reg"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /*! \page test
   - <b>adcreg [addr] [val]</b> ??? writes to an adc register \c addr with \c value in hexadecimal format. Only put!
	 */
    descrToFuncMap[i].m_pFuncName = "adcreg"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /*! \page test
   - <b>setbit</b> ???  Only put!
	 */
    descrToFuncMap[i].m_pFuncName = "setbit"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /*! \page test
   - <b>clearbit </b> ??? Only put!
	 */
    descrToFuncMap[i].m_pFuncName = "clearbit"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /*! \page test
   - <b>getbit </b> ??? Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "getbit"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRegister;
    ++i;

    /* Acquisition and status commands */
    /*! \page acquisition Acquition commands
   Commands to control the acquisition
	 */
    /*! \page acquisition
   - \b acquire blocking acquisition (like calling sls_detector_acquire). Starts receiver and detector, writes and processes the data, stops detector. Only get!
     \c Returns (string)\c "acquire unsuccessful" if fails, else \c "Acquired (int)", where int is number of frames caught.
	 */
    descrToFuncMap[i].m_pFuncName = "acquire"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAcquire;
    ++i;

    /*! \page acquisition
   - <b>  busy i</b> sets/gets acquiring flag. \c 1 the acquisition is active, \c 0 otherwise. Acquire command will set this flag to 1 at the beginning and to 0 at the end. Use this to clear flag if acquisition terminated unexpectedly. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "busy"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdStatus;
    ++i;

    /*! \page acquisition
   - <b> status [s] </b> starts or stops acquisition in detector in non blocking mode. When using stop acquisition and if acquisition is done, it will restream the stop packet from receiver (if data streaming in receiver is on). Eiger can also provide an internal software trigger. \c s: [\c start, \c stop, \c trigger(EIGER only)]. \c Returns the detector status: [\c running, \c error, \c transmitting, \c finished, \c waiting, \c idle]. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "status"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdStatus;
    ++i;

    /*! \page acquisition
   - \b data gets all data from the detector (if any) processes them and writes them to file according to the preferences already setup (Eigerr store in ram only). Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "data"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdData;
    ++i;

    /*! \page acquisition
   - <b>readctr </b> Reads the counters from the detector memory (analog detector returning values translated into number of photons - only GOTTHARD). Cannot put.
	 */
    descrToFuncMap[i].m_pFuncName = "readctr"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdCounter;
    ++i;

    /*! \page acquisition
   - <b>resetctr i </b> Resets counter in detector, restarts acquisition if i=1(analog detector returning values translated into number of photons - only GOTTHARD). Cannot put.
	 */
    descrToFuncMap[i].m_pFuncName = "resetctr"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdCounter;
    ++i;

    /*! \page acquisition
   - <b>resmat i </b> sets/resets counter bit in detector.gets the counter bit in detector ????
	 */
    descrToFuncMap[i].m_pFuncName = "resmat"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdCounter;
    ++i;

    /*! \page config Configuration commands
    Commands to configure the detector. these commands are often left to the configuration file.
	 - \ref configstructure "Data Structure": commands to configure detector data structure
	 - \ref configstatus "Status": commands to configure detector status
	 - \ref configsize "Data Size": commands to configure detector data size
	 - \ref configflags "Flags": commands to configure detector flags
	 - \ref configchip "Chip": commands to configure chip of the detector
	 - \ref configversions "Versions": commands to check version of each subsytem
	 - \ref configspeed "Speed": commands to configure speed of detector
	 - \ref configsettings "Detector Parameters": commands to configure/retrieve configuration of detector
	 */
    /*! \page timing Timing commands
	  Commands to setup the timing
	 */
    /* Detector structure configuration and debugging commands */
    /*! \page config
		\section configstructure Data Structure
   commands to configure detector data structure
	 */

    /*! \page config
   - \b free Free shared memory on the control PC
	 */
    descrToFuncMap[i].m_pFuncName = "free"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdFree;
    ++i;

    /*! \page config
   - <b>hostname</b> \c put frees shared memory and sets the hostname (or IP adress). Only allowed at multi detector level. \c Returns the list of the hostnames of the multi-detector structure. \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "hostname"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdHostname;
    ++i;

    /*! \page config
   - \b add appends a hostname (or IP address) at the end of the multi-detector structure. Only allowed at multi detector level. Cannot get. \c Returns the current list of detector hostnames. \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "add"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdHostname;
    ++i;

    /*! \page config
   - <b>replace</b> \c Sets the hostname (or IP adress) for a single detector. Only allowed at single detector level.  Cannot get. \c Returns the hostnames for that detector \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "replace"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdHostname;
    ++i;

    /*! \page config
   - <b>user</b> \c Returns user details from shared memory. Only allowed at multi detector level.  Cannot put. \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "user"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdUser;
    ++i;

    /*! \page config
		\section configstatus Status
   commands to configure detector status
	 */

    /*! \page config
    - <b>online [i]</b> sets the detector in online (1) or offline (0) mode. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "online"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdOnline;
    ++i;

    /*! \page config
    - <b>checkonline</b> returns the hostnames of all detectors without connecting to them. \c Returns (string) "All online" or "[list of offline hostnames] : Not online".
	 */
    descrToFuncMap[i].m_pFuncName = "checkonline"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdOnline;
    ++i;
    /*! \page config
    - <b>activate [b] [p]</b> Activates/Deactivates the detector. \c b is 1 for activate, 0 for deactivate. Deactivated detector does not send data. \c p is optional and can be padding (default) or nonpadding for receivers for deactivated detectors. Used for EIGER only. \c Returns \c (int) (string)
	 */
    descrToFuncMap[i].m_pFuncName = "activate"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdOnline;
    ++i;

    /* detector and data size */
    /*! \page config
		\section configsize Data Size
   commands to configure detector data size
	 */

    /*! \page config
   - <b>dr [i]</b> sets/gets the dynamic range of detector. Eiger [4,8,16,32]. Others cannot put! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "dr"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /*! \page config
   - <b>roi [i] [xmin] [xmax] [ymin] [ymax]  </b> sets region of interest of the detector, where i is number of rois;i=0 to clear rois. Used for GOTTHARD only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "roi"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /*! \page config
   - <b>detsizechan [xmax] [ymax]</b> sets the maximum number of channels in each dimension for complete detector set; -1 is no limit. Use for multi-detector system as first command in config file. \c Returns \c ("int int")
	 */
    descrToFuncMap[i].m_pFuncName = "detsizechan"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /*! \page config
   - <b>flippeddatax [i]</b> enables/disables data being flipped across x axis. 1 enables, 0 disables. Used for EIGER only. 1 for bottom half-module, 0 for top-half module. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "flippeddatax"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /*! \page config
   - <b>tengiga [i]</b> enables/disables 10GbE in system (detector & receiver). 1 enabled 10GbE, 0 enables 1GbE. Used in EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "tengiga"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page config
   - <b>gappixels [i]</b> enables/disables gap pixels in system (detector & receiver). 1 sets, 0 unsets. Used in EIGER only and only in multi detector level command. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "gappixels"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDetectorSize;
    ++i;

    /* flags */
    /*! \page config
		\section configflags Flags
   commands to configure detector flags
	 */

    /*! \page config
   - <b>flags [flag]</b> sets/gets the readout flags to mode. Options: none, storeinram, tot, continous, parallel, nonparallel, safe, digital, analog_digital, overflow, nooverflow, unknown. Used for EIGER only. \c Returns \c (string). put takes one string and \c returns concatenation of all active flags separated by spaces.
	 */
    descrToFuncMap[i].m_pFuncName = "flags";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>extsig:[i] [flag]</b> sets/gets the mode of the external signal i. Options: \c off, \c gate_in_active_high, \c gate_in_active_low, \c trigger_in_rising_edge, \c trigger_in_falling_edge,
   \c ro_trigger_in_rising_edge, \c ro_trigger_in_falling_edge, \c gate_out_active_high, \c gate_out_active_low, \c trigger_out_rising_edge, \c trigger_out_falling_edge, \c ro_trigger_out_rising_edge,
   \c ro_trigger_out_falling_edge. \n Used in GOTTHARDonly. \c Returns \c (string)
	*/
    descrToFuncMap[i].m_pFuncName = "extsig"; /* find command! */
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /* fpga */

    /*! \page config
   - <b>programfpga [file]</b> programs the FPGA with file f (with .pof extension). Used for JUNGFRAU only. Only put! \c Returns \c ("successful", "unsuccessful")
	 */
    descrToFuncMap[i].m_pFuncName = "programfpga";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>resetfpga [f]</b> resets FPGA, where f can be any value. Used for JUNGFRAU only. Only put! \c Returns \c ("successful", "unsuccessful")
	 */
    descrToFuncMap[i].m_pFuncName = "resetfpga";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /* chip */
    /*! \page config
		\section configchip Chip
   commands to configure chip of the detector
	 */

    /*! \page config
   - <b>powerchip [i]</b> Powers on/off the chip. 1 powers on, 0 powers off. Can also get the power status. Used for JUNGFRAU only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "powerchip";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>led [i]</b> sets/gets the led status. 1 on, 0 off. Used for MOENCH only ?? \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "led";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>auto_comp_disable i </b> Currently not implemented. this mode disables the on-chip gain switching comparator automatically after 93.75% of exposure time (only for longer than 100us). 1 enables mode, 0 disables mode. By default, mode is disabled (comparator is enabled throughout). (JUNGFRAU only). \c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "auto_comp_disable"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdAdvanced;
    ++i;

    /*! \page config
   - <b>pulse [n] [x] [y]</b> pulses pixel at coordinates (x,y) n number of times. Used in EIGER only. Only put! \c Returns \c ("successful", "unsuccessful")
	 */
    descrToFuncMap[i].m_pFuncName = "pulse"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPulse;
    ++i;

    /*! \page config
   - <b>pulsenmove [n] [x] [y]</b> pulses pixel n number of times and moves relatively by x value (x axis) and y value(y axis). Used in EIGER only. Only put! \c Returns \c ("successful", "unsuccessful")
	 */
    descrToFuncMap[i].m_pFuncName = "pulsenmove"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPulse;
    ++i;

    /*! \page config
   - <b>pulsechip [n]</b>pulses chip n number of times, while n=-1 will reset it to normal mode. Used in EIGER only. Only put! \c Returns \c ("successful", "unsuccessful")
	 */
    descrToFuncMap[i].m_pFuncName = "pulsechip"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPulse;
    ++i;

    /* versions/ serial numbers  getId */
    /*! \page config
		\section configversions Versions
   Commands to check versions of each subsystem
	 */

    /*! \page config
   - <b>checkdetversion</b> Checks the version compatibility with detector server (if hostname is in shared memory). Only get! Only for Eiger, Jungfrau & Gotthard. \c Returns \c ("compatible", "incompatible")
	 */
    descrToFuncMap[i].m_pFuncName = "checkdetversion"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>checkrecversion</b> Checks the version compatibility with receiver server (if rx_hostname is in shared memory). Only get! Only for Eiger, Jungfrau & Gotthard. \c Returns \c ("compatible", "incompatible")
	 */
    descrToFuncMap[i].m_pFuncName = "checkrecversion"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>detectornumber</b> Gets the serial number or MAC of detector. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "detectornumber"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>detectorversion</b> Gets the firmware version of detector. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "detectorversion"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>softwareversion</b> Gets the software version of detector server. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "softwareversion"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>thisversion</b> Gets the software version of this client software. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "thisversion"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /*! \page config
   - <b>receiverversion</b> Gets the software version of receiver. Only get! \c Returns \c (long int) in hexadecimal
	 */
    descrToFuncMap[i].m_pFuncName = "receiverversion"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSN;
    ++i;

    /* r/w timers */

    /*! \page timing
   - <b>timing [mode]</b> sets/gets synchronization mode of the detector. Mode: auto, trigger, ro_trigger, gating, triggered_gating (string)
	*/
    descrToFuncMap[i].m_pFuncName = "timing"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTiming;
    ++i;

    /*! \page timing
   - <b>exptime [i]</b> sets/gets exposure time in s. \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "exptime"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>subexptime [i]</b> sets/gets sub exposure time in s. Used in EIGER only in 32 bit mode. \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "subexptime"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>period [i]</b> sets/gets frame period in s. \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "period"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>subdeadtime [i]</b> sets/gets sub frame dead time in s. Subperiod is set in the detector = subexptime + subdeadtime. This value is normally a constant in the config file. Used in EIGER only in 32 bit mode. \c Returns \c (double with 9 decimal digits)
     */
    descrToFuncMap[i].m_pFuncName = "subdeadtime"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>delay [i]</b> sets/gets delay in s. Used in GOTTHARD only. \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "delay"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>gates [i]</b> sets/gets number of gates. Used in GOTTHARD only. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "gates"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>frames [i]</b> sets/gets number of frames. If \c timing is not \c auto, then it is the number of frames per cycle/trigger. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "frames"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>cycles [i]</b> sets/gets number of triggers. Timing mode should be set appropriately. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "cycles"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>measurements [i]</b> sets/gets number of measurements. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "measurements"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>samples [i]</b> sets/gets number of samples expected from the jctb. Used in CHIP TEST BOARD only. \c Returns \c (long long int)
	 */
    descrToFuncMap[i].m_pFuncName = "samples"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>storagecells [i]</b> sets/gets number of additional storage cells per acquisition. For very advanced users only! For JUNGFRAU only. Range: 0-15. The #images = #frames * #cycles * (#storagecells +1). \c Returns \c (long long int)
     */
    descrToFuncMap[i].m_pFuncName = "storagecells"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /*! \page timing
   - <b>storagecell_start [i]</b> sets/gets the storage cell that stores the first acquisition of the series. Default is 15(0xf).. For very advanced users only! For JUNGFRAU only. Range: 0-15. \c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "storagecell_start"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimer;
    ++i;

    /* read only timers */

    /*! \page timing
   - <b>exptimel</b> gets exposure time left. Used in GOTTHARD only. Only get! \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "exptimel"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>periodl</b> gets frame period left. Used in GOTTHARD  and Jungfrau only. Only get! \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "periodl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>delayl</b> gets delay left. Used in GOTTHARD only. Only get! \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "delayl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>gatesl</b> gets number of gates left. Used in GOTTHARD only. Only get! \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "gatesl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page config
   - <b>framesl</b> gets number of frames left. Used in GOTTHARD and Jungfrau only. Only get! \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "framesl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>cyclesl</b> gets number of cylces left. Used in GOTTHARD  and Jungfrau only. Only get! \c Returns \c (double with 9 decimal digits)
	 */
    descrToFuncMap[i].m_pFuncName = "cyclesl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>now</b> Getting actual time of the detector from start. For Jungfrau only. Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "now"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>timestamp</b> Getting timestamp. For Jungfrau only. Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "timestamp"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>nframes</b> Frames from start run control. Only Jungfrau. Only get!
	 */
    descrToFuncMap[i].m_pFuncName = "nframes"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>measuredperiod</b> gets the measured frame period (time between last frame and the previous one) in s. For Eiger only. Makes sense only for acquisitions of more than 1 frame. \c Returns \c  (double with 9 decimal digits)
     */
    descrToFuncMap[i].m_pFuncName = "measuredperiod"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /*! \page timing
   - <b>measuredsubperiod</b> gets the measured subframe period (time between last subframe and the previous one) in s. For Eiger only and in 32 bit mode. \c Returns \c  (double with 9 decimal digits)
     */
    descrToFuncMap[i].m_pFuncName = "measuredsubperiod"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTimeLeft;
    ++i;

    /* speed */
    /*! \page config
		\section configspeed Speed
   commands to configure speed of detector
	 */

    /*! \page config
   - <b>clkdivider [i]</b> sets/gets the readout clock divider. EIGER, JUNGFRAU [0(fast speed), 1(half speed), 2(quarter speed)]. Jungfrau, full speed is not implemented and overwrites adcphase to recommended default.  \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "clkdivider"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>phasestep [i]</b> Only put for gotthard. Moves the phase of the ADC clock.\c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "phasestep"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>oversampling [i]</b> Sets/gets the number of adcsamples per clock. For the new chiptestboard.\c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "oversampling"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>adcclk [i]</b> sets/gets the ADC clock frequency in MHz. For the new chiptestboard!\c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "adcclk"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>adcphase [i]</b> Sets/gets phase of the sampling clock. For JUNGFRAU, setting speed (clkdivider) overwrites adcphase to its default recommended value. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "adcphase"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>adcpipeline [i]</b> Sets/gets the pipeline of the ADC. For the new chiptestbaord!\c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "adcpipeline"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>dbitclk [i]</b> Sets/gets the clock frequency of the latching of the digital bits in MHz. For the new chiptestboard!\c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "dbitclk"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>dbitphase [i]</b> Sets/gets the phase of the clock for  latching of the digital bits. For the new chiptestboard!?\c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "dbitphase"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /*! \page config
   - <b>dbitpipeline [i]</b> Sets/gets the pipeline of the latching of the digital bits. For the new chiptestbaord!\c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "dbitpipeline"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSpeed;
    ++i;

    /* settings dump/retrieve */
    /*! \page config
		\section configsettings Detector Parameters
   commands to configure/retrieve configuration of detector
	 */

    /*! \page config
   - <b>config [fname]</b> sets/saves detector/receiver to configuration contained in fname. Same as executing sls_detector_put for every line. Normally a one time operation. \c Returns \c (string) fname
	 */
    descrToFuncMap[i].m_pFuncName = "config";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;

    /* settings dump/retrieve */
    /*! \page config
   - <b>rx_printconfig</b> prints the receiver configuration. Only get! \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_printconfig";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;

    /*! \page config
   - <b>parameters [fname]</b> sets/saves detector parameters contained in fname. Normally once per different measurement. \c Returns \c (string) fname
	 */
    descrToFuncMap[i].m_pFuncName = "parameters";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;

    /*! \page config
   - <b>setup [fname]</b> sets/saves detector complete setup contained in fname (extensions automatically generated), including trimfiles, ff coefficients etc.  \c Returns \c (string) fname
	 */
    descrToFuncMap[i].m_pFuncName = "setup";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfiguration;
    ++i;

    /* data processing commands */

    /*! \page data Data processing commands
   Commands to setup the data processing
	 */

    /*! \page data
   - <b>ratecorr [ns]</b> Returns the dead time used for rate correections in ns (int). \c put sets the deadtime correction constant in ns, -1  will set it to default tau of settings (0 unset).  \c Returns \c (int). For Eiger only.
	 */
    descrToFuncMap[i].m_pFuncName = "ratecorr"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdRateCorr;
    ++i;

    // 	/*! \page data
    //    - <b>threaded [i]</b> Sets/gets the data processing threaded flag. 1 is threaded, 0 unthreaded.
    // 	 */
    // 	descrToFuncMap[i].m_pFuncName="threaded"; //
    // 	descrToFuncMap[i].m_pFuncPtr=&slsDetectorCommand::cmdThreaded;
    // 	++i;

    /*! \page data
   - <b>darkimage fn</b> Loads the dark image to the detector from file fn (pedestal image). Cannot get. For Gotthard only.
	 */
    descrToFuncMap[i].m_pFuncName = "darkimage"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdImage;
    ++i;

    /*! \page data
   - <b>gainimage fn</b> Loads the gain image to the detector from file fn (gain map for translation into number of photons of an analog detector). Cannot get. For Gotthard only.
	 */
    descrToFuncMap[i].m_pFuncName = "gainimage"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdImage;
    ++i;

    /*! \page settings Detector settings commands
   Commands to setup the settings of the detector
    - \ref settingsdir "Settings, trim & cal Directories": commands to setup settings/trim/cal directories
    - \ref settingssett "Settings and Threshold": commands to configure settings and threshold of detector
    - \ref settingsdacs "DACs": commands to configure DACs of detector
    - \ref settingsadcs "ADCs": commands to readout ADCs of detector
    - \ref settingstmp  "Temp Control": commands to monitor and handle temperature overshoot (only JUNGFRAU)
	 */

    /* trim/cal directories */
    /*! \page settings
		\section settingsdir Settings, trim & cal Directories
   commands to setup settings/trim/cal directories
	 */
    /*! \page settings
   - <b>settingsdir [dir]</b> Sets/gets the directory where the settings files are located. \c Returns \c (string) dir
	 */
    descrToFuncMap[i].m_pFuncName = "settingsdir"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettingsDir;
    ++i;
    /*! \page settings
   - <b>trimdir [dir]</b> obsolete \c settingsdir. \c Returns \c (string) dir
	 */
    descrToFuncMap[i].m_pFuncName = "trimdir"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettingsDir;
    ++i;

    /*! \page settings
   - <b>trimen [n e0 e1...e(n-1)]</b> Sets/gets the number of energies n at which the detector has default trim file and their values in eV (int). \c Returns \c (int int...) n e0 e1...e(n-1)
	 */
    descrToFuncMap[i].m_pFuncName = "trimen";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTrimEn;
    ++i;

    /* settings, threshold */
    /*! \page settings
		\section settingssett Settings and Threshold
   commands to configure settings and threshold of detector
	 */

    /*! \page settings
   - <b>settings [s]</b> sets/gets the settings of the detector. Options: \c standard, \c fast, \c highgain, \c dynamicgain, \c lowgain, \c mediumgain, \c veryhighgain,
   \c lownoise, \c dynamichg0, \c fixgain1, \c fixgain2, \c forceswitchg1, \c forceswitchg2.
   \n In Eiger, only sets in client shared memory. Use \c threshold or \c thresholdnotb to pass to detector. Gets from detector.  \c Returns \c (string) s
	 */
    descrToFuncMap[i].m_pFuncName = "settings"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

    /*! \page settings
   - <b>threshold [eV] [sett] </b> sets/gets the detector threshold in eV. sett is optional and if provided also sets the settings. Use this for Eiger instead of \c settings. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "threshold"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

    /*! \page settings
   - <b>thresholdnotb [eV] [sett] </b> sets/gets the detector threshold in eV without loading trimbits. sett is optional and if provided also sets the settings. Use this for Eiger instead of \c settings. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "thresholdnotb"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

    /*! \page settings
   - <b>trimbits [fname] </b> loads/stores the trimbits to/from the detector. If no extension is specified, the serial number of each module will be attached. \c Returns \c (string) fname
	 */
    descrToFuncMap[i].m_pFuncName = "trimbits"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

    /*! \page settings
   - <b>trimval [i]</b> sets all trimbits to i. Used in EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "trimval"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdSettings;
    ++i;

    /* pots */
    /*! \page settings
		\section settingsdacs DACs
   commands to configure DACs of detector
	 */

    /*! \page settings
   - <b>vthreshold [i] [mv]</b> Sets/gets detector threshold voltage for single photon counters. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vthreshold"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcalibration [i] [mv]</b> Sets/gets the voltage of the calibration pulses. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcalibration"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vtrimbit [i] [mv]</b> Sets/gets the voltage to set the width of the trimbits. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vtrimbit"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vpreamp [i] [mv]</b> Sets/gets the voltage to define the preamplifier feedback resistance. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vpreamp"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vhaper1 [i] [mv]</b> Sets/gets the voltage to define the  feedback resistance of the first shaper. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshaper1"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vshaper2 [i] [mv]</b> Sets/gets the voltage to define the  feedback resistance of the second shaper. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshaper2"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vhighvoltage [i]</b> Sets/gets the high voltage to the sensor in V. \c Returns \c (int ["mV"]).
	 */
    descrToFuncMap[i].m_pFuncName = "vhighvoltage"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vapower [i]</b> Sets/gets the analog power supply for the old chiptest board in DAC units. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vapower"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vddpower [i]</b> Sets/gets the digital power supply for the old chiptest board in DAC units. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vddpower"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vshpower [i]</b> Sets/gets the comparator power supply for the old chiptest board in DAC units. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vshpower"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>viopower [i]</b> Sets/gets the power supply of the FPGA I/Os for the old chiptest board in DAC units. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "viopower"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vrefds [i] [mv]</b> Sets/gets vrefds. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_ds"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcascn_pb [i] [mv]</b> Sets/gets vcascn_pb. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcascn_pb"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcasc_pb [i] [mv]</b> Sets/gets vcasc_pb. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcascp_pb"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vout_cm [i] [mv]</b> Sets/gets vout_cm. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vout_cm"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcasc_out [i] [mv]</b> Sets/gets vcasc_out. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcasc_out"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vin_com [i] [mv]</b> Sets/gets vin_com. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vin_cm"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vref_comp [i] [mv]</b> Sets/gets vref_comp. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vref_comp"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>ib_test_c [i] [mv]</b> Sets/gets ib_test_c. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "ib_test_c"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vsvp [i] [mv]</b> Sets/gets vsvp. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vsvp"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vsvn [i] [mv]</b> Sets/gets vsvn. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vsvn"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vtr [i] [mv]</b> Sets/gets vtr. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vtr"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vrf [i] [mv]</b> Sets/gets vrf. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vrf"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vrs [i] [mv]</b> Sets/gets vrs. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vrs"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vtgstv [i] [mv]</b> Sets/gets vtgstv. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vtgstv"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcmp_ll [i] [mv]</b> Sets/gets vcmp_ll. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcmp_ll"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcmp_lr [i] [mv]</b> Sets/gets vcmp_lr. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcmp_lr"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcal_l [i] [mv]</b> Sets/gets vcal_l. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcall"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcomp_rl [i] [mv]</b> Sets/gets vcomp_rl. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcmp_rl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcomp_rr [i] [mv]</b> Sets/gets vcomp_rr. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcmp_rr"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>rxb_rb [i] [mv]</b> Sets/gets rxb_rb. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "rxb_rb"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>rxb_lb [i] [mv]</b> Sets/gets rxb_lb. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "rxb_lb"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcp [i] [mv]</b> Sets/gets vcp. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcp"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vcn [i] [mv]</b> Sets/gets vcn. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vcn"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>vis [i] [mv]</b> Sets/gets vis. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "vis"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>iodelay [i] [mv]</b> Sets/gets iodelay. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "iodelay"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>dac:j [i] [mv]</b> Sets/gets value for DAC number j for the new chiptestboard. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "dac"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>adcvpp [i] </b> Sets/gets the Vpp of the ADC  0 -> 1V ; 1 -> 1.14V ; 2 -> 1.33V ; 3 -> 1.6V ; 4 -> 2V . \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "adcvpp"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_a [i] mv</b> Sets/gets value for Va on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_a"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_b [i] mv</b> Sets/gets value for Vb on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_b"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_c [i] mv</b> Sets/gets value for Vc on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_c"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_d [i] mv</b> Sets/gets value for Vd on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_d"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_io [i] mv</b> Sets/gets value for Vio on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_io"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_chip [i] mv</b> Sets/gets value for Vchip on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"]). Normally don't use it!
	 */
    descrToFuncMap[i].m_pFuncName = "v_chip"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>v_limit [i] mv</b> Sets/gets a soft limit for the power supplies and the DACs on the new chiptest board. Must be in mV. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "v_limit"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /* MYTHEN 3.01  
	all values are in DACu */

    descrToFuncMap[i].m_pFuncName = "vIpre"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "VcdSh"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>Vth1</b> Sets/gets first detector threshold voltage for Mythen 3.01. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "Vth1"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>Vth1</b> Sets/gets second detector threshold voltage for Mythen 3.01. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "Vth2"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /*! \page settings
   - <b>Vth1</b> Sets/gets third detector threshold voltage for Mythen 3.01. Normally in DAC units unless \c mv is specified at the end of the command line. \c Returns \c (int ["mV"])
	 */
    descrToFuncMap[i].m_pFuncName = "Vth3"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "VPL"; // baseline for analog pulsing
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "Vtrim"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vIbias"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vIinSh"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "cas"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "casSh"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vIbiasSh"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vIcin"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    descrToFuncMap[i].m_pFuncName = "vIpreOut"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDAC;
    ++i;

    /* r/w timers */
    /*! \page settings
		\section settingsadcs ADCs
   commands to readout ADCs of detector
	 */

    /*! \page settings
   - <b>temp_adc</b> Gets the ADC temperature. \c Returns \c EIGER,JUNGFRAU(double"°C") Others \c (int"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_adc"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_fpga</b> Gets the FPGA temperature. \c Returns \c EIGER,JUNGFRAU(double"°C") Others \c (int"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_fpga"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_fpgaext</b> Gets the external FPGA temperature. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_fpgaext"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_10ge</b> Gets the 10Gbe temperature. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_10ge"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_dcdc</b> Gets the temperature of the DC/DC converter. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_dcdc"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_sodl</b> Gets the temperature of the left so-dimm memory . Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_sodl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_sodr</b> Gets the temperature of the right so-dimm memory. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_sodr"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>adc:j</b> Gets the values of the slow ADC number j for the new chiptest board. \c Returns \c (int"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "adc"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_fpgal</b> Gets the temperature of the left frontend FPGA. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_fpgafl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>temp_fpgar</b> Gets the temperature of the right frontend FPGA. Used in EIGER only. \c Returns \c EIGER(double"°C")
	 */
    descrToFuncMap[i].m_pFuncName = "temp_fpgafr"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_a</b> Gets the current of the power supply a on the new chiptest board. \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_a"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_b</b> Gets the current of the power supply b on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_b"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_c</b> Gets the current of the power supply c on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_c"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_d</b> Gets the current of the power supply d on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_d"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>i_io</b> Gets the current of the power supply io on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "i_io"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_a</b> Gets the measured voltage of the power supply a on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_a"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_b</b> Gets the measured voltage of the power supply b on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_b"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_c</b> Gets the measured voltage of the power supply c on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_c"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_d</b> Gets the measured voltage of the power supply d on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_d"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /*! \page settings
   - <b>vm_io</b> Gets the measured voltage of the power supply io on the new chiptest board \c Returns \c (int"mV")
	 */
    descrToFuncMap[i].m_pFuncName = "vm_io"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdADC;
    ++i;

    /* temperature control */
    /*! \page settings
        \section settingstmp Temp Control
  commands to monitor and handle temperature overshoot (only JUNGFRAU)
     */

    /*! \page settings
   - <b>temp_threshold</b> Sets/gets the threshold temperature. JUNGFRAU ONLY. \c Returns \c (double"°C")
     */
    descrToFuncMap[i].m_pFuncName = "temp_threshold"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTempControl;
    ++i;

    /*! \page settings
   - <b>temp_control</b> Enables/Disables the temperature control. 1 enables, 0 disables.  JUNGFRAU ONLY. \c Returns \c int
     */
    descrToFuncMap[i].m_pFuncName = "temp_control"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTempControl;
    ++i;

    /*! \page settings
   - <b>temp_event</b> Resets/gets over-temperative event. Put only with option 0 to clear event. Gets 1 if temperature went over threshold and control is enabled, else 0. /Disables the temperature control.  JUNGFRAU ONLY. \c Returns \c int
     */
    descrToFuncMap[i].m_pFuncName = "temp_event"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdTempControl;
    ++i;

    /* file name */

    /*! \page output Output settings
   Commands to setup the file destination and format
	 */

    /*! \page output
   - <b>outdir [dir]</b> Sets/gets the file output directory. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "outdir"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdOutDir;
    ++i;

    /*! \page output
   - <b>fname [fn]</b> Sets/gets the root of the output file name \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "fname"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdFileName;
    ++i;

    /*! \page output
   - <b>index [i]</b> Sets/gets the current file index. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "index"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdFileIndex;
    ++i;

    /*! \page output
   - <b>enablefwrite [i]</b> Enables/disables file writing. 1 enables, 0 disables. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "enablefwrite"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdEnablefwrite;
    ++i;

    /*! \page output
    - <b>overwrite [i]</b> enables(1) /disables(0) file overwriting. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "overwrite"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdOverwrite;
    ++i;

    /*! \page output
    - <b>fileformat</b> sets/gets the file format for data in receiver. Options: [ascii, binary, hdf5]. Ascii is not implemented in Receiver. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "fileformat"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdFileName;
    ++i;

    /* communication configuration */

    /*! \page network Network
    Commands to setup the network between client, detector and receiver
    - <b>rx_hostname [s]</b> sets/gets the receiver hostname or IP address, configures detector mac with all network parameters and updates receiver with acquisition parameters. Normally used for single detectors (Can be multi-detector). \c none disables. If used, use as last network command in configuring detector MAC. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_hostname"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;
    /*! \page network
   - <b>rx_udpip [ip]</b> sets/gets the ip address of the receiver UDP interface where the data from the detector will be streamed to. Normally used for single detectors (Can be multi-detector). Used if different from eth0. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_udpip"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>rx_udpmac [mac]</b> sets/gets the mac address of the receiver UDP interface where the data from the detector will be streamed to. Normally used for single detectors (Can be multi-detector). \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_udpmac"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>rx_udpport [port]</b> sets/gets the port of the receiver UDP interface where the data from the detector will be streamed to. Use single-detector command. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_udpport"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>rx_udpport2 [port]</b> sets/gets the second port of the receiver UDP interface where the data from the second half of the detector will be streamed to. Use single-detector command. Used for EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_udpport2"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>rx_udpsocksize [size]</b> sets/gets the UDP socket buffer size. Already trying to set by default to 100mb, 2gb for Jungfrau. Does not remember in client shared memory, so must be initialized each time after setting receiver hostname in config file.\c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "rx_udpsocksize"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>rx_realudpsocksize [size]</b> gets the actual UDP socket buffer size. Usually double the set udp socket buffer size due to kernel bookkeeping. Get only. \c Returns \c (int)
     */
    descrToFuncMap[i].m_pFuncName = "rx_realudpsocksize"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>detectormac [mac]</b> sets/gets the mac address of the detector UDP interface from where the detector will stream data. Use single-detector command. Normally unused. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "detectormac"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>detectorip [ip]</b> sets/gets the ip address of the detector UDP interface from where the detector will stream data. Use single-detector command. Keep in same subnet as rx_udpip (if rx_udpip specified). \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "detectorip"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>txndelay_left [delay]</b> sets/gets the transmission delay of first packet in an image being streamed out from the detector's left UDP port. Use single-detector command. Used for EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "txndelay_left"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>txndelay_right [delay]</b> sets/gets the transmission delay of first packet in an image being streamed out from the detector's right UDP port. Use single-detector command. Used for EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "txndelay_right"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>txndelay_frame [delay]</b> sets/gets the transmission frame period of entire frame being streamed out from the detector for both ports. Use single-detector command. Used for EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "txndelay_frame"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>flowcontrol_10g [delay]</b> Enables/disables 10 GbE flow control. 1 enables, 0 disables. Used for EIGER only. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "flowcontrol_10g"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>zmqport [port]</b> sets/gets the 0MQ (TCP) port of the client to where final data is streamed to (eg. for GUI). The default already connects with rx_zmqport for the GUI. Use single-detector command to set individually or multi-detector command to calculate based on \c port for the rest. Must restart zmq client streaming in gui/external gui \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "zmqport"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b>rx_zmqport [port]</b> sets/gets the 0MQ (TCP) port of the receiver from where data is streamed from (eg. to GUI or another process for further processing). Use single-detector command to set individually or multi-detector command to calculate based on \c port for the rest.  put restarts streaming in receiver with new port. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_zmqport"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    ++i;

    /*! \page network
   - <b> rx_datastream </b>enables/disables data streaming from receiver. 1 enables 0MQ data stream from receiver (creates streamer threads), while 0 disables (destroys streamer threads). Switching to Gui enables data streaming in receiver and switching back to command line acquire will require disabling data streaming in receiver for fast applications \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_datastream"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdDataStream;
    ++i;

    /*! \page network
   - <b>zmqip [ip]</b> sets/gets the 0MQ (TCP) ip of the client to where final data is streamed to (eg. for GUI). For Experts only! Default is ip of rx_hostname and works for GUI. This command to change from default can be used from command line when sockets are not already open as the command line is not aware/create the 0mq sockets in the client side. This is usually used to stream in from an external process. . If no custom ip, empty until first time connect to receiver. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "zmqip"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    i++;

    /*! \page network
   - <b>rx_zmqip [ip]</b> sets/gets the 0MQ (TCP) ip of the receiver from where data is streamed from (eg. to GUI or another process for further processing). For Experts only! Default is ip of rx_hostname and works for GUI. This is usually used to stream out to an external process for further processing. . If no custom ip, empty until first time connect to receiver. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_zmqip"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdNetworkParameter;
    i++;

    /*! \page network
   - <b>configuremac [i]</b> configures the MAC of the detector with these parameters: detectorip, detectormac, rx_udpip, rx_udpmac, rx_udpport, rx_udpport2 (if applicable). This command is already included in \c rx_hsotname. Only put!. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "configuremac"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdConfigureMac;
    ++i;

    /*! \page network
   - <b>rx_tcpport [port]</b> sets/gets the port of the client-receiver TCP interface. Use single-detector command. Is different for each detector if same \c rx_hostname used. Must be first command to communicate with receiver. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_tcpport"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPort;
    ++i;

    /*! \page network
   - <b>port [port]</b> sets/gets the port of the client-detector control server TCP interface. Use single-detector command. Default value is 1952 for all detectors. Normally not changed. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "port"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPort;
    ++i;

    /*! \page network
   - <b>stopport [port]</b> sets/gets the port of the client-detector stop server TCP interface. Use single-detector command. Default value is 1953 for all detectors. Normally not changed. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "stopport"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPort;
    ++i;

    /*! \page network
   - <b>lock [i]</b> Locks/Unlocks the detector to communicate with this client. 1 locks, 0 unlocks. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "lock"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdLock;
    ++i;

    /*! \page network
   - <b>lastclient </b> Gets the last client communicating with the detector. Cannot put!. \c Returns \c (string)
	 */
    descrToFuncMap[i].m_pFuncName = "lastclient"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdLastClient;
    ++i;

    /* receiver functions */

    /*! \page receiver Receiver commands
   Commands to configure the receiver.
	 */

    /*! \page receiver
   - <b>receiver [s]</b> starts/stops the receiver to listen to detector packets. Options: [ \c start, \c stop]. \c Returns \c (string) status of receiver[ \c idle, \c running].
	 */
    descrToFuncMap[i].m_pFuncName = "receiver";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
   - <b>r_online [i]</b> sets/gets the receiver in online/offline mode. 1 is online, 0 is offline. Get is from shared memory. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "r_online";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdOnline;
    ++i;

    /*! \page receiver
   - <b>r_checkonline</b> Checks the receiver if it is online/offline mode. Only get! \c Returns (string) "All online" or "[list of offline hostnames] : Not online".
	 */
    descrToFuncMap[i].m_pFuncName = "r_checkonline";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdOnline;
    ++i;

    /*! \page receiver
   - <b>framescaught</b> gets the number of frames caught by receiver. Average of all for multi-detector command. Only get! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "framescaught";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
   - <b>resetframescaught [i]</b> resets the number of frames caught to 0. i can be any number. Use this if using status start, instead of acquire (this command is included). Only put! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "resetframescaught";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
   - <b>frameindex [i]</b> gets the current frame index of receiver. Average of all for multi-detector command. Only get! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "frameindex";
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
   - <b>r_lock [i]</b> locks/unlocks the receiver to communicate with only this client. 1 locks, 0 unlocks. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "r_lock"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdLock;
    ++i;

    /*! \page receiver
   - <b>r_lastclient</b> gets the last client communicating with the receiver. Only get! \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "r_lastclient"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdLastClient;
    ++i;

    /*! \page receiver
   - <b>r_readfreq [i]</b> sets/gets the stream frequency of data from receiver to client. i > 0 is the nth frame being streamed. 0 sets frequency to a default timer (200ms). \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "r_readfreq"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
   - <b>rx_fifodepth [i]</b> sets/gets receiver fifo (between Listener and Writer Threads) depth to i number of frames. Can improve listener packet loss (loss due to packet processing time in Listener threads), not if limited by writing. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "rx_fifodepth"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
   - <b>r_silent [i]</b> sets/gets receiver in silent mode, ie. it will not print anything during real time acquisition. 1 sets, 0 unsets. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "r_silent"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
    - <b>r_framesperfile [i]</b> sets/gets the frames per file in receiver to i. 0 means infinite or all frames in a single file. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "r_framesperfile"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
    - <b>r_discardpolicy</b> sets/gets the frame discard policy in the receiver. nodiscard (default) - discards nothing, discardempty - discard only empty frames, discardpartial(fastest) - discards all partial frames. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "r_discardpolicy"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
    - <b>r_padding</b> sets/gets the frame padding in the receiver. 0 does not pad partial frames(fastest), 1 (default) pads partial frames. \c Returns \c (int)
	 */
    descrToFuncMap[i].m_pFuncName = "r_padding"; //OK
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    ++i;

    /*! \page receiver
    - <b>rx_jsonaddheader [t]</b> sets/gets additional json header to be streamed out with the zmq from receiver. Default is empty. \c t must be in the format "\"label1\":\"value1\",\"label2\":\"value2\"" etc. Use only if it needs to be processed by an intermediate process. \c Returns \c (string)
     */
    descrToFuncMap[i].m_pFuncName = "rx_jsonaddheader"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    i++;

    /*! \page receiver
    - <b>rx_jsonpara [k] [v]</b> sets/gets  value v for additional json header parameter k to be streamed out with the zmq from receiver. If empty, then no parameter found Use only if it needs to be processed by an intermediate process. \c Returns \c (string)
     */
    descrToFuncMap[i].m_pFuncName = "rx_jsonpara"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdReceiver;
    i++;


    /* pattern generator */

    /*! \page prototype Protoype (Chip Test Board / Moench)
	  Commands specific for the chiptest board or moench
	 */


    /*! \page prototype
   - <b>adcinvert [mask]</b> Sets/gets ADC inversion mask (8 digits hex format)
	 */
    descrToFuncMap[i].m_pFuncName = "adcinvert"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>adcdisable [mask]</b> Sets/gets ADC disable mask (8 digits hex format)
	 */
    descrToFuncMap[i].m_pFuncName = "adcdisable"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>json_emin [i] </b> Sets/gets detector minimum energy threshold for the  Moench (soft setting). It is only set in the json header for the processor. \c Returns string
     */
    descrToFuncMap[i].m_pFuncName = "json_emin"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdProcessor;
    ++i;

    /*! \page prototype
   - <b>json_emax [i] </b> Sets/gets detector maximum energy threshold for the Moench (soft setting). It is only set in the json header for the processor. \c Returns string
     */
    descrToFuncMap[i].m_pFuncName = "json_emax"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdProcessor;
    ++i;

    /*! \page prototype
   - <b>json_framemode [i] </b> Sets/gets readoutmode for the Moench (soft setting). It is only set in the json header for the processor. Options: pedestal, newpedestal, flatfield, newflatfield, frame. \c Returns string
     */
    descrToFuncMap[i].m_pFuncName = "json_framemode"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdProcessor;
    ++i;

    /*! \page prototype
   - <b>json_detectormode [i] </b> Sets/gets detector mode for the Moench (soft setting). It is only set in the json header for the processor.Options: analog, counting, interpolating. \c Returns string
     */
    descrToFuncMap[i].m_pFuncName = "json_detectormode"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdProcessor;
    ++i;

    /*! \page prototype
   - <b>pattern fn</b> loads binary pattern file fn
	 */
    descrToFuncMap[i].m_pFuncName = "pattern"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patword addr [word]</b> sets/gets 64 bit word at address addr of pattern memory. Both address and word in hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patword"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patioctrl [word]</b> sets/gets 64 bit mask defining input (0) and output (1) signals. hex format.
	 */
    descrToFuncMap[i].m_pFuncName = "patioctrl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patclkctrl [word]</b> sets/gets 64 bit mask defining if output signal is a clock and runs. hex format. Unused at the moment.
	 */
    descrToFuncMap[i].m_pFuncName = "patclkctrl"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patlimits [addr1 addr2]</b> sets/gets the start and stop limits of the pattern to be executed. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patlimits"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patloop0 [addr1 addr2]</b> sets/gets the start and stop limits of the level 0 loop. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patloop0"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patnloop0 [n]</b> sets/gets the number of cyclesof the  level 0 loop (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patnloop0"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwait0 [addr]</b> sets/gets the address of the level 0 wait point. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patwait0"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwaittime0 [n]</b> sets/gets the duration of the witing of the 0 waiting point in clock cycles (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patwaittime0"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patloop1 [addr1 addr2]</b> sets/gets the start and stop limits of the level 1 loop. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patloop1"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patnloop1 [n]</b> sets/gets the number of cyclesof the  level 1 loop (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patnloop1"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwait1 [addr]</b> sets/gets the address of the level 1 wait point. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patwait1"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwaittime1 [n]</b> sets/gets the duration of the witing of the 1 waiting point in clock cycles (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patwaittime1"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patloop2 [addr1 addr2]</b> sets/gets the start and stop limits of the level 2 loop. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patloop2"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patnloop2 [n]</b> sets/gets the number of cyclesof the  level 2 loop (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patnloop2"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwait2 [addr]</b> sets/gets the address of the level 2 wait point. hex format. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "patwait2"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>patwaittime2 [n]</b> sets/gets the duration of the waiting of the 2 waiting point in clock cycles (int).
	 */
    descrToFuncMap[i].m_pFuncName = "patwaittime2"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    /*! \page prototype
   - <b>dut_clk [i]</b> sets/gets the signal to be used as a clock for the digital data coming from the device under test. Advanced!
	 */
    descrToFuncMap[i].m_pFuncName = "dut_clk"; //
    descrToFuncMap[i].m_pFuncPtr = &slsDetectorCommand::cmdPattern;
    ++i;

    numberOfCommands = i;

    // #ifdef VERBOSE
    //   std::cout << "Number of commands is " << numberOfCommands << std::endl;
    // #endif
}

//-----------------------------------------------------------

/*!
 */

//-----------------------------------------------------------

std::string slsDetectorCommand::executeLine(int narg, char *args[], int action, int detPos) {

    if (action == READOUT_ACTION)
        return cmdAcquire(narg, args, action, detPos);

    size_t s = std::string(args[0]).find(':');
    std::string key = std::string(args[0]).substr(0, s); // truncate at :

    if (action == PUT_ACTION && narg < 1)
        action = HELP_ACTION;

    for (int i = 0; i < numberOfCommands; ++i) {

        /* this works only if the command completely matches the key */
        /* otherwise one could try if truncated key is unique */

        if (key == descrToFuncMap[i].m_pFuncName) {
#ifdef VERBOSE
            std::cout << i << " command=" << descrToFuncMap[i].m_pFuncName << " key=" << key << std::endl;
#endif
            cmd = descrToFuncMap[i].m_pFuncName;

            MemFuncGetter memFunc = descrToFuncMap[i].m_pFuncPtr;
            std::string dResult = (this->*memFunc)(narg, args, action, detPos);

            return dResult;
        }
    }
    return cmdUnknown(narg, args, action, detPos);
}

/*! \page advanced Advanced Usage
This page is for advanced users.
Make sure you have first read \ref intro "the introduction".
 */

std::string slsDetectorCommand::cmdUnknown(int narg, char *args[], int action, int detPos) {
    return std::string("Unknown command ") + std::string(args[0]) + std::string("\n") + helpLine(0, args, action, detPos);
}
std::string slsDetectorCommand::cmdUnderDevelopment(int narg, char *args[], int action, int detPos) {
    return std::string("Must still develop ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
}

std::string slsDetectorCommand::helpLine(int narg, char *args[], int action, int detPos) {

    std::ostringstream os;

    if (action == READOUT_ACTION) {
        return helpAcquire(HELP_ACTION);
    }

    if (narg == 0) {
        os << "Command can be: " << std::endl;
        for (int i = 0; i < numberOfCommands; ++i) {
            os << descrToFuncMap[i].m_pFuncName << "\n";
        }
        os << std::endl;
        return os.str();
    }
    return executeLine(narg, args, HELP_ACTION, detPos);
}

std::string slsDetectorCommand::cmdAcquire(int narg, char *args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION) {
        return helpAcquire(HELP_ACTION);
    }
    if (!myDet->getNumberOfDetectors()) {
        cprintf(RED, "Error: This shared memory has no detectors added. Aborting.\n");
        return std::string("acquire unsuccessful");
    }
    if (detPos >= 0) {
        cprintf(RED, "Error: Individual detectors not allowed for readout. Aborting.\n");
        return std::string("acquire unsuccessful");
    }

    myDet->setOnline(ONLINE_FLAG, detPos);
    int r_online = myDet->setReceiverOnline(ONLINE_FLAG, detPos);



    if (myDet->acquire() == FAIL)
        return std::string("acquire unsuccessful");
    if (r_online) {
        char answer[100];
        sprintf(answer, "\nAcquired %d", myDet->getFramesCaughtByReceiver(detPos));
        return std::string(answer);
    }

    return std::string();
}

std::string slsDetectorCommand::helpAcquire(int action) {

    if (action == PUT_ACTION)
        return std::string("");
    std::ostringstream os;
    os << "Usage is " << std::endl
       << "sls_detector_acquire  id " << std::endl;
    os << "where id is the id of the detector " << std::endl;
    os << "the detector will be started, the data acquired, processed and written to file according to the preferences configured " << std::endl;
    return os.str();
}

std::string slsDetectorCommand::cmdData(int narg, char *args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    //int b;
    if (action == PUT_ACTION) {
        return std::string("cannot set");
    } else if (action == HELP_ACTION) {
        return helpData(HELP_ACTION);
    } else {
        // b=myDet->setThreadedProcessing(-1);
        // myDet->setThreadedProcessing(0);
        // myDet->setOnline(ONLINE_FLAG, detPos);
        // myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        // myDet->readAll(detPos);
        // //processdata in receiver is useful only for gui purposes
        // if(myDet->setReceiverOnline(detPos)==OFFLINE_FLAG)
        // 	myDet->processData();
        // myDet->setThreadedProcessing(b);
        return std::string("");
    }
}

std::string slsDetectorCommand::helpData(int action) {

    if (action == PUT_ACTION)
        return std::string("");
    else
        return std::string("data \t gets all data from the detector (if any) processes them and writes them to file according to the preferences already setup\n");
}

std::string slsDetectorCommand::cmdStatus(int narg, char *args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION)
        return helpStatus(action);

    if (cmd == "status") {
        myDet->setOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            //myDet->setThreadedProcessing(0);
            if (std::string(args[1]) == "start")
                myDet->startAcquisition(detPos);
            else if (std::string(args[1]) == "stop") {
                myDet->setReceiverOnline(ONLINE_FLAG, detPos); //restream stop
                myDet->stopAcquisition(detPos);
            } else if (std::string(args[1]) == "trigger") {
                myDet->sendSoftwareTrigger(detPos);
            } else
                return std::string("unknown action");
        }
        runStatus s = myDet->getRunStatus(detPos);
        return myDet->runStatusType(s);
    } else if (cmd == "busy") {
        if (action == PUT_ACTION) {
            int i;
            if (!sscanf(args[1], "%d", &i))
                return std::string("cannot parse busy mode");
            myDet->setAcquiringFlag(i);
        }
        char answer[100];
        sprintf(answer, "%d", myDet->getAcquiringFlag());
        return std::string(answer);
    } else
        return std::string("cannot scan command ") + std::string(cmd);
}

std::string slsDetectorCommand::helpStatus(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << std::string("status \t gets the detector status - can be: running, error, transmitting, finished, waiting or idle\n");
        os << std::string("busy \t gets the status of acquire- can be: 0 or 1. 0 for idle, 1 for running\n");
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << std::string("status \t controls the detector acquisition - can be start or stop or trigger(EIGER only).  When using stop acquisition and if acquisition is done, it will restream the stop packet from receiver (if data streaming in receiver is on). Eiger can also provide an internal software trigger\n");
        os << std::string("busy i\t sets the status of acquire- can be: 0(idle) or 1(running).Command Acquire sets it to 1 at beignning of acquire and back to 0 at the end. Clear Flag for unexpected acquire terminations. \n");
    }
    return os.str();
}

std::string slsDetectorCommand::cmdDataStream(int narg, char *args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    int ival = -1;
    char ans[100] = "";

    myDet->setOnline(ONLINE_FLAG, detPos);
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);

    if (action == HELP_ACTION)
        return helpDataStream(HELP_ACTION);

    if (action == PUT_ACTION) {
        if (!sscanf(args[1], "%d", &ival))
            return std::string("cannot scan rx_datastream mode");
        myDet->enableDataStreamingFromReceiver(ival, detPos);
    }

    sprintf(ans, "%d", myDet->enableDataStreamingFromReceiver(-1, detPos));
    return std::string(ans);
}

std::string slsDetectorCommand::helpDataStream(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("rx_datastream \t enables/disables data streaming from receiver. 1 is 0MQ data stream from receiver enabled, while 0 is 0MQ disabled. -1 for inconsistency between multiple receivers. \n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("rx_datastream i\t enables/disables data streaming from receiver. i is 1 enables 0MQ data stream from receiver (creates streamer threads), while 0 disables (destroys streamer threads). \n");
    return os.str();
}

std::string slsDetectorCommand::cmdFree(int narg, char *args[], int action, int detPos) {

#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    if (action == HELP_ACTION) {
        return helpFree(HELP_ACTION);
    }

    return ("Error: Should have been freed before creating constructor\n");
}

std::string slsDetectorCommand::helpFree(int action) {
    return std::string("free \t frees the shared memory\n");
}

std::string slsDetectorCommand::cmdHostname(int narg, char *args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION) {
        return helpHostname(HELP_ACTION);
    }
    if (action == GET_ACTION) {
        if ((cmd == "add") || (cmd == "replace"))
            return std::string("cannot get");
    }

    if (action == PUT_ACTION) {
        if (((cmd == "add") || (cmd == "hostname")) &&
            (detPos >= 0)) {
            return std::string("Wrong usage - setting hostname/add only from "
                               "multiDetector level");
        }
        if ((cmd == "replace") && (detPos < 0)) {
            return std::string("Wrong usage - replace only from "
                               "single detector level");
        }

        char hostname[1000];
        strcpy(hostname, "");
        // if each argument is a hostname
        for (int id = 1; id < narg; ++id) {
            strcat(hostname, args[id]);
            if (narg > 2)
                strcat(hostname, "+");
        }

        if (cmd == "add")
            myDet->addMultipleDetectors(hostname);
        else
            myDet->setHostname(hostname, detPos);
    }

    return myDet->getHostname(detPos);
}

std::string slsDetectorCommand::helpHostname(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << std::string("hostname \t returns the hostname(s) of the multi detector structure.\n");
        os << std::string("add \t cannot get\n");
        os << std::string("replace \t cannot get\n");
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << std::string("hostname name [name name]\t frees shared memory and "
                          "sets the hostname (or IP adress). Only allowed at multi detector level.\n");
        os << std::string("add det [det det]\t appends a hostname (or IP address) at "
                          "the end of the multi-detector structure. Only allowed at multi detector level."
                          "Returns hostnames in the multi detector structure\n");
        os << std::string("replace det \t Sets the hostname (or IP adress) for a "
                          "single detector. Only allowed at single detector level. "
                          "Returns the hostnames for that detector\n");
    }
    return os.str();
}

std::string slsDetectorCommand::cmdUser(int narg, char *args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION) {
        return helpHostname(HELP_ACTION);
    }
    if (action == PUT_ACTION) {
        return std::string("cannot put");
    }
    if (detPos >= 0) {
        return std::string("Wrong usage - getting user details only from "
                           "multiDetector level");
    }
    return myDet->getUserDetails();
}

std::string slsDetectorCommand::helpUser(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << std::string("user \t returns user details from shared memory without updating shared memory. "
                          "Only allowed at multi detector level.\n");
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << std::string("user \t cannot put\n");
    }
    return os.str();
}

std::string slsDetectorCommand::cmdHelp(int narg, char *args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    std::cout << narg << std::endl;

    if (narg >= 1)
        return helpLine(narg - 1, args, action, detPos);
    else
        return helpLine(0, args, action, detPos);
}

std::string slsDetectorCommand::cmdExitServer(int narg, char *args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    if (action == HELP_ACTION) {
        return helpExitServer(action);
    }

    if (action == PUT_ACTION) {
        if (cmd == "exitserver") {
            myDet->setOnline(ONLINE_FLAG, detPos);
            if (myDet->exitServer(detPos) == OK)
                return std::string("Server shut down.");
            else
                return std::string("Error closing server\n");
        } else if (cmd == "exitreceiver") {
            myDet->setReceiverOnline(ONLINE_FLAG, detPos);
            if (myDet->exitReceiver(detPos) == OK)
                return std::string("Receiver shut down\n");
            else
                return std::string("Error closing receiver\n");
        } else if (cmd == "execcommand") {
            myDet->setOnline(ONLINE_FLAG, detPos);
            if (myDet->execCommand(std::string(args[1]), detPos) == OK)
                return std::string("Command executed successfully\n");
            else
                return std::string("Command failed\n");
        } else if (cmd == "rx_execcommand") {
            myDet->setReceiverOnline(ONLINE_FLAG, detPos);
            if (myDet->execReceiverCommand(std::string(args[1]), detPos) == OK)
                return std::string("Command executed successfully\n");
            else
                return std::string("Command failed\n");
        } else
            return ("cannot decode command\n");
    } else
        return ("cannot get");
}

std::string slsDetectorCommand::helpExitServer(int action) {
    std::ostringstream os;
    os << std::string("exitserver \t shuts down all the detector servers. Don't use it!!!!\n");
    os << std::string("exitreceiver \t shuts down all the receiver servers.\n");
    os << std::string("execcommand \t executes command in detector server. Don't use it if you do not know what you are doing.\n");
    os << std::string("rx_execcommand \t executes command in receiver server. Don't use it if you do not know what you are doing.\n");
    return os.str();
}

std::string slsDetectorCommand::cmdSettingsDir(int narg, char *args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif
    if (action == HELP_ACTION) {
        return helpSettingsDir(action);
    }
    if (action == PUT_ACTION) {
        myDet->setSettingsDir(std::string(args[1]), detPos);
    }
    if (myDet->getSettingsDir(detPos) == "")
        return std::string("undefined");
    return myDet->getSettingsDir(detPos);
}

std::string slsDetectorCommand::helpSettingsDir(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("settingsdir \t  gets the directory where the settings files are located\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("settingsdir dir \t  sets the directory where the settings files are located\n");
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("trimdir \t  obsolete for settingsdir\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("trimdir dir \t  obsolete for settingsdir\n");
    return os.str();
}

std::string slsDetectorCommand::cmdTrimEn(int narg, char *args[], int action, int detPos) {
    int ival;
    int ip;

    char answer[1000];

    if (action == HELP_ACTION)
        return helpTrimEn(action);

    if (action == PUT_ACTION) {
        if (sscanf(args[1], "%d", &ival)) {
            int pos[ival];
            for (ip = 0; ip < ival; ++ip) {
                if ((2 + ip) < narg) {
                    if (sscanf(args[2 + ip], "%d", pos + ip)) {
                    } else
                        break;
                }
            }
            myDet->setTrimEn(ip, pos, detPos);
        }
    }
    int npos = myDet->getTrimEn(nullptr, detPos);
    if (npos != -1) {
        sprintf(answer, "%d", npos);
        int opos[npos];
        npos = myDet->getTrimEn(opos, detPos);
        if (npos != -1) {
            for (int ip = 0; ip < npos; ++ip) {
                sprintf(answer, "%s %d", answer, opos[ip]);
            }
        }
    }
    if (npos == -1)
        sprintf(answer, "%d", -1);
    return std::string(answer);
}

std::string slsDetectorCommand::helpTrimEn(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << "trimen ne [e0 e1...ene] \t sets the number of energies at which the detector has default trim files" << std::endl;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << "trimen  \t returns the number of energies at which the detector has default trim files and their values" << std::endl;
    return os.str();
}

std::string slsDetectorCommand::cmdOutDir(int narg, char *args[], int action, int detPos) {
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);
    if (action == HELP_ACTION)
        return helpOutDir(action);

    else if (action == PUT_ACTION)
        myDet->setFilePath(std::string(args[1]), detPos);

    return std::string(myDet->getFilePath(detPos));
}

std::string slsDetectorCommand::helpOutDir(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("outdir \t  gets the directory where the output files will be written\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("outdir dir \t  sets the directory where the output files will be written\n");
    return os.str();
}

std::string slsDetectorCommand::cmdFileName(int narg, char *args[], int action, int detPos) {
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);
    if (action == HELP_ACTION)
        return helpFileName(action);
    if (cmd == "fname") {
        if (action == PUT_ACTION)
            myDet->setFileName(std::string(args[1]), detPos);

        return std::string(myDet->getFileName(detPos));
    } else if (cmd == "fileformat") {
        if (action == PUT_ACTION) {
            if (std::string(args[1]) == "binary")
                myDet->setFileFormat(BINARY, detPos);
            else if (std::string(args[1]) == "ascii")
                myDet->setFileFormat(ASCII, detPos);
            else if (std::string(args[1]) == "hdf5")
                myDet->setFileFormat(HDF5, detPos);
            else
                return std::string("could not scan file format mode\n");
        }
        return myDet->fileFormats(myDet->getFileFormat(detPos));
    }
    return std::string("unknown command") + cmd;
}

std::string slsDetectorCommand::helpFileName(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << std::string("fname \t  gets the filename for the data without index and extension\n");
        os << std::string("fileformat \t  gets the file format for data\n");
    }
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << std::string("fname s \t  sets the filename for the data (index and extension will be automatically appended)\n");
        os << std::string("fileformat s \t  sets the file format for the data (binary, ascii, hdf5)\n");
    }
    return os.str();
}

std::string slsDetectorCommand::cmdEnablefwrite(int narg, char *args[], int action, int detPos) {

    int i;
    char ans[100];
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);
    if (action == HELP_ACTION) {
        return helpEnablefwrite(action);
    }
    if (action == PUT_ACTION) {
        if (sscanf(args[1], "%d", &i))
            myDet->enableWriteToFile(i, detPos);
        else
            return std::string("could not decode enable file write");
    }
    sprintf(ans, "%d", myDet->enableWriteToFile(-1, detPos));
    return std::string(ans);
}

std::string slsDetectorCommand::helpEnablefwrite(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("enablefwrite \t When Enabled writes the data into the file\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("enablefwrite i \t  should be 1 or 0 or -1\n");
    return os.str();
}

std::string slsDetectorCommand::cmdOverwrite(int narg, char *args[], int action, int detPos) {

    int i;
    char ans[100];
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);
    if (action == HELP_ACTION) {
        return helpOverwrite(action);
    }
    if (action == PUT_ACTION) {
        if (sscanf(args[1], "%d", &i))
            myDet->overwriteFile(i, detPos);
        else
            return std::string("could not decode overwrite");
    }
    sprintf(ans, "%d", myDet->overwriteFile(-1, detPos));
    return std::string(ans);
}

std::string slsDetectorCommand::helpOverwrite(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("overwrite \t When Enabled overwrites files\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("overwrite i \t  should be 1 or 0 or -1\n");
    return os.str();
}

std::string slsDetectorCommand::cmdFileIndex(int narg, char *args[], int action, int detPos) {
    char ans[100];
    int i;
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);
    if (action == HELP_ACTION) {
        return helpFileName(action);
    } else if (action == PUT_ACTION) {
        if (!sscanf(args[1], "%d", &i))
            return std::string("cannot parse file index");
        myDet->setFileIndex(i, detPos);
    }

    sprintf(ans, "%d", myDet->getFileIndex(detPos));
    return std::string(ans);
}

std::string slsDetectorCommand::helpFileIndex(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("index \t  gets the file index for the next the data file\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("index i \t  sets the fileindex for the next data file\n");
    return os.str();
}

std::string slsDetectorCommand::cmdRateCorr(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION) {
        return helpRateCorr(action);
    }
    int64_t ival;
    char answer[1000];

    myDet->setOnline(ONLINE_FLAG, detPos);

    if (action == PUT_ACTION) {
        sscanf(args[1], "%ld", &ival);
        myDet->setRateCorrection(ival, detPos);
    }
    sprintf(answer, "%ld", myDet->getRateCorrection(detPos));
    return std::string(answer);
}

std::string slsDetectorCommand::helpRateCorr(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("ratecorr \t  returns the dead time used for rate correections in ns \n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("ratecorr  ns \t  sets the deadtime correction constant in ns, -1 in Eiger will set it to default tau of settings\n");
    return os.str();
}

// std::string slsDetectorCommand::cmdThreaded(int narg, char *args[], int action, int detPos){
// 	int ival;
// 	char answer[1000];

// 	if (action==HELP_ACTION)
// 		return helpThreaded(action);

// 	if (action==PUT_ACTION) {
// 		if (sscanf(args[1],"%d",&ival))
// 			myDet->setThreadedProcessing(ival);
// 	}
// 	sprintf(answer,"%d",myDet->setThreadedProcessing());
// 	return std::string(answer);

// }

std::string slsDetectorCommand::helpThreaded(int action) {
    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("threaded \t  returns wether the data processing is threaded. \n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("threaded t \t  sets the threading flag ( 1sets, 0 unsets).\n");

    return os.str();
}

std::string slsDetectorCommand::cmdImage(int narg, char *args[], int action, int detPos) {
    std::string sval;
    int retval = FAIL;
    if (action == HELP_ACTION)
        return helpImage(HELP_ACTION);
    else if (action == GET_ACTION)
        return std::string("Cannot get");

    sval = std::string(args[1]);
    myDet->setOnline(ONLINE_FLAG, detPos);

    if (std::string(args[0]) == std::string("darkimage"))
        retval = myDet->loadImageToDetector(DARK_IMAGE, sval, detPos);
    else if (std::string(args[0]) == std::string("gainimage"))
        retval = myDet->loadImageToDetector(GAIN_IMAGE, sval, detPos);

    if (retval == OK)
        return std::string("Image loaded succesfully");
    else
        return std::string("Image load failed");
}

std::string slsDetectorCommand::helpImage(int action) {
    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "darkimage f \t  loads the image to detector from file f" << std::endl;
        os << "gainimage f \t  loads the image to detector from file f" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "darkimage \t  Cannot get" << std::endl;
        os << "gainimage \t  Cannot get" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdCounter(int narg, char *args[], int action, int detPos) {
    int ival;
    char answer[100];
    std::string sval;
    int retval = FAIL;
    if (action == HELP_ACTION)
        return helpCounter(HELP_ACTION);
    else if (action == PUT_ACTION)
        ival = atoi(args[1]);

    myDet->setOnline(ONLINE_FLAG, detPos);

    if (std::string(args[0]) == std::string("readctr")) {
        if (action == PUT_ACTION)
            return std::string("Cannot put");
        else {
            if (narg < 3)
                return std::string("should specify I/O file");
            sval = std::string(args[2]);
            retval = myDet->writeCounterBlockFile(sval, ival, detPos);
        }
    } else if (std::string(args[0]) == std::string("resetctr")) {
        if (action == GET_ACTION)
            return std::string("Cannot get");
        else
            retval = myDet->resetCounterBlock(ival, detPos);
    }

    else if (std::string(args[0]) == std::string("resmat")) {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &ival))
                return std::string("Could not scan resmat input ") + std::string(args[1]);
            if (ival >= 0)
                sprintf(answer, "%d", myDet->setCounterBit(ival, detPos));
        } else
            sprintf(answer, "%d", myDet->setCounterBit(-1, detPos));
        return std::string(answer);
    }

    if (retval == OK)
        return std::string("Counter read/reset succesfully");
    else
        return std::string("Counter read/reset failed");
}

std::string slsDetectorCommand::helpCounter(int action) {
    std::ostringstream os;
    os << std::endl;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "readctr \t  Cannot put" << std::endl;
        os << "resetctr i \t  resets counter in detector, restarts acquisition if i=1" << std::endl;
        os << "resmat i \t  sets/resets counter bit in detector" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "readctr i fname\t  reads counter in detector to file fname, restarts acquisition if i=1" << std::endl;
        os << "resetctr \t  Cannot get" << std::endl;
        os << "resmat i \t  gets the counter bit in detector" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdNetworkParameter(int narg, char *args[], int action, int detPos) {

	char ans[100] = {0};
    int i;
    if (action == HELP_ACTION)
        return helpNetworkParameter(action);

    myDet->setOnline(ONLINE_FLAG, detPos);
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);

    if (cmd == "detectormac") {
    	  if (action == PUT_ACTION) {
    		  myDet->setDetectorMAC(args[1], detPos);
    	  }
    	  return myDet->getDetectorMAC(detPos);
    } else if (cmd == "detectorip") {
  	  if (action == PUT_ACTION) {
  		myDet->setDetectorIP(args[1], detPos);
  	  }
  	  return myDet->getDetectorIP(detPos);
    } else if (cmd == "rx_hostname") {
    	  if (action == PUT_ACTION) {
    		  myDet->setReceiver(args[1], detPos);
    	  }
    	  return myDet->getReceiver(detPos);
    } else if (cmd == "rx_udpip") {
    	  if (action == PUT_ACTION) {
    		  myDet->setReceiverUDPIP(args[1], detPos);
    	  }
    	  return myDet->getReceiverUDPIP(detPos);
    } else if (cmd == "rx_udpmac") {
    	  if (action == PUT_ACTION) {
    		  myDet->setReceiverUDPMAC(args[1], detPos);
    	  }
    	  return myDet->getReceiverUDPMAC(detPos);
    } else if (cmd == "rx_udpport") {
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setReceiverUDPPort(i, detPos);
        }
        sprintf(ans, "%d", myDet->getReceiverUDPPort(detPos));
        return ans;
    } else if (cmd == "rx_udpport2") {
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setReceiverUDPPort2(i, detPos);
        }
        sprintf(ans, "%d", myDet->getReceiverUDPPort2(detPos));
        return ans;
    } else if (cmd == "rx_udpsocksize") {
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setReceiverUDPSocketBufferSize(i, detPos);
        }
        sprintf(ans, "%d", myDet->getReceiverUDPSocketBufferSize(detPos));
        return ans;
    } else if (cmd == "rx_realudpsocksize") {
        if (action == PUT_ACTION) {
            return ("cannot put!");
        }
        sprintf(ans, "%d", myDet->getReceiverRealUDPSocketBufferSize(detPos));
        return ans;
    } else if (cmd == "txndelay_left") {
    	networkParameter t = DETECTOR_TXN_DELAY_LEFT;
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setDetectorNetworkParameter(t, i, detPos);
        }
        sprintf(ans, "%d", myDet->setDetectorNetworkParameter(t, -1, detPos));
        return ans;
    } else if (cmd == "txndelay_right") {
    	networkParameter t = DETECTOR_TXN_DELAY_RIGHT;
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setDetectorNetworkParameter(t, i, detPos);
        }
        sprintf(ans, "%d", myDet->setDetectorNetworkParameter(t, -1, detPos));
        return ans;
    } else if (cmd == "txndelay_frame") {
    	networkParameter t = DETECTOR_TXN_DELAY_FRAME;
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setDetectorNetworkParameter(t, i, detPos);
        }
        sprintf(ans, "%d", myDet->setDetectorNetworkParameter(t, -1, detPos));
        return ans;
    } else if (cmd == "flowcontrol_10g") {
    	networkParameter t = FLOW_CONTROL_10G;
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setDetectorNetworkParameter(t, i, detPos);
        }
        sprintf(ans, "%d", myDet->setDetectorNetworkParameter(t, -1, detPos));
        return ans;
    } else if (cmd == "zmqport") {
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setClientDataStreamingInPort(i, detPos);
        }
        sprintf(ans, "%d", myDet->getClientStreamingPort(detPos));
        return ans;
    } else if (cmd == "rx_zmqport") {
        if (action == PUT_ACTION) {
            if (!(sscanf(args[1], "%d", &i))) {
                return ("cannot parse argument") + std::string(args[1]);
            }
            myDet->setReceiverDataStreamingOutPort(i, detPos);
        }
        sprintf(ans, "%d", myDet->getReceiverStreamingPort(detPos));
        return ans;
    } else if (cmd == "zmqip") {
    	  if (action == PUT_ACTION) {
    		  myDet->setClientDataStreamingInIP(args[1], detPos);
    	  }
    	  return myDet->getClientStreamingIP(detPos);
    } else if (cmd == "rx_zmqip") {
  	  if (action == PUT_ACTION) {
  		myDet->setReceiverDataStreamingOutIP(args[1], detPos);
  	  }
  	  return myDet->getReceiverStreamingIP(detPos);
    }

    return ("unknown network parameter") + cmd;
}

std::string slsDetectorCommand::helpNetworkParameter(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "detectormac mac \n sets detector mac to mac" << std::endl;
        os << "detectorip ip \n sets detector ip to ip" << std::endl;
        os << "rx_hostname name \n sets receiver ip/hostname to name" << std::endl;
        os << "rx_udpip ip \n sets receiver udp ip to ip" << std::endl;
        os << "rx_udpmac mac \n sets receiver udp mac to mac" << std::endl;
        os << "rx_udpport port \n sets receiver udp port to port" << std::endl;
        os << "rx_udpport2 port \n sets receiver udp port to port. For Eiger, it is the second half module and for other detectors, same as rx_udpport" << std::endl;
        os << "txndelay_left port \n sets detector transmission delay of the left port" << std::endl;
        os << "txndelay_right port \n sets detector transmission delay of the right port" << std::endl;
        os << "txndelay_frame port \n sets detector transmission delay of the entire frame" << std::endl;
        os << "flowcontrol_10g port \n sets flow control for 10g for eiger" << std::endl;
        os << "zmqport port \n sets the 0MQ (TCP) port of the client to where final data is streamed to (eg. for GUI). The default already connects with rx_zmqport for the GUI. "
              "Use single-detector command to set individually or multi-detector command to calculate based on port for the rest."
              "Must restart streaming in client with new port from gui/external gui"
           << std::endl;
        os << "rx_zmqport port \n sets the 0MQ (TCP) port of the receiver from where data is streamed from (eg. to GUI or another process for further processing). "
              "Use single-detector command to set individually or multi-detector command to calculate based on port for the rest."
              "Restarts streaming in receiver with new port"
           << std::endl;
        os << "zmqip ip \n sets the 0MQ (TCP) ip of the client to where final data is streamed to (eg. for GUI). Default is ip of rx_hostname and works for GUI. "
              "This is usually used to stream in from an external process."
              "Must restart streaming in client with new port from gui/external gui. "
           << std::endl;
        os << "rx_zmqip ip \n sets/gets the 0MQ (TCP) ip of the receiver from where data is streamed from (eg. to GUI or another process for further processing). "
              "Default is ip of rx_hostname and works for GUI. This is usually used to stream out to an external process for further processing."
              "restarts streaming in receiver with new port"
           << std::endl;
        os << "rx_udpsocksize [t]\n sets the UDP socket buffer size. Different defaults for Jungfrau. "
              "Does not remember in client shared memory, "
              "so must be initialized each time after setting receiver "
              "hostname in config file."
           << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "detectormac \n gets detector mac " << std::endl;
        os << "detectorip \n gets detector ip " << std::endl;
        os << "rx_hostname \n gets receiver ip " << std::endl;
        os << "rx_udpmac \n gets receiver udp mac " << std::endl;
        os << "rx_udpport \n gets receiver udp port " << std::endl;
        os << "rx_udpport2 \n gets receiver udp port. For Eiger, it is the second half module and for other detectors, same as rx_udpport" << std::endl;
        os << "txndelay_left \n gets detector transmission delay of the left port" << std::endl;
        os << "txndelay_right \n gets detector transmission delay of the right port" << std::endl;
        os << "txndelay_frame \n gets detector transmission delay of the entire frame" << std::endl;
        os << "flowcontrol_10g \n gets flow control for 10g for eiger" << std::endl;
        os << "zmqport \n gets the 0MQ (TCP) port of the client to where final data is streamed to" << std::endl;
        os << "rx_zmqport \n gets the 0MQ (TCP) port of the receiver from where data is streamed from" << std::endl;
        os << "zmqip \n gets the 0MQ (TCP) ip of the client to where final data is streamed to.If no custom ip, empty until first time connect to receiver" << std::endl;
        os << "rx_zmqip \n gets/gets the 0MQ (TCP) ip of the receiver from where data is streamed from. If no custom ip, empty until first time connect to receiver" << std::endl;
        os << "rx_udpsocksize \n gets the UDP socket buffer size." << std::endl;
        os << "rx_realudpsocksize \n gets the actual UDP socket buffer size. Usually double the set udp socket buffer size due to kernel bookkeeping." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdPort(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpPort(action);
    int val; //ret,
    char ans[MAX_STR_LENGTH];
    if (action == PUT_ACTION) {
        if (sscanf(args[1], "%d", &val))
            ;
        else
            return std::string("could not scan port number") + std::string(args[1]);
    }

    myDet->setOnline(ONLINE_FLAG, detPos);
    if (cmd == "port") {
        if (action == PUT_ACTION)
            myDet->setControlPort(val, detPos);
        sprintf(ans, "%d", myDet->setControlPort(-1, detPos));
    } else if (cmd == "rx_tcpport") {
        if (action == PUT_ACTION)
            myDet->setReceiverPort(val, detPos);
        sprintf(ans, "%d", myDet->setReceiverPort(-1, detPos));
    } else if (cmd == "stopport") {
        if (action == PUT_ACTION)
            myDet->setStopPort(val, detPos);
        sprintf(ans, "%d", myDet->setStopPort(-1, detPos));
    } else
        return std::string("unknown port type ") + cmd;

    return std::string(ans);
}

std::string slsDetectorCommand::helpPort(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "port i \n sets the communication control port" << std::endl;
        os << "rx_tcpport i \n sets the communication receiver port" << std::endl;
        os << "stopport i \n sets the communication stop port " << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "port  \n gets the communication control port" << std::endl;
        os << "rx_tcpport  \n gets the communication receiver port" << std::endl;
        os << "stopport \n gets the communication stop port " << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdLock(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpLock(action);

    int val; //, ret;
    char ans[1000];

    if (cmd == "lock") {
        myDet->setOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%d", &val))
                myDet->lockServer(val, detPos);
            else
                return std::string("could not lock status") + std::string(args[1]);
        }

        sprintf(ans, "%d", myDet->lockServer(-1, detPos));
    }

    else if (cmd == "r_lock") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%d", &val))
                myDet->lockReceiver(val, detPos);
            else
                return std::string("could not decode lock status") + std::string(args[1]);
        }
        sprintf(ans, "%d", myDet->lockReceiver(-1, detPos));
    }

    else
        return std::string("could not decode command");

    return std::string(ans);
}

std::string slsDetectorCommand::helpLock(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "lock i \n locks (1) or unlocks (0) the detector to communicate to this client" << std::endl;
        os << "r_lock i \n locks (1) or unlocks (0) the receiver to communicate to this client" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "lock \n returns the detector lock status" << std::endl;
        os << "r_lock \n returns the receiver lock status" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdLastClient(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpLastClient(action);

    if (action == PUT_ACTION)
        return std::string("cannot set");

    if (cmd == "lastclient") {
        myDet->setOnline(ONLINE_FLAG, detPos);
        return myDet->getLastClientIP(detPos);
    }

    else if (cmd == "r_lastclient") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        return myDet->getReceiverLastClientIP(detPos);
    }

    return std::string("cannot decode command");
}

std::string slsDetectorCommand::helpLastClient(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "lastclient \n returns the last client communicating with the detector" << std::endl;
        os << "r_lastclient \n returns the last client communicating with the receiver" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdOnline(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION) {
        return helpOnline(action);
    }
    int ival;
    char ans[1000];

    if (cmd == "online") {
        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%d", &ival))
                myDet->setOnline(ival, detPos);
            else
                return std::string("Could not scan online mode ") + std::string(args[1]);
        }
        sprintf(ans, "%d", myDet->setOnline(-1, detPos));
    } else if (cmd == "checkonline") {
        if (action == PUT_ACTION)
            return std::string("cannot set");
        strcpy(ans, myDet->checkOnline(detPos).c_str());
        if (!strlen(ans))
            strcpy(ans, "All online");
        else
            strcat(ans, " :Not online");
    } else if (cmd == "activate") {
        myDet->setOnline(ONLINE_FLAG, detPos);
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &ival))
                return std::string("Could not scan activate mode ") + std::string(args[1]);
            myDet->activate(ival, detPos);
            bool padding = true;
            if (narg > 2) {
                if (std::string(args[2]) == "padding")
                    padding = true;
                else if (std::string(args[2]) == "nopadding")
                    padding = false;
                else
                    return std::string("Could not scan activate mode's padding option " + std::string(args[2]));
                myDet->setDeactivatedRxrPaddingMode(padding, detPos);
            }
        }
        int ret = myDet->setDeactivatedRxrPaddingMode(-1, detPos);
        sprintf(ans, "%d %s", myDet->activate(-1, detPos), ret == 1 ? "padding" : (ret == 0 ? "nopadding" : "unknown"));
    } else if (cmd == "r_online") {
        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%d", &ival))
                myDet->setReceiverOnline(ival, detPos);
            else
                return std::string("Could not scan online mode ") + std::string(args[1]);
        }
        sprintf(ans, "%d", myDet->setReceiverOnline(-1, detPos));
    } else {
        if (action == PUT_ACTION)
            return std::string("cannot set");
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        strcpy(ans, myDet->checkReceiverOnline(detPos).c_str());
        if (!strlen(ans))
            strcpy(ans, "All receiver online");
        else
            strcat(ans, " :Not all receiver online");
    }

    return ans;
}

std::string slsDetectorCommand::helpOnline(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "online i \n sets the detector in online (1) or offline (0) mode" << std::endl;
        os << "r_online i \n sets the receiver in online (1) or offline (0) mode" << std::endl;
        os << "activate i [p]\n sets the detector in  activated (1) or deactivated (0) mode (does not send data).  p is optional and can be padding (default) or nonpadding for receivers for deactivated detectors. Only for Eiger." << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "online \n gets the detector online (1) or offline (0) mode" << std::endl;
        os << "checkonline \n returns the hostnames of all detectors in offline mode" << std::endl;
        os << "r_online \n gets the receiver online (1) or offline (0) mode" << std::endl;
        os << "r_checkonline \n returns the hostnames of all receiver in offline mode" << std::endl;
        os << "activate \n gets the detector activated (1) or deactivated (0) mode. And padding or nonpadding for the deactivated receiver. Only for Eiger." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdConfigureMac(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION) {
        return helpConfigureMac(action);
    }
    int ret;
    char ans[1000];

    if (action == PUT_ACTION) {
        myDet->setOnline(ONLINE_FLAG, detPos);
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        ret = myDet->configureMAC(detPos);
    } else
        return std::string("Cannot get ") + cmd;

    sprintf(ans, "%d", ret);
    return ans;
}

std::string slsDetectorCommand::helpConfigureMac(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << "configuremac i \n configures the MAC of the detector." << std::endl;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << "configuremac "
           << "Cannot get " << std::endl;

    return os.str();
}

std::string slsDetectorCommand::cmdDetectorSize(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpDetectorSize(action);
    int ret, val = -1, pos = -1, i;
    char ans[1000];

    myDet->setOnline(ONLINE_FLAG, detPos);

    if (cmd == "roi")
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);

    if (action == PUT_ACTION) {
        if (!sscanf(args[1], "%d", &val))
            return std::string("could not scan ") + std::string(args[0]) + std::string(" ") + std::string(args[1]);

        if (cmd == "roi") {
            //debug number of arguments
            if ((val < 0) || (narg != ((val * 4) + 2)))
                return helpDetectorSize(action);
            ROI allroi[val];
            pos = 2;
            for (i = 0; i < val; ++i) {
                if ((!sscanf(args[pos++], "%d", &allroi[i].xmin)) ||
                    (!sscanf(args[pos++], "%d", &allroi[i].xmax)) ||
                    (!sscanf(args[pos++], "%d", &allroi[i].ymin)) ||
                    (!sscanf(args[pos++], "%d", &allroi[i].ymax)))
                    return std::string("cannot parse arguments for roi");
            }
            myDet->setROI(val, allroi, detPos);
        }

        if (cmd == "detsizechan") {
            if ((sscanf(args[1], "%d", &val)) && (val > 0))
                myDet->setMaxNumberOfChannelsPerDetector(X, val);
            if ((narg > 2) && (sscanf(args[2], "%d", &val)) && (val > 0))
                myDet->setMaxNumberOfChannelsPerDetector(Y, val);
        }

        if (cmd == "flippeddatax") {
            if ((!sscanf(args[1], "%d", &val)) || (val != 0 && val != 1))
                return std::string("cannot scan flippeddata x mode: must be 0 or 1");
            myDet->setReceiverOnline(ONLINE_FLAG, detPos);
            myDet->setFlippedData(X, val, detPos);
        }

        if (cmd == "flippeddatay") {
            return std::string("Not required for this detector\n");
            if ((!sscanf(args[1], "%d", &val)) || (val != 0 && val != 1))
                return std::string("cannot scan flippeddata y mode: must be 0 or 1");
            myDet->setReceiverOnline(ONLINE_FLAG, detPos);
            myDet->setFlippedData(Y, val, detPos);
        }

        if (cmd == "gappixels") {
            if ((!sscanf(args[1], "%d", &val)) || (val != 0 && val != 1))
                return std::string("cannot scan gappixels mode: must be 0 or 1");
            myDet->setReceiverOnline(ONLINE_FLAG, detPos);
            if (detPos < 0) // only in multi detector level to update offsets etc.
                myDet->enableGapPixels(val, detPos);
        }
    }

    if (cmd == "dr") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        ret = myDet->setDynamicRange(val, detPos);
    } else if (cmd == "roi") {
        ROI* r = myDet->getROI(ret, detPos);
        if (r != NULL)
            delete [] r;
    } else if (cmd == "detsizechan") {
        sprintf(ans, "%d %d", myDet->getMaxNumberOfChannelsPerDetector(X), myDet->getMaxNumberOfChannelsPerDetector(Y));
        return std::string(ans);
    } else if (cmd == "flippeddatax") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        ret = myDet->getFlippedData(X, detPos);
    } else if (cmd == "flippeddatay") {
        return std::string("Not required for this detector\n");
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        ret = myDet->getFlippedData(Y, detPos);
    } else if (cmd == "gappixels") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        if (detPos >= 0) // only in multi detector level to update offsets etc.
            return std::string("Cannot execute this command from slsDetector level. Please use multiSlsDetector level.\n");
        ret = myDet->enableGapPixels(-1, detPos);
    }

    else
        return std::string("unknown command ") + cmd;

    sprintf(ans, "%d", ret);

    return std::string(ans);
}

std::string slsDetectorCommand::helpDetectorSize(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "dr i \n sets the dynamic range of the detector" << std::endl;
        os << "roi i xmin xmax ymin ymax \n sets region of interest where i is number of rois;i=0 to clear rois" << std::endl;
        os << "detsizechan x y \n sets the maximum number of channels for complete detector set in both directions; -1 is no limit" << std::endl;
        os << "flippeddatax x \n sets if the data should be flipped on the x axis" << std::endl;
        os << "flippeddatay y \n sets if the data should be flipped on the y axis" << std::endl;
        os << "gappixels i \n enables/disables gap pixels in system (detector & receiver). 1 sets, 0 unsets. Used in EIGER only and multidetector level." << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "dr \n gets the dynamic range of the detector" << std::endl;
        os << "roi \n gets region of interest" << std::endl;
        os << "detsizechan \n gets the maximum number of channels for complete detector set in both directions; -1 is no limit" << std::endl;
        os << "flippeddatax\n gets if the data will be flipped on the x axis" << std::endl;
        os << "flippeddatay\n gets if the data will be flipped on the y axis" << std::endl;
        os << "gappixels\n gets if gap pixels is enabled in system. Used in EIGER only and multidetector level." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdSettings(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpSettings(action);
    int val = -1; //ret,
    char ans[1000];

    //  portType index;
    //     if (sscanf(args[1],"%d",&val))
    //       ;
    //     else
    //       return std::string("could not scan port number")+std::string (args[1]);
    //   }

    myDet->setOnline(ONLINE_FLAG, detPos);

    if (cmd == "settings") {
        detectorSettings sett = GET_SETTINGS;
        if (action == PUT_ACTION) {
            sett = myDet->getDetectorSettings(std::string(args[1]));
            if (sett == -1)
                return std::string("unknown settings scanned " + std::string(args[1]));
            sett = myDet->setSettings(sett, detPos);
            if (myDet->getDetectorTypeAsEnum(detPos) == EIGER) {
                return myDet->getDetectorSettings(sett);
            }
        }
        return myDet->getDetectorSettings(myDet->getSettings(detPos));
    } else if (cmd == "threshold") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &val)) {
                return std::string("invalid threshold value");
            }
            detectorType type = myDet->getDetectorTypeAsEnum(detPos);
            if (type != EIGER || (type == EIGER && narg <= 2)) {
                myDet->setThresholdEnergy(val, GET_SETTINGS, 1, detPos);
            } else {
                detectorSettings sett = myDet->getDetectorSettings(std::string(args[2]));
                if (sett == -1)
                    return std::string("invalid settings value");
                myDet->setThresholdEnergy(val, sett, 1, detPos);
            }
        }
        sprintf(ans, "%d", myDet->getThresholdEnergy(detPos));
        return std::string(ans);
    } else if (cmd == "thresholdnotb") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &val)) {
                return std::string("invalid threshold value");
            }
            detectorType type = myDet->getDetectorTypeAsEnum(detPos);
            if (type != EIGER)
                return std::string("not implemented for this detector");
            if (narg <= 2) {
                myDet->setThresholdEnergy(val, GET_SETTINGS, 0, detPos);
            } else {
                detectorSettings sett = myDet->getDetectorSettings(std::string(args[2]));
                if (sett == -1)
                    return std::string("invalid settings value");
                myDet->setThresholdEnergy(val, sett, 0, detPos);
            }
        }
        sprintf(ans, "%d", myDet->getThresholdEnergy(detPos));
        return std::string(ans);
    } else if (cmd == "trimbits") {
        if (narg >= 2) {
            std::string sval = std::string(args[1]);
#ifdef VERBOSE
            std::cout << " trimfile " << sval << std::endl;
#endif
            int ret = OK;
            if (action == GET_ACTION) {
                //create file names
                ret = myDet->saveSettingsFile(sval, detPos);
            } else if (action == PUT_ACTION) {
                ret = myDet->loadSettingsFile(sval, detPos);
            }
            if (ret == OK)
                return sval;
            else
                return std::string("not successful");
        }
        return myDet->getSettingsFile(detPos);
    } else if (cmd == "trimval") {
        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%d", &val))
                myDet->setAllTrimbits(val, detPos);
            else
                return std::string("invalid trimbit value ") + cmd;
        }
        sprintf(ans, "%d", myDet->setAllTrimbits(-1, detPos));
        return ans;
    }
    return std::string("unknown settings command ") + cmd;
}

std::string slsDetectorCommand::helpSettings(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "settings s \n sets the settings of the detector - can be standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain"
              "lownoise, dynamichg0,fixgain1,fixgain2,forceswitchg1, forceswitchg2"
           << std::endl;
        os << "threshold eV [sett]\n sets the detector threshold in eV. If sett is provided for eiger, uses settings sett" << std::endl;
        os << "thresholdnotb eV [sett]\n sets the detector threshold in eV without loading trimbits. If sett is provided for eiger, uses settings sett" << std::endl;
        os << "trimbits fname\n loads the trimfile fname to the detector. If no extension is specified, the serial number of each module will be attached." << std::endl;
        os << "trimval i \n sets all the trimbits to i" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "settings \n gets the settings of the detector" << std::endl;
        os << "threshold V\n gets the detector threshold" << std::endl;
        os << "thresholdnotb V\n gets the detector threshold" << std::endl;
        os << "trimbits [fname]\n returns the trimfile loaded on the detector. If fname is specified the trimbits are saved to file. If no extension is specified, the serial number of each module will be attached." << std::endl;
        os << "trimval \n returns the value all trimbits are set to. If they are different, returns -1." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdSN(int narg, char *args[], int action, int detPos) {

    char answer[1000];

    if (action == PUT_ACTION)
        return std::string("cannot set");

    if (action == HELP_ACTION)
        return helpSN(action);

    if (cmd == "thisversion") {
        int64_t retval = myDet->getId(THIS_SOFTWARE_VERSION, detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }

    myDet->setOnline(ONLINE_FLAG, detPos);

    if (cmd == "detectornumber") {
        int64_t retval = myDet->getId(DETECTOR_SERIAL_NUMBER, detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }

    if (cmd == "detectorversion") {
        int64_t retval = myDet->getId(DETECTOR_FIRMWARE_VERSION, detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }

    if (cmd == "softwareversion") {
        int64_t retval = myDet->getId(DETECTOR_SOFTWARE_VERSION, detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }

    if (cmd == "receiverversion") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        int64_t retval = myDet->getId(RECEIVER_VERSION, detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        else
            sprintf(answer, "0x%lx", retval);
        return std::string(answer);
    }

    if (cmd == "checkdetversion") {
        int retval = myDet->checkDetectorVersionCompatibility(detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        sprintf(answer, "%s", retval == OK ? "compatible" : "incompatible");
        return std::string(answer);
    }

    if (cmd == "checkrecversion") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        int retval = myDet->checkReceiverVersionCompatibility(detPos);
        if (retval < 0)
            sprintf(answer, "%d", -1);
        sprintf(answer, "%s", retval == OK ? "compatible" : "incompatible");
        return std::string(answer);
    }

    return std::string("unknown id mode ") + cmd;
}

std::string slsDetectorCommand::helpSN(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "checkdetversion \n gets the version compatibility with detector server (if hostname is in shared memory). Only for Eiger, Jungfrau & Gotthard. Prints compatible/ incompatible." << std::endl;
        os << "checkrecversion \n gets the version compatibility with receiver server (if rx_hostname is in shared memory). Only for Eiger, Jungfrau & Gotthard. Prints compatible/ incompatible." << std::endl;
        os << "detectornumber \n gets the serial number of the detector (MAC)" << std::endl;
        os << "detectorversion \n gets the firmware version of the detector" << std::endl;
        os << "softwareversion \n gets the software version of the detector" << std::endl;
        os << "thisversion \n gets the version of this software" << std::endl;
        os << "receiverversion \n gets the version of the receiver" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdDigiTest(int narg, char *args[], int action, int detPos) {

    char answer[1000];

    if (action == HELP_ACTION)
        return helpSN(action);

    myDet->setOnline(ONLINE_FLAG, detPos);

    if (cmd == "bustest") {
        if (action == PUT_ACTION)
            return std::string("cannot set ") + cmd;
        sprintf(answer, "%d", myDet->digitalTest(DETECTOR_BUS_TEST));
        return std::string(answer);
    }

    else if (cmd == "firmwaretest") {
        if (action == PUT_ACTION)
            return std::string("cannot set ") + cmd;
        sprintf(answer, "%d", myDet->digitalTest(DETECTOR_FIRMWARE_TEST));
        return std::string(answer);
    }

    else if (cmd == "digibittest") {
        if (action == GET_ACTION)
            return std::string("cannot get ") + cmd;
        int ival = -1;
        if (sscanf(args[1], "%d", &ival)) {
            if ((ival == 0) || (ival == 1)) {
                sprintf(answer, "%d", myDet->digitalTest(DIGITAL_BIT_TEST, ival, detPos));
                return std::string(answer);
            } else
                return std::string("Use only 0 or 1 to set/clear digital test bit\n");
        } else
            return std::string("undefined number");
    }

    return std::string("unknown test mode ") + cmd;
}

std::string slsDetectorCommand::helpDigiTest(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "digibittest:i \t performs digital test of the module i. Returns 0 if succeeded, otherwise error mask.Gotthard only." << std::endl;
        os << "bustest \t performs test of the bus interface between FPGA and embedded Linux system. Can last up to a few minutes. Jungfrau only." << std::endl;
        os << "firmwaretest \t performs the firmware test. Jungfrau only." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdRegister(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpRegister(action);

    int addr, val, n;
    char answer[1000];

    myDet->setOnline(ONLINE_FLAG, detPos);

    // "reg" //

    // "setbit" //

    // "clearbit" //

    // "getbit" //

    if (action == PUT_ACTION) {
        if (cmd == "getbit")
            return std::string("Cannot put");

        if (narg < 3) {
            if (cmd == "reg")
                return std::string("wrong usage: should specify both address and value (hexadecimal fomat) ");
            else
                return std::string("wrong usage: should specify both address (hexadecimal fomat) and bit number");
        }

        if (sscanf(args[1], "%x", &addr))
            ;
        else
            return std::string("Could not scan address  (hexadecimal fomat) ") + std::string(args[1]);

        if (cmd == "reg") {
            if (sscanf(args[2], "%x", &val))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[2]);
            sprintf(answer, "0x%x", myDet->writeRegister(addr, val, detPos));
        } else if (cmd == "adcreg") {
            if (sscanf(args[2], "%x", &val))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[2]);
            sprintf(answer, "%s", (myDet->writeAdcRegister(addr, val, detPos) == OK) ? "successful" : "failed");
        } else {

            if (sscanf(args[2], "%d", &n))
                ;
            else
                return std::string("Could not scan bit number ") + std::string(args[2]);

            if (n < 0 || n > 31)
                return std::string("Bit number out of range") + std::string(args[2]);

            if (cmd == "setbit")
                sprintf(answer, "0x%x", myDet->setBit(addr, n, detPos));
            if (cmd == "clearbit")
                sprintf(answer, "0x%x", myDet->clearBit(addr, n, detPos));
        }

    } else {
        if (cmd == "setbit")
            return std::string("Cannot get");
        if (cmd == "clearbit")
            return std::string("Cannot get");
        if (cmd == "adcreg")
            return std::string("Cannot get");

        if (cmd == "reg") {
            if (narg < 2)
                return std::string("wrong usage: should specify address  (hexadecimal fomat) ");
            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan address  (hexadecimal fomat) ") + std::string(args[1]);

            sprintf(answer, "0x%x", myDet->readRegister(addr, detPos));
        }

        if (cmd == "getbit") {

            if (narg < 3)
                return std::string("wrong usage: should specify both address (hexadecimal fomat) and bit number");

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%d", &n))
                ;
            else
                return std::string("Could not scan bit number ") + std::string(args[2]);

            if (n < 0 || n > 31)
                return std::string("Bit number out of range") + std::string(args[2]);

            sprintf(answer, "%d", (myDet->readRegister(addr, detPos) >> n) & 1);
        }
    }

    return std::string(answer);
}

std::string slsDetectorCommand::helpRegister(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "reg addr val \n writes the register addr with the value val (hexadecimal format)" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "reg addr \n reads the register addr" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdDAC(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpDAC(action);

    dacIndex dac;
    int val = -1;
    char answer[1000];
    int mode = 0;

    int idac = -1;
    if (sscanf(args[0], "dac:%d", &idac) == 1) {
        //printf("chiptestboard!\n");
        dac = (dacIndex)idac;
    } else if (cmd == "adcvpp")
        dac = ADC_VPP;
    else if (cmd == "vthreshold")
        dac = THRESHOLD;
    else if (cmd == "vcalibration")
        dac = CALIBRATION_PULSE;
    else if (cmd == "vtrimbit")
        dac = TRIMBIT_SIZE;
    else if (cmd == "vpreamp")
        dac = PREAMP;
    else if (cmd == "vshaper1")
        dac = SHAPER1;
    else if (cmd == "vshaper2")
        dac = SHAPER2;
    else if (cmd == "vhighvoltage")
        dac = HIGH_VOLTAGE;
    else if (cmd == "vapower")
        dac = VA_POT;
    else if (cmd == "vddpower")
        dac = VDD_POT;
    else if (cmd == "vshpower")
        dac = VSH_POT;
    else if (cmd == "viopower")
        dac = VIO_POT;
    else if (cmd == "vref_ds")
        dac = G_VREF_DS;
    else if (cmd == "vcascn_pb")
        dac = G_VCASCN_PB;
    else if (cmd == "vcascp_pb")
        dac = G_VCASCP_PB;
    else if (cmd == "vout_cm")
        dac = G_VOUT_CM;
    else if (cmd == "vcasc_out")
        dac = G_VCASC_OUT;
    else if (cmd == "vin_cm")
        dac = G_VIN_CM;
    else if (cmd == "vref_comp")
        dac = G_VREF_COMP;
    else if (cmd == "ib_test_c")
        dac = G_IB_TESTC;

    else if (cmd == "vsvp")
        dac = E_SvP;
    else if (cmd == "vsvn")
        dac = E_SvN;
    else if (cmd == "vtr")
        dac = E_Vtr;
    else if (cmd == "vrf")
        dac = E_Vrf;
    else if (cmd == "vrs")
        dac = E_Vrs;
    else if (cmd == "vtgstv")
        dac = E_Vtgstv;
    else if (cmd == "vcmp_ll")
        dac = E_Vcmp_ll;
    else if (cmd == "vcmp_lr")
        dac = E_Vcmp_lr;
    else if (cmd == "vcall")
        dac = E_cal;
    else if (cmd == "vcmp_rl")
        dac = E_Vcmp_rl;
    else if (cmd == "vcmp_rr")
        dac = E_Vcmp_rr;
    else if (cmd == "rxb_rb")
        dac = E_rxb_rb;
    else if (cmd == "rxb_lb")
        dac = E_rxb_lb;
    else if (cmd == "vcp")
        dac = E_Vcp;
    else if (cmd == "vcn")
        dac = E_Vcn;
    else if (cmd == "vis")
        dac = E_Vis;
    else if (cmd == "iodelay")
        dac = IO_DELAY;
    else if (cmd == "v_a")
        dac = V_POWER_A;
    else if (cmd == "v_b")
        dac = V_POWER_B;
    else if (cmd == "v_c")
        dac = V_POWER_C;
    else if (cmd == "v_d")
        dac = V_POWER_D;
    else if (cmd == "v_io")
        dac = V_POWER_IO;
    else if (cmd == "v_chip")
        dac = V_POWER_CHIP;
    else if (cmd == "v_limit")
        dac = V_LIMIT;
    else if (cmd == "vIpre")
        dac = M_vIpre;
    else if (cmd == "vIbias")
        dac = M_vIbias;
    else if (cmd == "vIinSh")
        dac = M_vIinSh;
    else if (cmd == "VcdSh")
        dac = M_VdcSh;
    else if (cmd == "Vth1")
        dac = THRESHOLD;
    else if (cmd == "Vth2")
        dac = M_Vth2;
    else if (cmd == "Vth3")
        dac = M_Vth3;
    else if (cmd == "VPL")
        dac = M_VPL;
    else if (cmd == "Vtrim")
        dac = TRIMBIT_SIZE;
    else if (cmd == "casSh")
        dac = M_casSh;
    else if (cmd == "cas")
        dac = M_cas;
    else if (cmd == "vIcin")
        dac = M_vIcin;
    else if (cmd == "vIbiasSh")
        dac = M_vIbiasSh;
    else if (cmd == "vIpreOut")
        dac = M_vIpreOut;

    else
        return std::string("cannot decode dac ") + cmd;

    myDet->setOnline(ONLINE_FLAG, detPos);

    if (action == PUT_ACTION) {

        if (narg >= 3)
            if (!strcasecmp(args[2], "mv"))
                mode = 1;

        if (sscanf(args[1], "%d", &val))
            ;
        else
            return std::string("cannot scan DAC value ") + std::string(args[1]);

        myDet->setDAC(val, dac, mode, detPos);
    }

    else if (narg >= 2)
        if (!strcasecmp(args[1], "mv"))
            mode = 1;

    sprintf(answer, "%d", myDet->setDAC(-1, dac, mode, detPos));
    if (mode)
        strcat(answer, " mV");
    return std::string(answer);
}

std::string slsDetectorCommand::helpDAC(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "vthreshold dacu\t sets the detector threshold in dac units (0-1024) or mV. The energy is approx 800-15*keV" << std::endl;
        os << std::endl;

        os << "vcalibration "
           << "dacu\t sets the calibration pulse amplitude in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vtrimbit "
           << "dacu\t sets the trimbit amplitude in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vpreamp "
           << "dacu\t sets the preamp feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshaper1 "
           << "dacu\t sets the shaper1 feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshaper2 "
           << "dacu\t sets the  shaper2 feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vhighvoltage "
           << "dacu\t CHIPTEST BOARD ONLY - sets the detector HV in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vapower "
           << "dacu\t CHIPTEST BOARD ONLY - sets the analog power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vddpower "
           << "dacu\t CHIPTEST BOARD ONLY - sets the digital power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshpower "
           << "dacu\t CHIPTEST BOARD ONLY - sets the comparator power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "viopower "
           << "dacu\t CHIPTEST BOARD ONLY - sets the FPGA I/O power supply in dac units (0-1024)." << std::endl;

        os << "vrefds "
           << "dacu\t sets vrefds" << std::endl;
        os << "vcascn_pb "
           << "dacu\t sets vcascn_pb" << std::endl;
        os << "vcascp_pb "
           << "dacu\t sets vcascp_pb" << std::endl;
        os << "vout_cm "
           << "dacu\t sets vout_cm" << std::endl;
        os << "vin_cm "
           << "dacu\t sets vin_cm" << std::endl;
        os << "vcasc_out "
           << "dacu\t sets vcasc_out" << std::endl;
        os << "vref_comp "
           << "dacu\t sets vref_comp" << std::endl;
        os << "ib_test_c "
           << "dacu\t sets ib_test_c" << std::endl;

        os << "vsvp"
           << "dacu\t sets vsvp" << std::endl;
        os << "vsvn"
           << "dacu\t sets vsvn" << std::endl;
        os << "vtr"
           << "dacu\t sets vtr" << std::endl;
        os << "vrf"
           << "dacu\t sets vrf" << std::endl;
        os << "vrs"
           << "dacu\t sets vrs" << std::endl;
        os << "vtgstv"
           << "dacu\t sets vtgstv" << std::endl;
        os << "vcmp_ll"
           << "dacu\t sets vcmp_ll" << std::endl;
        os << "vcmp_lr"
           << "dacu\t sets vcmp_lr" << std::endl;
        os << "vcall"
           << "dacu\t sets vcall" << std::endl;
        os << "vcmp_rl"
           << "dacu\t sets vcmp_rl" << std::endl;
        os << "vcmp_rr"
           << "dacu\t sets vcmp_rr" << std::endl;
        os << "rxb_rb"
           << "dacu\t sets rxb_rb" << std::endl;
        os << "rxb_lb"
           << "dacu\t sets rxb_lb" << std::endl;
        os << "vcp"
           << "dacu\t sets vcp " << std::endl;
        os << "vcn"
           << "dacu\t sets vcn " << std::endl;
        os << "vis"
           << "dacu\t sets vis " << std::endl;

        os << "<dac name> mv <value> if you want in mV else <dac name> <value> in dac units " << std::endl;
    }

    if (action == GET_ACTION || action == HELP_ACTION) {

        os << "vthreshold \t Gets the detector threshold in dac units (0-1024). The energy is approx 800-15*keV" << std::endl;
        os << std::endl;

        os << "vcalibration "
           << "dacu\t gets the calibration pulse amplitude in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vtrimbit "
           << "dacu\t gets the trimbit amplitude in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vpreamp "
           << "dacu\t gets the preamp feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshaper1 "
           << "dacu\t gets the shaper1 feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshaper2 "
           << "dacu\t gets the  shaper2 feedback voltage in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vhighvoltage "
           << "dacu\t CHIPTEST BOARD ONLY - gets the detector HV in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vapower "
           << "dacu\t CHIPTEST BOARD ONLY - gets the analog power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vddpower "
           << "dacu\t CHIPTEST BOARD ONLY - gets the digital power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "vshpower "
           << "dacu\t CHIPTEST BOARD ONLY - gets the comparator power supply in dac units (0-1024)." << std::endl;
        os << std::endl;
        os << "viopower "
           << "dacu\t CHIPTEST BOARD ONLY - gets the FPGA I/O power supply in dac units (0-1024)." << std::endl;
        os << std::endl;

        os << "vrefds "
           << "\t gets vrefds" << std::endl;
        os << "vcascn_pb "
           << "\t gets vcascn_pb" << std::endl;
        os << "vcascp_pb "
           << "\t gets vcascp_pb" << std::endl;
        os << "vout_cm "
           << "\t gets vout_cm" << std::endl;
        os << "vin_cm "
           << "\t gets vin_cm" << std::endl;
        os << "vcasc_out "
           << "\t gets vcasc_out" << std::endl;
        os << "vref_comp "
           << "\t gets vref_comp" << std::endl;
        os << "ib_test_c "
           << "\t gets ib_test_c" << std::endl;

        os << "vsvp"
           << "dacu\t gets vsvp" << std::endl;
        os << "vsvn"
           << "dacu\t gets vsvn" << std::endl;
        os << "vtr"
           << "dacu\t gets vtr" << std::endl;
        os << "vrf"
           << "dacu\t gets vrf" << std::endl;
        os << "vrs"
           << "dacu\t gets vrs" << std::endl;
        os << "vtgstv"
           << "dacu\t gets vtgstv" << std::endl;
        os << "vcmp_ll"
           << "dacu\t gets vcmp_ll" << std::endl;
        os << "vcmp_lr"
           << "dacu\t gets vcmp_lr" << std::endl;
        os << "vcall"
           << "dacu\t gets vcall" << std::endl;
        os << "vcmp_rl"
           << "dacu\t gets vcmp_rl" << std::endl;
        os << "vcmp_rr"
           << "dacu\t gets vcmp_rr" << std::endl;
        os << "rxb_rb"
           << "dacu\t gets rxb_rb" << std::endl;
        os << "rxb_lb"
           << "dacu\t gets rxb_lb" << std::endl;
        os << "vcp"
           << "dacu\t gets vcp " << std::endl;
        os << "vcn"
           << "dacu\t gets vcn " << std::endl;
        os << "vis"
           << "dacu\t gets vis " << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdADC(int narg, char *args[], int action, int detPos) {

    dacIndex adc;
    int idac;
    //  double val=-1;
    char answer[1000];

    if (action == HELP_ACTION)
        return helpADC(action);
    else if (action == PUT_ACTION)
        return std::string("cannot set ") + cmd;

	if (sscanf(args[0],"adc:%d",&idac)==1) {
		//  printf("chiptestboard!\n");
		adc=(dacIndex)(idac+SLOW_ADC0);
        if (idac < 0 || idac > SLOW_ADC_TEMP - SLOW_ADC0)
            return (std::string ("cannot set adc, must be between ") + std::to_string(0) +
                    std::string (" and ") + std::to_string(SLOW_ADC_TEMP - SLOW_ADC0));
	} else if (cmd=="temp_adc")
		adc=TEMPERATURE_ADC;
	else if (cmd=="temp_fpga")
		adc=TEMPERATURE_FPGA;
	else if (cmd=="temp_fpgaext")
		adc=TEMPERATURE_FPGAEXT;
	else if (cmd=="temp_10ge")
		adc=TEMPERATURE_10GE;
	else if (cmd=="temp_dcdc")
		adc=TEMPERATURE_DCDC;
	else if (cmd=="temp_sodl")
		adc=TEMPERATURE_SODL;
	else if (cmd=="temp_sodr")
		adc=TEMPERATURE_SODR;
	else if (cmd=="temp_fpgafl")
		adc=TEMPERATURE_FPGA2;
	else if (cmd=="temp_fpgafr")
		adc=TEMPERATURE_FPGA3;
	else if (cmd=="i_a")
		adc=I_POWER_A;
	else if (cmd=="i_b")
		adc=I_POWER_B;
	else if (cmd=="i_c")
		adc=I_POWER_C;
	else if (cmd=="i_d")
		adc=I_POWER_D;
	else if (cmd=="vm_a")
		adc=V_POWER_A;
	else if (cmd=="vm_b")
		adc=V_POWER_B;
	else if (cmd=="vm_c")
		adc=V_POWER_C;
	else if (cmd=="vm_d")
		adc=V_POWER_D;
	else if (cmd=="vm_io")
		adc=V_POWER_IO;
	else if (cmd=="i_io")
		adc=I_POWER_IO;
	else
		return std::string("cannot decode adc ")+cmd;

	myDet->setOnline(ONLINE_FLAG, detPos);
	if (myDet->getDetectorTypeAsEnum(detPos) == EIGER || myDet->getDetectorTypeAsEnum(detPos) == JUNGFRAU){
		int val = myDet->getADC(adc, detPos);
		if (val == -1)
			sprintf(answer,"%d",val);
		else
			sprintf(answer,"%.2f", (double)val/1000.000);
	}
	else sprintf(answer,"%d",myDet->getADC(adc, detPos));

	//if ((adc == TEMPERATURE_ADC) || (adc == TEMPERATURE_FPGA))
	if (adc<=100 || adc == SLOW_ADC_TEMP)
		strcat(answer,"°C");
	else
		strcat(answer,"mV");

	return std::string(answer);

}

std::string slsDetectorCommand::helpADC(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "temp_adc "
           << "Cannot be set" << std::endl;
        os << "temp_fpga "
           << "Cannot be set" << std::endl;
        os << "temp_fpgaext "
           << "Cannot be set" << std::endl;
        os << "temp_10ge "
           << "Cannot be set" << std::endl;
        os << "temp_dcdc "
           << "Cannot be set" << std::endl;
        os << "temp_sodl "
           << "Cannot be set" << std::endl;
        os << "temp_sodr "
           << "Cannot be set" << std::endl;
        os << "temp_fpgafl "
           << "Cannot be set" << std::endl;
        os << "temp_fpgafr "
           << "Cannot be set" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "temp_adc "
           << "\t gets the temperature of the adc" << std::endl;
        os << "temp_fpga "
           << "\t gets the temperature of the fpga" << std::endl;
        os << "temp_fpgaext "
           << "\t gets the temperature close to the fpga" << std::endl;
        os << "temp_10ge "
           << "\t gets the temperature close to the 10GE" << std::endl;
        os << "temp_dcdc "
           << "\t gets the temperature close to the dc dc converter" << std::endl;
        os << "temp_sodl "
           << "\t gets the temperature close to the left so-dimm memory" << std::endl;
        os << "temp_sodr "
           << "\t gets the temperature close to the right so-dimm memory" << std::endl;
        os << "temp_fpgafl "
           << "\t gets the temperature of the left front end board fpga" << std::endl;
        os << "temp_fpgafr "
           << "\t gets the temperature of the left front end board fpga" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdTempControl(int narg, char *args[], int action, int detPos) {
    char answer[1000] = "";
    int val = -1;

    if (action == HELP_ACTION)
        return helpTempControl(action);

    myDet->setOnline(ONLINE_FLAG, detPos);

    if (cmd == "temp_threshold") {
        if (action == PUT_ACTION) {
            double fval = 0.0;
            if (!sscanf(args[1], "%lf", &fval))
                return std::string("cannot scan temp control value ") + std::string(args[1]);
            val = fval * 1000;
            myDet->setThresholdTemperature(val, detPos);
        }
        val = myDet->setThresholdTemperature(-1, detPos);
        if (val == -1)
            sprintf(answer, "%d", val);
        else
            sprintf(answer, "%.2f°C", (double)val / 1000.000);
    }

    else if (cmd == "temp_control") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &val))
                return std::string("cannot scan temp control value ") + std::string(args[1]);
            if ((val != 0) && (val != 1))
                return std::string("temp_control option must be 0 or 1");
            myDet->setTemperatureControl(val, detPos);
        }
        sprintf(answer, "%d", myDet->setTemperatureControl(-1, detPos));
    }

    else if (cmd == "temp_event") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &val))
                return std::string("cannot scan temp control value ") + std::string(args[1]);
            if (val != 0)
                return std::string("temp_event option must be 0 to clear event");
            myDet->setTemperatureEvent(val, detPos);
        }
        sprintf(answer, "%d", myDet->setTemperatureEvent(-1, detPos));
    }

    else
        return std::string("cannot scan command " + cmd);

    return std::string(answer);
}

std::string slsDetectorCommand::helpTempControl(int action) {
    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "temp_threshold t \t sets the threshold temperature. Jungfrau only" << std::endl;
        os << "temp_control t \t Enables/Disables the temperature control. 1 enables, 0 disables. JUNGFRAU ONLY" << std::endl;
        os << "temp_event t \t Resets over-temperative event. Put only with option 0 to clear event. JUNGFRAU ONLY." << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "temp_threshold  \t gets the threshold temperature. Jungfrau only." << std::endl;
        os << "temp_control  \t gets temperature control enable. 1 enabled, 0 disabled. JUNGFRAU ONLY" << std::endl;
        os << "temp_event  \t gets over-temperative event. Gets 1 if temperature went over threshold and control is enabled, else 0. /Disables the temperature control. JUNGFRAU ONLY." << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdTiming(int narg, char *args[], int action, int detPos) {
#ifdef VERBOSE
    std::cout << std::string("Executing command ") + std::string(args[0]) + std::string(" ( ") + cmd + std::string(" )\n");
#endif

    if (action == HELP_ACTION) {
        return helpTiming(HELP_ACTION);
    }
    myDet->setOnline(ONLINE_FLAG, detPos);
    if (action == PUT_ACTION) {
        if (myDet->externalCommunicationType(std::string(args[1])) == GET_EXTERNAL_COMMUNICATION_MODE)
            return helpTiming(action);
        myDet->setExternalCommunicationMode(myDet->externalCommunicationType(std::string(args[1])), detPos);
    }
    return myDet->externalCommunicationType(myDet->setExternalCommunicationMode(GET_EXTERNAL_COMMUNICATION_MODE, detPos));
}
std::string slsDetectorCommand::helpTiming(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION)
        os << std::string("timing \t gets the timing mode of the detector (auto, trigger, ro_trigger, gating, triggered_gating)\n");
    if (action == PUT_ACTION || action == HELP_ACTION)
        os << std::string("timing mode \t sets synchronization mode of the detector. Can be auto, trigger, ro_trigger, gating, triggered_gating \n");
    return os.str();
}

std::string slsDetectorCommand::cmdTimer(int narg, char *args[], int action, int detPos) {
    timerIndex index;
    int64_t t = -1, ret;
    double val, rval;

    char answer[1000];

    if (action == HELP_ACTION)
        return helpTimer(action);

    if (cmd == "exptime")
        index = ACQUISITION_TIME;
    else if (cmd == "subexptime")
        index = SUBFRAME_ACQUISITION_TIME;
    else if (cmd == "period")
        index = FRAME_PERIOD;
    else if (cmd == "subdeadtime")
        index = SUBFRAME_DEADTIME;
    else if (cmd == "delay")
        index = DELAY_AFTER_TRIGGER;
    else if (cmd == "gates")
        index = GATES_NUMBER;
    else if (cmd == "frames")
        index = FRAME_NUMBER;
    else if (cmd == "cycles")
        index = CYCLES_NUMBER;
    else if (cmd == "measurements")
        index = MEASUREMENTS_NUMBER;
    else if (cmd == "samples")
        index = SAMPLES;
    else if (cmd == "storagecells")
        index = STORAGE_CELL_NUMBER;
    else if (cmd == "storagecell_start") {
        myDet->setOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            int ival = -1;
            if (!sscanf(args[1], "%d", &ival))
                return std::string("cannot scan storage cell start value ") + std::string(args[1]);
            myDet->setStoragecellStart(ival, detPos);
        }
        sprintf(answer, "%d", myDet->setStoragecellStart(-1, detPos));
        return std::string(answer);
    } else
        return std::string("could not decode timer ") + cmd;

    if (action == PUT_ACTION) {
        if (sscanf(args[1], "%lf", &val))
            ; //printf("value:%0.9lf\n",val);
        else
            return std::string("cannot scan timer value ") + std::string(args[1]);
        if (index == ACQUISITION_TIME || index == SUBFRAME_ACQUISITION_TIME ||
            index == FRAME_PERIOD || index == DELAY_AFTER_TRIGGER ||
            index == SUBFRAME_DEADTIME) {
            // 	+0.5 for precision of eg.0.0000325
            t = (val * 1E9 + 0.5);
        } else
            t = (int64_t)val;
    }

    myDet->setOnline(ONLINE_FLAG, detPos);
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);

    ret = myDet->setTimer(index, t, detPos);

    if ((ret != -1) && (index == ACQUISITION_TIME || index == SUBFRAME_ACQUISITION_TIME || index == FRAME_PERIOD || index == DELAY_AFTER_TRIGGER ||
                        index == SUBFRAME_DEADTIME)) {
        rval = (double)ret * 1E-9;
        sprintf(answer, "%0.9f", rval);
    } else
        sprintf(answer, "%lld", (long long int)ret);

    return std::string(answer);
}

std::string slsDetectorCommand::helpTimer(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "exptime t \t sets the exposure time in s" << std::endl;
        os << "subexptime t \t sets the exposure time of subframe in s" << std::endl;
        os << "period t \t sets the frame period in s" << std::endl;
        os << "delay t \t sets the delay after trigger in s" << std::endl;
        os << "frames t \t sets the number of frames per cycle (e.g. after each trigger)" << std::endl;
        os << "cycles t \t sets the number of cycles (e.g. number of triggers)" << std::endl;
        os << "samples t \t sets the number of samples expected from the jctb" << std::endl;
        os << "storagecells t \t sets number of storage cells per acquisition. For very advanced users only! For JUNGFRAU only. Range: 0-15. The #images = #frames * #cycles * (#storagecells+1)." << std::endl;
        os << "storagecell_start t \t sets the storage cell that stores the first acquisition of the series. Default is 15(0xf). For very advanced users only! For JUNGFRAU only. Range: 0-15." << std::endl;
        os << "subdeadtime t \t sets sub frame dead time in s. Subperiod is set in the detector = subexptime + subdeadtime. This value is normally a constant in the config file. Used in EIGER only in 32 bit mode. " << std::endl;
        os << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {

        os << "exptime  \t gets the exposure time in s" << std::endl;
        os << "subexptime  \t gets the exposure time of subframe in s" << std::endl;
        os << "period  \t gets the frame period in s" << std::endl;
        os << "delay  \t gets the delay after trigger in s" << std::endl;
        os << "frames  \t gets the number of frames per cycle (e.g. after each trigger)" << std::endl;
        os << "cycles  \t gets the number of cycles (e.g. number of triggers)" << std::endl;
        os << "samples \t gets the number of samples expected from the jctb" << std::endl;
        os << "storagecells \t gets number of storage cells per acquisition.For JUNGFRAU only." << std::endl;
        os << "storagecell_start \t gets the storage cell that stores the first acquisition of the series." << std::endl;
        os << "subperiod \t gets sub frame dead time in s. Used in EIGER in 32 bit only." << std::endl;
        os << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdTimeLeft(int narg, char *args[], int action, int detPos) {
    timerIndex index;
    int64_t ret;
    double rval;

    char answer[1000];

    if (action == HELP_ACTION)
        return helpTimeLeft(action);

    if (cmd == "exptimel")
        index = ACQUISITION_TIME;
    else if (cmd == "periodl")
        index = FRAME_PERIOD;
    else if (cmd == "delayl")
        index = DELAY_AFTER_TRIGGER;
    else if (cmd == "gatesl")
        index = GATES_NUMBER;
    else if (cmd == "framesl")
        index = FRAME_NUMBER;
    else if (cmd == "cyclesl")
        index = CYCLES_NUMBER;
    else if (cmd == "now")
        index = ACTUAL_TIME;
    else if (cmd == "timestamp")
        index = MEASUREMENT_TIME;
    else if (cmd == "nframes")
        index = FRAMES_FROM_START;
    else if (cmd == "measuredperiod")
        index = MEASURED_PERIOD;
    else if (cmd == "measuredsubperiod")
        index = MEASURED_SUBPERIOD;
    else
        return std::string("could not decode timer ") + cmd;

    if (action == PUT_ACTION) {
        return std::string("cannot set ") + std::string(args[1]);
    }

    myDet->setOnline(ONLINE_FLAG, detPos);

    ret = myDet->getTimeLeft(index, detPos);

    if ((ret != -1) && (index == ACQUISITION_TIME || index == FRAME_PERIOD || index == DELAY_AFTER_TRIGGER || index == ACTUAL_TIME || index == MEASUREMENT_TIME ||
                        MEASURED_PERIOD || MEASURED_SUBPERIOD))
        rval = (double)ret * 1E-9;
    else
        rval = ret;

    sprintf(answer, "%0.9f", rval);
    return std::string(answer);
}

std::string slsDetectorCommand::helpTimeLeft(int action) {

    std::ostringstream os;
    if (action == GET_ACTION || action == HELP_ACTION) {

        os << "exptimel  \t gets the exposure time left" << std::endl;
        os << "periodl \t gets the frame period left" << std::endl;
        os << "delayl  \t gets the delay left" << std::endl;
        os << "framesl  \t gets the number of frames left" << std::endl;
        os << "cyclesl  \t gets the number of cycles left" << std::endl;
        os << "measuredperiod \t gets the measured frame period (time between last frame and the previous one) in s. For Eiger only. Makes sense only for acquisitions of more than 1 frame." << std::endl;
        os << "measuredsubperiod \t gets the measured subframe period (time between last subframe and the previous one) in s. For Eiger only and in 32 bit mode." << std::endl;
        os << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdSpeed(int narg, char *args[], int action, int detPos) {

    speedVariable index;
    int t = -1, ret;

    char answer[1000];

    if (action == HELP_ACTION)
        return helpSpeed(action);

    if (cmd == "clkdivider")
        index = CLOCK_DIVIDER;
    else if (cmd == "phasestep") {
        index = PHASE_SHIFT;
        t = 100000;
    } else if (cmd == "oversampling")
        index = OVERSAMPLING;
    else if (cmd == "adcclk")
        index = ADC_CLOCK;
    else if (cmd == "adcphase") {
        index = ADC_PHASE;
        t = 100000;
    } else if (cmd == "adcpipeline")
        index = ADC_PIPELINE;
    else if (cmd == "dbitclk")
        index = DBIT_CLOCK;
    else if (cmd == "dbitphase") {
        index = DBIT_PHASE;
        t = 100000;
    } else if (cmd == "dbitpipeline")
        index = DBIT_PIPELINE;
    else
        return std::string("could not decode speed variable ") + cmd;

    if (action == PUT_ACTION) {
        if (sscanf(args[1], "%d", &t))
            ;
        else
            return std::string("cannot scan speed value ") + std::string(args[1]);
    }

    myDet->setOnline(ONLINE_FLAG, detPos);

    ret = myDet->setSpeed(index, t, detPos);

    sprintf(answer, "%d", ret);
    return std::string(answer);
}

std::string slsDetectorCommand::helpSpeed(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {

        os << "clkdivider c  \t sets readout clock divider. For Jungfrau, it also overwrites adcphase to recommended default" << std::endl;
        os << "adcphase  c\t Sets phase of the sampling clock. For JUNGFRAU, setting speed (clkdivider) overwrites adcphase to its default recommended value. (Not for EIGER)" << std::endl;
        os << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {

        os << "clkdivider  \t gets readout clock divider. For Jungfrau, it also overwrites adcphase to recommended default" << std::endl;
        os << "adcphase \t gets phase of the sampling clock. For JUNGFRAU, setting speed (clkdivider) overwrites adcphase to its default recommended value. (Not for EIGER)" << std::endl;
        os << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdAdvanced(int narg, char *args[], int action, int detPos) {

    int retval;
    char answer[1000] = "";

    if (action == HELP_ACTION)
        return helpAdvanced(action);

    if (cmd == "flags") {

        readOutFlags flag = GET_READOUT_FLAGS;

        if (action == PUT_ACTION) {
            std::string sval = std::string(args[1]);
            if (sval == "none")
                flag = NORMAL_READOUT;
            else if (sval == "storeinram")
                flag = STORE_IN_RAM;
            else if (sval == "tot")
                flag = TOT_MODE;
            else if (sval == "continous")
                flag = CONTINOUS_RO;
            else if (sval == "parallel")
                flag = PARALLEL;
            else if (sval == "nonparallel")
                flag = NONPARALLEL;
            else if (sval == "safe")
                flag = SAFE;
            else if (sval == "digital")
                flag = DIGITAL_ONLY;
            else if (sval == "analog_digital")
                flag = ANALOG_AND_DIGITAL;
            else if (sval == "overflow")
                flag = SHOW_OVERFLOW;
            else if (sval == "nooverflow")
                flag = NOOVERFLOW;
            else
                return std::string("could not scan flag ") + std::string(args[1]);
        }

        myDet->setOnline(ONLINE_FLAG, detPos);
        retval = myDet->setReadOutFlags(flag, detPos);

        // std::cout << std::hex << flag << " " << retval << std::endl;

        if (retval == NORMAL_READOUT)
            return std::string("none");

        if (retval & STORE_IN_RAM)
            strcat(answer, "storeinram ");
        if (retval & TOT_MODE)
            strcat(answer, "tot ");
        if (retval & CONTINOUS_RO)
            strcat(answer, "continous ");
        if (retval & PARALLEL)
            strcat(answer, "parallel ");
        if (retval & NONPARALLEL)
            strcat(answer, "nonparallel ");
        if (retval & SAFE)
            strcat(answer, "safe ");
        if (retval & DIGITAL_ONLY)
            strcat(answer, "digital ");
        if (retval & ANALOG_AND_DIGITAL)
            strcat(answer, "analog_digital ");
        if (retval & SHOW_OVERFLOW)
            strcat(answer, "overflow ");
        if (retval & NOOVERFLOW)
            strcat(answer, "nooverflow ");
        if (strlen(answer))
            return std::string(answer);

        return std::string("unknown");

    } else if (cmd == "extsig") {
        externalSignalFlag flag = GET_EXTERNAL_SIGNAL_FLAG;
        int is = -1;
        if (sscanf(args[0], "extsig:%d", &is))
            ;
        else
            return std::string("could not scan signal number ") + std::string(args[0]);

        if (action == PUT_ACTION) {
            flag = myDet->externalSignalType(args[1]);
            if (flag == GET_EXTERNAL_SIGNAL_FLAG)
                return std::string("could not scan external signal mode ") + std::string(args[1]);
        }
        myDet->setOnline(ONLINE_FLAG, detPos);

        return myDet->externalSignalType(myDet->setExternalSignalFlags(flag, is, detPos));

    } else if (cmd == "programfpga") {
        if (action == GET_ACTION)
            return std::string("cannot get");
        if (strstr(args[1], ".pof") == nullptr)
            return std::string("wrong usage: programming file should have .pof extension");
        std::string sval = std::string(args[1]);
#ifdef VERBOSE
        std::cout << " programming file " << sval << std::endl;
#endif
        myDet->setOnline(ONLINE_FLAG, detPos);
        if (myDet->programFPGA(sval, detPos) == OK)
            return std::string("successful");
        return std::string("unsuccessful");
    }

    else if (cmd == "resetfpga") {
        if (action == GET_ACTION)
            return std::string("cannot get");
#ifdef VERBOSE
        std::cout << " resetting fpga " << std::endl;
#endif
        myDet->setOnline(ONLINE_FLAG, detPos);
        if (myDet->resetFPGA(detPos) == OK)
            return std::string("successful");
        return std::string("unsuccessful");
    }

    else if (cmd == "powerchip") {
        char ans[100];
        myDet->setOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            int ival = -1;
            if (!sscanf(args[1], "%d", &ival))
                return std::string("could not scan powerchip parameter " + std::string(args[1]));
            myDet->powerChip(ival, detPos);
        }
        sprintf(ans, "%d", myDet->powerChip(-1, detPos));
        return std::string(ans);
    }

    else if (cmd == "led") {
        char ans[100];
        int val = 0;
        myDet->setOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            int ival = -1;
            if (!sscanf(args[1], "%d", &ival))
                return std::string("could not scan powerchip parameter " + std::string(args[1]));
            val = myDet->readRegister(0x4d, detPos);
            myDet->writeRegister(0x4d, (val & (~1)) | ((~ival) & 1), detPos); //config register
        }
        sprintf(ans, "%d", ~(myDet->readRegister(0x4d, detPos)) & 1);
        return std::string(ans);
    }

    else if (cmd == "auto_comp_disable") {
        char ans[100];
        myDet->setOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            int ival = -1;
            if (!sscanf(args[1], "%d", &ival))
                return std::string("could not scan auto_comp_control parameter " + std::string(args[1]));
            myDet->setAutoComparatorDisableMode(ival, detPos);
        }
        sprintf(ans, "%d", myDet->setAutoComparatorDisableMode(-1, detPos));
        return std::string(ans);
    } else
        return std::string("unknown command ") + cmd;
}

std::string slsDetectorCommand::helpAdvanced(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {

        os << "extsig:i mode \t sets the mode of the external signal i. can be  \n \t \t \t off, \n \t \t \t gate_in_active_high, \n \t \t \t gate_in_active_low, \n \t \t \t trigger_in_rising_edge, \n \t \t \t trigger_in_falling_edge, \n \t \t \t ro_trigger_in_rising_edge, \n \t \t \t ro_trigger_in_falling_edge, \n \t \t \t gate_out_active_high, \n \t \t \t gate_out_active_low, \n \t \t \t trigger_out_rising_edge, \n \t \t \t trigger_out_falling_edge, \n \t \t \t ro_trigger_out_rising_edge, \n \t \t \t ro_trigger_out_falling_edge" << std::endl;
        os << "flags mode \t sets the readout flags to mode. can be none, storeinram, tot, continous, parallel, nonparallel, safe, digital, analog_digital, overlow, nooverflow, unknown." << std::endl;

        os << "programfpga f \t programs the fpga with file f (with .pof extension)." << std::endl;
        os << "resetfpga f \t resets fpga, f can be any value" << std::endl;

        os << "led s \t sets led status (0 off, 1 on)" << std::endl;
        os << "powerchip i \t powers on or off the chip. i = 1 for on, i = 0 for off" << std::endl;
        os << "auto_comp_disable i \t Currently not implemented. this mode disables the on-chip gain switching comparator automatically after 93.75% of exposure time (only for longer than 100us). 1 enables mode, 0 disables mode. By default, mode is disabled (comparator is enabled throughout). (JUNGFRAU only). " << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {

        os << "extsig:i \t gets the mode of the external signal i. can be  \n \t \t \t off, \n \t \t \t gate_in_active_high, \n \t \t \t gate_in_active_low, \n \t \t \t trigger_in_rising_edge, \n \t \t \t trigger_in_falling_edge, \n \t \t \t ro_trigger_in_rising_edge, \n \t \t \t ro_trigger_in_falling_edge, \n \t \t \t gate_out_active_high, \n \t \t \t gate_out_active_low, \n \t \t \t trigger_out_rising_edge, \n \t \t \t trigger_out_falling_edge, \n \t \t \t ro_trigger_out_rising_edge, \n \t \t \t ro_trigger_out_falling_edge" << std::endl;

        os << "flags \t gets the readout flags. can be none, storeinram, tot, continous, parallel, nonparallel, safe, digital, analog_digital, overflow, nooverflow, unknown" << std::endl;
        os << "led \t returns led status (0 off, 1 on)" << std::endl;
        os << "powerchip \t gets if the chip has been powered on or off" << std::endl;
        os << "auto_comp_disable \t Currently not implemented. gets if the automatic comparator diable mode is enabled/disabled" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdConfiguration(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpConfiguration(action);

    std::string sval;

    if (narg < 2 && cmd != "rx_printconfig")
        return std::string("should specify I/O file");

    myDet->setOnline(ONLINE_FLAG, detPos);

    if (cmd == "config") {
        if (action == PUT_ACTION) {
            sval = std::string(args[1]);
            myDet->readConfigurationFile(sval);
        } else if (action == GET_ACTION) {
            sval = std::string(args[1]);
            myDet->writeConfigurationFile(sval);
        }
        return sval;
    } else if (cmd == "rx_printconfig") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION)
            return std::string("cannot put");
        myDet->printReceiverConfiguration(detPos);
        return std::string("");
    } else if (cmd == "parameters") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            sval = std::string(args[1]);
            myDet->retrieveDetectorSetup(sval, 0);
        } else if (action == GET_ACTION) {
            sval = std::string(args[1]);
            myDet->dumpDetectorSetup(sval, 0);
        }
        return sval;
    } else if (cmd == "setup") {
        myDet->setReceiverOnline(ONLINE_FLAG, detPos);
        if (action == PUT_ACTION) {
            sval = std::string(args[1]);
            myDet->retrieveDetectorSetup(sval, 2);
        } else if (action == GET_ACTION) {
            sval = std::string(args[1]);
            myDet->dumpDetectorSetup(sval, 2);
        }
        return sval;
    }
    return std::string("could not decode conf mode");
}

std::string slsDetectorCommand::helpConfiguration(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {

        os << "config fname \t sets the detector to the configuration contained in fname" << std::endl;
        os << "parameters fname \t sets the detector parameters to those contained in fname" << std::endl;
        os << "setup fname \t sets the detector complete detector setup to that contained in fname (extensions automatically generated), including trimfiles, ff coefficients etc." << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "rx_printconfig \t prints the receiver configuration" << std::endl;
        os << "config fname \t saves the detector to the configuration to fname" << std::endl;
        os << "parameters fname \t saves the detector parameters to  fname" << std::endl;
        os << "setup fname \t saves the detector complete detector setup to  fname (extensions automatically generated), including trimfiles, ff coefficients etc." << std::endl;
    }

    return os.str();
}

std::string slsDetectorCommand::cmdReceiver(int narg, char *args[], int action, int detPos) {
    char answer[100];
    int ival = -1;

    if (action == HELP_ACTION)
        return helpReceiver(action);

    myDet->setOnline(ONLINE_FLAG, detPos);
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);

    if (cmd == "receiver") {
        if (action == PUT_ACTION) {
            if (!strcasecmp(args[1], "start"))
                myDet->startReceiver(detPos);
            else if (!strcasecmp(args[1], "stop"))
                myDet->stopReceiver(detPos);
            else
                return helpReceiver(action);
        }
        return myDet->runStatusType(myDet->getReceiverStatus(detPos));
    }

    else if (cmd == "framescaught") {
        if (action == PUT_ACTION)
            return std::string("cannot put");
        else {
            sprintf(answer, "%d", myDet->getFramesCaughtByReceiver(detPos));
            return std::string(answer);
        }
    }

    else if (cmd == "resetframescaught") {
        if (action == GET_ACTION)
            return std::string("cannot get");
        else {
            if (myDet->resetFramesCaught(detPos) == FAIL)
                strcpy(answer, "failed");
            else
                strcpy(answer, "successful");
            return std::string(answer);
        }
    }

    else if (cmd == "frameindex") {
        if (action == PUT_ACTION)
            return std::string("cannot put");
        else {
            sprintf(answer, "%d", myDet->getReceiverCurrentFrameIndex(detPos));
            return std::string(answer);
        }
    } else if (cmd == "r_readfreq") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &ival))
                return std::string("Could not scan read frequency mode ") + std::string(args[1]);
            if (ival >= 0)
                myDet->setReceiverStreamingFrequency(ival, detPos);
        }
        sprintf(answer, "%d", myDet->setReceiverStreamingFrequency(-1, detPos));
        return std::string(answer);

    }

    else if (cmd == "tengiga") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &ival))
                return std::string("Could not scan tengiga input ") + std::string(args[1]);
            if (ival >= 0)
                sprintf(answer, "%d", myDet->enableTenGigabitEthernet(ival, detPos));
        } else
            sprintf(answer, "%d", myDet->enableTenGigabitEthernet(-1, detPos));
        return std::string(answer);

    }

    else if (cmd == "rx_fifodepth") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &ival))
                return std::string("Could not scan rx_fifodepth input ") + std::string(args[1]);
            if (ival >= 0)
                sprintf(answer, "%d", myDet->setReceiverFifoDepth(ival, detPos));
        } else
            sprintf(answer, "%d", myDet->setReceiverFifoDepth(-1, detPos));
        return std::string(answer);

    }

    else if (cmd == "r_silent") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1], "%d", &ival))
                return std::string("Could not scan r_online input ") + std::string(args[1]);
            if (ival >= 0)
                sprintf(answer, "%d", myDet->setReceiverSilentMode(ival, detPos));
        } else
            sprintf(answer, "%d", myDet->setReceiverSilentMode(-1, detPos));
        return std::string(answer);

    }

    else if (cmd == "r_framesperfile") {
        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%d", &ival)) {
                myDet->setReceiverFramesPerFile(ival, detPos);
            } else
                return std::string("could not scan max frames per file\n");
        }
        char answer[100];
        memset(answer, 0, 100);
        sprintf(answer, "%d", myDet->setReceiverFramesPerFile(-1, detPos));
        return std::string(answer);
    }

    else if (cmd == "r_discardpolicy") {
        if (action == PUT_ACTION) {
            frameDiscardPolicy f = myDet->getReceiverFrameDiscardPolicy(std::string(args[1]));
            if (f == GET_FRAME_DISCARD_POLICY)
                return std::string("could not scan frame discard policy. Options: nodiscard, discardempty, discardpartial\n");
            myDet->setReceiverFramesDiscardPolicy(f);
        }
        return myDet->getReceiverFrameDiscardPolicy(myDet->setReceiverFramesDiscardPolicy(GET_FRAME_DISCARD_POLICY, detPos));
    }

    else if (cmd == "r_padding") {
        if (action == PUT_ACTION) {
            if (sscanf(args[1], "%d", &ival)) {
                myDet->setReceiverPartialFramesPadding(ival, detPos);
            } else
                return std::string("could not scan receiver padding enable\n");
        }
        char answer[100];
        memset(answer, 0, 100);
        sprintf(answer, "%d", myDet->setReceiverPartialFramesPadding(-1, detPos));
        return std::string(answer);
    }

    else if (cmd == "rx_jsonaddheader") {
        if (action == PUT_ACTION) {
            myDet->setAdditionalJsonHeader(args[1], detPos);
        }
        return myDet->getAdditionalJsonHeader(detPos);
    }

    else if (cmd == "rx_jsonpara") {
        if (action == PUT_ACTION) {
            myDet->setAdditionalJsonParameter(args[1], args[2], detPos);
        }
        return myDet->getAdditionalJsonParameter(args[1], detPos);
    }

    return std::string("could not decode command");
}

std::string slsDetectorCommand::helpReceiver(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "receiver [status] \t starts/stops the receiver to listen to detector packets. - can be start, stop." << std::endl;
        os << "resetframescaught [any value] \t resets frames caught by receiver" << std::endl;
        os << "r_readfreq \t sets the gui read frequency of the receiver, 0 if gui requests frame, >0 if receiver sends every nth frame to gui" << std::endl;
        os << "tengiga \t sets system to be configure for 10Gbe if set to 1, else 1Gbe if set to 0" << std::endl;
        os << "rx_fifodepth [val]\t sets receiver fifo depth to val" << std::endl;
        os << "r_silent [i]\t sets receiver in silent mode, ie. it will not print anything during real time acquisition. 1 sets, 0 unsets." << std::endl;
        os << "r_framesperfile s\t sets the number of frames per file in receiver. 0 means infinite or all frames in a single file." << std::endl;
        os << "r_discardpolicy s\t sets the frame discard policy in the receiver. nodiscard (default) - discards nothing, discardempty - discard only empty frames, discardpartial(fastest) - discards all partial frames." << std::endl;
        os << "r_padding s\t enables/disables partial frames to be padded in the receiver. 0 does not pad partial frames(fastest), 1 (default) pads partial frames." << std::endl;
        os << "rx_jsonaddheader [t]\n sets additional json header to be streamed "
              "out with the zmq from receiver. Default is empty. t must be in the format '\"label1\":\"value1\",\"label2\":\"value2\"' etc."
              "Use only if it needs to be processed by an intermediate process." << std::endl;
        os << "rx_jsonpara [k] [v]\n sets value to v for additional json header parameter k to be streamed out with the zmq from receiver." << std::endl;

    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "receiver \t returns the status of receiver - can be running or idle" << std::endl;
        os << "framescaught \t returns the number of frames caught by receiver(average for multi)" << std::endl;
        os << "frameindex \t returns the current frame index of receiver(average for multi)" << std::endl;
        os << "r_readfreq \t returns the gui read frequency of the receiver" << std::endl;
        os << "tengiga \t returns 1 if the system is configured for 10Gbe else 0 for 1Gbe" << std::endl;
        os << "rx_fifodepth \t returns receiver fifo depth" << std::endl;
        os << "r_silent \t returns receiver silent mode enable. 1 is silent, 0 not silent." << std::endl;
        os << "r_framesperfile \t gets the number of frames per file in receiver. 0 means infinite or all frames in a single file." << std::endl;
        os << "r_discardpolicy \t gets the frame discard policy in the receiver. nodiscard (default) - discards nothing, discardempty - discard only empty frames, discardpartial(fastest) - discards all partial frames." << std::endl;
        os << "r_padding \t gets partial frames padding enable in the receiver. 0 does not pad partial frames(fastest), 1 (default) pads partial frames." << std::endl;
        os << "rx_jsonaddheader \n gets additional json header to be streamed "
              "out with the zmq from receiver." << std::endl;
        os << "rx_jsonpara [k] \n gets value of additional json header parameter k to be streamed out with the zmq from receiver. If empty, then no parameter found." << std::endl;

    }
    return os.str();
}

std::string slsDetectorCommand::helpPattern(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "pattern fname \t loads pattern file" << std::endl;
        os << "patword addr word \t writes pattern word - only very advanced users!" << std::endl;
        os << "patioctrl reg\t configures inputs/outputs of the chiptest board - only advanced users!" << std::endl;
        os << "patclkctrl reg\t configures output clk enable of the chiptest board- only advanced users! " << std::endl;
        os << "patlimits addr1 addr2\t defines pattern limits between addr1 and addr2" << std::endl;
        os << "patloop0 addr1 adrr2 \t configures the limits of the 0 loop " << std::endl;
        os << "patloop1 addr1 adrr2 \t configures the limits of the 1 loop " << std::endl;
        os << "patloop2 addr1 adrr2 \t configures the limits of the 2 loop " << std::endl;
        os << "patnloop0 n \t sets number of cycles of the 0 loop " << std::endl;
        os << "patnloop1 n \t sets number of cycles of the 1 loop " << std::endl;
        os << "patnloop2 n \t sets number of cycles of the 2 loop " << std::endl;
        os << "patwait0 addr \t configures pattern wait 0 address " << std::endl;
        os << "patwait1 addr \t configures pattern wait 1 address " << std::endl;
        os << "patwait2 addr \t configures pattern wait 2 address " << std::endl;
        os << "patwaittime0 nclk \t sets wait 0 waiting time in clock number " << std::endl;
        os << "patwaittime1 nclk \t sets wait 1 waiting time in clock number " << std::endl;
        os << "patwaittime2 nclk \t sets wait 2 waiting time in clock number " << std::endl;
        os << "adcinvert mask\t  sets the adcinversion mask (hex)" << std::endl;
        os << "adcdisable mask\t  sets the adcdisable mask (hex)" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "pattern \t cannot get" << std::endl;
        os << "patword \t cannot get" << std::endl;
        os << "patioctrl \t returns inputs/outputs of the chiptest board - only advanced users!" << std::endl;
        os << "patclkctrl\t returns output clk enable of the chiptest board- only advanced users! " << std::endl;
        os << "patlimits \t returns pattern limits between addr1 and addr2" << std::endl;
        os << "patloop0  \t returns the limits of the 0 loop " << std::endl;
        os << "patloop1  \t returns the limits of the 1 loop " << std::endl;
        os << "patloop2  \t returns the limits of the 2 loop " << std::endl;
        os << "patnloop0 \t returns the number of cycles of the 0 loop " << std::endl;
        os << "patnloop1 \t returns the number of cycles of the 1 loop " << std::endl;
        os << "patnloop2 \t  returns the number of cycles of the 2 loop " << std::endl;
        os << "patwait0 \t  returns the pattern wait 0 address " << std::endl;
        os << "patwait1 \t  returns the pattern wait 1 address " << std::endl;
        os << "patwait2 \t  returns the pattern wait 2 address " << std::endl;
        os << "patwaittime0 \t  returns the wait 0 waiting time in clock number " << std::endl;
        os << "patwaittime1 \t  returns the wait 1 waiting time in clock number " << std::endl;
        os << "patwaittime2 \t  returns the wait 2 waiting time in clock number " << std::endl;
        os << "adcinvert \t  returns the adcinversion mask " << std::endl;

        os << "adcdisable \t  returns the adcdisable mask " << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdPattern(int narg, char *args[], int action, int detPos) {

    if (action == HELP_ACTION)
        return helpPattern(action);
    /********

  Must implement set ctb functions in slsDetector and multiSlsDetector

	 **********/
    std::string fname;
    int addr, start, stop, n;
    uint64_t word, t;

    myDet->setOnline(ONLINE_FLAG, detPos);

    std::ostringstream os;
    if (cmd == "pattern") {
        //get fname fron stdin

        if (action == PUT_ACTION) {
            fname = std::string(args[1]);
            os << myDet->setCTBPattern(fname, detPos);
        } else if (action == GET_ACTION)
            os << "Cannot get";
    } else if (cmd == "patword") {

        if (action == PUT_ACTION) {
            //get addr, word from stdin

            if (narg < 3)
                return std::string("wrong usage: should specify both address and value (hexadecimal fomat) ");

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan address (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%lx", &word))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[2]);

            os << std::hex << myDet->setCTBWord(addr, word, detPos) << std::dec;
        } else if (action == GET_ACTION)
            os << "Cannot get";

    } else if (cmd == "patioctrl") {
        //get word from stdin

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%lx", &word))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[1]);

            myDet->setCTBWord(-1, word, detPos);
        }

        os << std::hex << myDet->setCTBWord(-1, -1, detPos) << std::dec;
    } else if (cmd == "patclkctrl") {
        //get word from stdin

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%lx", &word))
                ;
            else
                return std::string("Could not scan value  (hexadecimal fomat) ") + std::string(args[1]);

            myDet->setCTBWord(-2, word, detPos);
        }

        os << std::hex << myDet->setCTBWord(-2, -1, detPos) << std::dec;

    } else if (cmd == "patlimits") {
        //get start, stop from stdin
        if (action == PUT_ACTION) {
            if (narg < 3)
                return std::string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
            n = -1;
            if (sscanf(args[1], "%x", &start))
                ;
            else
                return std::string("Could not scan start address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%x", &stop))
                ;
            else
                return std::string("Could not scan stop address  (hexadecimal fomat) ") + std::string(args[2]);

            myDet->setCTBPatLoops(-1, start, stop, n, detPos);
        }

        start = -1;
        stop = -1;
        n = -1;
        myDet->setCTBPatLoops(-1, start, stop, n, detPos);
        os << std::hex << start << " " << stop; // << " "<< std::dec << n ;
    } else if (cmd == "patloop0") {
        //get start, stop from stdin

        //get start, stop from stdin
        if (action == PUT_ACTION) {
            if (narg < 3)
                return std::string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
            n = -1;
            if (sscanf(args[1], "%x", &start))
                ;
            else
                return std::string("Could not scan start address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%x", &stop))
                ;
            else
                return std::string("Could not scan stop address  (hexadecimal fomat) ") + std::string(args[2]);

            myDet->setCTBPatLoops(0, start, stop, n, detPos);
        }

        start = -1;
        stop = -1;
        n = -1;
        myDet->setCTBPatLoops(0, start, stop, n, detPos);
        os << std::hex << start << " " << stop; // << " "<< std::dec << n ;

    } else if (cmd == "patloop1") {

        //get start, stop from stdin
        if (action == PUT_ACTION) {
            if (narg < 3)
                return std::string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
            n = -1;
            if (sscanf(args[1], "%x", &start))
                ;
            else
                return std::string("Could not scan start address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%x", &stop))
                ;
            else
                return std::string("Could not scan stop address  (hexadecimal fomat) ") + std::string(args[2]);

            myDet->setCTBPatLoops(1, start, stop, n, detPos);
        }

        start = -1;
        stop = -1;
        n = -1;
        myDet->setCTBPatLoops(1, start, stop, n, detPos);
        os << std::hex << start << " " << stop; // << " "<< std::dec << n ;

    } else if (cmd == "patloop2") {

        //get start, stop from stdin
        if (action == PUT_ACTION) {
            if (narg < 3)
                return std::string("wrong usage: should specify both start and stop address (hexadecimal fomat) ");
            n = -1;
            if (sscanf(args[1], "%x", &start))
                ;
            else
                return std::string("Could not scan start address  (hexadecimal fomat) ") + std::string(args[1]);

            if (sscanf(args[2], "%x", &stop))
                ;
            else
                return std::string("Could not scan stop address  (hexadecimal fomat) ") + std::string(args[2]);

            myDet->setCTBPatLoops(2, start, stop, n, detPos);
        }

        start = -1;
        stop = -1;
        n = -1;
        myDet->setCTBPatLoops(2, start, stop, n, detPos);
        os << std::hex << start << " " << stop << std::dec; // << " "<< std::dec << n ;

    } else if (cmd == "patnloop0") {
        start = -1;
        stop = -1;

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%d", &n))
                ;
            else
                return std::string("Could not scan number of loops ") + std::string(args[1]);

            myDet->setCTBPatLoops(0, start, stop, n, detPos);
        }

        start = -1;
        stop = -1;
        n = -1;
        myDet->setCTBPatLoops(0, start, stop, n, detPos);
        os << n;
    } else if (cmd == "patnloop1") {

        start = -1;
        stop = -1;

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%d", &n))
                ;
            else
                return std::string("Could not scan number of loops ") + std::string(args[1]);

            myDet->setCTBPatLoops(1, start, stop, n, detPos);
        }

        start = -1;
        stop = -1;
        n = -1;
        myDet->setCTBPatLoops(1, start, stop, n, detPos);
        os << n;

    } else if (cmd == "patnloop2") {

        start = -1;
        stop = -1;

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%d", &n))
                ;
            else
                return std::string("Could not scan number of loops ") + std::string(args[1]);

            myDet->setCTBPatLoops(2, start, stop, n, detPos);
        }

        start = -1;
        stop = -1;
        n = -1;
        myDet->setCTBPatLoops(2, start, stop, n, detPos);
        os << n;

    } else if (cmd == "patwait0") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan wait address (hex format)") + std::string(args[1]);

            myDet->setCTBPatWaitAddr(0, addr, detPos);
        }

        os << std::hex << myDet->setCTBPatWaitAddr(0, -1, detPos) << std::dec ;

    } else if (cmd == "patwait1") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan wait address (hex format)") + std::string(args[1]);

            myDet->setCTBPatWaitAddr(1, addr, detPos);
        }

        os << std::hex << myDet->setCTBPatWaitAddr(1, -1, detPos) << std::dec ;

    } else if (cmd == "patwait2") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan wait address (hex format)") + std::string(args[1]);

            myDet->setCTBPatWaitAddr(2, addr, detPos);
        }

        os << std::hex << myDet->setCTBPatWaitAddr(2, -1, detPos) << std::dec ;

    } else if (cmd == "patwaittime0") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%ld", &t))
                ;
            else
                return std::string("Could not scan wait time") + std::string(args[1]);

            myDet->setCTBPatWaitTime(0, t, detPos);
        }

        os << myDet->setCTBPatWaitTime(0, -1, detPos);

    } else if (cmd == "patwaittime1") {

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%ld", &t))
                ;
            else
                return std::string("Could not scan wait time ") + std::string(args[1]);

            myDet->setCTBPatWaitTime(1, t, detPos);
        }

        os << myDet->setCTBPatWaitTime(1, -1, detPos);

    } else if (cmd == "patwaittime2") {
        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%ld", &t))
                ;
            else
                return std::string("Could not scan wait time ") + std::string(args[1]);

            myDet->setCTBPatWaitTime(2, t, detPos);
        }

        os << myDet->setCTBPatWaitTime(2, -1, detPos);

    } else if (cmd == "adcinvert") {
        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan adcinvert reg ") + std::string(args[1]);

            myDet->writeRegister(67, addr, detPos);
        }

        os << std::hex << myDet->readRegister(67, detPos) << std::dec ;

    } else if (cmd == "dut_clk") {
        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan dut_clk reg ") + std::string(args[1]);

            myDet->writeRegister(123, addr, detPos); //0x7b
        }

        os << std::hex << myDet->readRegister(123, detPos) << std::dec ; //0x7b
    } else if (cmd == "adcdisable") {

        int nroi = 0;
        ROI roiLimits[MAX_ROIS];

        if (action == PUT_ACTION) {

            if (sscanf(args[1], "%x", &addr))
                ;
            else
                return std::string("Could not scan adcdisable reg ") + std::string(args[1]);

            /******USE ROI?!?!?!?*********/
            // roiLimits[i].xmin;roiLimits[i].xmax;roiLimits[i].ymin;roiLimits[i].ymin;roiLimits[i].ymax
            //int mask=1;
            int ii = 0;
            while (ii < 32) {
                ++nroi;
                roiLimits[nroi - 1].xmin = ii;
                roiLimits[nroi - 1].ymin = 0;
                roiLimits[nroi - 1].ymax = 0;
                while ((addr & (1 << ii))) {
                    ++ii;
                    if (ii >= 32)
                        break;
                }
                if (ii >= 32) {
                    break;
                    std::cout << "ROI " << nroi << " xmin " << roiLimits[nroi - 1].xmin << " xmax " << roiLimits[nroi - 1].xmax << std::endl;
                    roiLimits[nroi - 1].xmax = 31;
                    break;
                }
                roiLimits[nroi - 1].xmin = ii;
                while ((addr & (1 << ii)) == 0) {
                    ++ii;
                    if (ii >= 32)
                        break;
                }
                roiLimits[nroi - 1].xmax = ii - 1;
                if (ii >= 32) {
                    std::cout << "ROI " << nroi << " xmin " << roiLimits[nroi - 1].xmin << " xmax " << roiLimits[nroi - 1].xmax << std::endl;
                    ++nroi;
                    break;
                }
                std::cout << "ROI " << nroi << " xmin " << roiLimits[nroi - 1].xmin << " xmax " << roiLimits[nroi - 1].xmax << std::endl;
            }
            std::cout << "********ROI " << nroi << std::endl;
            myDet->setROI(nroi - 1, roiLimits, detPos);
            //  myDet->writeRegister(94,addr, detPos);
            // myDet->writeRegister(120,addr, detPos);
        }

        ROI *aa = myDet->getROI(nroi, detPos);

        int reg = 0xffffffff;
        if (nroi < 1)
            reg = 0;
        else {
            for (int iroi = 0; iroi < nroi; ++iroi) {
                std::cout << iroi << " xmin " << (aa + iroi)->xmin << " xmax " << (aa + iroi)->xmax << std::endl;
                for (int ich = (aa + iroi)->xmin; ich <= (aa + iroi)->xmax; ++ich) {
                    reg &= ~(1 << ich);
                }
            }
        }
        os << std::hex << reg << std::dec ;
        if (aa != NULL)
            delete [] aa;
        //os <<" "<< std::hex << myDet->readRegister(120, detPos) << std::dec ;

    }

    else
        return helpPattern(action);

    return os.str();
}

std::string slsDetectorCommand::helpPulse(int action) {

    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "pulse [n] [x] [y] \t pulses pixel at coordinates (x,y) n number of times" << std::endl;
        os << "pulsenmove [n] [x] [y]\t pulses pixel n number of times and moves relatively by x value (x axis) and y value(y axis)" << std::endl;
        os << "pulsechip [n] \t pulses chip n number of times, while n=-1 will reset it to normal mode" << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "pulse \t cannot get" << std::endl;
        os << "pulsenmove \t cannot get" << std::endl;
        os << "pulsechip \t cannot get" << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdPulse(int narg, char *args[], int action, int detPos) {
    int retval = FAIL;

    if (action == HELP_ACTION)
        return helpPulse(action);
    else if (action == GET_ACTION)
        return std::string("cannot get ") + cmd;

    myDet->setOnline(ONLINE_FLAG, detPos);

    int ival1 = -1;
    if (!sscanf(args[1], "%d", &ival1))
        return std::string("Could not scan 1st argument ") + std::string(args[1]);

    if (std::string(args[0]) == std::string("pulsechip"))
        retval = myDet->pulseChip(ival1, detPos);

    else {
        //next commands requires 3 addnl. arguments
        int ival2 = -1, ival3 = -1;
        if (narg < 4)
            return std::string("insufficient arguments:\n" + helpPulse(action));
        if (!sscanf(args[2], "%d", &ival2))
            return std::string("Could not scan 2nd argument ") + std::string(args[2]);
        if (!sscanf(args[3], "%d", &ival3))
            return std::string("Could not scan 3rd argument ") + std::string(args[3]);

        if (std::string(args[0]) == std::string("pulse"))
            retval = myDet->pulsePixel(ival1, ival2, ival3, detPos);

        else if (std::string(args[0]) == std::string("pulsenmove"))
            retval = myDet->pulsePixelNMove(ival1, ival2, ival3, detPos);

        else
            return std::string("could not decode command") + cmd;
    }

    if (retval == OK)
        return std::string(" successful");
    else
        return std::string(" unsuccessful");
}


std::string slsDetectorCommand::helpProcessor(int action) {
    std::ostringstream os;
    if (action == PUT_ACTION || action == HELP_ACTION) {
        os << "json_emin [t]\n sets value to t for minimum threshold (emin) in additional json header to be streamed out with the zmq from receiver. For Moench." << std::endl;
        os << "json_emax [t]\n sets value to t for maximum threshold (emax) in additional json header to be streamed out with the zmq from receiver. For Moench." << std::endl;
        os << "json_framemode [s]\n sets readoutmode for the Moench (soft setting). It is only set in the json header for the processor. Options: pedestal, newpedestal, flatfield, newflatfield, frame. " << std::endl;
        os << "json_detectormode [s]\n sets detector mode for the Moench (soft setting). It is only set in the json header for the processor.Options: analog, counting, interpolating. " << std::endl;
    }
    if (action == GET_ACTION || action == HELP_ACTION) {
        os << "json_emin \n gets value of minimum threshold (emin) in additional json header to be streamed out with the zmq from receiver. If no parameter found, it returns empty string. For Moench." << std::endl;
        os << "json_emin \n gets value of maximum threshold (emax) in additional json header to be streamed out with the zmq from receiver. If no parameter found, it returns empty string. For Moench." << std::endl;
        os << "json_framemode [s]\n gets readoutmode for the Moench (soft setting). It is only set in the json header for the processor. Options: pedestal, newpedestal, flatfield, newflatfield, frame. " << std::endl;
        os << "json_detectormode [s]\n gets detector mode for the Moench (soft setting). It is only set in the json header for the processor.Options: analog, counting, interpolating. " << std::endl;
    }
    return os.str();
}

std::string slsDetectorCommand::cmdProcessor(int narg, char *args[], int action, int detPos) {
    if (action == HELP_ACTION)
        return helpProcessor(action);

    myDet->setOnline(ONLINE_FLAG, detPos);
    myDet->setReceiverOnline(ONLINE_FLAG, detPos);

    int imode = getJsonHeaderParameterTypeAsEnum(cmd);
    jsonHeaderParameterType mode = JSON_EMIN;
    if (imode != -1)
        mode = (jsonHeaderParameterType)imode;
    int ival = -1;

    if (cmd == "json_emin" || cmd == "json_emax") {
        if (action == PUT_ACTION) {
            if (!sscanf(args[1],"%d", &ival))
                return std::string("cannot scan value ") + std::string(args[1]) + std::string(" for command ") + cmd;
            myDet->setAdditionalJsonSpecificParameter(mode, ival);
        }
        return std::to_string(myDet->getAdditionalJsonSpecificParameter(mode));
    }

    else if (cmd == "json_framemode" || cmd == "json_detectormode") {
        if (action == PUT_ACTION) {
            ival = getJsonHeaderParameterValuesAsEnum(args[1]);
            if (ival == -1)
                return std::string("cannot scan value ") + std::string(args[1]) + std::string(" for command ") + cmd;
            myDet->setAdditionalJsonSpecificParameter(mode, ival);
        }
        int retval = myDet->getAdditionalJsonSpecificParameter(mode);
        if (retval == -1)
            return std::string("unknown");
        return getJsonHeaderParameterValueAsString((jsonHeaderParameterValuesType)retval);
    }

   return std::string("could not decode command");
}

