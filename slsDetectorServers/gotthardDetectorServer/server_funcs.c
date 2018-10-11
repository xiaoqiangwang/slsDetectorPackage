#include "sls_detector_defs.h"
#include "server_funcs.h"
#include "server_defs.h"
#include "firmware_funcs.h"
#include "registers_g.h"
#include "gitInfoGotthard.h"
#include "AD9257.h"     // include "commonServerFunctions.h"
#include "versionAPI.h"

#define FIFO_DATA_REG_OFF     0x50<<11
#define CONTROL_REG           0x24<<11
// Global variables

int (*flist[256])(int);

//defined in the detector specific file
#ifdef GOTTHARDD
const enum detectorType myDetectorType=GOTTHARD;
#else
const enum detectorType myDetectorType=GENERIC;
#endif


extern int  storeInRAM;

extern int lockStatus;
extern char lastClientIP[INET_ADDRSTRLEN];
extern char thisClientIP[INET_ADDRSTRLEN];
extern int differentClients;

/* global variables for optimized readout */

char mess[MAX_STR_LENGTH];
int digitalTestBit = 0;


int init_detector( int b) {

	if (mapCSP0()==FAIL) {
		printf("Could not map memory\n");
		exit(-1);
	}

	//confirm if it is really gotthard
	if (((bus_r(PCB_REV_REG)  & DETECTOR_TYPE_MASK)>> DETECTOR_TYPE_OFFSET) == MOENCH_MODULE){
		printf("This is a MOENCH detector. Exiting Gotthard Server.\n\n");
		exit(-1);
	}

	if (b) {
		printf("***This is a GOTTHARD detector with %d chips per module***\n", NCHIP);
		printf("\nBoard Revision:0x%x\n",(bus_r(PCB_REV_REG)&BOARD_REVISION_MASK));
		initDetector();
		printf("Initializing Detector\n");
	}
	strcpy(mess,"dummy message");
	strcpy(lastClientIP,"none");
	strcpy(thisClientIP,"none1");
	lockStatus=0;
	return OK;
}


int decode_function(int file_des) {
	int fnum,n;
	int retval=FAIL;
#ifdef VERBOSE
	printf( "receive data\n");
#endif 
	n = receiveDataOnly(file_des,&fnum,sizeof(fnum));
	if (n <= 0) {
#ifdef VERBOSE
		printf("ERROR reading from socket %d, %d %d\n", n, fnum, file_des);
#endif
		return FAIL;
	}
#ifdef VERBOSE
	else
		printf("size of data received %d\n",n);
#endif

#ifdef VERBOSE
	printf( "calling function fnum = %d %x\n",fnum,(unsigned int)(flist[fnum]));
#endif
	if (fnum<0 || fnum>255)
		fnum=255;
	retval=(*flist[fnum])(file_des);
	if (retval==FAIL)
		printf( "Error executing the function = %d \n",fnum);
	return retval;
}


int function_table() {
	int i;
	for (i=0;i<256;i++){
		flist[i]=&M_nofunc;
	}
	flist[F_EXEC_COMMAND]						= &exec_command;
	flist[F_GET_DETECTOR_TYPE]					= &get_detector_type;
	flist[F_SET_EXTERNAL_SIGNAL_FLAG]			= &set_external_signal_flag;
	flist[F_SET_EXTERNAL_COMMUNICATION_MODE]	= &set_external_communication_mode;
	flist[F_GET_ID]								= &get_id;
	flist[F_DIGITAL_TEST]						= &digital_test;
	flist[F_SET_DAC]							= &set_dac;
	flist[F_GET_ADC]							= &get_adc;
	flist[F_WRITE_REGISTER]						= &write_register;
	flist[F_READ_REGISTER]						= &read_register;
	flist[F_SET_MODULE]							= &set_module;
	flist[F_GET_MODULE]							= &get_module;
	flist[F_SET_SETTINGS]						= &set_settings;
	flist[F_GET_THRESHOLD_ENERGY]				= &M_nofunc;
	flist[F_START_ACQUISITION]					= &start_acquisition;
	flist[F_STOP_ACQUISITION]					= &stop_acquisition;
	flist[F_START_READOUT]						= &start_readout;
	flist[F_GET_RUN_STATUS]						= &get_run_status;
	flist[F_START_AND_READ_ALL]					= &start_and_read_all;
	flist[F_READ_ALL]							= &read_all;
	flist[F_SET_TIMER]							= &set_timer;
	flist[F_GET_TIME_LEFT]						= &get_time_left;
	flist[F_SET_DYNAMIC_RANGE]					= &set_dynamic_range;
	flist[F_SET_READOUT_FLAGS]					= &set_readout_flags;
	flist[F_SET_ROI]							= &set_roi;
	flist[F_SET_SPEED]							= &set_speed;
	flist[F_EXIT_SERVER]						= &exit_server;
	flist[F_LOCK_SERVER]						= &lock_server;
	flist[F_GET_LAST_CLIENT_IP]					= &get_last_client_ip;
	flist[F_SET_PORT]							= &set_port;
	flist[F_UPDATE_CLIENT]						= &update_client;
	flist[F_CONFIGURE_MAC]						= &configure_mac;
	flist[F_LOAD_IMAGE]							= &load_image;
	flist[F_READ_COUNTER_BLOCK]					= &read_counter_block;
	flist[F_RESET_COUNTER_BLOCK]				= &reset_counter_block;
	flist[F_CALIBRATE_PEDESTAL]					= &M_nofunc;
	flist[F_ENABLE_TEN_GIGA]					= &M_nofunc;
	flist[F_SET_ALL_TRIMBITS]					= &M_nofunc;
	flist[F_SET_CTB_PATTERN]					= &M_nofunc;
	flist[F_WRITE_ADC_REG]						= &write_adc_register;
	flist[F_SET_COUNTER_BIT]					= &M_nofunc;
	flist[F_PULSE_PIXEL]						= &M_nofunc;
	flist[F_PULSE_PIXEL_AND_MOVE]				= &M_nofunc;
	flist[F_PULSE_CHIP]							= &M_nofunc;
	flist[F_SET_RATE_CORRECT]					= &M_nofunc;
	flist[F_GET_RATE_CORRECT]					= &M_nofunc;
	flist[F_SET_NETWORK_PARAMETER]				= &M_nofunc;
	flist[F_PROGRAM_FPGA]						= &M_nofunc;
	flist[F_RESET_FPGA]							= &M_nofunc;
	flist[F_POWER_CHIP]							= &M_nofunc;
	flist[F_ACTIVATE]							= &M_nofunc;
	flist[F_PREPARE_ACQUISITION]				= &M_nofunc;
	flist[F_THRESHOLD_TEMP]                     = &M_nofunc;
	flist[F_TEMP_CONTROL]                       = &M_nofunc;
	flist[F_TEMP_EVENT]                         = &M_nofunc;
	flist[F_AUTO_COMP_DISABLE]                  = &M_nofunc;
	flist[F_STORAGE_CELL_START]                 = &M_nofunc;
	flist[F_CHECK_VERSION]                 		= &check_version;
	flist[F_SOFTWARE_TRIGGER]                 	= &M_nofunc;
	return OK;
}


