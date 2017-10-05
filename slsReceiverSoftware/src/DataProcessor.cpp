/************************************************
 * @file DataProcessor.cpp
 * @short creates data processor thread that
 * pulls pointers to memory addresses from fifos
 * and processes data stored in them & writes them to file
 ***********************************************/


#include "DataProcessor.h"
#include "GeneralData.h"
#include "Fifo.h"
#include "BinaryFile.h"
#ifdef HDF5C
#include "HDF5File.h"
#endif
#include "DataStreamer.h"

#include <iostream>
#include <errno.h>
#include <cstring>
using namespace std;

const string DataProcessor::TypeName = "DataProcessor";

int DataProcessor::NumberofDataProcessors(0);

uint64_t DataProcessor::ErrorMask(0x0);

uint64_t DataProcessor::RunningMask(0x0);

pthread_mutex_t DataProcessor::Mutex = PTHREAD_MUTEX_INITIALIZER;

bool DataProcessor::SilentMode(false);


DataProcessor::DataProcessor(Fifo*& f, fileFormat* ftype, bool* fwenable, bool* dsEnable, bool* gpEnable, uint32_t* dr,
		uint32_t* freq, uint32_t* timer,
		void (*dataReadycb)(uint64_t, uint32_t, uint32_t, uint64_t, uint64_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t, uint16_t, uint8_t, uint8_t,
				char*, uint32_t, void*),
		void *pDataReadycb) :

		ThreadObject(NumberofDataProcessors),
		generalData(0),
		fifo(f),
		file(0),
		dataStreamEnable(dsEnable),
		fileFormatType(ftype),
		fileWriteEnable(fwenable),
		gapPixelsEnable(gpEnable),
		dynamicRange(dr),
		streamingFrequency(freq),
		streamingTimerInMs(timer),
		tempBuffer(0),
		currentFreqCount(0),
		acquisitionStartedFlag(false),
		measurementStartedFlag(false),
		firstAcquisitionIndex(0),
		firstMeasurementIndex(0),
		numTotalFramesCaught(0),
		numFramesCaught(0),
		currentFrameIndex(0),
		rawDataReadyCallBack(dataReadycb),
		pRawDataReady(pDataReadycb)
{
	if(ThreadObject::CreateThread()){
		pthread_mutex_lock(&Mutex);
		ErrorMask ^= (1<<index);
		pthread_mutex_unlock(&Mutex);
	}

	NumberofDataProcessors++;
	FILE_LOG (logDEBUG) << "Number of DataProcessors: " << NumberofDataProcessors;

	memset((void*)&timerBegin, 0, sizeof(timespec));
}


DataProcessor::~DataProcessor() {
	if (file) delete file;
	if (tempBuffer) delete [] tempBuffer;
	ThreadObject::DestroyThread();
	NumberofDataProcessors--;
}

/** static functions */

uint64_t DataProcessor::GetErrorMask() {
	return ErrorMask;
}

uint64_t DataProcessor::GetRunningMask() {
	return RunningMask;
}

void DataProcessor::ResetRunningMask() {
	RunningMask = 0x0;
}

void DataProcessor::SetSilentMode(bool mode) {
	SilentMode = mode;
}

/** non static functions */
/** getters */
string DataProcessor::GetType(){
	return TypeName;
}

bool DataProcessor::IsRunning() {
	return ((1 << index) & RunningMask);
}

bool DataProcessor::GetAcquisitionStartedFlag(){
	return acquisitionStartedFlag;
}

bool DataProcessor::GetMeasurementStartedFlag(){
	return measurementStartedFlag;
}

uint64_t DataProcessor::GetNumTotalFramesCaught() {
	return numTotalFramesCaught;
}

uint64_t DataProcessor::GetNumFramesCaught() {
	return numFramesCaught;
}

uint64_t DataProcessor::GetActualProcessedAcquisitionIndex() {
	return currentFrameIndex;
}

uint64_t DataProcessor::GetProcessedAcquisitionIndex() {
	return currentFrameIndex - firstAcquisitionIndex;
}

uint64_t DataProcessor::GetProcessedMeasurementIndex() {
	return currentFrameIndex - firstMeasurementIndex;
}



/** setters */
void DataProcessor::StartRunning() {
	pthread_mutex_lock(&Mutex);
	RunningMask |= (1<<index);
	pthread_mutex_unlock(&Mutex);
}


void DataProcessor::StopRunning() {
	pthread_mutex_lock(&Mutex);
	RunningMask ^= (1<<index);
	pthread_mutex_unlock(&Mutex);
}

void DataProcessor::SetFifo(Fifo*& f) {
	fifo = f;
}

