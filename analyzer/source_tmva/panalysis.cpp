#include <cstdlib> // for exit() function
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <TMVA/Reader.h>
#include <TMVA/Tools.h>
#include <TMVA/Config.h>
#include "panalysis.h"

#define SSTR( x ) dynamic_cast< std::ostringstream & > \
        ( std::ostringstream() << std::dec << x ).str()

double min(double a, double b){
  return (a < b) ? a : b;
}

double max(double a, double b){
  return (a > b) ? a : b;
}

using namespace std;

PAnalysis::PAnalysis(PConfig *config){
  #ifdef P_LOG
    cout << "Initializing analysis " << config->GetAnaName() << "." << endl;
  #endif

  mySig = -1;
  myCut = 0;
  myEvalOnTrained = false;
  myDiscriminantIsDef = false;
  myBkgEff = 0;
  mySigEff = 0;
  myGini = 0.;
  
  myConfig = config;
  myName = myConfig->GetAnaName();
  myOutput = myConfig->GetOutputDir() + "/" + myConfig->GetOutputName();
  myMvaMethod = myConfig->GetMvaMethod();
    
  for(unsigned int k=0; k<myConfig->GetNInputVars(); k++)
    myInputVars.push_back(0.);

  for(unsigned int i=0; i<myConfig->GetNProc(); i++)
    AddProc( (PProc*) new PProc(myConfig, i) );

  myOutputFile = (TFile*) new TFile(myOutput+".root", "RECREATE");
  if(!myOutputFile->IsOpen()){
    cerr << "Failure opening file " << myOutput+".root" << ".\n";
    exit(1);
  }

  myStack = NULL;
  myFactory = NULL;
  myReader = NULL;
  myCnvTraining = NULL;
  myCnvPlot = NULL;
  myLegend = NULL;
  myCnvEff = NULL;
  myROC = NULL;
  myLine = NULL;
  myCutLine = NULL;
}

void PAnalysis::AddProc(PProc* proc){
  #ifdef P_LOG
    cout << "Adding process " << proc->GetName() << " to analysis " << myName << "." << endl;
  #endif

  myProc.push_back(proc);
  switch(proc->GetType()){
    case 0:
      myBkgs.push_back(myProc.size()-1);
      break;
    case 1:
      if(mySig >= 0){
        cerr << "ERROR: Only one signal can be assigned to the analysis!" << endl;
        exit(1);
      }
      mySig = myProc.size()-1;
      break;
    default:
      break;
  }
}

void PAnalysis::OpenAllProc(void){
  for(unsigned int i=0; i<myProc.size(); i++)
    myProc.at(i)->Open();
}

void PAnalysis::CloseAllProc(void){
  for(unsigned int i=0; i<myProc.size(); i++)
    myProc.at(i)->Close();
}

void PAnalysis::BuildDiscriminant(void){
  if(mySig < 0 || !myBkgs.size()){
    cerr << "ERROR: Cannot build discriminant: at least two processes (one signal and one background) have to be assigned to the analysis!" << endl;
    exit(1);
  }
  
  if(myMvaMethod == "Singleton"){
    DefineSingleton();
  }else{
    DefineAndTrainFactory();
  }

  myDiscriminantIsDef = true;
}

void PAnalysis::DefineSingleton(void){
  // Define single variable on which to cut
  // For now, do nothing, but one might want to choose
  // to normalize it, for instance.
  
  #ifdef P_LOG
    cout << "Initialising discriminant variable " << myConfig->GetInputVar(0) << ".\n";
  #endif

}