int  M_nofunc(int file_des){

	int ret=FAIL;
	int n = 1;
	while (n > 0)
		n = receiveData(file_des,mess,MAX_STR_LENGTH,OTHER);

	sprintf(mess,"Unrecognized Function. Please do not proceed.\n");
	cprintf(BG_RED,"Error: %s",mess);

	sendDataOnly(file_des,&ret,sizeof(ret));
	sendDataOnly(file_des,mess,sizeof(mess));
	return GOODBYE;
}





int exec_command(int file_des) {
	char cmd[MAX_STR_LENGTH];
	char answer[MAX_STR_LENGTH];
	int retval=OK;
	int sysret=0;
	int n=0;

	/* receive arguments */
	n = receiveDataOnly(file_des,cmd,MAX_STR_LENGTH);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		retval=FAIL;
	}

	/* execute action if the arguments correctly arrived*/
	if (retval==OK) {
#ifdef VERBOSE
		printf("executing command %s\n", cmd);
#endif
		if (lockStatus==0 || differentClients==0)
			sysret=system(cmd);

		//should be replaced by popen
		if (sysret==0) {
			sprintf(answer,"Succeeded\n");
			if (lockStatus==1 && differentClients==1)
				sprintf(answer,"Detector locked by %s\n", lastClientIP);
		} else {
			sprintf(answer,"Failed\n");
			retval=FAIL;
		}
	} else {
		sprintf(answer,"Could not receive the command\n");
	}

	/* send answer */
	n = sendDataOnly(file_des,&retval,sizeof(retval));
	n = sendDataOnly(file_des,answer,MAX_STR_LENGTH);
	if (n < 0) {
		sprintf(mess,"Error writing to socket");
		retval=FAIL;
	}


	/*return ok/fail*/
	return retval;

}



int get_detector_type(int file_des) {
	int n=0;
	enum detectorType retval;
	int ret = OK;

	/* execute action */
	retval=myDetectorType;

#ifdef VERBOSE
	printf("Returning detector type %d\n",ret);
#endif

	if (differentClients==1)
		ret=FORCE_UPDATE;

	/* send OK/failed */
	n += sendDataOnly(file_des,&ret,sizeof(ret));
	/* send return argument */
	n += sendDataOnly(file_des,&retval,sizeof(retval));

	/*return ok/fail*/
	return retval;


}



int set_external_signal_flag(int file_des) {
	int n;
	int arg[2];
	int ret=OK;
	int signalindex;
	enum externalSignalFlag flag, retval;

	sprintf(mess,"Can't set external signal flag\n");

	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	retval=SIGNAL_OFF;
	if (ret==OK) {
		signalindex=arg[0];
		flag=arg[1];
		/* execute action */
		switch (flag) {
		case GET_EXTERNAL_SIGNAL_FLAG:
			retval=getExtSignal(signalindex);
			break;

		default:
			if (lockStatus && differentClients) {
				ret=FAIL;
				sprintf(mess,"Detector locked by %s\n", lastClientIP);
			} else if (signalindex > 0) {
				ret=FAIL;
				sprintf(mess,"Signal index %d is reserved. Only index 0 can be configured.\n", signalindex);
			} else {
				retval=setExtSignal(flag);
			}
		}
#ifdef VERBOSE
		printf("Setting external signal %d to flag %d\n",signalindex,flag );
		printf("Set to flag %d\n",retval);
#endif

	}

	if (ret==OK && differentClients!=0)
		ret=FORCE_UPDATE;


	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}


	/*return ok/fail*/
	return ret;

}


int set_external_communication_mode(int file_des) {
	int n;
	enum externalCommunicationMode arg, ret=GET_EXTERNAL_COMMUNICATION_MODE;
	int retval=OK;

	sprintf(mess,"Can't set external communication mode\n");


	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		retval=FAIL;
	}

	if (retval==OK) {
		/* execute action */

		ret=setTiming(arg);


#ifdef VERBOSE
		printf("Setting external communication mode to %d\n", arg);
#endif
	} else
		ret=FAIL;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&retval,sizeof(retval));
	if (retval!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&ret,sizeof(ret));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return retval;
}