void DataProcessor::ResetParametersforNewAcquisition() {
	numTotalFramesCaught = 0;
	firstAcquisitionIndex = 0;
	currentFrameIndex = 0;
	acquisitionStartedFlag = false;
}

void DataProcessor::ResetParametersforNewMeasurement(){
	numFramesCaught = 0;
	firstMeasurementIndex = 0;
	measurementStartedFlag = false;

	if (tempBuffer) {
		delete [] tempBuffer;
		tempBuffer = 0;
	}
	if (*gapPixelsEnable >= 0) {
		tempBuffer = new char[generalData->imageSize];
		memset(tempBuffer, 0, generalData->imageSize);
	}
}


void DataProcessor::RecordFirstIndices(uint64_t fnum) {
	//listen to this fnum, later +1
	currentFrameIndex = fnum;

	measurementStartedFlag = true;
	firstMeasurementIndex = fnum;

	//start of entire acquisition
	if (!acquisitionStartedFlag) {
		acquisitionStartedFlag = true;
		firstAcquisitionIndex = fnum;
	}

#ifdef VERBOSE
	bprintf(BLUE,"%d First Acquisition Index:%lld\tFirst Measurement Index:%lld\n",
			index, (long long int)firstAcquisitionIndex, (long long int)firstMeasurementIndex);
#endif
}


void DataProcessor::SetGeneralData(GeneralData* g) {
	generalData = g;
#ifdef VERY_VERBOSE
	generalData->Print();
#endif
	if (file) {
		file->SetMaxFramesPerFile(generalData->maxFramesPerFile);
		if (file->GetFileType() == HDF5) {
			file->SetNumberofPixels(generalData->nPixelsX, generalData->nPixelsY);
		}
	}
}


int DataProcessor::SetThreadPriority(int priority) {
	struct sched_param param;
	param.sched_priority = priority;
	if (pthread_setschedparam(thread, SCHED_FIFO, &param) == EPERM)
		return FAIL;
	FILE_LOG(logINFO) << "Processor Thread Priority set to " << priority;
	return OK;
}


void DataProcessor::SetFileFormat(const fileFormat f) {
	if (file->GetFileType() != f) {
		//remember the pointer values before they are destroyed
		int nd[MAX_DIMENSIONS];nd[0] = 0; nd[1] = 0;
		char* fname=0; char* fpath=0; uint64_t* findex=0;
		bool* owenable=0; int* dindex=0; int* nunits=0; uint64_t* nf = 0; uint32_t* dr = 0; uint32_t* port = 0;
		file->GetMemberPointerValues(nd, fname, fpath, findex, owenable, dindex, nunits, nf, dr, port);
		//create file writer with same pointers
		SetupFileWriter(nd, fname, fpath, findex, owenable, dindex, nunits, nf, dr, port);
	}
}



void DataProcessor::SetupFileWriter(int* nd, char* fname, char* fpath, uint64_t* findex,
		bool* owenable, int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
		GeneralData* g)
{
	if (g)
		generalData = g;

	if (file)
		delete file;

	switch(*fileFormatType){
#ifdef HDF5C
	case HDF5:
		file = new HDF5File(index, generalData->maxFramesPerFile,
				nd, fname, fpath, findex, owenable,
				dindex, nunits, nf, dr, portno,
				generalData->nPixelsX, generalData->nPixelsY, &SilentMode);
		break;
#endif
	default:
		file = new BinaryFile(index, generalData->maxFramesPerFile,
				nd, fname, fpath, findex, owenable,
				dindex, nunits, nf, dr, portno, &SilentMode);
		break;
	}
}

// only the first file
int DataProcessor::CreateNewFile(bool en, uint64_t nf, uint64_t at, uint64_t st, uint64_t ap) {
	file->CloseAllFiles();
	if (file->CreateMasterFile(en,	generalData->imageSize, generalData->nPixelsX, generalData->nPixelsY,
			at, st, ap) == FAIL)
		return FAIL;
	if (file->CreateFile(currentFrameIndex) == FAIL)
		return FAIL;
	return OK;
}


void DataProcessor::CloseFiles() {
	if (file)
		file->CloseAllFiles();
}

void DataProcessor::EndofAcquisition(uint64_t numf) {
	if (*fileWriteEnable && file->GetFileType() == HDF5) {
		file->EndofAcquisition(numf);
	}
}