void PAnalysis::DefineAndTrainFactory(void){
  // Defining Factory and MVA

  #ifdef P_LOG
    cout << "Initialising factory of type " << myMvaMethod;
    if(myMvaMethod == "MLP")
      cout << " and of topology " << myConfig->GetTopology() << ".\n";
    else
      cout << ".\n";
  #endif

  // Change MVA output file... Because myFactory::SetOutputDir() has been abducted by Aliens, do it a simpler way:
  TMVA::Tools::Instance();
  (TMVA::gConfig().GetIONames()).fWeightFileDir = myConfig->GetOutputDir();
  
  #ifdef P_LOG
    myFactory = (TMVA::Factory*) new TMVA::Factory(myName, myOutputFile, "!DrawProgressBar");
  #else
    myFactory = (TMVA::Factory*) new TMVA::Factory(myName, myOutputFile, "Silent:!DrawProgressBar");
  #endif

  // Open PProcs and define input trees
  
  OpenAllProc();
  
  myFactory->AddSignalTree(myProc.at(mySig)->GetTree(), myProc.at(mySig)->GetGlobWeight());
  for(unsigned int i=0; i<myBkgs.size(); i++)
    myFactory->AddBackgroundTree(myProc.at(myBkgs.at(i))->GetTree(), myProc.at(myBkgs.at(i))->GetGlobWeight());

  // Will also use weights defined in the input process dataset
  // Careful if these weights are negative!
  myFactory->SetWeightExpression(myConfig->GetCommonEvtWeight().c_str());

  for(unsigned int i=0; i<myConfig->GetNInputVars(); i++)
    myFactory->AddVariable(myConfig->GetInputVar(i));

  // Events: train/test/train/test/...
  // ? for each background tree or for all the brackgrounds together ?
  myFactory->PrepareTrainingAndTestTree(
    "", 
    "nTrain_Signal="+SSTR(myConfig->GetTrainEntries())
    +":nTrain_Background="+SSTR(myConfig->GetTrainEntries())
    +":nTest_Signal="+SSTR(myConfig->GetTrainEntries())
    +":nTest_Background="+SSTR(myConfig->GetTrainEntries())
    +":SplitMode=Alternate"
    +":NormMode=EqualNumEvents"
    );

  if(myMvaMethod == "MLP"){
    myFactory->BookMethod(
      TMVA::Types::kMLP, 
      myMvaMethod+"_"+myName, 
      TString("!H:V")
      +":NeuronType=tanh"
      +":VarTransform=Norm"
      +":IgnoreNegWeightsInTraining=True"
      +":NCycles="+SSTR(myConfig->GetIterations())
      +":HiddenLayers="+myConfig->GetTopology()
      +":TestRate=5"
      +":TrainingMethod=BFGS"
      +":SamplingTraining=False"
      +":ConvergenceTests=50"
      );
  }else if(myMvaMethod == "BDT"){
    myFactory->BookMethod(
      TMVA::Types::kBDT, 
      myMvaMethod+"_"+myName, 
      TString("!H:V")
      +":NTrees="+SSTR(myConfig->GetIterations())
      );
  }else{
    cerr << "ERROR: Couldn't recognize MVA method.\n";
    exit(1);
  }

  // Training and stuff

  myFactory->TrainAllMethods();
  myFactory->TestAllMethods();
  myFactory->EvaluateAllMethods();

  CloseAllProc();

  delete myFactory; myFactory = NULL;
}

// Creates a TMVA::Reader, if it hasn't been created yet
// The Reader will be deleted when PAnalysis is deleted
void PAnalysis::InitDiscriminant(){
  if(myMvaMethod != "Singleton"){
   
    if(!myReader)
      myReader = (TMVA::Reader*) new TMVA::Reader("!V:Color");
   
    for(unsigned int k=0; k<myConfig->GetNInputVars(); k++)
      myReader->AddVariable(myConfig->GetInputVar(k), &myInputVars.at(k));
    
    myReader->BookMVA(myName, myConfig->GetOutputDir()+"/"+myName+"_"+myMvaMethod+"_"+myName+".weights.xml");
    
  }
}

double PAnalysis::EvalDiscriminant(PProc *proc){
  if(!proc){
    cerr << "ERROR in PAnalysis::EvalDiscriminant: process is not defined!" << endl;
    exit(1);
  }
  if(!myDiscriminantIsDef){
    cerr << "WARNING in PAnalysis::EvalDiscriminant(): discriminant has not been defined/trained yet. Attempting to call BuildDiscriminant()." << endl;
    BuildDiscriminant();
  }

  if(myMvaMethod == "Singleton"){
  
    return proc->GetInputVar(myConfig->GetInputVar(0));
  
  }else{

    if(!myReader)
      InitDiscriminant();
    
    // While double-type inputs are OK for TMVA training, evaluation requires float
    // See http://sourceforge.net/p/tmva/mailman/message/33528693/ (no further answer received)
    for(unsigned int k=0; k<myConfig->GetNInputVars(); k++)
      myInputVars.at(k) = (float) proc->GetInputVar(myConfig->GetInputVar(k));

    return Transform((double)myReader->EvaluateMVA(myName));

  }
}

double PAnalysis::Transform(double input){
  // BDT output is between -1 and 1 => get it between 0 and 1, just as NN
  // And be sure all the output is between 0 and 1.

  double output;
  
  if(myMvaMethod == "BDT")
    output = (input + 1.)/2.;
  else
    output = input;
  
  if(output > 1)
    return 1;
  else if(output < 0)
    return 0;
  else
    return output;
}

