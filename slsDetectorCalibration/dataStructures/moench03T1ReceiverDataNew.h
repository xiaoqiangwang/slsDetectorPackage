#ifndef MOENCH03T1RECDATANEW_H
#define  MOENCH03T1RECDATANEW_H
#include "slsDetectorData.h"

//#define VERSION_V2
    /**
        @short  structure for a Detector Packet or Image Header
        @li frameNumber is the frame number
        @li expLength is the subframe number (32 bit eiger) or real time exposure time in 100ns (others)
        @li packetNumber is the packet number
        @li bunchId is the bunch id from beamline
        @li timestamp is the time stamp with 10 MHz clock
        @li modId is the unique module id (unique even for left, right, top, bottom)
        @li xCoord is the x coordinate in the complete detector system
        @li yCoord is the y coordinate in the complete detector system
        @li zCoord is the z coordinate in the complete detector system
        @li debug is for debugging purposes
        @li roundRNumber is the round robin set number
        @li detType is the detector type see :: detectorType
        @li version is the version number of this structure format
    */
    typedef struct {
        uint64_t frameNumber;    /**< is the frame number */
        uint32_t expLength;        /**< is the subframe number (32 bit eiger) or real time exposure time in 100ns (others) */
        uint32_t packetNumber;    /**< is the packet number */
        uint64_t bunchId;        /**< is the bunch id from beamline */
        uint64_t timestamp;        /**< is the time stamp with 10 MHz clock */
        uint16_t modId;            /**< is the unique module id (unique even for left, right, top, bottom) */
        uint16_t xCoord;        /**< is the x coordinate in the complete detector system */
        uint16_t yCoord;        /**< is the y coordinate in the complete detector system */
        uint16_t zCoord;        /**< is the z coordinate in the complete detector system */
        uint32_t debug;            /**< is for debugging purposes */
        uint16_t roundRNumber;    /**< is the round robin set number */
        uint8_t detType;        /**< is the detector type see :: detectorType */
        uint8_t version;        /**< is the version number of this structure format */
#ifndef VERSION_V1
      uint64_t packetCaught[8];        /**< is the version number of this structure format */
#endif

    } sls_detector_header;




class moench03T1ReceiverDataNew : public slsDetectorData<uint16_t> {
 
 private:
  
  int iframe;
  int nadc;
  int sc_width;
  int sc_height;
  const int nSamples;

  double ghost[200][25];

 public:




  /**
     Implements the slsReceiverData structure for the moench02 prototype read out by a module i.e. using the slsReceiver
     (160x160 pixels, 40 packets 1286 large etc.)
     \param c crosstalk parameter for the output buffer

  */
 moench03T1ReceiverDataNew(int ns=5000): slsDetectorData<uint16_t>(400, 400, ns*2*32+sizeof(sls_detector_header)),  nSamples(ns) {

    int nadc=32;
    int sc_width=25;
    int sc_height=200;

    int adc_nr[32]={300,325,350,375,300,325,350,375,		\
    		    200,225,250,275,200,225,250,275,\
    		    100,125,150,175,100,125,150,175,\
    		    0,25,50,75,0,25,50,75};

    int row, col;

    int isample;
    int iadc;
    int ix, iy;
    
     int npackets=40;
    int i;
    int adc4(0);

    for (int ip=0; ip<npackets; ip++) {
      for (int is=0; is<128; is++) {

	for (iadc=0; iadc<nadc; iadc++) {
	  i=128*ip+is;
	  adc4=(int)iadc/4;
	  if (i<sc_width*sc_height) {
	    //  for (int i=0; i<sc_width*sc_height; i++) {
	    col=adc_nr[iadc]+(i%sc_width);
	    if (adc4%2==0) {
	      row=199-i/sc_width;
	    } else {
	      row=200+i/sc_width;
	    }
	    dataMap[row][col]=sizeof(sls_detector_header)+(nadc*i+iadc)*2;//+16*(ip+1);  
#ifdef HIGHZ
	    dataMask[row][col]=0x3fff; //invert data
#endif
	    if (dataMap[row][col]<0 || dataMap[row][col]>=nSamples*2*32)
	      cout << "Error: pointer " << dataMap[row][col] << " out of range "<< endl;
	  }
	}
      }
    }
    // double ghost[200][25];

    for (int ix=0; ix<25; ix++)
      for (int iy=0; iy<200; iy++)
	ghost[iy][ix]=0.;

    int ipacket;
    int ibyte;
    int ii=0;
    for (ibyte=0; ibyte<sizeof(sls_detector_header)/2; ibyte++){
      xmap[ibyte]=-1;
      ymap[ibyte]=-1;
    }
    int off=sizeof(sls_detector_header)/2;
    for (ipacket=0; ipacket<npackets; ipacket++) {
      for (ibyte=0;  ibyte< 8192/2; ibyte++) {
	i=ipacket*8208/2+ibyte;
	  isample=ii/nadc;
	  if (isample<nSamples) {
	  iadc=ii%nadc;
	  adc4 = (int)iadc/4;
	  ix=isample%sc_width;
	  iy=isample/sc_width;
	  if (adc4%2==0) {
	    xmap[i+off]=adc_nr[iadc]+ix;
	    ymap[i+off]=ny/2-1-iy;
	  } else {
	    xmap[i+off]=adc_nr[iadc]+ix;
	    ymap[i+off]=ny/2+iy;
	  }
	  }
	ii++;
	//	}
      }
    }
    
    iframe=0;
    //  cout << "data struct created" << endl;
  };
    

 
  /**
     Returns the value of the selected channel for the given dataset as double.
     \param data pointer to the dataset (including headers etc)
     \param ix pixel number in the x direction
     \param iy pixel number in the y direction
     \returns data for the selected channel, with inversion if required as double

  */
  virtual double getValue(char *data, int ix, int iy=0) {
    /* cout << " x "<< ix << " y"<< iy << " val " << getChannel(data, ix, iy)<< endl;*/
    /* double val=0, vout=getChannel(data, ix, iy); */
    /* int x1=ix%25; */
    /* for (int ix=0; ix<16; ix++) { */
    /*   for (int ii=0; ii<2; ii++) { */
    /* 	val+=getChannel(data,x1+25*ix,iy); */
    /* 	val+=getChannel(data,x1+25*ix,399-iy); */
    /*   } */
    /* } */
    /* vout+=0.0008*val-6224; */
    /* return vout; //(double)getChannel(data, ix, iy);
     */
    uint16_t val=getChannel(data, ix, iy)&0x3fff;
    return val;
  };

   

  virtual void calcGhost(char *data, int ix, int iy) {
    double val=0;
    /* for (int ix=0; ix<25; ix++){ */
    /*   for (int iy=0; iy<200; iy++) { */
	val=0;
	//	cout << "** "; 
	for (int isc=0; isc<16; isc++) {
	  //  for (int ii=0; ii<2; ii++) {
	   val+=getChannel(data,ix+25*isc,iy);
	   //  cout << "(" << isc << "," << val << " " ;
	    val+=getChannel(data,ix+25*isc,399-iy);
	    // cout << val << " " ;
	    // }
	}
	ghost[iy][ix]=val;//-6224;
	//	cout << " --"<< endl;
    /*   }  */
    /* } */
    // cout << "*" << endl;
    
  }



  virtual void calcGhost(char *data) {
    for (int ix=0; ix<25; ix++){
      for (int iy=0; iy<200; iy++) {
	calcGhost(data, ix,iy);
      } 
    }
    // cout << "*" << endl;
  }


  double getGhost(int ix, int iy) {
    if (iy<200) return ghost[iy][ix%25]; 
    if (iy<400) return ghost[399-iy][ix%25];
    return 0;
  };

     /**

     Returns the frame number for the given dataset. Purely virtual func.
     \param buff pointer to the dataset
     \returns frame number

  */

/* class jfrau_packet_header_t { */
/*  public: */
/* 	unsigned char reserved[4]; */
/* 	unsigned char packetNumber[1]; */
/* 	unsigned char frameNumber[3]; */
/* 	unsigned char bunchid[8]; */
/* }; */