int get_id(int file_des) {
	// sends back 64 bits!
	int64_t retval=-1;
	int ret=OK;
	int n=0;
	enum idMode arg;

	sprintf(mess,"Can't return id\n");

	/* receive arguments */
	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef VERBOSE
	printf("Getting id %d\n", arg);
#endif

	switch (arg) {
	case DETECTOR_SERIAL_NUMBER:
		retval=getDetectorNumber();
		break;
	case DETECTOR_FIRMWARE_VERSION:
		retval = (getFirmwareVersion() & 0xFFFFFF);
		break;
	case DETECTOR_SOFTWARE_VERSION:
		retval = (GITDATE & 0xFFFFFF);
		break;
	case CLIENT_SOFTWARE_API_VERSION:
		return APIGOTTHARD;
	default:
		printf("Required unknown id %d \n", arg);
		ret=FAIL;
		retval=FAIL;
		break;
	}

#ifdef VERBOSE
	printf("Id is %llx\n", retval);
#endif  

	if (differentClients==1)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}

int digital_test(int file_des) {

	int retval;
	int ret=OK;
	int n=0;
	int ival;
	enum digitalTestMode arg;

	sprintf(mess,"Can't send digital test\n");

	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef VERBOSE
	printf("Digital test mode %d\n",arg );
#endif  

	switch (arg) {
	case DETECTOR_FIRMWARE_TEST:
		retval=testFpga();
		break;
	case DETECTOR_BUS_TEST:
		retval=testBus();
		break;
	case DIGITAL_BIT_TEST:
		n = receiveDataOnly(file_des,&ival,sizeof(ival));
		if (n < 0) {
			sprintf(mess,"Error reading from socket\n");
			retval=FAIL;
		}
#ifdef VERBOSE
		printf("with value %d\n", ival);
#endif
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
			break;
		}
		digitalTestBit = ival;
		retval=digitalTestBit;
		break;
	default:
		printf("Unknown digital test required %d\n",arg);
		ret=FAIL;
		retval=FAIL;
		break;
	}

#ifdef VERBOSE
	printf("digital test result is 0x%x\n", retval);
#endif  
	//Always returns force update such that the dynamic range is always updated on the client

	// if (differentClients==1 && ret==OK)
	ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}




int set_dac(int file_des) {
	//default:all mods
	int retval[2];retval[1]=-1;
	int temp;
	int ret=OK;
	int arg[2];
	enum dacIndex ind;
	int n;
	int val;
	int mV;
	int idac=0;

	sprintf(mess,"Can't set DAC\n");

	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ind=arg[0];
	mV=arg[1];

	n = receiveDataOnly(file_des,&val,sizeof(val));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef VERBOSE
	printf("Setting DAC %d to %d V\n", ind, val);
#endif 

	switch (ind) {
	case G_VREF_DS :
		idac=VREF_DS;
		break;
	case G_VCASCN_PB:
		idac=VCASCN_PB;
		break;
	case G_VCASCP_PB:
		idac=VCASCP_PB;
		break;
	case G_VOUT_CM:
		idac=VOUT_CM;
		break;
	case G_VCASC_OUT:
		idac=VCASC_OUT;
		break;
	case G_VIN_CM:
		idac=VIN_CM;
		break;
	case G_VREF_COMP:
		idac=VREF_COMP;
		break;
	case G_IB_TESTC:
		idac=IB_TESTC;
		break;
	case HV_POT:
		idac=HIGH_VOLTAGE;
		break;

	default:
		printf("Unknown DAC index %d\n",ind);
		sprintf(mess,"Unknown DAC index %d\n",ind);
		ret=FAIL;
		break;
	}

	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else{
			if(idac==HIGH_VOLTAGE){
				retval[0]=initHighVoltage(val);
				ret=FAIL;
				if(retval[0]==-2)
					strcpy(mess,"Invalid Voltage.Valid values are 0,90,110,120,150,180,200");
				else if(retval[0]==-3)
					strcpy(mess,"Weird value read back or it has not been set yet\n");
				else
					ret=OK;
			}else{
				setDAC(idac,val,mV,retval);
				ret=FAIL;
				if(mV)
					temp = retval[1];
				else
					temp = retval[0];
				if ((abs(temp-val)<=3) || val==-1) {
					ret=OK;
#ifdef VERBOSE
					printf("DAC set to %d  in dac units and %d mV\n",  retval[0],retval[1]);
#endif
				}
			}
		}
	}


	if(ret==FAIL)
		printf("Setting dac %d: wrote %d but read %d\n", ind, val, temp);
	else{
		if (differentClients)
			ret=FORCE_UPDATE;
	}


	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}


	/*return ok/fail*/
	return ret;

}



int get_adc(int file_des) {
	//default: mod 0
	int retval;
	int ret=OK;
	int arg;
	enum dacIndex ind;
	int n;
	int idac=0;

	sprintf(mess,"Can't read ADC\n");


	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	ind=arg;

#ifdef VERBOSE
	printf("Getting ADC %d\n", ind);
#endif

	switch (ind) {
	case TEMPERATURE_FPGA:
		idac=TEMP_FPGA;
		break;
	case TEMPERATURE_ADC:
		idac=TEMP_ADC;
		break;
	default:
		printf("Unknown DAC index %d\n",ind);
		sprintf(mess,"Unknown DAC index %d\n",ind);
		ret=FAIL;
		break;
	}

	if (ret==OK)
		retval=getTemperature(idac);

#ifdef VERBOSE
	printf("ADC is %d V\n",  retval);
#endif  
	if (ret==FAIL) {
		printf("Getting adc %d failed\n", ind);
	}

	if (differentClients)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}




int write_register(int file_des) {

	int retval;
	int ret=OK;
	int arg[2];
	int addr, val;
	int n;
	u_int32_t address;

	sprintf(mess,"Can't write to register\n");

	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	addr=arg[0];
	val=arg[1];

#ifdef VERBOSE
	printf("writing to register 0x%x data 0x%x\n", addr, val);
#endif

	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}


	if(ret!=FAIL){
		address=(addr<<11);
		if((address==FIFO_DATA_REG_OFF)||(address==CONTROL_REG))
			ret = bus_w16(address,val);
		else
			ret=bus_w(address,val);
		if(ret==OK){
			if((address==FIFO_DATA_REG_OFF)||(address==CONTROL_REG))
				retval=bus_r16(address);
			else
				retval=bus_r(address);
		}
	}


