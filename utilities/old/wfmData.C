#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TObject.h"
#include "TVirtualFFT.h"
#include "TMath.h"
#include "TGraph.h"
#include "TGraphErrors.h"

void wfmData(){

  /////////////////////////////////////
  ///// Calling ROOT
  /////////////////////////////////////

  TFile *f = TFile::Open("/scratch1/Digitizer/data/00000001.root");
  TTree *t = (TTree*)f->Get("trdata");
  Int_t pts;
  Double_t wfmData[200000];
  Int_t recordNum;
  Double_t initialTime;
  Double_t samplingRate;

  t->SetBranchAddress("pts", &pts);
  t->SetBranchAddress("wfmData", wfmData);
  t->SetBranchAddress("recordNum", &recordNum);
  t->SetBranchAddress("initialTime", &initialTime);
  t->SetBranchAddress("samplingRate", &samplingRate);
  
  t->GetEntry(0);
  Double_t tData[pts];

  for (Int_t i_pt=0; i_pt<pts; i_pt++){
    tData[i_pt]=initialTime+i_pt/samplingRate;
  }

  TGraph* gr = new TGraph(pts,tData,wfmData);
  gr->Draw();

}