void PAnalysis::DoHist(void){
  #ifdef P_LOG
    cout << "Filling output histograms.\n";
  #endif
  
  if(!myDiscriminantIsDef){
    cerr << "WARNING in PAnalysis::DoHist(): discriminant has not been defined/trained yet. Attempting to call BuildDiscriminant().\n";
    BuildDiscriminant();
  }
  
  // Filling histograms

  for(unsigned int j=0; j<myProc.size(); j++){
    PProc* proc = (PProc*) myProc.at(j);
    proc->Open();

    for(long i=0; i<proc->GetTree()->GetEntries(); i++){
      // this would have to be discussed
      //if(proc->GetType() >= 0 && i%2==0 && i < 2*myConfig->GetTrainEntries() && !myEvalOnTrained)
      //  continue;
      
      proc->GetTree()->GetEntry(i);
     
      double mvaOut = EvalDiscriminant(proc);
      double weight = proc->GetEvtWeight()*proc->GetGlobWeight();

      proc->GetHist()->Fill(mvaOut, weight);
      proc->GetAbsHist()->Fill(mvaOut, abs(weight));
    }

    proc->GetHist()->SetLineWidth(3);
    proc->GetHist()->SetLineColor(proc->GetColor());
    proc->GetHist()->SetXTitle("MVA output");
    proc->GetHist()->SetStats(0);
    if(proc->GetType() < 0)
      proc->GetHist()->SetLineStyle(2);
    else
      proc->GetHist()->SetLineStyle(0);

    proc->GetAbsHist()->SetLineWidth(3);
    proc->GetAbsHist()->SetLineColor(proc->GetColor());
    proc->GetAbsHist()->SetXTitle("MVA output");
    proc->GetAbsHist()->SetStats(0);
    if(proc->GetType() < 0)
      proc->GetAbsHist()->SetLineStyle(2);
    else
      proc->GetAbsHist()->SetLineStyle(0);

    proc->Close();
  }
}

void PAnalysis::DoPlot(void){
  #ifdef P_LOG
    cout << "Plotting output histograms.\n";
  #endif
  if(!myProc.at(0)->GetHist()->GetEntries()){
    cerr << "WARNING: Cannot plot MVA output histograms without histograms! Attempting to call DoHist().\n";
    DoHist();
  }
  
  myCnvPlot = (TCanvas*) new TCanvas(myName+"_Discr output","Discriminant output");
  myLegend = new TLegend(0.73,0.7,0.89,0.89);
  myLegend->SetFillColor(0);
  myStack = (THStack*) new THStack("output_stack","Discriminant output");
  
  for(unsigned int j=0; j<myProc.size(); j++)
    myLegend->AddEntry(myProc.at(j)->GetHist(), myProc.at(j)->GetName().c_str(), "f");

  TH1D* tempSigHist = (TH1D*) myProc.at(mySig)->GetHist()->Rebin(myConfig->GetHistBins()/myConfig->GetPlotBins(),"rebinned");

  FillStack(tempSigHist->Integral());
  
  if(myStack->GetMaximum() > tempSigHist->GetMaximum()){
    myStack->Draw("hist");
    tempSigHist->Draw("same hist");
  }else{
    tempSigHist->Draw("hist");
    myStack->Draw("same hist");
  }

  myLegend->Draw();
}

void PAnalysis::FillStack(double integralSig){
  vector<PProc*> pointers;
  for(unsigned int i=0; i<myProc.size(); i++){
    if(myProc.at(i)->GetType() != 1)
      pointers.push_back(myProc.at(i));
  }
  
  sort(pointers.begin(), pointers.end(), &compareProc);

  double integralBkg = 0;
  for(unsigned int i=0; i<pointers.size(); i++)
    integralBkg += pointers.at(i)->GetHist()->Integral();
  
  for(unsigned int i=0; i<pointers.size(); i++){
    TH1D* tempHist = (TH1D*)pointers.at(i)->GetHist()->Rebin(myConfig->GetHistBins()/myConfig->GetPlotBins(), "rebinned_" + i);
    tempHist->Scale(integralSig/integralBkg);
    myStack->Add(tempHist);
  }
}

