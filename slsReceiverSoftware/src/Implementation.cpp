#include "Implementation.h"
#include "DataProcessor.h"
#include "DataStreamer.h"
#include "Fifo.h"
#include "GeneralData.h"
#include "Listener.h"
#include "ZmqSocket.h" //just for the zmq port define
#include "file_utils.h"
#include "ToString.h"

#include <cerrno>  //eperm
#include <cstdlib> //system
#include <cstring>
#include <cstring> //strcpy
#include <fstream>
#include <iostream>
#include <sys/stat.h> // stat
#include <unistd.h>

/** cosntructor & destructor */

Implementation::Implementation(const detectorType d) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    InitializeMembers();
    setDetectorType(d);
}

Implementation::~Implementation() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    DeleteMembers();
}

void Implementation::DeleteMembers() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    if (generalData) {
        delete generalData;
        generalData = nullptr;
    }

    additionalJsonHeader.clear();
    ctbDbitList.clear();
    listener.reset();
    dataProcessor.reset();
    dataStreamer.reset();
    fifo.reset();
}

void Implementation::InitializeMembers() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    // config parameters
    myDetectorType = GENERIC;
    for (int i = 0; i < MAX_DIMENSIONS; ++i) {
        numDet[i] = 0;
        numRx[i] = 0;
    }
    detID = 0;
    detHostname = "";
    silentMode = false;
    fifoDepth = 0;
    frameDiscardMode = NO_DISCARD;
    framePadding = true;

    // file parameters
    fileFormatType = BINARY;
    filePath = "/";
    fileName = "run";
    fileIndex = 0;
    fileWriteEnable = true;
    masterFileWriteEnable = true;
    overwriteEnable = true;
    framesPerFile = 0;

    // acquisition
    status = IDLE;
    stoppedFlag = false;

    // network configuration (UDP)
    interfaceId = 0;
    numUDPInterfaces = 1;
    eth = "";
    udpPortNum =  DEFAULT_UDP_PORTNO;
    udpSocketBufferSize = 0;
    actualUDPSocketBufferSize = 0;

    // zmq parameters
    dataStreamEnable = false;
    streamingFrequency = 1;
    streamingTimerInMs = DEFAULT_STREAMING_TIMER_IN_MS;
    streamingPort = 0;
    streamingSrcIP = sls::IpAddr{};

    // detector parameters
    numberOfTotalFrames = 0;
    numberOfFrames = 1;
    numberOfTriggers = 1;
    numberOfBursts = 1;
    numberOfAdditionalStorageCells = 0;
    timingMode = AUTO_TIMING;
    burstMode = BURST_OFF;
    acquisitionPeriod = SAMPLE_TIME_IN_NS;
    acquisitionTime = 0;
    subExpTime = 0;
    subPeriod = 0;
    numberOfAnalogSamples = 0;
    numberOfDigitalSamples = 0;
    numberOfCounters = 0;
    dynamicRange = 16;
    roi.xmin = -1;
    roi.xmax = -1;
    tengigaEnable = false;
    flippedDataX = 0;
    quadEnable = false;
    activated = true;
    deactivatedPaddingEnable = true;
    numLinesReadout = MAX_EIGER_ROWS_PER_READOUT;
    readoutType = ANALOG_ONLY;
    adcEnableMaskOneGiga = BIT32_MASK;
    adcEnableMaskTenGiga = BIT32_MASK;

    ctbDbitOffset = 0;
    ctbAnalogDataBytes = 0;

    // callbacks
    startAcquisitionCallBack = nullptr;
    pStartAcquisition = nullptr;
    acquisitionFinishedCallBack = nullptr;
    pAcquisitionFinished = nullptr;
    rawDataReadyCallBack = nullptr;
    rawDataModifyReadyCallBack = nullptr;
    pRawDataReady = nullptr;

    // class objects
    generalData = nullptr;
}

void Implementation::SetLocalNetworkParameters() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    // to increase Max length of input packet queue
    int max_back_log;
    const char *proc_file_name = "/proc/sys/net/core/netdev_max_backlog";
    {
        std::ifstream proc_file(proc_file_name);
        proc_file >> max_back_log;
    }

    if (max_back_log < MAX_SOCKET_INPUT_PACKET_QUEUE) {
        std::ofstream proc_file(proc_file_name);
        if (proc_file.good()) {
            proc_file << MAX_SOCKET_INPUT_PACKET_QUEUE << std::endl;
            LOG(logINFOBLUE)
                << "Max length of input packet queue "
                   "[/proc/sys/net/core/netdev_max_backlog] modified to "
                << MAX_SOCKET_INPUT_PACKET_QUEUE;
        } else {
            LOG(logWARNING)
                << "Could not change max length of "
                   "input packet queue [net.core.netdev_max_backlog]. (No Root "
                   "Privileges?)";
        }
    }
}

void Implementation::SetThreadPriorities() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    listener->SetThreadPriority(LISTENER_PRIORITY);
}

