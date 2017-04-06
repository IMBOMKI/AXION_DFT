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

void RatioOfDeadTime(){

  Double_t SamplingRate[5]={20,50,100,500,1000};
  Double_t DeadRatio_root[5];
  Double_t DeadRatio_bin[5];
  Double_t MeasTime[5]={2.5,1.0,0.5,0.1,0.05};
  Double_t DeadTime_root=6.4;
  Double_t DeadTime_bin=3.7;
  
  for (Int_t i=0; i<5; i++){
    DeadRatio_root[i]=DeadTime_root/(MeasTime[i]+DeadTime_root)*100;
    DeadRatio_bin[i]=DeadTime_bin/(MeasTime[i]+DeadTime_bin)*100;
    std::cout << DeadRatio_root[i] << "   " << DeadRatio_bin[i] << std::endl;
    
  }


  TGraph *gr_bin = new TGraph(5,SamplingRate,DeadRatio_bin);
  gr_bin->SetMarkerStyle(20);
  gr_bin->SetMarkerColor(4);
  gr_bin->Draw("APL");

  TGraph *gr_root = new TGraph(5,SamplingRate,DeadRatio_root);
  gr_root->SetMarkerStyle(20);
  gr_root->SetMarkerColor(2);
  //  gr_root->SetRangeUser(50,100);
  gr_root->Draw("same");



}