void PAnalysis::DoROC(void){
  #ifdef P_LOG
    cout << "Drawing ROC curve.\n";
  #endif
  
  if(mySig<0 || !myBkgs.size()){
    cerr << "ERROR: Cannot draw ROC curve without signal and background processes!\n";
    exit(1);
  }else if(!myProc.at(0)->GetHist()->GetEntries()){
    cerr << "WARNING: Cannot draw ROC curve without process histograms! Attempting to call DoHist().\n";
    DoHist();
  }
 
  double countS = myProc.at(mySig)->GetHist()->Integral();
  double totS = myProc.at(mySig)->GetAbsHist()->Integral();
  
  double countBkg = 0, totBkg = 0;
  for(unsigned int i=0; i<myBkgs.size(); i++){
    countBkg += myProc.at(myBkgs.at(i))->GetHist()->Integral();
    totBkg += myProc.at(myBkgs.at(i))->GetAbsHist()->Integral();
  }
  
  unsigned int nBins = myConfig->GetHistBins();
  double sigEff[nBins+2];
  double bkgEff[nBins+2];

  for(unsigned int i=0; i<=nBins+1; i++){
    countS -= myProc.at(mySig)->GetHist()->GetBinContent(i);
    for(unsigned int j=0; j<myBkgs.size(); j++)
      countBkg -= myProc.at(myBkgs.at(j))->GetHist()->GetBinContent(i);
    sigEff[i] = abs(countS)/totS;
    bkgEff[i] = abs(countBkg)/totBkg;
  }

  //sigEff[0] = 1.;
  sigEff[nBins+1] = 0.;
  //bkgEff[0] = 1.;
  bkgEff[nBins+1] = 0.;
  
  myROC = (TGraph*) new TGraph(nBins+2, bkgEff, sigEff);
  myROC->GetXaxis()->SetTitle("Background eff.");
  myROC->GetYaxis()->SetTitle("Signal eff.");
  myROC->SetTitle("Signal eff. vs. background eff. (cut on discriminant)");
  myROC->SetMarkerColor(kBlue);
  myROC->SetMarkerStyle(20);
  
  myCnvEff = (TCanvas*) new TCanvas("ROC","Signal efficiency vs bkg. efficiency");
  myCnvEff->SetGrid(20,20);
  myROC->Draw("ALP");
  myLine = (TLine*) new TLine(0,0,1,1);
  myLine->Draw();
}

// We will want to sort the (MVA,weight) vectors according to increasing MVA values
bool mvaOutputSorter(std::vector<double> i, std::vector<double> j){ return i[0] < j[0]; }