#ifdef VERBOSE
	printf("Data set to 0x%x\n",  retval);
#endif
	if (retval==val) {
		ret=OK;
		if (differentClients)
			ret=FORCE_UPDATE;
	} else {
		ret=FAIL;
		sprintf(mess,"Writing to register 0x%x failed: wrote 0x%x but read 0x%x\n", addr, val, retval);
	}

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}

int read_register(int file_des) {

	int retval;
	int ret=OK;
	int arg;
	int addr;
	int n;
	u_int32_t address;

	sprintf(mess,"Can't read register\n");

	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	addr=arg;



	//#ifdef VERBOSE
	printf("reading  register 0x%x\n", addr);
	//#endif

	if(ret!=FAIL){
		address=(addr<<11);
		if((address==FIFO_DATA_REG_OFF)||(address==CONTROL_REG))
			retval=bus_r16(address);
		else
			retval=bus_r(address);
	}



#ifdef VERBOSE
	printf("Returned value 0x%x\n",  retval);
#endif
	if (ret==FAIL) {
		ret=FAIL;
		printf("Reading register 0x%x failed\n", addr);
	} else if (differentClients)
		ret=FORCE_UPDATE;


	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;

}


int set_module(int file_des) {
	sls_detector_module myModule;
	int *myChip=malloc(NCHIP*sizeof(int));
	int *myChan=malloc(NCHIP*NCHAN*sizeof(int));
	int *myDac=malloc(NDAC*sizeof(int));/**dhanya*/
	int *myAdc=malloc(NADC*sizeof(int));/**dhanya*/
	int retval, n;
	int ret=OK;
	int dr;// ow;

	dr=DYNAMIC_RANGE;

	if (myDac)
		myModule.dacs=myDac;
	else {
		sprintf(mess,"could not allocate dacs\n");
		ret=FAIL;
	}
	if (myAdc)
		myModule.adcs=myAdc;
	else {
		sprintf(mess,"could not allocate adcs\n");
		ret=FAIL;
	}
	if (myChip)
		myModule.chipregs=myChip;
	else {
		sprintf(mess,"could not allocate chips\n");
		ret=FAIL;
	}
	if (myChan)
		myModule.chanregs=myChan;
	else {
		sprintf(mess,"could not allocate chans\n");
		ret=FAIL;
	}

	myModule.ndac=NDAC;
	myModule.nchip=NCHIP;
	myModule.nchan=NCHAN*NCHIP;
	myModule.nadc=NADC;


#ifdef VERBOSE
	printf("Setting module\n");
#endif 
	ret=receiveModule(file_des, &myModule);

	if (ret>=0)
		ret=OK;
	else
		ret=FAIL;


#ifdef VERBOSE
	printf("module number is %d,register is %d, nchan %d, nchip %d, ndac %d, nadc %d, gain %f, offset %f\n",myModule.module, myModule.reg, myModule.nchan, myModule.nchip, myModule.ndac,  myModule.nadc, myModule.gain,myModule.offset);
#endif


	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else {
			retval=setModule(myModule);
		}
	}

	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* Maybe this is done inside the initialization funcs */
	//copyChip(detectorChips[myChip.module]+(myChip.chip), &myChip);

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	free(myChip);
	free(myChan);
	free(myDac);
	free(myAdc);

	//  setDynamicRange(dr);  always 16 commented out


	return ret;
}




int get_module(int file_des) {


	int ret=OK;
	int n;



	sls_detector_module myModule;
	int *myChip=malloc(NCHIP*sizeof(int));
	int *myChan=malloc(NCHIP*NCHAN*sizeof(int));
	int *myDac=malloc(NDAC*sizeof(int));
	int *myAdc=malloc(NADC*sizeof(int));


	if (myDac)
		myModule.dacs=myDac;
	else {
		sprintf(mess,"could not allocate dacs\n");
		ret=FAIL;
	}
	if (myAdc)
		myModule.adcs=myAdc;
	else {
		sprintf(mess,"could not allocate adcs\n");
		ret=FAIL;
	}
	if (myChip)
		myModule.chipregs=myChip;
	else {
		sprintf(mess,"could not allocate chips\n");
		ret=FAIL;
	}
	if (myChan)
		myModule.chanregs=myChan;
	else {
		sprintf(mess,"could not allocate chans\n");
		ret=FAIL;
	}

	myModule.ndac=NDAC;
	myModule.nchip=NCHIP;
	myModule.nchan=NCHAN*NCHIP;
	myModule.nadc=NADC;

	if (ret==OK) {
			getModule(&myModule);

#ifdef VERBOSE
			printf("Returning module register %x\n", myModule.reg);
#endif 

	}

	if (differentClients==1 && ret==OK)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		ret=sendModule(file_des, &myModule);
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}



	free(myChip);
	free(myChan);
	free(myDac);
	free(myAdc);


	/*return ok/fail*/
	return ret;

}