void Implementation::SetupFifoStructure() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    // create fifo structure
    try {
        fifo = sls::make_unique<Fifo>( 0,
            (generalData->imageSize) + (generalData->fifoBufferHeaderSize),
            fifoDepth);
    } catch (...) {
        fifoDepth = 0;
        throw sls::RuntimeError("Could not allocate memory for fifo structure "
            ". FifoDepth is now 0.");
    }
    // set the listener & dataprocessor threads to point to the right fifo
    listener->SetFifo(fifo.get());
    dataProcessor->SetFifo(fifo.get());
    if (dataStreamEnable)
        dataStreamer->SetFifo(fifo.get());

    LOG(logINFO) << "Memory Allocated: "
                      << (double)(((size_t)(generalData->imageSize) +
                           (size_t)(generalData->fifoBufferHeaderSize)) *
                          (size_t)fifoDepth) / (double)(1024 * 1024)
                      << " MB";
    LOG(logINFO) << " Fifo structure(s) reconstructed";
}



/**************************************************
 *                                                 *
 *   Configuration Parameters                      *
 *                                                 *
 * ************************************************/

void Implementation::setDetectorType(const detectorType d) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    myDetectorType = d;
    switch (myDetectorType) {
    case GOTTHARD:
    case EIGER:
    case JUNGFRAU:
    case CHIPTESTBOARD:
    case MOENCH:
    case MYTHEN3:
    case GOTTHARD2:
        LOG(logINFO) << " ***** " << sls::ToString(d)
                          << " Receiver *****";
        break;
    default:
        throw sls::RuntimeError("This is an unknown receiver type "  + std::to_string(static_cast<int>(d)));
    }

    // set detector specific variables
    switch (myDetectorType) {
    case GOTTHARD:
        generalData = new GotthardData();
        break;
    case EIGER:
        generalData = new EigerData();
        break;
    case JUNGFRAU:
        generalData = new JungfrauData();
        break;
    case CHIPTESTBOARD:
        generalData = new ChipTestBoardData();
        break;
    case MOENCH:
        generalData = new MoenchData();
        break;
    case MYTHEN3:
        generalData = new Mythen3Data();
        break;   
    case GOTTHARD2:
        generalData = new Gotthard2Data();
        break;                 
    default:
        break;
    }
    fifoDepth = generalData->defaultFifoDepth;
    udpSocketBufferSize = generalData->defaultUdpSocketBufferSize;
    framesPerFile = generalData->maxFramesPerFile;

    SetLocalNetworkParameters();
    SetupFifoStructure();

    try {
        auto fifo_ptr = fifo.get();
        listener = sls::make_unique<Listener>(
            0, myDetectorType, fifo_ptr, &status, &udpPortNum, &eth,
            &numberOfTotalFrames, &dynamicRange, &udpSocketBufferSize,
            &actualUDPSocketBufferSize, &framesPerFile, &frameDiscardMode,
            &activated, &deactivatedPaddingEnable, &silentMode);
        dataProcessor = sls::make_unique<DataProcessor>(
            0, myDetectorType, fifo_ptr, &fileFormatType, fileWriteEnable,
            &masterFileWriteEnable, &dataStreamEnable, 
            &dynamicRange, &streamingFrequency, &streamingTimerInMs,
            &framePadding, &activated, &deactivatedPaddingEnable,
            &silentMode, &quadEnable, &ctbDbitList, &ctbDbitOffset,
            &ctbAnalogDataBytes);
    } catch (...) {
        listener.reset();
        dataProcessor.reset();
        throw sls::RuntimeError("Could not create listener/dataprocessor threads");
    }

    // set up writer and callbacks
    listener->SetGeneralData(generalData);
    dataProcessor->SetGeneralData(generalData);
    SetThreadPriorities();

    LOG(logDEBUG) << " Detector type set to " << sls::ToString(d);
}

int *Implementation::getMultiDetectorSize() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return (int *)numDet;
}

void Implementation::setDetectorSize(const int *size) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numDet[X] = size[X];
    numDet[Y] = size[Y];
    numRx[X] = numDet[X];
    numRx[Y] = numDet[Y];

    // calculating receivers shape
    switch (myDetectorType) {
    case EIGER:
        if (quadEnable) {
            numRx[X] = 1;
            numRx[Y] = 2;
        } else {
            numRx[X] = numDet[X] * 2;
        }
        break;
    case JUNGFRAU:
        if (numUDPInterfaces == 2) {
            numRx[Y] = numDet[X] * 2;
        } else {
            numRx[Y] = numDet[X];
        }
        break;
    default:
        break;
    }
    if (dataStreamEnable)
    	dataStreamer->SetReceiverShape(numRx);
    setDetectorPositionId(detID);

    LOG(logINFO) << "Receiver Shape: (" << numRx[X] << ", " 
        << numRx[Y] << ")";
} 


int Implementation::getDetectorPositionId() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return detID;
}

void Implementation::setDetectorPositionId(const int id) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    detID = id;
    LOG(logINFO) << "Detector Position Id:" << detID;

    // update zmq port
    streamingPort = DEFAULT_ZMQ_RX_PORTNO +
                       (detID * (myDetectorType == EIGER ? 2 : 1));
                       
    dataProcessor->SetupFileWriter(
        fileWriteEnable, (int *)numRx, &framesPerFile, &fileName, &filePath,
        &fileIndex, &overwriteEnable, &detID, &numUDPInterfaces, &numberOfTotalFrames,
        &dynamicRange, &udpPortNum, generalData);

    assert(numRx[1] != 0);
    uint16_t row = 0, col = 0;

    row = (detID % numRx[1]) * ((myDetectorType == JUNGFRAU && 
        numUDPInterfaces == 2) ? 2 : 1); // row
    col = (detID / numRx[1]) * ((myDetectorType == EIGER) ? 2 : 1) +
              interfaceId; // col for horiz. udp ports
    listener->SetHardCodedPosition(row, col);
}