void PAnalysis::BkgEffWPPrecise(void){
  double workingPoint = myConfig->GetWorkingPoint();
  
  #ifdef P_LOG
    cout << "Computing precise cut for signal efficiency = " << workingPoint << "... " << endl;
  #endif
  
  if(!myDiscriminantIsDef){
    cerr << "WARNING in PAnalysis::BkgEffWPPrecise(): discriminant has not been defined/trained yet. Attempting to call BuildDiscriminant().\n";
    BuildDiscriminant();
  }
  
  if(workingPoint >= 1 || workingPoint <= 0){
    cerr << "ERROR: Working point must be a double strictly between 0 and 1!\n";
    exit(1);
  }

  std::string condition = "";
  if(!myEvalOnTrained && myMvaMethod != "Singleton")
    condition = "!(Entry$%2==0 && Entry$ < " + std::to_string(2*myConfig->GetTrainEntries()) + ")";

  // We will evaluate the MVA on the signal, 
  // build a vector of (MVA output, weight), 
  // sort it according to increasing values of MVA output, 
  // and find the MVA cut value X such that abs(sum(weights|mva>=X))/sum(abs(weights)) = working point

  // Fetching the signal
  PProc* sig = (PProc*) myProc.at(mySig);
  sig->Open();
  TTree* tree = sig->GetTree();
  double sigEffEntriesAbs = sig->GetEffEntriesAbs(condition);
  double sigEffEntries = sig->GetEffEntries(condition);
  
  /*if(workingPoint > abs(sigEffEntries)/sigEffEntriesAbs){
    cout << "ERROR in BkgEffWPPrecise(): not possible to achieve requested working point, since the maximum attainable efficiency \
given the negative event weights is " << abs(sigEffEntries)/sigEffEntriesAbs << "." << endl;
    exit(1);
  }*/

  std::vector< std::vector<double> > mvaOutput;

  for(uint64_t i=0; i < static_cast<uint64_t>(tree->GetEntries()); i++){
    if(!myEvalOnTrained && i%2==0 && i<myConfig->GetTrainEntries()*2 && myMvaMethod != "Singleton")
      continue;

    tree->GetEntry(i);
    
    std::vector<double> outputAndWeight;

    outputAndWeight.push_back( EvalDiscriminant(sig) );
    outputAndWeight.push_back( sig->GetEvtWeight() );

    //cout << "event " << i << ", output = " << EvalDiscriminant(sig) << ", weight = " << sig->GetEvtWeight() << endl;

    mvaOutput.push_back(outputAndWeight);
  }

  sort(mvaOutput.begin(), mvaOutput.end(), mvaOutputSorter);
  //for(uint64_t i=0; i < static_cast<uint64_t>(tree->GetEntries()); i++)
  //  cout << "event " << i << ", output = " << mvaOutput.at(i).at(0) << ", weight = " << mvaOutput.at(i).at(1) << endl;
  
  // the sort is in ascending MVA values, so we have to cut st. abs(sum(weights|mva>X))/sum(abs(weights)) = abs(sum(weights))/sum(abs(weights)) - working point
  int cutIndex = 0;
  double integral = 0.;
  for(unsigned int i = 0; i < mvaOutput.size(); ++i){
    if( (abs(sigEffEntries) - abs(integral) )/sigEffEntriesAbs <= workingPoint)
      break;
    integral += mvaOutput[i][1];
    cutIndex++;
  }

  mySigEff = (abs(sigEffEntries) - abs(integral))/sigEffEntriesAbs;

  // just a security in the limiting case of workingPoint = 0.
  if(cutIndex == tree->GetEntries())
    myCut = mvaOutput.end()->at(0) + 1.;
  else
    myCut = mvaOutput.at(cutIndex).at(0);

  sig->Close();
  
  if(abs(mySigEff - workingPoint)/workingPoint > 0.1){
    cout << "ERROR in BkgEffWPPrecise(): computed signal efficiency is more than 10% off from the requested working point." << endl;
    exit(1);
  }

  // Draw cut line on plot canvas, if it exists
  if(myCnvPlot){
    myCnvPlot->cd();
    myCutLine = (TLine*) new TLine(myCut, min(myStack->GetMinimum(), sig->GetHist()->GetMinimum()), myCut, max(myStack->GetMaximum(), sig->GetHist()->GetMaximum()));
    myCutLine->SetLineWidth(3);
    myCutLine->Draw();
  }

  // Compute background efficiency using this cut:
  
  double bkgSelectedYield = 0.;
  double bkgTotYieldAbs = 0.;
  
  for(unsigned int j=0; j<myBkgs.size(); j++){
    PProc* proc = (PProc*) myProc.at(myBkgs.at(j));

    proc->Open();

    double integral = 0;

    for(uint64_t i=0; i < static_cast<uint64_t>(proc->GetTree()->GetEntries()); i++){
      if(!myEvalOnTrained && i%2==0 && i<myConfig->GetTrainEntries()*2 && myMvaMethod != "Singleton")
        continue;
      
      proc->GetTree()->GetEntry(i);
      
      if(EvalDiscriminant(proc) >= myCut)
        integral += proc->GetEvtWeight();
    }

    bkgSelectedYield += proc->GetGlobWeight() * integral;
    bkgTotYieldAbs += proc->GetYieldAbs(condition);

    proc->Close();
  }

  myBkgEff = abs(bkgSelectedYield)/bkgTotYieldAbs;
  
  if(mySigEff*myBkgEff == 0.){
    cout << "ERROR in BkgEffWPPrecise(): found cut value which rejects all the data!" << endl;
    exit(1);
  }
  
  #ifdef P_LOG
    cout << "For signal efficiency " << mySigEff << ", cut on discriminant is " << myCut << ", and background efficiency is " << myBkgEff << ".\n";
  #endif
}