int set_settings(int file_des) {

	int retval;
	int ret=OK;
	int arg;
	int n;
	enum detectorSettings isett;


	n = receiveDataOnly(file_des,&arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}
	isett=arg;


#ifdef VERBOSE
	printf("Changing settings to %d\n",  isett);
#endif 

	if (differentClients==1 && lockStatus==1 && arg!=GET_SETTINGS) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		switch(isett) {
		case GET_SETTINGS:
		case UNINITIALIZED:
		case DYNAMICGAIN:
		case HIGHGAIN:
		case LOWGAIN:
		case MEDIUMGAIN:
		case VERYHIGHGAIN:
			break;
		default:
			ret = FAIL;
			sprintf(mess,"Setting (%d) is not implemented for this detector.\n"
					"Options are dynamicgain, highgain, lowgain, mediumgain and "
					"veryhighgain.\n", isett);
			cprintf(RED, "Warning: %s", mess);
			break;
		}
		if (ret != FAIL) {
			retval=setSettings(isett);
#ifdef VERBOSE
			printf("Settings changed to %d\n",retval);
#endif
			if (retval != isett && isett >= 0) {
				ret=FAIL;
				sprintf(mess, "Changing settings: wrote %d but read %d\n", isett, retval);
				printf("Warning: %s",mess);
			}

			else {
				ret = setDefaultDacs();
				if (ret == FAIL) {
					strcpy(mess,"Could change settings, but could not set to default dacs\n");
					cprintf(RED, "Warning: %s", mess);
				}
			}
		}
	}
	if (ret==OK && differentClients==1)
		ret=FORCE_UPDATE;

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	} else
		n += sendDataOnly(file_des,&retval,sizeof(retval));



	return ret;


}




int start_acquisition(int file_des) {

	int ret=OK;
	int n;


	sprintf(mess,"can't start acquisition\n");

#ifdef VERBOSE
	printf("Starting acquisition\n");
#endif

	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		ret=startStateMachine();
	}
	if (ret==FAIL)
		sprintf(mess,"Start acquisition failed\n");
	else if (differentClients)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	return ret;

}

int stop_acquisition(int file_des) {

	int ret=OK;
	int n;

	sprintf(mess,"can't stop acquisition\n");

	cprintf(BG_RED,"Client command received to stop acquisition\n");



	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		ret=stopStateMachine();
	}

	if (ret==FAIL)
		sprintf(mess,"Stop acquisition failed\n");
	else if (differentClients)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	return ret;


}

int start_readout(int file_des) {


	int ret=OK;
	int n;


	sprintf(mess,"can't start readout\n");

#ifdef VERBOSE
	printf("Starting readout\n");
#endif     
	if (differentClients==1 && lockStatus==1) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	} else {
		ret=startReadOut();
	}
	if (ret==FAIL)
		sprintf(mess,"Start readout failed\n");
	else if (differentClients)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}
	return ret;



}

int get_run_status(int file_des) {  

	int ret=OK;
	int n;


	enum runStatus s;
	sprintf(mess,"getting run status\n");

#ifdef VERBOSE
	printf("Getting status\n");
#endif 

	int retval = getStatus();

	if (ret!=OK) {
		printf("get status failed %04x\n",retval);
		sprintf(mess, "get status failed %08x\n", retval);

	} else if (differentClients)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n += sendDataOnly(file_des,&s,sizeof(s));
	}
	return ret;



}





int start_and_read_all(int file_des) {
#ifdef VERBOSE
	printf("Starting and reading all frames\n");
#endif
	int dataret = FAIL;
	if (differentClients==1 && lockStatus==1) {
		dataret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		sendDataOnly(file_des,&dataret,sizeof(dataret));
		sendDataOnly(file_des,mess,sizeof(mess));
		return dataret;

	}

	startStateMachine();
	dataret = read_all(file_des);
#ifdef VERBOSE
	printf("Frames finished\n");
#endif

	return dataret;


}



int read_all(int file_des) {
	int dataret = FAIL;
	strcpy(mess,"wait for read frame failed\n");

	if (differentClients==1 && lockStatus==1) {
		dataret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
		cprintf(RED,"%s\n",mess);
		sendDataOnly(file_des,&dataret,sizeof(dataret));
		sendDataOnly(file_des,mess,sizeof(mess));
		return dataret;
	}


#ifdef VIRTUAL
	dataret = FINISHED;
	strcpy(mess,"acquisition successfully finished\n");
#else
	waitForAcquisitionFinish();

	// set return value and message
	if(getFrames()>-2) {
		dataret = FAIL;
		sprintf(mess,"no data and run stopped: %d frames left\n",(int)(getFrames()+2));
		cprintf(RED,"%s\n",mess);
	} else {
		dataret = FINISHED;
		sprintf(mess,"acquisition successfully finished\n");
		cprintf(GREEN,"%s",mess);

	}
#endif

	if (differentClients)
		dataret=FORCE_UPDATE;

	sendDataOnly(file_des,&dataret,sizeof(dataret));
	sendDataOnly(file_des,mess,sizeof(mess));
	return dataret;
}





int set_timer(int file_des) {
	enum timerIndex ind;
	int64_t tns;
	int n;
	int64_t retval;
	int ret=OK;


	sprintf(mess,"can't set timer\n");

	n = receiveDataOnly(file_des,&ind,sizeof(ind));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	n = receiveDataOnly(file_des,&tns,sizeof(tns));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	if (ret!=OK) {
		printf(mess);
	}

	//#ifdef VERBOSE
	printf("setting timer %d to %lld ns\n",ind,tns);
	//#endif
	if (ret==OK) {

		if (differentClients==1 && lockStatus==1 && tns!=-1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		}  else {
			switch(ind) {
			case FRAME_NUMBER:
				retval=setFrames(tns);
				break;
			case ACQUISITION_TIME:
				retval=setExposureTime(tns);
				break;
			case FRAME_PERIOD:
				retval=setPeriod(tns);
				break;
			case DELAY_AFTER_TRIGGER:
				retval=setDelay(tns);
				break;
			case GATES_NUMBER:
				retval=setGates(tns);
				break;
			case CYCLES_NUMBER:
				retval=setTrains(tns);
				break;
			default:
				ret=FAIL;
				sprintf(mess,"timer index unknown %d\n",ind);
				break;
			}
		}
	}
	if (ret!=OK) {
		printf(mess);
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	if (ret!=OK) {
		printf(mess);
		printf("set timer failed\n");
	}

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
#ifdef VERBOSE
		printf("returning ok %d\n",(int)(sizeof(retval)));
#endif 

		n = sendDataOnly(file_des,&retval,sizeof(retval));
	}

	return ret;

}