void DataProcessor::ThreadExecution() {
	char* buffer=0;
	fifo->PopAddress(buffer);
#ifdef FIFODEBUG
	if (!index) bprintf(BLUE,"DataProcessor %d, pop 0x%p buffer:%s\n", index,(void*)(buffer),buffer);
#endif

	//check dummy
	uint32_t numBytes = (uint32_t)(*((uint32_t*)buffer));
#ifdef VERBOSE
	if (!index) bprintf(BLUE,"DataProcessor %d, Numbytes:%u\n", index,numBytes);
#endif
	if (numBytes == DUMMY_PACKET_VALUE) {
		StopProcessing(buffer);
		return;
	}

	ProcessAnImage(buffer + FIFO_HEADER_NUMBYTES);

	//stream (if time/freq to stream) or free
	if (*dataStreamEnable && SendToStreamer())
		fifo->PushAddressToStream(buffer);
	else
		fifo->FreeAddress(buffer);
}


void DataProcessor::StopProcessing(char* buf) {
#ifdef VERBOSE
	if (!index)
		bprintf(RED,"DataProcessing %d: Dummy\n", index);
#endif
	//stream or free
	if (*dataStreamEnable)
		fifo->PushAddressToStream(buf);
	else
		fifo->FreeAddress(buf);

	file->CloseCurrentFile();
	StopRunning();
#ifdef VERBOSE
	FILE_LOG(logINFO) << index << ": Processing Completed";
#endif
}

/** buf includes only the standard header */
void DataProcessor::ProcessAnImage(char* buf) {

	sls_detector_header* header = (sls_detector_header*) (buf);
	uint64_t fnum = header->frameNumber;
	currentFrameIndex = fnum;
	uint32_t nump = header->packetNumber;
	if (nump == generalData->packetsPerFrame) {
		numFramesCaught++;
		numTotalFramesCaught++;
	}


#ifdef VERBOSE
	if (!index)
		bprintf(BLUE,"DataProcessing %d: fnum:%lu\n", index, fnum);
#endif

	if (!measurementStartedFlag) {
#ifdef VERBOSE
		if (!index) bprintf(BLUE,"DataProcessing %d: fnum:%lu\n", index, fnum);
#endif
		RecordFirstIndices(fnum);

		if (*dataStreamEnable) {
			//restart timer
			clock_gettime(CLOCK_REALTIME, &timerBegin);
			timerBegin.tv_sec -= (*streamingTimerInMs) / 1000;
			timerBegin.tv_nsec -= ((*streamingTimerInMs) % 1000) * 1000000;

			//to send first image
			currentFreqCount = *streamingFrequency;
		}
	}

	if (*gapPixelsEnable && (*dynamicRange!=4))
		InsertGapPixels(buf + sizeof(sls_detector_header), *dynamicRange);

	if (*fileWriteEnable)
		file->WriteToFile(buf, generalData->imageSize + sizeof(sls_detector_header), fnum-firstMeasurementIndex, nump);

	if (rawDataReadyCallBack) {
		rawDataReadyCallBack(
				header->frameNumber,
				header->expLength,
				header->packetNumber,
				header->bunchId,
				header->timestamp,
				header->modId,
				header->xCoord,
				header->yCoord,
				header->zCoord,
				header->debug,
				header->roundRNumber,
				header->detType,
				header->version,
				buf + sizeof(sls_detector_header),
				generalData->imageSize,
				pRawDataReady);
	}

}



bool DataProcessor::SendToStreamer() {
	//skip
	if (!(*streamingFrequency)) {
		if (!CheckTimer())
			return false;
	} else {
		if (!CheckCount())
			return false;
	}
	return true;
}


bool DataProcessor::CheckTimer() {
	struct timespec end;
	clock_gettime(CLOCK_REALTIME, &end);
#ifdef VERBOSE
	bprintf(BLUE,"%d Timer elapsed time:%f seconds\n", index, ( end.tv_sec - timerBegin.tv_sec ) + ( end.tv_nsec - timerBegin.tv_nsec ) / 1000000000.0);
#endif
	//still less than streaming timer, keep waiting
	if((( end.tv_sec - timerBegin.tv_sec )	+ ( end.tv_nsec - timerBegin.tv_nsec ) / 1000000000.0) < ((double)*streamingTimerInMs/1000.00))
		return false;

	//restart timer
	clock_gettime(CLOCK_REALTIME, &timerBegin);
	return true;
}


bool DataProcessor::CheckCount() {
	if (currentFreqCount == *streamingFrequency ) {
		currentFreqCount = 1;
		return true;
	}
	currentFreqCount++;
	return false;
}


void DataProcessor::SetPixelDimension() {
	if (file) {
		if (file->GetFileType() == HDF5) {
			file->SetNumberofPixels(generalData->nPixelsX, generalData->nPixelsY);
		}
	}
}