std::map<std::string, std::vector<double>> PAnalysis::FiguresOfMerit(void){
  // Return a map with {"Type of figure e.g. SoverB", pointer to array [bestCut, signal efficiency, bkg efficiency]} 
  // bestCut is chosen to maximize the down fluctuated "Type of figure"
  #ifdef P_LOG
    cout << "Computing figures of merit." << endl;
  #endif
  if(mySig<0 || !myBkgs.size()){
    cerr << "Cannot compute figures of merit without signal and background proc!\n";
    exit(1);
  }else if(!myProc.at(0)->GetHist()->GetEntries()){
    cerr << "Cannot compute figures of merit without proc histograms!\n";
    exit(1);
  }
  double yieldErrorCriteria = 0.5;
  double sB = 0, SRootB = 0, SRootSB = 0;
  double sB_down = 0, SRootB_down = 0, SRootSB_down = 0, Gini_father = 0, Gini_sigBox = 0, Gini_bkgBox = 0, Gini_tot = 0;

  double totExpBkg = 0, totExpSig = (double) myProc.at(mySig)->GetHist()->Integral(0, myConfig->GetHistBins()+1);
  TH1D* bkg_th1 = new TH1D("Bkgth1", "Bkgth1", myConfig->GetHistBins(), myConfig->GetHistLoX(), myConfig->GetHistHiX()); 
  bkg_th1->Sumw2();
  for(unsigned int j=0; j<myBkgs.size(); j++){
    bkg_th1->Add(myProc.at(myBkgs.at(j))->GetHist());
  }  
  totExpBkg = bkg_th1->Integral(0, myConfig->GetHistBins()+1);
  
  double purity_father = totExpSig / (totExpSig + totExpBkg); 
  Gini_father = (totExpSig + totExpBkg) * purity_father * (1 - purity_father);

  double purity_sigBox = 0, purity_bkgBox = 0;
  double expBkg, expBkg_err, expSig, expSig_err;
  double expBkg_leftSide, expBkg_err_leftSide, expSig_leftSide, expSig_err_leftSide;
  double tempSB_down, tempSRootB_down, tempSRootSB_down, tempGini_tot;
  double bestCutSB = std::numeric_limits<double>::lowest(), bestCutSRootB = std::numeric_limits<double>::lowest(), bestCutSRootSB = std::numeric_limits<double>::lowest(), bestCutGini = std::numeric_limits<double>::lowest();
  double bestSBsig = 0, bestSBbkg = 0, bestGinisig = 0, bestGinibkg = 0;
  double sigEff_SB = 0, bkgEff_SB = 0, sigEff_SRootB = 0, bkgEff_SRootB = 0, sigEff_SRootSB = 0, bkgEff_SRootSB = 0, sigEff_Gini = 0, bkgEff_Gini = 0;

  double max_efficiency = 0.99999;

  double relYieldErr, relYieldErr_leftSide;
  for(int i=1; i<=myConfig->GetHistBins()+1; i++){
    // Signal daughter Box
    expSig = (double) myProc.at(mySig)->GetHist()->IntegralAndError(i, myConfig->GetHistBins()+1, expSig_err);
    expSig_err = expSig_err;
    expBkg = bkg_th1->IntegralAndError(i, myConfig->GetHistBins()+1, expBkg_err);
    expBkg_err = expBkg_err;
    if (expBkg <= 0 || expSig <= 0)
      continue;
    relYieldErr = sqrt(expBkg_err*expBkg_err + expSig_err*expSig_err) / (expSig + expBkg); 
    if (relYieldErr > yieldErrorCriteria)
        continue;

    // Baackground daughter Box
    expSig_leftSide = (double) myProc.at(mySig)->GetHist()->IntegralAndError(0, i-1, expSig_err_leftSide);
    expSig_err_leftSide = expSig_err_leftSide;
    expBkg_leftSide = bkg_th1->IntegralAndError(0, i-1, expBkg_err_leftSide);
    expBkg_err_leftSide = expBkg_err_leftSide;
    if (expBkg_leftSide <= 0 || expSig_leftSide <= 0)
        continue;
    relYieldErr_leftSide = sqrt(expBkg_err_leftSide*expBkg_err_leftSide + expSig_err_leftSide*expSig_err_leftSide) / (expSig_leftSide + expBkg_leftSide); 
    if (relYieldErr_leftSide > yieldErrorCriteria)
        continue;
    

    tempSB_down = expSig/expBkg - std::abs(expSig/expBkg)*sqrt(pow(expSig_err/expSig, 2) + pow(expBkg_err/expBkg, 2));
    tempSRootB_down = expSig/sqrt(expBkg) - sqrt( pow(expSig_err, 2)/expBkg + pow(expSig*expBkg_err, 2)/(4*pow(expBkg, 3)) );
    tempSRootSB_down = expSig/sqrt(expBkg+expSig) - sqrt(pow(((sqrt(expSig+expBkg) - expSig/(2*sqrt(expBkg+expSig)))*expSig_err)/(expBkg+expSig), 2) + pow(expSig*expBkg_err/2, 2)*pow(expBkg+expSig,-3));
    
    purity_sigBox = expSig / (expBkg + expSig);
    Gini_sigBox = (expBkg + expSig) * purity_sigBox * (1 - purity_sigBox);
    purity_bkgBox = expSig_leftSide / (expBkg_leftSide + expSig_leftSide);
    Gini_bkgBox = (expBkg_leftSide + expSig_leftSide) * purity_bkgBox * (1 - purity_bkgBox);

    tempGini_tot = Gini_father - Gini_sigBox - Gini_bkgBox;

    if (tempGini_tot > Gini_tot){
        Gini_tot = tempGini_tot;
        bestGinisig = expSig;
        bestGinibkg = expBkg;
        sigEff_Gini = std::min(expSig/totExpSig, max_efficiency);
        bkgEff_Gini = std::min(expBkg/totExpBkg, max_efficiency);
        bestCutGini = myProc.at(mySig)->GetHist()->GetBinLowEdge(i);
    }


    //if(i > 1){
      if(tempSB_down > sB_down){
        bestSBsig = expSig;
        bestSBbkg = expBkg;
        sigEff_SB = std::min(expSig/totExpSig, max_efficiency);
        bkgEff_SB = std::min(expBkg/totExpBkg, max_efficiency);
        sB = expSig/expBkg;
        sB_down = tempSB_down;
        bestCutSB = myProc.at(mySig)->GetHist()->GetBinLowEdge(i);
      }
      if(tempSRootB_down > SRootB_down){
        SRootB = expSig/sqrt(expBkg);
        SRootB_down = tempSRootB_down;
        sigEff_SRootB = std::min(expSig/totExpSig, max_efficiency);
        bkgEff_SRootB = std::min(expBkg/totExpBkg, max_efficiency);
        bestCutSRootB = myProc.at(mySig)->GetHist()->GetBinLowEdge(i);
      }
      if(tempSRootSB_down > SRootSB_down){
        SRootSB = expSig/sqrt(expBkg+expSig);
        SRootSB_down = tempSRootSB_down;
        sigEff_SRootSB = std::min(expSig/totExpSig, max_efficiency);
        bkgEff_SRootSB = std::min(expBkg/totExpBkg, max_efficiency);
        bestCutSRootSB = myProc.at(mySig)->GetHist()->GetBinLowEdge(i);
      }
    //}
  }
  delete bkg_th1;
  #ifdef P_LOG
    cout << "Found best figures:" << endl;
    cout << "  S/B = " << sB << " if we cut at " << bestCutSB << ". We have then " << bestSBsig << " signal events and " << bestSBbkg << " background events." << endl;
    cout << "  S/sqrt(B) = " << SRootB << " if we cut at " << bestCutSRootB << endl;
    cout << "  S/sqrt(S+B) = " << SRootSB << " if we cut at " << bestCutSRootSB << endl;
    cout << "  Gini = " << Gini_tot << " if we cut at " << bestCutGini << ". We have then " << bestGinisig << " signal events and " << bestGinibkg << " background events." << endl;
  #endif

  std::vector<double> bestCutSB_sigEff_bkgEff = {bestCutSB, sigEff_SB, bkgEff_SB, sB};
  std::vector<double> bestCutSRootB_sigEff_bkgEff = {bestCutSRootB, sigEff_SRootB, bkgEff_SRootB, SRootB};
  std::vector<double> bestCutSRootSB_sigEff_bkgEff = {bestCutSRootSB, sigEff_SRootSB, bkgEff_SRootSB, SRootSB};
  std::vector<double> bestCutGini_sigEff_bkgEff = {bestCutGini, sigEff_Gini, bkgEff_Gini, Gini_tot};
  return std::map<std::string, std::vector<double>> {{"SoverB", bestCutSB_sigEff_bkgEff}, {"SoverSqrtB", bestCutSRootB_sigEff_bkgEff}, {"SoverSqrtSB", bestCutSRootSB_sigEff_bkgEff}, {"Gini", bestCutGini_sigEff_bkgEff}};
}