std::string Implementation::getDetectorHostname() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return detHostname;
}

void Implementation::setDetectorHostname(const std::string& c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (!c.empty())
        detHostname = c;
    LOG(logINFO) << "Detector Hostname: " << detHostname;
}

bool Implementation::getSilentMode() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return silentMode;
}

void Implementation::setSilentMode(const bool i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    silentMode = i;
    LOG(logINFO) << "Silent Mode: " << i;
}

uint32_t Implementation::getFifoDepth() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fifoDepth;
}

void Implementation::setFifoDepth(const uint32_t i) {
    if (fifoDepth != i) {
        fifoDepth = i;
        SetupFifoStructure();
    }
    LOG(logINFO) << "Fifo Depth: " << i;
}

slsDetectorDefs::frameDiscardPolicy
Implementation::getFrameDiscardPolicy() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return frameDiscardMode;
}

void Implementation::setFrameDiscardPolicy(
    const frameDiscardPolicy i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (i >= 0 && i < NUM_DISCARD_POLICIES)
        frameDiscardMode = i;

    LOG(logINFO) << "Frame Discard Policy: "
                      << sls::ToString(frameDiscardMode);
}

bool Implementation::getFramePaddingEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return framePadding;
}

void Implementation::setFramePaddingEnable(const bool i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    framePadding = i;
    LOG(logINFO) << "Frame Padding: " << framePadding;
}


/**************************************************
 *                                                 *
 *   File Parameters                               *
 *                                                 *
 * ************************************************/
slsDetectorDefs::fileFormat Implementation::getFileFormat() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileFormatType;
}

void Implementation::setFileFormat(const fileFormat f) {
    switch (f) {
#ifdef HDF5C
    case HDF5:
        fileFormatType = HDF5;
        break;
#endif
    default:
        fileFormatType = BINARY;
        break;
    }

    dataProcessor->SetFileFormat(f);

    LOG(logINFO) << "File Format: " << sls::ToString(fileFormatType);
}

std::string Implementation::getFilePath() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return filePath;
}

void Implementation::setFilePath(const std::string& c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (!c.empty()) {
        mkdir_p(c); //throws if it can't create
        filePath = c;
    }
    LOG(logINFO) << "File path: " << filePath;
}

std::string Implementation::getFileName() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileName;
}

void Implementation::setFileName(const std::string& c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (!c.empty())
       fileName = c;
    LOG(logINFO) << "File name: " << fileName;
}

uint64_t Implementation::getFileIndex() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileIndex;
}

void Implementation::setFileIndex(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    fileIndex = i;
    LOG(logINFO) << "File Index: " << fileIndex;
}

bool Implementation::getFileWriteEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return fileWriteEnable;
}

void Implementation::setFileWriteEnable(const bool b) {
    if (fileWriteEnable != b) {
        fileWriteEnable = b;
        dataProcessor->SetupFileWriter(
            fileWriteEnable, (int *)numRx, &framesPerFile, &fileName,
            &filePath, &fileIndex, &overwriteEnable, &detID, &numUDPInterfaces,
            &numberOfTotalFrames, &dynamicRange, &udpPortNum, generalData);
    }

    LOG(logINFO) << "File Write Enable: " << (fileWriteEnable ? "enabled" : "disabled");
}

bool Implementation::getMasterFileWriteEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return masterFileWriteEnable;
}

void Implementation::setMasterFileWriteEnable(const bool b) {
    masterFileWriteEnable = b;

    LOG(logINFO) << "Master File Write Enable: "
                      << (masterFileWriteEnable ? "enabled" : "disabled");
}

bool Implementation::getOverwriteEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return overwriteEnable;
}

void Implementation::setOverwriteEnable(const bool b) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    overwriteEnable = b;
    LOG(logINFO) << "Overwrite Enable: " << (overwriteEnable ? "enabled" : "disabled");
}

uint32_t Implementation::getFramesPerFile() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return framesPerFile;
}

void Implementation::setFramesPerFile(const uint32_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    framesPerFile = i;
    LOG(logINFO) << "Frames per file: " << framesPerFile;
}


/**************************************************
 *                                                 *
 *   Acquisition                                   *
 *                                                 *
 * ************************************************/
slsDetectorDefs::runStatus Implementation::getStatus() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return status;
}

uint64_t Implementation::getFramesCaught() const {
    // no data processed
    if (!dataProcessor->GetStartedFlag()) {
        return 0;
    }
    return dataProcessor->GetNumFramesCaught();
}

uint64_t Implementation::getAcquisitionIndex() const {
    // no data processed
    if (!dataProcessor->GetStartedFlag()) {
        return 0;
    }
    return dataProcessor->GetCurrentFrameIndex();
}

int Implementation::getProgress() const {
    uint64_t currentFrameIndex = 0;
    // data processed
    if (dataProcessor->GetStartedFlag()) {
        currentFrameIndex = dataProcessor->GetProcessedIndex();
    }
    return (100.00 * ((double)(currentFrameIndex) / (double)numberOfTotalFrames));
}