int get_time_left(int file_des) {

	enum timerIndex ind;
	int n;
	int64_t retval;
	int ret=OK;

	sprintf(mess,"can't get timer\n");
	n = receiveDataOnly(file_des,&ind,sizeof(ind));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


#ifdef VERBOSE

	printf("getting time left on timer %d \n",ind);
#endif 

	if (ret==OK) {
		switch(ind) {
		case FRAME_NUMBER:
			retval=getFrames();
			break;
		case ACQUISITION_TIME:
			retval=getExposureTime();
			break;
		case FRAME_PERIOD:
			retval=getPeriod();
			break;
		case DELAY_AFTER_TRIGGER:
			retval=getDelay();
			break;
		case GATES_NUMBER:
			retval=getGates();
			break;
		case CYCLES_NUMBER:
			retval=getTrains();
			break;
		case ACTUAL_TIME:
			retval=getActualTime();
			break;
		case MEASUREMENT_TIME:
			retval=getMeasurementTime();
			break;
		default:
			ret=FAIL;
			sprintf(mess,"timer index unknown %d\n",ind);
			break;
		}
	}


	if (ret!=OK) {
		printf("get time left failed\n");
	} else if (differentClients)
		ret=FORCE_UPDATE;

#ifdef VERBOSE

	printf("time left on timer %d is %lld\n",ind, retval);
#endif 

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=OK) {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&retval,sizeof(retval));
	}
#ifdef VERBOSE

	printf("data sent\n");
#endif 

	return ret;


}

int set_dynamic_range(int file_des) {



	int dr;
	int n;
	int retval;
	int ret=OK;


	sprintf(mess,"can't set dynamic range\n");


	n = receiveDataOnly(file_des,&dr,sizeof(dr));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


	if (differentClients==1 && lockStatus==1 && dr>=0) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		retval=DYNAMIC_RANGE;
	}

	//if (dr>=0 && retval!=dr)   ret=FAIL;
	if (ret!=OK) {
		sprintf(mess,"set dynamic range failed\n");
	}

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&retval,sizeof(retval));
	}
	return ret;
}





int set_readout_flags(int file_des) {

	enum readOutFlags arg;
	int ret=FAIL;


	receiveDataOnly(file_des,&arg,sizeof(arg));

#ifdef PROPIXD
	sprintf(mess,"can't set readout flags for propix\n");
#else
	sprintf(mess,"can't set readout flags for gotthard\n");
#endif

	sendDataOnly(file_des,&ret,sizeof(ret));
	sendDataOnly(file_des,mess,sizeof(mess));

	return ret;
}




int set_roi(int file_des) {

	int i;
	int ret=OK;
	int nroi=-1;
	int n=0;
	int retvalsize=0;
	ROI arg[MAX_ROIS];
	ROI* retval=0;

	strcpy(mess,"Could not set/get roi\n");


	n = receiveDataOnly(file_des,&nroi,sizeof(nroi));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef PROPIXD
	sprintf(mess,"can't set roi for propix\n");
	ret = FAIL;
#endif
	if(ret != FAIL){
		if(nroi!=-1){
			n = receiveDataOnly(file_des,arg,nroi*sizeof(ROI));
			if (n != (nroi*sizeof(ROI))) {
				sprintf(mess,"Received wrong number of bytes for ROI\n");
				ret=FAIL;
			}
			//#ifdef VERBOSE
			printf("\n\nSetting ROI: nroi=%d\n",nroi);
			for( i=0;i<nroi;i++)
				printf("\t%d\t%d\t%d\t%d\n",arg[i].xmin,arg[i].xmax,arg[i].ymin,arg[i].ymax);
			//#endif
		}
		/* execute action if the arguments correctly arrived*/

		if (lockStatus==1 && differentClients==1){//necessary???
			sprintf(mess,"Detector locked by %s\n", lastClientIP);
			ret=FAIL;
		}
		else{
			retval=setROI(nroi,arg,&retvalsize,&ret);

			if (ret==FAIL){
				printf("mess:%s\n",mess);
				sprintf(mess,"Could not set all roi, should have set %d rois, but only set %d rois\n",nroi,retvalsize);
			}
		}

	}

	if(ret==OK && differentClients){
		printf("Force update\n");
		ret=FORCE_UPDATE;
	}

	/* send answer */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if(ret==FAIL)
		n = sendDataOnly(file_des,mess,sizeof(mess));
	else{
		sendDataOnly(file_des,&retvalsize,sizeof(retvalsize));
		sendDataOnly(file_des,retval,retvalsize*sizeof(ROI));
	}
	/*return ok/fail*/
	return ret;
}



int set_speed(int file_des) {

	enum speedVariable arg;
	int val;
	int ret=FAIL;

	receiveDataOnly(file_des,&arg,sizeof(arg));
	receiveDataOnly(file_des,&val,sizeof(val));

#ifdef PROPIXD
	sprintf(mess,"can't set speed variable for propix\n");
#else
	sprintf(mess,"can't set speed variable for gotthard\n");
#endif



	sendDataOnly(file_des,&ret,sizeof(ret));
	sendDataOnly(file_des,mess,sizeof(mess));

	return ret;
}






int exit_server(int file_des) {
	int ret=OK;
	sprintf(mess,"closing server\n");
	cprintf(BG_RED,"Command: %s",mess);
	sendDataOnly(file_des,&ret,sizeof(ret));
	sendDataOnly(file_des,mess,sizeof(mess));
	return GOODBYE;
}