  int getFrameNumber(char *buff){return ((sls_detector_header*)buff)->frameNumber;};//*((int*)(buff+5))&0xffffff;};   

  /**

     Returns the packet number for the given dataset. purely virtual func
     \param buff pointer to the dataset
     \returns packet number number



  */
  int getPacketNumber(char *buff){return ((sls_detector_header*)buff)->packetNumber;}//((*(((int*)(buff+4))))&0xff)+1;};   

/*    /\** */

/*      Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). purely virtual func */
/*      \param data pointer to the memory to be analyzed */
/*      \param ndata reference to the amount of data found for the frame, in case the frame is incomplete at the end of the memory slot */
/*      \param dsize size of the memory slot to be analyzed */
/*      \returns pointer to the beginning of the last good frame (might be incomplete if ndata smaller than dataSize), or NULL if no frame is found  */

/*   *\/ */
/*     virtual  char *findNextFrame(char *data, int &ndata, int dsize){ndata=dsize; setDataSize(dsize);  return data;}; */


/*    /\** */

/*      Loops over a file stream until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). Can be overloaded for different kind of detectors!  */
/*      \param filebin input file stream (binary) */
/*      \returns pointer to the begin of the last good frame, NULL if no frame is found or last frame is incomplete */

/*   *\/ */
/*     virtual char *readNextFrame(ifstream &filebin){ */
/*       //	int afifo_length=0;   */
/*       uint16_t *afifo_cont;  */
/*       int ib=0; */
/*       if (filebin.is_open()) { */
/* 	afifo_cont=new uint16_t[dataSize/2]; */
/*  	while (filebin.read(((char*)afifo_cont)+ib,2)) { */
/* 	  ib+=2; */
/* 	  if (ib==dataSize) break; */
/* 	} */
/* 	if (ib>0) { */
/* 	  iframe++; */
/* 	  // cout << ib << "-" << endl; */
/* 	  return (char*)afifo_cont; */
/* 	} else { */
/* 	  delete [] afifo_cont; */
/* 	  return NULL; */
/* 	} */
/*       }      */
/*       return NULL; */
/*     }; */


  virtual char *readNextFrame(ifstream &filebin) {
    int ff=-1, np=-1;
    return readNextFrame(filebin, ff, np);
  };

  virtual char *readNextFrame(ifstream &filebin, int &ff) {
    int np=-1;
    return readNextFrame(filebin, ff, np);
  };

  virtual char *readNextFrame(ifstream &filebin, int& ff, int &np) {
	  char *data=new char[dataSize];
	  char *d=readNextFrame(filebin, ff, np, data);
	  if (d==NULL) {delete [] data; data=NULL;}
	  return data;
  }




  virtual char *readNextFrame(ifstream &filebin, int& ff, int &np, char *data) {
	  char *retval=0;
	  int  nd;
	  int fnum = -1;
	  np=0;
	  int  pn;
	  
	  //  cout << dataSize << endl;
	  if (ff>=0)
	    fnum=ff;

	  if (filebin.is_open()) {
	    if (filebin.read(data, dataSize) ){
	      ff=getFrameNumber(data);
	      np=getPacketNumber(data);
	      return data;
	    }
	  }
	    return NULL;
	    
	    
	  
  };



  /**

     Loops over a memory slot until a complete frame is found (i.e. all packets 0 to nPackets, same frame number). purely virtual func
     \param data pointer to the memory to be analyzed
     \param ndata reference to the amount of data found for the frame, in case the frame is incomplete at the end of the memory slot
     \param dsize size of the memory slot to be analyzed
     \returns pointer to the beginning of the last good frame (might be incomplete if ndata smaller than dataSize), or NULL if no frame is found 
     
  */
  virtual  char *findNextFrame(char *data, int &ndata, int dsize){
    if (dsize<dataSize) ndata=dsize;
    else ndata=dataSize;
    return data;

  }

  //int getPacketNumber(int x, int y) {return dataMap[y][x]/packetSize;};

};




#endif