uint64_t Implementation::getNumMissingPackets() const {
    uint64_t mp = 0;
    int np = generalData->packetsPerFrame;
    uint64_t totnp = np;
    // partial readout
    if (numLinesReadout != MAX_EIGER_ROWS_PER_READOUT) {
        totnp = ((numLinesReadout * np) / MAX_EIGER_ROWS_PER_READOUT);
    }     
    totnp *= numberOfTotalFrames;
    mp = listener->GetNumMissingPacket(stoppedFlag, totnp);
    return mp;
}

void Implementation::startReceiver() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    LOG(logINFO) << "Starting Receiver";
    stoppedFlag = false;
    ResetParametersforNewAcquisition();

    // listener
    CreateUDPSocket();

    // callbacks
    if (startAcquisitionCallBack) {
        startAcquisitionCallBack(filePath, fileName, fileIndex,
                                 (generalData->imageSize) +
                                     (generalData->fifoBufferHeaderSize),
                                 pStartAcquisition);
        if (rawDataReadyCallBack != nullptr) {
            LOG(logINFO) << "Data Write has been defined externally";
        }
    }

    // processor->writer
    if (fileWriteEnable) {
        SetupWriter();
    } else
        LOG(logINFO) << "File Write Disabled";

    LOG(logINFO) << "Ready ...";

    // status
    status = RUNNING;

    // Let Threads continue to be ready for acquisition
    StartRunning();

    LOG(logINFO) << "Receiver Started";
    LOG(logINFO) << "Status: " << sls::ToString(status);
}

void Implementation::setStoppedFlag(bool stopped) {
    stoppedFlag = stopped;
}

void Implementation::stopReceiver() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    LOG(logINFO) << "Stopping Receiver";

    // set status to transmitting
    startReadout();

    // wait for the processes (Listener and DataProcessor) to be done
    bool running = true;
    while (running) {
        running = false;
        if (listener->IsRunning())
            running = true;
        if (dataProcessor->IsRunning())
            running = true;
        usleep(5000);
    }

    // create virtual file
    if (fileWriteEnable && fileFormatType == HDF5 && interfaceId == 0) {
        // to create virtual file & set files/acquisition to 0 (only hdf5 at the
        // moment)
        dataProcessor->EndofAcquisition(
            dataProcessor->GetStartedFlag(), 
            dataProcessor->GetProcessedIndex());
    }

    // wait for the processes (dataStreamer) to be done
    if (dataStreamEnable) {
        running = true;
        while (running) {
            running = dataStreamer->IsRunning();
            usleep(5000);
        }
    }

    status = RUN_FINISHED;
    LOG(logINFO) << "Status: " << sls::ToString(status);

    { // statistics
        uint64_t mp = getNumMissingPackets();
        uint64_t tot = dataProcessor->GetNumFramesCaught();

        TLogLevel lev =
            (((int64_t)mp) > 0) ? logINFORED : logINFOGREEN;
        LOG(lev) <<
            // udp port number could be the second if selected interface is
            // 2 for jungfrau
            "Summary of Port " << udpPortNum
                        << "\n\tMissing Packets\t\t: " << mp
                        << "\n\tComplete Frames\t\t: " << tot
                        << "\n\tLast Frame Caught\t: "
                        << listener->GetLastFrameIndexCaught();
        if (!activated) {
            LOG(logINFORED) << "Deactivated Receiver";
        }
        // callback
        if (acquisitionFinishedCallBack)
            acquisitionFinishedCallBack(tot, pAcquisitionFinished);
    }

    // change status
    status = IDLE;

    LOG(logINFO) << "Receiver Stopped";
    LOG(logINFO) << "Status: " << sls::ToString(status);
}

void Implementation::startReadout() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    if (status == RUNNING) {
        // wait for incoming delayed packets
        int totalPacketsReceived = listener->GetPacketsCaught();
        int previousValue = -1;

        // wait for all packets
        const int numPacketsToReceive =
            numberOfTotalFrames * generalData->packetsPerFrame;
        if (totalPacketsReceived != numPacketsToReceive) {
            while (totalPacketsReceived != previousValue) {
                LOG(logDEBUG3)
                    << "waiting for all packets, previousValue:"
                    << previousValue
                    << " totalPacketsReceived: " << totalPacketsReceived;
                usleep(5 * 1000); /* TODO! Need to find optimal time **/
                previousValue = totalPacketsReceived;
                totalPacketsReceived = listener->GetPacketsCaught();

                LOG(logDEBUG3) << "\tupdated:  totalPacketsReceived:"
                                    << totalPacketsReceived;
            }
        }
        status = TRANSMITTING;
        LOG(logINFO) << "Status: Transmitting";
    }
    // shut down udp socket to make listeners push dummy (end) packets for
    // processors
    shutDownUDPSocket();
}

void Implementation::shutDownUDPSocket() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    listener->ShutDownUDPSocket();
}

void Implementation::closeFiles() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    dataProcessor->CloseFiles();
    // to create virtual file & set files/acquisition to 0 (only hdf5 at the
    // moment)
    if (interfaceId == 0) {
        dataProcessor->EndofAcquisition(
            dataProcessor->GetStartedFlag(),
            dataProcessor->GetProcessedIndex());
    }
}

void Implementation::restreamStop() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    if (dataStreamEnable) {
        dataStreamer->RestreamStop();
        LOG(logINFO) << "Restreaming Dummy Header via ZMQ successful";
    }
}

