#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include "AgMD1.h"
#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"
#include "TObject.h"
#include "time.h"
#include "sys/time.h"

using namespace std;
///
/// Agilent IVI-C Driver Example Program
///
/// Initializes the driver and reads a few Identity interface properties.
/// May include additional instrument specific functionality.
///
/// See driver help topic "Programming with the IVI-COM Driver in Various Development Environments"
/// for additional programming information.
///

std::string ZeroPadNumber(int num);

int main()
{

  struct timespec start, end;

  struct timespec iniOfMeas, endOfMeas, endOfTransfer, endOfWriting;
  uint64_t MeasTime=0; 
  uint64_t DeadTime=0;

  ViInt64 NumRecords=500;
  ViInt64 points=100000;
  ViReal64 SampleRate=50.0E6;

  ViStatus status;
  ViSession session;
  ViChar str[128];
  ViReal64 real64;
  ViInt32 int32;
  ViBoolean simulate;
    
  ViReal64 WfmData[points+100]; // waveform data buffer

  //ViReal64 * WfmData = new ViReal64[points*2];
  //ViChar WfmData[points*2]; // waveform data buffer
  //ViInt16 WfmData[points*2];
  //ViInt32 WfmInt32[points*2];

  ViInt64 WaveformArrayActualSize;
  ViInt64 ActualRecords;
  ViInt64 ActualPoints[NumRecords];
  ViInt64 FirstValidPoint[NumRecords];
  ViReal64 InitialXOffset[NumRecords];
  ViReal64 InitialXTimeSeconds[NumRecords];
  ViReal64 InitialXTimeFraction[NumRecords];
  ViReal64 XIncrement;
  ViReal64 ScaleFactor=1;
  ViReal64 ScaleOffset=0;
  ViChar errMsg[256];
  
  // Edit resource and options as needed.  resource is ignored if option Simulate=true
  char resource[] = "PCI::INSTR0";
  
  // If desired, use 'DriverSetup= CAL=0' to prevent digitizer from doing a SelfCalibration each time
  // it is initialized or reset which is the default behavior.
  char options[]  = "DriverSetup=CAL=0";
  
  ViBoolean idQuery = VI_TRUE;
  ViBoolean reset   = VI_TRUE;
  
  // Initialize the driver.  See driver help topic "Initializing the IVI-C Driver" for additional information
  status = AgMD1_InitWithOptions(resource, idQuery, reset, options, &session);
  if(status)
    {
      // Initialization failed
      fprintf(stderr, "** InitWithOptions() Failed!\n");
		AgMD1_error_message(session, status, errMsg);
		fprintf(stderr, "Error: %s\n", errMsg);
		return 1;
    }
  fprintf(stderr, "Driver Initialized \n");
  

  //////////////////////////////////////
  // Measure single waveform on Channel1
  //////////////////////////////////////
  

  // Setup Channel 1
  status = AgMD1_ConfigureChannel(session, "Channel1", 2.0, 0.0, AGMD1_VAL_TRIGGER_COUPLING_DC, VI_TRUE); // Range, Offset, Coupling, Enabled
  
  // Setup acquisition - Records must be 1 for Channel.Measurement methods.
  // For multiple records use Channel.MutiRecordMeasurement methods.
  status = AgMD1_ConfigureAcquisition(session, NumRecords, points, SampleRate); // Records, PointsPerRecord, SampleRate
  status = AgMD1_GetAttributeViReal64(session, "", AGMD1_ATTR_SAMPLE_RATE, &real64);
  printf("# Sampling Rate:  %f\n", real64);  
  printf("# Recording Times:  %d\n", NumRecords);  
  printf("# Sampling Number per Record:  %d\n", points);
  status = AgMD1_SetAttributeViString(session, "", AGMD1_ATTR_ACTIVE_TRIGGER_SOURCE, "External1");
  status = AgMD1_SetAttributeViInt32(session, "External1", AGMD1_ATTR_TRIGGER_TYPE, AGMD1_VAL_IMMEDIATE_TRIGGER); // No trigger required
  
  // Calibrate, initiate measurement, and read the waveform data
  fprintf(stderr, "Calibrating...\n");
  status = AgMD1_SelfCalibrate(session);
  fprintf(stderr, "Measuring Waveform on Channel1...\n");

  clock_gettime(CLOCK_MONOTONIC_RAW, &start); 



  /////////////////////////////////////////
  ////////////// ROOT Setup ///////////////
  /////////////////////////////////////////
  
  Int_t fileNum=0;
  Int_t LoopNum=10;

  while (fileNum<LoopNum){


  clock_gettime(CLOCK_MONOTONIC_RAW, &iniOfMeas);


  status =AgMD1_InitiateAcquisition(session);
  fprintf(stderr, "Start Acquisition of %08d-th File \n", fileNum+1);    

  status = AgMD1_WaitForAcquisitionComplete(session, 10000);

  clock_gettime(CLOCK_MONOTONIC_RAW, &endOfMeas);

  string ZeroPadNum = ZeroPadNumber(fileNum+1);
  string fileName = ZeroPadNum+".root";
  string fileDir = "/scratch1/Digitizer/etc/";

  TFile *f = TFile::Open(TString(fileDir+fileName), "recreate");
  TTree *t = new TTree("trdata", "data tree");
  
  Int_t pts=0;
  Double_t wfmData[points+100];
  Int_t recordNum;
  Double_t initialTime;
  Double_t samplingRate;
  
  t->Branch("pts", &pts, "pts/I");
  t->Branch("wfmData", wfmData, "wfmData[pts]/D");
  t->Branch("recordNum", &recordNum, "recordNum/I");
  t->Branch("initialTime", &initialTime, "initialTime/D");
  t->Branch("samplingRate", &SampleRate, "SampleRate/D");
  
  /////////////////////////////////////////
  ////////////////// DAQ //////////////////
  /////////////////////////////////////////
   
  for (Int_t i_rec=0; i_rec<NumRecords; i_rec++){
    status = AgMD1_FetchMultiRecordWaveformReal64(
		session,
      		"Channel1",
		i_rec, // First Record index
		1, // # of Records
		1, // Offset Within Record
		points,
		sizeof(WfmData)/sizeof(WfmData[0]),		
		WfmData,
		&WaveformArrayActualSize,
		&ActualRecords,
		ActualPoints,
		FirstValidPoint,
		InitialXOffset,
		InitialXTimeSeconds,
		InitialXTimeFraction,
		&XIncrement
		//&ScaleFactor,
                //&ScaleOffset
		);
       
    for (Int_t i = FirstValidPoint[0]; i < ActualPoints[0]; i++)
      {       
	wfmData[pts]=WfmData[i];
	pts++;	
      }
    

    /////////////////////////////
    /////// Binary //////////////
    /////////////////////////////

    /*
    std::ofstream datFile;
    std::string datIndex = ZeroPadNumber(i_rec+1);
    std::string datName =  datIndex+".dat";
    std::string datDir = "/scratch1/Digitizer/etc/";
    //std::string datDir = "/home/bomki/repository/trash/";
    datFile.open(datDir+datName, ios::binary | ios::out);
    datFile.write((char*)WfmData,sizeof(WfmData));   
    datFile.close();
    */
    ///////////////////////////////
    

    recordNum=i_rec;
    initialTime=InitialXTimeSeconds[0]+InitialXTimeFraction[0];
    //t->Fill();    
    pts=0;
    memset(wfmData,0,sizeof(wfmData));
    memset(WfmData,0,sizeof(WfmData));
    
  }  

  f->cd();
  //t->Write();
  f->Close();

  clock_gettime(CLOCK_MONOTONIC_RAW, &endOfWriting);

  MeasTime += (endOfMeas.tv_sec - iniOfMeas.tv_sec)*1000000 + (endOfMeas.tv_nsec - iniOfMeas.tv_nsec) / 1000;
  DeadTime += (endOfWriting.tv_sec - endOfMeas.tv_sec)*1000000 + (endOfWriting.tv_nsec - endOfMeas.tv_nsec) / 1000;

  fileNum++;

  }


  std::cout << "Took in Microseconds: " << MeasTime/LoopNum << std::endl;
  std::cout << "Took in Microseconds: " << DeadTime/LoopNum << std::endl;


  //////////////////////////////////////////////////////////////////
  
  if (status)
    {
      AgMD1_error_message(session, status, errMsg);
      fprintf(stderr, "Error: %s\n", errMsg);
      goto exit;
    }
  
 exit:
  // Close the driver

  
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  uint64_t delta_us = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  std::cout << "Took in Microseconds: " << delta_us << std::endl;
  

 


  status = AgMD1_close(session);
  if(status)
    fprintf(stderr, "\n** Close() Failed!\n");
  else
    fprintf(stderr, "\nDriver Closed \n");
   
  
  return 0;
}

std::string ZeroPadNumber(int num)
{
  stringstream ss;
 
  // the number is converted to string with the help of stringstream
  ss << num; 
  string ret;
  ss >> ret;
  
  // Append zero chars
  int str_length = ret.length();
  for (int i = 0; i < 8 - str_length; i++)
    ret = "0" + ret;
  return ret;
}