/** eiger specific */
void DataProcessor::InsertGapPixels(char* buf, uint32_t dr) {

	memset(tempBuffer, 0xFF, generalData->imageSize);

	const uint32_t nx = generalData->nPixelsX;
	const uint32_t ny = generalData->nPixelsY;
	const uint32_t npx = nx * ny;

	char* srcptr = 0;
	char* dstptr = 0;

	const uint32_t b1px = generalData->imageSize / (npx); // not double as not dealing with 4 bit mode
	const uint32_t b2px = 2 * b1px;
	const uint32_t b1pxofst = (index ? b1px : 0); // left fpga (index 0) has no extra 1px offset, but right fpga has
	const uint32_t b1chip = 256 * b1px;
	const uint32_t b1line = (nx * b1px);

	// copying line by line
	srcptr = buf;
	dstptr = tempBuffer + b1line + b1pxofst;		// left fpga (index 0) has no extra 1px offset, but right fpga has
	for (int i = 0; i < (ny-1); ++i) {
		memcpy(dstptr, srcptr, b1chip);
		srcptr += b1chip;
		dstptr += (b1chip + b2px);
		memcpy(dstptr, srcptr, b1chip);
		srcptr += b1chip;
		dstptr += (b1chip + b1px);
	}

	// vertical filling of values
	{
		char* srcgp1 = 0; char* srcgp2 = 0; char* srcgp3 = 0;
		char* dstgp1 = 0; char* dstgp2 = 0; char* dstgp3 = 0;
		const uint32_t b3px = 3 * b1px;

		srcptr = tempBuffer + b1line;
		dstptr = tempBuffer + b1line;

		for (int i = 0; i < (ny-1); ++i) {
			srcgp1 = srcptr + b1pxofst + b1chip - b1px;
			dstgp1 = srcgp1 + b1px;
			srcgp2 = srcgp1 + b3px;
			dstgp2 = dstgp1 + b1px;
			if (!index) {
				srcgp3 = srcptr + b1line - b2px;
				dstgp3 = srcgp3 + b1px;
			} else {
				srcgp3 = srcptr + b1px;
				dstgp3 = srcptr;
			}
			switch (dr) {
			case 8:
				(*((uint8_t*)srcgp1)) = (*((uint8_t*)srcgp1))/2;	(*((uint8_t*)dstgp1)) = (*((uint8_t*)srcgp1));
				(*((uint8_t*)srcgp2)) = (*((uint8_t*)srcgp2))/2;	(*((uint8_t*)dstgp2)) = (*((uint8_t*)srcgp2));
				(*((uint8_t*)srcgp3)) = (*((uint8_t*)srcgp3))/2;	(*((uint8_t*)dstgp3)) = (*((uint8_t*)srcgp3));
				break;
			case 16:
				(*((uint16_t*)srcgp1)) = (*((uint16_t*)srcgp1))/2;	(*((uint16_t*)dstgp1)) = (*((uint16_t*)srcgp1));
				(*((uint16_t*)srcgp2)) = (*((uint16_t*)srcgp2))/2;	(*((uint16_t*)dstgp2)) = (*((uint16_t*)srcgp2));
				(*((uint16_t*)srcgp3)) = (*((uint16_t*)srcgp3))/2;	(*((uint16_t*)dstgp3)) = (*((uint16_t*)srcgp3));
				break;
			default:
				(*((uint32_t*)srcgp1)) = (*((uint32_t*)srcgp1))/2;	(*((uint32_t*)dstgp1)) = (*((uint32_t*)srcgp1));
				(*((uint32_t*)srcgp2)) = (*((uint32_t*)srcgp2))/2;	(*((uint32_t*)dstgp2)) = (*((uint32_t*)srcgp2));
				(*((uint32_t*)srcgp3)) = (*((uint32_t*)srcgp3))/2;	(*((uint32_t*)dstgp3)) = (*((uint32_t*)srcgp3));
				break;
			}
			srcptr += b1line;
			dstptr += b1line;
		}

	}

	// horizontal filling of values
	srcptr = tempBuffer + b1line;
	dstptr = tempBuffer;
	for (int i = 0; i < nx; ++i) {
		switch (dr) {
		case 8:	(*((uint8_t*)srcptr)) = (*((uint8_t*)srcptr))/2; (*((uint8_t*)dstptr)) = (*((uint8_t*)srcptr)); break;
		case 16:(*((uint16_t*)srcptr)) = (*((uint16_t*)srcptr))/2; (*((uint16_t*)dstptr)) = (*((uint16_t*)srcptr)); break;
		default:(*((uint32_t*)srcptr)) = (*((uint32_t*)srcptr))/2; (*((uint32_t*)dstptr)) = (*((uint32_t*)srcptr)); break;
		}
		srcptr += b1px;
		dstptr += b1px;
	}

	memcpy(buf, tempBuffer, generalData->imageSize);
	return;
}