void Implementation::ResetParametersforNewAcquisition() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    listener->ResetParametersforNewAcquisition();
    dataProcessor->ResetParametersforNewAcquisition();

    if (dataStreamEnable) {
        std::ostringstream os;
        os << filePath << '/' << fileName;
        std::string fnametostream = os.str();
        dataStreamer->ResetParametersforNewAcquisition(fnametostream);
    }
}

void Implementation::CreateUDPSocket() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    try{
        listener->CreateUDPSocket();
    } catch(const sls::RuntimeError &e) {
        shutDownUDPSocket();
        throw sls::RuntimeError("Could not create UDP Socket.");
    }

    LOG(logDEBUG) << "UDP socket(s) created successfully.";
}

void Implementation::SetupWriter() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
	masterAttributes attr;
	attr.detectorType = myDetectorType;
	attr.dynamicRange = dynamicRange;
	attr.tenGiga = tengigaEnable;
	attr.imageSize = generalData->imageSize;
	attr.nPixelsX = generalData->nPixelsX;
	attr.nPixelsY = generalData->nPixelsY;
	attr.maxFramesPerFile = framesPerFile;
	attr.totalFrames = numberOfTotalFrames;
	attr.exptimeNs = acquisitionTime;
	attr.subExptimeNs = subExpTime;
	attr.subPeriodNs = subPeriod;
	attr.periodNs = acquisitionPeriod;
    attr.quadEnable = quadEnable;
    attr.analogFlag = (readoutType == ANALOG_ONLY || readoutType == ANALOG_AND_DIGITAL) ? 1 : 0;
    attr.digitalFlag = (readoutType == DIGITAL_ONLY || readoutType == ANALOG_AND_DIGITAL) ? 1 : 0;
    attr.adcmask = tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga;
    attr.dbitoffset = ctbDbitOffset;
    attr.dbitlist = 0;
    attr.roiXmin = roi.xmin;
    attr.roiXmax = roi.xmax;
    for (auto &i : ctbDbitList) {
        attr.dbitlist |= (1 << i);
    }

    try {
        dataProcessor->CreateNewFile(attr);
    } catch(const sls::RuntimeError &e) {
        shutDownUDPSocket();
        closeFiles();
        throw sls::RuntimeError("Could not create file.");
    }
}

void Implementation::StartRunning() {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    // set running mask and post semaphore to start the inner loop in execution
    // thread
    listener->StartRunning();
    listener->Continue();
    dataProcessor->StartRunning();
    dataProcessor->Continue();
    if (dataStreamEnable) {
        dataStreamer->StartRunning();
        dataStreamer->Continue();
    }
}


/**************************************************
 *                                                 *
 *   Network Configuration (UDP)                   *
 *                                                 *
 * ************************************************/
int Implementation::getNumberofUDPInterfaces() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numUDPInterfaces;
}

void Implementation::setNumberofUDPInterfaces(const int n) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    if (numUDPInterfaces != n) {

        numUDPInterfaces = n;
        generalData->SetNumberofInterfaces(n);
        udpSocketBufferSize = generalData->defaultUdpSocketBufferSize;

        SetupFifoStructure();
        setDetectorSize(numDet);
        // test socket buffer size with current set up
        setUDPSocketBufferSize(0);
    }

    LOG(logINFO) << "Number of Interfaces: " << numUDPInterfaces;
}

int Implementation::getInterfaceId() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return interfaceId;
}

void Implementation::setInterfaceId(const int i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    interfaceId = i;
    LOG(logINFO) << "Interface Id: " << interfaceId;
}

std::string Implementation::getEthernetInterface() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return eth;
}

void Implementation::setEthernetInterface(const std::string &c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    eth = c;
    LOG(logINFO) << "Ethernet Interface: " << eth;
}

uint32_t Implementation::getUDPPortNumber() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpPortNum;
}

void Implementation::setUDPPortNumber(const uint32_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    udpPortNum = i;
    LOG(logINFO) << "UDP Port Number[0]: " << udpPortNum;
}

int64_t Implementation::getUDPSocketBufferSize() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return udpSocketBufferSize;
}

void Implementation::setUDPSocketBufferSize(const int64_t s) {
    int64_t size = (s == 0) ? udpSocketBufferSize : s;
    listener->CreateDummySocketForUDPSocketBufferSize(size);
}

int64_t Implementation::getActualUDPSocketBufferSize() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return actualUDPSocketBufferSize;
}


/**************************************************
 *                                                 *
 *   ZMQ Streaming Parameters (ZMQ)                *
 *                                                 *
 * ************************************************/
bool Implementation::getDataStreamEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return dataStreamEnable;
}

void Implementation::setDataStreamEnable(const bool enable) {

    if (dataStreamEnable != enable) {
        dataStreamEnable = enable;

        // data sockets have to be created again as the client ones are
        dataStreamer.reset();

        if (enable) {
            try {
                int fd = flippedDataX;
                if (quadEnable) {
                    fd = interfaceId;
                }
                dataStreamer = sls::make_unique<DataStreamer>(
                    0, fifo.get(), &dynamicRange, &roi, &fileIndex,
                    fd, (int*)numRx, &quadEnable, &numberOfTotalFrames);
                dataStreamer->SetGeneralData(generalData);
                dataStreamer->CreateZmqSockets(
                    &numUDPInterfaces, streamingPort, streamingSrcIP);
                dataStreamer->SetAdditionalJsonHeader(additionalJsonHeader);
            } catch (...) {
                dataStreamer.reset();
                dataStreamEnable = false;
                throw sls::RuntimeError("Could not set data stream enable.");
            }
            SetThreadPriorities();
        }
    }
    LOG(logINFO) << "Data Send to Gui: " << dataStreamEnable;
}

