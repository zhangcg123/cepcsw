#include <algorithm>
#include <iomanip>
#include <iostream>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TF1.h>
#include <TRandom3.h>
#include <RooRealVar.h>
#include <RooGaussian.h>
#include <RooDataHist.h>
#include <RooFitResult.h>
#include <RooPlot.h>
#include "/afs/ihep.ac.cn/users/g/guofy/RootUtils/AtlasStyle.C"
#include "/afs/ihep.ac.cn/users/g/guofy/RootUtils/AtlasUtils.C"
#include "/afs/ihep.ac.cn/users/g/guofy/RootUtils/AtlasLabels.C"
#include "/afs/ihep.ac.cn/users/g/guofy/HggTwoSidedCBPdf.cxx"
#include "/afs/ihep.ac.cn/users/g/guofy/HggTwoSidedCBPdf.h"

using namespace std;
using namespace RooFit;

vector<double> get_rms90(vector<double> data);

vector<double> roofit_jets(){
  SetAtlasStyle();
  //gStyle->SetLegendTextSize(0.04);
  gStyle->SetLegendFont(42);
  gStyle->SetErrorX(0.5); 
  gStyle->SetOptStat(0);
  gStyle->SetLabelSize(0.05);


  // Read data from txt file
  double m_jj, jet1_costheta, jet2_costheta, invPt, barrelRatio;
  std::vector<double> event;
  std::vector<double> inv_mass; 
  std::vector<double> mc_chargeE; 
  std::vector<double> mc_neuhadE; 
  std::vector<double> rec_trackE;

  // std::ifstream file("out/invmass_fullrec_ecal1.1_hcal70.txt"); 
  // std::ifstream file("out/invmass_MCtrk_recHCAL_ecal1.1_hcal70.txt"); 
  // std::ifstream file("scan_calibration/out/invmass_ECALCali1.100_HCALCali70.0.txt"); 

/*  std::ifstream file("output.txt"); 
  if(file.is_open()) 
  {
    // double ll_event, ll_invmass, ll_chargeE, ll_neuhadE, ll_rectrack, ll_EECAL, ll_EHCAL;
    // while (file >> ll_event >> ll_invmass ) 
    // {  // 逐行读取文件中的数据
    //   // double dE = ll_chargeE - ll_rectrack;
    //   // if( (dE<-5) || (dE>5) ) continue;
    //   event.push_back(ll_event);
    //   inv_mass.push_back(ll_invmass);  // 将每个数添加到vector中
    //   // mc_chargeE.push_back(ll_chargeE);
    //   // mc_neuhadE.push_back(ll_neuhadE);
    //   // rec_trackE.push_back(ll_rectrack);
    // }
    // file.close();  // 关闭文件
    double ll_invmass;
    while (file >> ll_invmass ) 
    {  // 逐行读取文件中的数据
      inv_mass.push_back(ll_invmass);  // 将每个数添加到vector中
    }
    file.close();  // 关闭文件
  }
*/
  TFile* rfile = new TFile("jet_all.root","read");
  TTree* rtree = (TTree*)rfile->Get("jets");
  rtree->SetBranchAddress("mass_allParticles", &m_jj);
  rtree->SetBranchAddress("GEN_jet1_costheta", &jet1_costheta);
  rtree->SetBranchAddress("GEN_jet2_costheta", &jet2_costheta);
  rtree->SetBranchAddress("GEN_inv_pt", &invPt);
  rtree->SetBranchAddress("barrelRatio", &barrelRatio);

  for(int ievt=0; ievt<rtree->GetEntries(); ievt++){
    //if(ievt>10000) continue;
    rtree->GetEntry(ievt);
    //if(fabs(jet1_costheta)>0.6 || fabs(jet2_costheta)>0.6 || invPt>1) continue;
    if(barrelRatio<0.95) continue;
    inv_mass.push_back(m_jj);
  }

  // 创建RooFit变量
  // RooRealVar x("x", "Invariant mass / GeV", 90, 150);
  RooRealVar x("x", "Invariant mass / GeV", 80, 180);
  // 创建空的RooDataSet
  RooDataSet data("data", "data", RooArgSet(x));
  for(int i=0; i<inv_mass.size();i++){
    if(inv_mass[i]<80) continue;
    x.setVal(inv_mass[i]);
    data.add(RooArgSet(x));
  }
  data.Print();

  // RooRealVar rf_x("x", "x", 90, 150);
  RooRealVar rf_mean("mean", "mean of gaussian", 123, 115, 135);
  RooRealVar rf_sigma("sigma", "width of gaussian", 5, 3, 12);

  TCanvas *c2 = new TCanvas("c2", "c2", 800, 600);
  c2->cd();
  //RooGaussian rf_gauss("gauss", "gaussian PDF", x, rf_mean, rf_sigma);
  //vector<double> rms90_range = get_rms90(inv_mass);
  //cout << "yyy:   " << rms90_range[0] << " " << rms90_range[1] << endl;
  //x.setRange("rms90", rms90_range[0], rms90_range[1]);
  ////x.setRange(110, 140);
  //rf_gauss.fitTo(data, Range("rms90"));



  RooRealVar alphaLo("alphaLo","alphaLo",0,5);
  RooRealVar nLo("nLo","nLo",0,200);
  RooRealVar alphaHi("alphaHi","alphaHi",0,5);
  RooRealVar nHi("nHi","nHi",0,150);
 
  RooAbsPdf* rf_dscb = new HggTwoSidedCBPdf("rf_dscb", "rf_dscb", x, rf_mean, rf_sigma, alphaLo, nLo, alphaHi, nHi);
  rf_dscb->fitTo(data);
  RooPlot* xframe = x.frame();
  data.plotOn(xframe, Name("gr_data"), DataError(RooAbsData::SumW2), Binning(50), Invisible());
  //rf_gauss.plotOn(xframe, Name("rf_gauss"), LineColor(2), LineWidth(2));
  rf_dscb->plotOn(xframe, Name("rf_dscb"), LineColor(2), LineWidth(2));
  //rf_dscb->paramOn(xframe);

  
  //xframe->GetYaxis()->SetRangeUser(0, 390);
  xframe->GetYaxis()->SetTitle("Events / GeV");
  xframe->GetXaxis()->SetTitleSize(0.06);
  xframe->GetYaxis()->SetTitleSize(0.06);
  xframe->GetXaxis()->CenterTitle();
  xframe->GetYaxis()->CenterTitle();
  xframe->GetXaxis()->SetLabelSize(0.05);
  xframe->GetYaxis()->SetLabelSize(0.05);
  xframe->Draw();


  TGraphAsymmErrors* gr1 = (TGraphAsymmErrors*)(xframe->getObject(0));
  TGraphAsymmErrors* gr2 = new TGraphAsymmErrors();
  gr2->SetName("gr_data");
  for(int i=0; i<gr1->GetN(); i++){
    if(gr1->GetPointY(i)==0) continue;
    else{
      gr2->SetPoint(i, gr1->GetPointX(i), gr1->GetPointY(i));
      gr2->SetPointError(i, 0, 0, sqrt(gr1->GetPointY(i)), sqrt(gr1->GetPointY(i)));
    }
  }

  gr2->GetXaxis()->SetRangeUser(100, 160);
  gr2->GetYaxis()->SetRangeUser(0, 260);
  gr2->GetXaxis()->SetTitle("m_{jj} [GeV]");
  gr2->GetYaxis()->SetTitle("Events / GeV");
  gr2->Draw("P same");

  //xframe->getObject(1)->Print();

  //TF1* func = (TF1*)xframe->getObject(1);
  //func->Print();
  //func->SetName("rf_dscb");
  //func->SetLineStyle(1);
  //func->SetLineWidth(2);
  //func->GetXaxis()->SetRangeUser(105,160);
  //func->GetYaxis()->SetRangeUser(0, 4400);
  //func->Draw("C same");

  TLegend* l1 = new TLegend(0.23, 0.65, 0.50, 0.9);
  l1->SetBorderSize(0);
  l1->AddEntry("gr_data", "Rec. events", "PE" );
  l1->AddEntry("rf_dscb", "H #rightarrow jj, DSCB fit", "L");
  //l1->AddEntry("rf_gauss", "H #rightarrow jj, DSCB fit", "L");
  l1->Draw();

  //TLatex text_CEPC;
  //text_CEPC.SetNDC();
  //text_CEPC.SetTextSize(0.050);
  //text_CEPC.SetTextFont(22);
  //text_CEPC.DrawLatex(0.2, 0.8, "CEPC Simulation");

  //text_CEPC.SetTextFont(62);
  //text_CEPC.SetTextSize(0.04);
  //text_CEPC.DrawLatex(0.2, 0.75, "e^{+}e^{-} #rightarrow ZH, H #rightarrow #gamma#gamma");
  //text_CEPC.DrawLatex(0.2, 0.7, "#sqrt{s} = 240 GeV");

  // 获取拟合参数的值和误差
  double fittedMean = rf_mean.getVal();
  double fittedMeanError = rf_mean.getError();
  double fittedSigma = rf_sigma.getVal();
  double fittedSigmaError = rf_sigma.getError();
  double BMR_fit = fittedSigma / fittedMean;
  double BMR_error = BMR_error = BMR_fit * 
                                  sqrt( (fittedMeanError / fittedMean)*(fittedMeanError / fittedMean) + 
                                        (fittedSigmaError / fittedSigma)*(fittedSigmaError / fittedSigma) );

  // 打印拟合结果
  // data.Print();
  std::cout << "Fitted mean: " << fittedMean << " +/- " << fittedMeanError << std::endl;
  std::cout << "Fitted sigma: " << fittedSigma << " +/- " << fittedSigmaError << std::endl;
  cout << "BMR = " << BMR_fit*100. << " +/- " << BMR_error*100. << " %" << endl;

  vector<double> output; 
  output.push_back(BMR_fit*100.);
  output.push_back(BMR_error*100.);

  return output; 
}


vector<double> get_rms90(vector<double> data){
  std::sort(data.begin(), data.end());
  int num_data = data.size();
  int num_data90 = num_data*0.90;
  int num_data10 = num_data*0.10;

  double lower_range = 0;
  double upper_range = 0;
  double range = 10000.0;
  for(int i=0; i<num_data10-3; i++){
    double temp_lower_range = data[i];
    double temp_upper_range = data[i+num_data90];
    double temp_range = temp_upper_range - temp_lower_range;
    if (temp_range < range){
      range = temp_range;
      lower_range = temp_lower_range;
      upper_range = temp_upper_range;
    }
  }

  vector<double> aa{lower_range, upper_range};
  return aa;
}