void PAnalysis::WPFromFigureOfMerit(void){
  std::map<std::string, std::vector<double>> figuresOfMerit = FiguresOfMerit();
  myCut = figuresOfMerit.at(myConfig->GetSplitMode())[0];
  mySigEff = figuresOfMerit.at(myConfig->GetSplitMode())[1];
  myBkgEff = figuresOfMerit.at(myConfig->GetSplitMode())[2];
  myGini = figuresOfMerit.at(myConfig->GetSplitMode())[3];
  if (myCut == std::numeric_limits<double>::lowest()){
      myGini = 0.;
    #ifdef P_LOG
      cout << "No figure of merit could be defined (too big stat error or no down fluctuation bigger than 0) " << endl; //, switching to fixed efficiency mode." << endl;
    #endif
    exit(1);
    //BkgEffWPPrecise();
  }
  else if (myBkgEff == 1) {
    #ifdef P_LOG
      cout << "Best down fluctuated figure of merit lead to no background rejection " << endl; //, switching to fixed efficiency mode." << endl;
    #endif
    exit(1);
  }
  else{
    #ifdef P_LOG
      cout << "For signal efficiency " << mySigEff << ", cut on discriminant is " << myCut << ", and background efficiency is " << myBkgEff << ".\n";
    #endif
  }
}
  