uint32_t Implementation::getStreamingFrequency() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingFrequency;
}

void Implementation::setStreamingFrequency(const uint32_t freq) {
    if (streamingFrequency != freq) {
        streamingFrequency = freq;
    }
    LOG(logINFO) << "Streaming Frequency: " << streamingFrequency;
}

uint32_t Implementation::getStreamingTimer() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingTimerInMs;
}

void Implementation::setStreamingTimer(const uint32_t time_in_ms) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    streamingTimerInMs = time_in_ms;
    LOG(logINFO) << "Streamer Timer: " << streamingTimerInMs;
}

uint32_t Implementation::getStreamingPort() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingPort;
}

void Implementation::setStreamingPort(const uint32_t i) {
    streamingPort = i;

    LOG(logINFO) << "Streaming Port: " << streamingPort;
}

sls::IpAddr Implementation::getStreamingSourceIP() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return streamingSrcIP;
}

void Implementation::setStreamingSourceIP(const sls::IpAddr ip) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    streamingSrcIP = ip;
    LOG(logINFO) << "Streaming Source IP: " << streamingSrcIP;
}

std::map<std::string, std::string> Implementation::getAdditionalJsonHeader() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return additionalJsonHeader;
}

void Implementation::setAdditionalJsonHeader(const std::map<std::string, std::string> &c) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    additionalJsonHeader = c;
    if (dataStreamEnable) {
	    dataStreamer->SetAdditionalJsonHeader(c);
    }
    LOG(logINFO) << "Additional JSON Header: " << sls::ToString(additionalJsonHeader);
}

std::string Implementation::getAdditionalJsonParameter(const std::string &key) const {
    if (additionalJsonHeader.find(key) != additionalJsonHeader.end()) {
        return additionalJsonHeader.at(key);
    }
    throw sls::RuntimeError("No key " + key + " found in additional json header");
}

void Implementation::setAdditionalJsonParameter(const std::string &key, const std::string &value) {
    auto pos = additionalJsonHeader.find(key);

    // if value is empty, delete
    if (value.empty()) {
        // doesnt exist
        if (pos == additionalJsonHeader.end()) {
            LOG(logINFO) << "Additional json parameter (" << key << ") does not exist anyway";
        } else {
            LOG(logINFO) << "Deleting additional json parameter (" << key << ")";
            additionalJsonHeader.erase(pos);
        }
    }
    // if found, set it
    else if (pos != additionalJsonHeader.end()) {
        additionalJsonHeader[key] = value;
        LOG(logINFO) << "Setting additional json parameter (" << key << ") to " << value;
    } 
    // append if not found
    else {
        additionalJsonHeader[key] = value;
        LOG(logINFO) << "Adding additional json parameter (" << key << ") to " << value;
    }
    if (dataStreamEnable) {
        dataStreamer->SetAdditionalJsonHeader(additionalJsonHeader);
    }
    LOG(logINFO) << "Additional JSON Header: " << sls::ToString(additionalJsonHeader);
}

/**************************************************
 *                                                 *
 *   Detector Parameters                           *
 *                                                 *
 * ************************************************/
void Implementation::updateTotalNumberOfFrames() {
    int64_t repeats = numberOfTriggers;
    // gotthard2: auto mode 
    // burst mode: (bursts instead of triggers)
    // non burst mode: no bursts or triggers
    if (myDetectorType == GOTTHARD2 &&timingMode == AUTO_TIMING) {
        if (burstMode == BURST_OFF) {
            repeats = numberOfBursts;
        } else {
            repeats = 1;
        }
    }
    numberOfTotalFrames = numberOfFrames * repeats * 
        (int64_t)(numberOfAdditionalStorageCells + 1);
    if (numberOfTotalFrames == 0) {
        throw sls::RuntimeError("Invalid total number of frames to receive: 0");
    }
    LOG(logINFO) << "Total Number of Frames: " << numberOfTotalFrames;
}

uint64_t Implementation::getNumberOfFrames() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfFrames;
}

void Implementation::setNumberOfFrames(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numberOfFrames = i;
    LOG(logINFO) << "Number of Frames: " << numberOfFrames;
    updateTotalNumberOfFrames();
}

uint64_t Implementation::getNumberOfTriggers() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfTriggers;
}

void Implementation::setNumberOfTriggers(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numberOfTriggers = i;
    LOG(logINFO) << "Number of Triggers: " << numberOfTriggers;
    updateTotalNumberOfFrames();
}

uint64_t Implementation::getNumberOfBursts() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfBursts;
}

void Implementation::setNumberOfBursts(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numberOfBursts = i;
    LOG(logINFO) << "Number of Bursts: " << numberOfBursts;
    updateTotalNumberOfFrames();
}

int Implementation::getNumberOfAdditionalStorageCells() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfAdditionalStorageCells;
}

void Implementation::setNumberOfAdditionalStorageCells(const int i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    numberOfAdditionalStorageCells = i;
    LOG(logINFO) << "Number of Additional Storage Cells: " << numberOfAdditionalStorageCells;
    updateTotalNumberOfFrames();
}