int lock_server(int file_des) {


	int n;
	int ret=OK;

	int lock;
	n = receiveDataOnly(file_des,&lock,sizeof(lock));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (lock)\n");
		ret=FAIL;
	}
	if (lock>=0) {
		if (lockStatus==0 || strcmp(lastClientIP,thisClientIP)==0 || strcmp(lastClientIP,"none")==0)
			lockStatus=lock;
		else {
			ret=FAIL;
			sprintf(mess,"Server already locked by %s\n", lastClientIP);
		}
	}
	if (differentClients && ret==OK)
		ret=FORCE_UPDATE;

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	}  else
		n = sendDataOnly(file_des,&lockStatus,sizeof(lockStatus));

	return ret;

}


int get_last_client_ip(int file_des) {
	int ret=OK;
	int n;
	if (differentClients )
		ret=FORCE_UPDATE;
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	n = sendDataOnly(file_des,lastClientIP,sizeof(lastClientIP));

	return ret;

}


int set_port(int file_des) {
	int n;
	int ret=OK;
	int sd=-1;

	enum portType p_type; /** data? control? stop? Unused! */
	int p_number; /** new port number */

	n = receiveDataOnly(file_des,&p_type,sizeof(p_type));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (ptype)\n");
		ret=FAIL;
	}

	n = receiveDataOnly(file_des,&p_number,sizeof(p_number));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		printf("Error reading from socket (pnum)\n");
		ret=FAIL;
	}
	if (differentClients==1 && lockStatus==1 ) {
		ret=FAIL;
		sprintf(mess,"Detector locked by %s\n",lastClientIP);
	}  else {
		if (p_number<1024) {
			sprintf(mess,"Too low port number %d\n", p_number);
			printf("\n");
			ret=FAIL;
		}

		printf("set port %d to %d\n",p_type, p_number);

		sd=bindSocket(p_number);
	}
	if (sd>=0) {
		ret=OK;
		if (differentClients )
			ret=FORCE_UPDATE;
	} else {
		ret=FAIL;
		sprintf(mess,"Could not bind port %d\n", p_number);
		printf("Could not bind port %d\n", p_number);
		if (sd==-10) {
			sprintf(mess,"Port %d already set\n", p_number);
			printf("Port %d already set\n", p_number);

		}
	}

	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else {
		n = sendDataOnly(file_des,&p_number,sizeof(p_number));
		closeConnection(file_des);
		exitServer(sockfd);
		sockfd=sd;

	}

	return ret;

}



int send_update(int file_des) {

	int ret=OK;
	enum detectorSettings t;
	int n;//int thr, n;
	//int it;
	int64_t retval, tns=-1;
	n = sendDataOnly(file_des,lastClientIP,sizeof(lastClientIP));
	int k = DYNAMIC_RANGE;
	n = sendDataOnly(file_des,&k,sizeof(k));
	k = DATA_BYTES;
	n = sendDataOnly(file_des,&k,sizeof(k));
	t=setSettings(GET_SETTINGS);
	n = sendDataOnly(file_des,&t,sizeof(t));
	retval=setFrames(tns);
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	retval=setExposureTime(tns);
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	retval=setPeriod(tns);
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	retval=setDelay(tns);
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	retval=setGates(tns);
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));
	retval=setTrains(tns);
	n = sendDataOnly(file_des,&retval,sizeof(int64_t));

	if (lockStatus==0) {
		strcpy(lastClientIP,thisClientIP);
	}

	return ret;


}
int update_client(int file_des) {

	int ret=OK;

	sendDataOnly(file_des,&ret,sizeof(ret));
	return send_update(file_des);



}


int configure_mac(int file_des) {

	int ret=OK;
	char arg[6][50];
	int n;

	int ipad;
	long long int imacadd;
	long long int idetectormacadd;
	int udpport;
	int detipad;
	int retval=-100;

	sprintf(mess,"Can't configure MAC\n");


	n = receiveDataOnly(file_des,arg,sizeof(arg));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	sscanf(arg[0], "%x", 		&ipad);
	sscanf(arg[1], "%llx", 	&imacadd);
	sscanf(arg[2], "%x", 		&udpport);
	sscanf(arg[3], "%llx",	&idetectormacadd);
	sscanf(arg[4], "%x",		&detipad);
	//arg[5] is udpport2 for eiger
#ifdef VERBOSE
	int i;
	printf("\ndigital_test_bit in server %d\t",digitalTestBit);
	printf("\nipadd %x\t",ipad);
	printf("destination ip is %d.%d.%d.%d = 0x%x \n",(ipad>>24)&0xff,(ipad>>16)&0xff,(ipad>>8)&0xff,(ipad)&0xff,ipad);
	printf("macad:%llx\n",imacadd);
	for (i=0;i<6;i++)
		printf("mac adress %d is 0x%x \n",6-i,(unsigned int)(((imacadd>>(8*i))&0xFF)));
	printf("udp port:0x%x\n",udpport);
	printf("detector macad:%llx\n",idetectormacadd);
	for (i=0;i<6;i++)
		printf("detector mac adress %d is 0x%x \n",6-i,(unsigned int)(((idetectormacadd>>(8*i))&0xFF)));
	printf("detipad %x\n",detipad);
	printf("\n");
#endif


	//#ifdef VERBOSE
	printf("Configuring MAC at port %x\n", udpport);
	//#endif
	if (ret==OK){
		if(runBusy()){
			ret=stopStateMachine();
			if(ret==FAIL)
				strcpy(mess,"could not stop detector acquisition to configure mac");
		}

		if(ret==OK)
			configureMAC(ipad,imacadd,idetectormacadd,detipad,digitalTestBit,udpport);
		retval=getAdcConfigured();
	}
	if (ret==FAIL)
		printf("configuring MAC failed\n");
	else
		printf("Configuremac successful and adc %d\n",retval);

	if (differentClients)
		ret=FORCE_UPDATE;

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL)
		n += sendDataOnly(file_des,mess,sizeof(mess));
	else
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	/*return ok/fail*/
	return ret;

}