void PAnalysis::WriteOutput(void){
  #ifdef P_LOG
    cout << "Writing output to " << myOutput << ".root.\n"; 
  #endif
  
  const std::vector<std::string>& options = myConfig->GetWriteOptions();

  myOutputFile->cd();
  
  if(myCnvEff && contains(options, "ROC"))
    myCnvEff->Write("ROC");
  
  if(myCnvPlot && contains(options, "plot"))
    myCnvPlot->Write("Plot");

  if(myProc.at(0)->GetHist()->GetEntries() && contains(options, "hist")) {
    for(unsigned int i=0; i<myProc.size(); i++){
      myProc.at(i)->GetHist()->Write();
      myProc.at(i)->GetAbsHist()->Write();
    }
  }
  
  if(myROC && contains(options, "ROC"))
    myROC->Write("ROC curve");
}

void PAnalysis::WriteSplitRootFiles(void){
  #ifdef P_LOG
    cout << "Splitting root files and writing output files.\n"; 
  #endif
  
  if(!myCut){
    cerr << "ERROR Cannot write split root files without knowing the cut value! Attempting to call BkgEffWPPrecise().\n";
    BkgEffWPPrecise();
  }
  
  TString outputDir = myConfig->GetOutputDir();
   
  for(unsigned int j=0; j<myProc.size(); j++){
    PProc* proc = (PProc*) myProc.at(j);

    proc->Open();

    TFile* outFileSig = new TFile(outputDir + "/" + myConfig->GetSplitName() + "_SigLike_proc_" + proc->GetName() + ".root","RECREATE");
    TTree* treeSig = proc->GetTree()->CloneTree(0);
    
    TFile* outFileBkg = new TFile(outputDir + "/" + myConfig->GetSplitName() + "_BkgLike_proc_" + proc->GetName() + ".root","RECREATE");
    TTree* treeBkg = proc->GetTree()->CloneTree(0);
    
    if(!outFileSig->IsOpen() || !outFileBkg->IsOpen()){
      cerr << "Error creating split output files.\n";
      exit(1);
    }
  
    // Adding this MVA's output to the output trees
    double mvaOutput;
    //TString outBranchName = "MVAOUT__" + outputDir + "/" + myName;
    //outBranchName.ReplaceAll("//","__");
    //outBranchName.ReplaceAll("/","__");
    //treeSig->Branch(outBranchName, &mvaOutput);
    //treeBkg->Branch(outBranchName, &mvaOutput);
    
    for(long i=0; i<proc->GetTree()->GetEntries(); i++){
      proc->GetTree()->GetEntry(i);
      mvaOutput = EvalDiscriminant(proc); 
      if(mvaOutput < myCut)
        treeBkg->Fill();
      else
        treeSig->Fill();
    }

    outFileSig->cd();
    treeSig->Write();
    outFileSig->Close();
    
    outFileBkg->cd();
    treeBkg->Write();
    outFileBkg->Close();

    proc->Close();
  }
}

void PAnalysis::WriteResult(void){
  TString output = myConfig->GetOutputDir()+"/"+myConfig->GetLogName();
  
  #ifdef P_LOG
    cout << "Writing cut efficiencies to " << output << ".\n";
  #endif
  
  if(mySigEff == 0 || myBkgEff == 0){
    cerr << "ERROR in PAnalysis::WriteResult(): The cut efficiencies have to be computed first. Attempting to call BkgEffWPPrecise().\n";
    BkgEffWPPrecise();
  }
  
  ofstream logFile;
  logFile.open(output);
  logFile << mySigEff << endl << myBkgEff << endl << myCut << endl << myGini;
  logFile.close();
}  

PAnalysis::~PAnalysis(){
  #ifdef P_LOG
    cout << "Destroying PAnalysis " << myName << ".\n";
  #endif

  for(unsigned i=0; i<myProc.size(); i++){
    delete myProc.at(i); myProc.at(i) = NULL;
  }
  delete myStack; myStack = NULL;
  delete myCnvPlot; myCnvPlot = NULL;
  delete myLegend; myLegend = NULL;
  delete myCnvEff; myCnvEff = NULL;
  delete myROC; myROC = NULL;
  delete myLine; myLine = NULL;
  delete myCutLine; myCutLine = NULL;
  delete myOutputFile; myOutputFile = NULL;
  delete myFactory; myFactory = NULL;
  delete myReader; myReader = NULL;
}