slsDetectorDefs::timingMode Implementation::getTimingMode() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return timingMode;
}

void Implementation::setTimingMode(const slsDetectorDefs::timingMode i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    timingMode = i;
    LOG(logINFO) << "Timing Mode: " << timingMode;
    updateTotalNumberOfFrames();
}

slsDetectorDefs::burstMode Implementation::getBurstMode() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return burstMode;
}

void Implementation::setBurstMode(const slsDetectorDefs::burstMode i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    burstMode = i;
    LOG(logINFO) << "Burst Mode: " << burstMode;
    updateTotalNumberOfFrames();
}

uint64_t Implementation::getAcquisitionPeriod() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return acquisitionPeriod;
}

void Implementation::setAcquisitionPeriod(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionPeriod = i;
    LOG(logINFO) << "Acquisition Period: "
                      << (double)acquisitionPeriod / (1E9) << "s";
}

uint64_t Implementation::getAcquisitionTime() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return acquisitionTime;
}

void Implementation::setAcquisitionTime(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    acquisitionTime = i;
    LOG(logINFO) << "Acquisition Time: " << (double)acquisitionTime / (1E9)
                      << "s";
}

uint64_t Implementation::getSubExpTime() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return subExpTime;
}

void Implementation::setSubExpTime(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    subExpTime = i;
    LOG(logINFO) << "Sub Exposure Time: " << (double)subExpTime / (1E9)
                      << "s";
}

uint64_t Implementation::getSubPeriod() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return subPeriod;
}

void Implementation::setSubPeriod(const uint64_t i) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";

    subPeriod = i;
    LOG(logINFO) << "Sub Period: " << (double)subPeriod / (1E9)
                      << "s";
}

uint32_t Implementation::getNumberofAnalogSamples() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfAnalogSamples;
}

void Implementation::setNumberofAnalogSamples(const uint32_t i) {
    if (numberOfAnalogSamples != i) {
        numberOfAnalogSamples = i;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);
        
        dataProcessor->SetPixelDimension();
        SetupFifoStructure();
    }
    LOG(logINFO) << "Number of Analog Samples: " << numberOfAnalogSamples;
    LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

uint32_t Implementation::getNumberofDigitalSamples() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfDigitalSamples;
}

void Implementation::setNumberofDigitalSamples(const uint32_t i) {
    if (numberOfDigitalSamples != i) {
        numberOfDigitalSamples = i;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);
        
        dataProcessor->SetPixelDimension();
        SetupFifoStructure();
    }
    LOG(logINFO) << "Number of Digital Samples: "
                      << numberOfDigitalSamples;
    LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

int Implementation::getNumberofCounters() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return numberOfCounters;
}

void Implementation::setNumberofCounters(const int i) {
    if (numberOfCounters != i) {
        numberOfCounters = i;

        if (myDetectorType == MYTHEN3) {
            generalData->SetNumberofCounters(i, dynamicRange);
            // to update npixelsx, npixelsy in file writer
            dataProcessor->SetPixelDimension();
            SetupFifoStructure();
        }
    }
    LOG(logINFO) << "Number of Counters: " << numberOfCounters;
}

uint32_t Implementation::getDynamicRange() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return dynamicRange;
}

void Implementation::setDynamicRange(const uint32_t i) {

    if (dynamicRange != i) {
        dynamicRange = i;

        if (myDetectorType == EIGER || myDetectorType == MYTHEN3) {
            generalData->SetDynamicRange(i, tengigaEnable);
            // to update npixelsx, npixelsy in file writer
            dataProcessor->SetPixelDimension();
            fifoDepth = generalData->defaultFifoDepth;
            SetupFifoStructure();
        }
    }
    LOG(logINFO) << "Dynamic Range: " << dynamicRange;
}

slsDetectorDefs::ROI Implementation::getROI() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return roi;
}

void Implementation::setROI(slsDetectorDefs::ROI arg) {
    if (roi.xmin != arg.xmin || roi.xmax != arg.xmax) {
        roi.xmin = arg.xmin;
        roi.xmax = arg.xmax;

        // only for gotthard
        generalData->SetROI(arg);
        framesPerFile = generalData->maxFramesPerFile;
        dataProcessor->SetPixelDimension();
        SetupFifoStructure();
    }

    LOG(logINFO) << "ROI: [" << roi.xmin << ", " << roi.xmax << "]";;
    LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

bool Implementation::getTenGigaEnable() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return tengigaEnable;
}

void Implementation::setTenGigaEnable(const bool b) {
    if (tengigaEnable != b) {
        tengigaEnable = b;
        // side effects
        switch (myDetectorType) {
        case EIGER:
            generalData->SetTenGigaEnable(b, dynamicRange);
            break;
        case MOENCH:
        case CHIPTESTBOARD:
            ctbAnalogDataBytes = generalData->setImageSize(
                tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
                numberOfAnalogSamples, numberOfDigitalSamples,
                tengigaEnable, readoutType);
            break;
        default:
            break;
        }

        SetupFifoStructure();
    }
    LOG(logINFO) << "Ten Giga: " << (tengigaEnable ? "enabled" : "disabled");
    LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

int Implementation::getFlippedDataX() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return flippedDataX;
}