int load_image(int file_des) {
	int retval;
	int ret=OK;
	int n;
	enum imageType index;
	short int ImageVals[NCHAN*NCHIP];

	sprintf(mess,"Loading image failed\n");

	n = receiveDataOnly(file_des,&index,sizeof(index));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	n = receiveDataOnly(file_des,ImageVals,DATA_BYTES);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

#ifdef PROPIXD
	sprintf(mess,"can't load image for propix\n");
	ret = FAIL;
#endif

	switch (index) {
	case DARK_IMAGE :
#ifdef VERBOSE
		printf("Loading Dark image\n");
#endif
		break;
	case GAIN_IMAGE :
#ifdef VERBOSE
		printf("Loading Gain image\n");
#endif
		break;
	default:
		printf("Unknown index %d\n",index);
		sprintf(mess,"Unknown index %d\n",index);
		ret=FAIL;
		break;
	}

	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else{
			retval=loadImage(index,ImageVals);
			if (retval==-1)
				ret = FAIL;
		}
	}

	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,&retval,sizeof(retval));
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;
}





int read_counter_block(int file_des) {

	int ret=OK;
	int n;
	int startACQ;
	//char *retval=NULL;
	short int CounterVals[NCHAN*NCHIP];

	sprintf(mess,"Read counter block failed\n");

	n = receiveDataOnly(file_des,&startACQ,sizeof(startACQ));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


#ifdef PROPIXD
	sprintf(mess,"can't read counter block for propix\n");
	ret = FAIL;
#endif

	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else{
			ret=readCounterBlock(startACQ,CounterVals);
#ifdef VERBOSE
			int i;
			for(i=0;i<6;i++)
				printf("%d:%d\t",i,CounterVals[i]);
#endif
		}
	}

	if(ret!=FAIL){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret!=FAIL) {
		/* send return argument */
		n += sendDataOnly(file_des,CounterVals,DATA_BYTES);//1280*2
	} else {
		n += sendDataOnly(file_des,mess,sizeof(mess));
	}

	/*return ok/fail*/
	return ret;
}





int reset_counter_block(int file_des) {

	int ret=OK;
	int n;
	int startACQ;

	sprintf(mess,"Reset counter block failed\n");

	n = receiveDataOnly(file_des,&startACQ,sizeof(startACQ));
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	if (ret==OK) {
		if (differentClients==1 && lockStatus==1) {
			ret=FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
		} else
			ret=resetCounterBlock(startACQ);
	}

	if(ret==OK){
		if (differentClients)
			ret=FORCE_UPDATE;
	}

	/* send answer */
	/* send OK/failed */
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	if (ret==FAIL)
		n += sendDataOnly(file_des,mess,sizeof(mess));

	/*return ok/fail*/
	return ret;
}







int write_adc_register(int file_des) {
	int ret=OK;
	int n=0;
	int retval=-1;
	sprintf(mess,"write to adc register failed\n");

	// receive arguments
	int arg[2]={-1,-1};
	n = receiveData(file_des,arg,sizeof(arg),INT32);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}

	int addr=arg[0];
	int val=arg[1];

	// execute action
	if (ret == OK) {
		if (differentClients && lockStatus) {
			ret = FAIL;
			sprintf(mess,"Detector locked by %s\n",lastClientIP);
			cprintf(RED, "Warning: %s", mess);
		}
		else {
#ifdef VERBOSE
			printf("writing to register 0x%x data 0x%x\n", addr, val);
#endif
			setAdc(addr,val);
#ifdef VERBOSE
			printf("Data set to 0x%x\n",  retval);
#endif
			if (ret==OK && differentClients)
				ret=FORCE_UPDATE;
		}
	}
	// send ok / fail
	n = sendDataOnly(file_des,&ret,sizeof(ret));
	// send return argument
	if (ret==FAIL) {
		n = sendDataOnly(file_des,mess,sizeof(mess));
	} else
		n = sendDataOnly(file_des,&retval,sizeof(retval));

	// return ok / fail
	return ret;
}




int check_version(int file_des) {
	int ret=OK,ret1=OK;
	int n=0;
	sprintf(mess,"check version failed\n");



	// receive arguments
	int64_t arg=-1;
	n = receiveData(file_des,&arg,sizeof(arg),INT64);
	if (n < 0) {
		sprintf(mess,"Error reading from socket\n");
		ret=FAIL;
	}


	// execute action
	if (ret == OK) {
#ifdef VERBOSE
		printf("Checking versioning compatibility with value %d\n",arg);
#endif
		int64_t client_requiredVersion = arg;
		int64_t det_apiVersion = APIGOTTHARD;
		int64_t det_version = (GITDATE & 0xFFFFFF);

		// old client
		if (det_apiVersion > client_requiredVersion) {
			ret = FAIL;
			sprintf(mess,"Client's detector SW API version: (0x%llx). "
					"Detector's SW API Version: (0x%llx). "
					"Incompatible, update client!\n",
					client_requiredVersion, det_apiVersion);
			cprintf(RED, "Warning: %s", mess);
		}

		// old software
		else if (client_requiredVersion > det_version) {
			ret = FAIL;
			sprintf(mess,"Detector SW Version: (0x%llx). "
					"Client's detector SW API Version: (0x%llx). "
					"Incompatible, update detector software!\n",
					det_version, client_requiredVersion);
			cprintf(RED, "Warning: %s", mess);
		}
	}



	// ret could be swapped during sendData
	ret1 = ret;
	// send ok / fail
	n = sendData(file_des,&ret1,sizeof(ret),INT32);
	// send return argument
	if (ret==FAIL) {
		n += sendData(file_des,mess,sizeof(mess),OTHER);
	}

	// return ok / fail
	return ret;
}