void Implementation::setFlippedDataX(int enable) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    flippedDataX = (enable == 0) ? 0 : 1;

    if (dataStreamEnable) {
        if (!quadEnable) {
            dataStreamer->SetFlippedDataX(flippedDataX);
        } else {
            dataStreamer->SetFlippedDataX(interfaceId);	
        }
    }

    LOG(logINFO) << "Flipped Data X: " << flippedDataX;
}

bool Implementation::getQuad() const {
	LOG(logDEBUG) << __AT__ << " starting";
	return quadEnable;
}

void Implementation::setQuad(const bool b) {
	if (quadEnable != b) {
		quadEnable = b;

        if (dataStreamEnable) {
            if (!quadEnable) {
                dataStreamer->SetReceiverShape(numRx);
                dataStreamer->SetFlippedDataX(flippedDataX);
            } else {
                int size[2] = {1, 2};
                dataStreamer->SetReceiverShape(size);
                dataStreamer->SetFlippedDataX(interfaceId);	
            }
        }
	}
	LOG(logINFO)  << "Quad Enable: " << quadEnable;
}

bool Implementation::getActivate() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return activated;
}

bool Implementation::setActivate(bool enable) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    activated = enable;
    LOG(logINFO) << "Activation: " << (activated ? "enabled" : "disabled");
    return activated;
}

bool Implementation::getDeactivatedPadding() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return deactivatedPaddingEnable;
}

bool Implementation::setDeactivatedPadding(bool enable) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    deactivatedPaddingEnable = enable;
    LOG(logINFO) << "Deactivated Padding Enable: "
                      << (deactivatedPaddingEnable ? "enabled" : "disabled");
    return deactivatedPaddingEnable;
}

int Implementation::getReadNLines() const {
	LOG(logDEBUG) << __AT__ << " starting";
	return numLinesReadout;
}

void Implementation::setReadNLines(const int value) {
    numLinesReadout = value;
	LOG(logINFO)  << "Number of Lines to readout: " << numLinesReadout;
}

slsDetectorDefs::readoutMode
Implementation::getReadoutMode() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return readoutType;
}

void Implementation::setReadoutMode(const readoutMode f) {
    if (readoutType != f) {
        readoutType = f;

        // side effects
        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);
        dataProcessor->SetPixelDimension();
        SetupFifoStructure();
    }

    LOG(logINFO) << "Readout Mode: " << sls::ToString(f);
    LOG(logINFO) << "Packets per Frame: "
                          << (generalData->packetsPerFrame);
}

uint32_t Implementation::getADCEnableMask() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return adcEnableMaskOneGiga;
}

void Implementation::setADCEnableMask(uint32_t mask) {
    if (adcEnableMaskOneGiga != mask) {
        adcEnableMaskOneGiga = mask;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);

        dataProcessor->SetPixelDimension();
        SetupFifoStructure();
    }

    LOG(logINFO) << "ADC Enable Mask for 1Gb mode: 0x" << std::hex << adcEnableMaskOneGiga
                      << std::dec;
    LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

uint32_t Implementation::getTenGigaADCEnableMask() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return adcEnableMaskTenGiga;
}

void Implementation::setTenGigaADCEnableMask(uint32_t mask) {
    if (adcEnableMaskTenGiga != mask) {
        adcEnableMaskTenGiga = mask;

        ctbAnalogDataBytes = generalData->setImageSize(
            tengigaEnable ? adcEnableMaskTenGiga : adcEnableMaskOneGiga, 
            numberOfAnalogSamples, numberOfDigitalSamples,
            tengigaEnable, readoutType);

        dataProcessor->SetPixelDimension();
        SetupFifoStructure();
    }

    LOG(logINFO) << "ADC Enable Mask for 10Gb mode: 0x" << std::hex << adcEnableMaskTenGiga
                      << std::dec;
    LOG(logINFO) << "Packets per Frame: "
                      << (generalData->packetsPerFrame);
}

std::vector<int> Implementation::getDbitList() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return ctbDbitList;
}

void Implementation::setDbitList(const std::vector<int> v) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    ctbDbitList = v;
}

int Implementation::getDbitOffset() const {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    return ctbDbitOffset;
}

void Implementation::setDbitOffset(const int s) {
    LOG(logDEBUG3) << __SHORT_AT__ << " called";
    ctbDbitOffset = s;
}


/**************************************************
 *                                                *
 *    Callbacks                                   *
 *                                                *
 * ************************************************/
void Implementation::registerCallBackStartAcquisition(
    int (*func)(std::string, std::string, uint64_t, uint32_t, void *), void *arg) {
    startAcquisitionCallBack = func;
    pStartAcquisition = arg;
}

void Implementation::registerCallBackAcquisitionFinished(
    void (*func)(uint64_t, void *), void *arg) {
    acquisitionFinishedCallBack = func;
    pAcquisitionFinished = arg;
}

void Implementation::registerCallBackRawDataReady(
    void (*func)(char *, char *, uint32_t, void *), void *arg) {
    rawDataReadyCallBack = func;
    pRawDataReady = arg;
    dataProcessor->registerCallBackRawDataReady(rawDataReadyCallBack, pRawDataReady);
}

void Implementation::registerCallBackRawDataModifyReady(
    void (*func)(char *, char *, uint32_t &, void *), void *arg) {
    rawDataModifyReadyCallBack = func;
    pRawDataReady = arg;
    dataProcessor->registerCallBackRawDataModifyReady(rawDataModifyReadyCallBack,
                                               pRawDataReady);
}

