# RDAnalysis
RDAnalysis is based on ROOT::RDataFrame.RDataFrame supports declarative programming and implicit multithreading, ensuring that analysis code is simplified while also making analysis much more efficient and enabling user transparent taskbased parallelism.About RDataFrame,you can learn more by https://root.cern/doc/master/classROOT_1_1RDataFrame.html.Now,it can be used for jetclustering and kinematic fittiing.It has now merged with CEPC.You can input  the root file after PFO and get higgs.png, which you can do in the example section.  
RDAnalysis source code in 
```shell
CEPCSW/build.103.0.2.x86_64-centos7-gcc11-opt/_deps/rdanalysis-src
```
## Feature
The main feature of RDAnalysis is parallel analysis based on RDataFrame.Parallelism allows programs to run on multiple threads.  
Use it in python by following.
```shell
import ROOT as cepc_ana
cepc_ana.ROOT.EnableImplicitMT(n)
```
n is the number of threads.
## Tools and data models
* Particles are selected by selLep(int i).Pdg of particles which equal to i are selected.
* In order to read information about jets or particles such as energy momentum,define get_lep_px(),get_jet_px(),etc
* The basic data types that kinematics fitting software migrates from, such as LeptonFitObject,JetFitObject, cannot be used directly in parallel analysis frameworks. To ensure that the output ROOT file can hold these objects, define their copy constructors and default functions within these classes.
* In order to input the particle or jet to be fitted into the kinematics fitting program, the particle and jet are defined into data types required by kinematics fitting, such as vector<LeptonFitObject>, vector<JetFitObject>, etc. 
* In order to use the four-momentum information of particles and jets,the data types required for a defined fit are defined by getLepton(RVec<float> px, RVec<float> py,RVec<float> pz, RVec<float> energy) and other functions. Due to the kinematic fitting using (1/pt,θ,φ), so the getLepton function also adds type conversion capabilities.
### Interface  
The interface for common functions(jetclustering and kinematic fittiing) is defined in RDAnalysis, everything is defined in C++ and finally called in Python.  
Firstly,link dynamic libraries and set multithreading mode.  
```shell
import ROOT as cepc_ana
cepc_ana.ROOT.EnableImplicitMT(n)
cepc_ana.gSystem.Load('libRDAnalysis.so')
load=cepc_ana.f()
```
#### JetClustering

For the input rootfile which is after PFO, create the RDataFrame object firstly.  
```shell
df = cepc_ana.RDataFrame("events","./data/2muon2jet.root")  
```
Secondly,remove muon and define 4 momentum of these particles.
```shell
d1=df.Define("cutMuon","ArborPFOs.type!=13&&ArborPFOs.type!=-13")\#remove muon by the corresponding pdg number
.Define("PX1",cepc_ana.get_lep_px("cutMuon"))\
.Define("PY1",cepc_ana.get_lep_py("cutMuon"))\
.Define("PZ1",cepc_ana.get_lep_pz("cutMuon"))\
.Define("E1",cepc_ana.get_lep_e("cutMuon"))\
```
The four-momentum of these particles is defined by the defined get_lep_px, get_lep_py,get_lep_pz,get_lep_e() in src.  
Then jetclustering
```shell
d2=d1.Define("pseudo_jets","RDAna::JetClusteringUtils::set_pseudoJets(PX1,PY1,PZ1,E1)")\#enter 4 momentum about these particles into jetclustering program
.Define("CEPCAnalysesJets_ee_genkt", "JetClustering::clustering_ee_kt(2, 2, 0, 0 )(pseudo_jets)")\#Set clustering parameters and make jetclustering
.Define("jets_ee_genkt", "RDAna::JetClusteringUtils::get_pseudoJets(CEPCAnalysesJets_ee_genkt)")\#create a column to store the jets
```  
set_pseudoJets and get_pseudoJets are used for data conversion.Jetclustering is done by JetClustering::clustering_ee_kt(2, 2, 0, 0 )(pseudo_jets) which is the interface we defined in the source code .  
For
```shell
JetClustering::clustering_ee_kt(exclusive, njet, sorted_by_what, recombination )(pseudo_jets)
```
* exclusive

  exclusive=2:use clust_seq.exclusive_jets(njet)

* sorted_by_what

  sorted_by_what=1:use fastjet::sorted_by_E  
  sorted_by_what=0:use fastjet::sorted_by_pt

* recombination

  recombination=0:use fastjet::RecombinationScheme::E_scheme  
  recombination=1:use fastjet::RecombinationScheme::pt_scheme
  
Finally,we define 4 momentum of jets and save 2jet in rootfile.  
```shell
d3=d2.Define("jets_ee_genkt_px","RDAna::JetClusteringUtils::get_px(jets_ee_genkt)")\#define 4 momentum of jets
.Define("jets_ee_genkt_py","RDAna::JetClusteringUtils::get_py(jets_ee_genkt)")\
.Define("jets_ee_genkt_pz","RDAna::JetClusteringUtils::get_pz(jets_ee_genkt)")\
.Define("jets_ee_genkt_e","RDAna::JetClusteringUtils::get_e(jets_ee_genkt)")\
.Filter("jets_ee_genkt_px.size()==2")\#get 2jet evnet
.Define("j2","getJet(jets_ee_genkt_px,jets_ee_genkt_py,jets_ee_genkt_pz,jets_ee_genkt_e)")\
.Snapshot("events","./data/afterjet.root",{"j2","ArborPFOs"})#save
```
#### Kinfit

Link dynamic libraries and define RDataFrame.  
Particles are selected by selLep.
```shell
df1=df.Define("Muon",cepc_ana.selLep(13))\
```
Save 4 momentum by get_lep_px,get_lep_py,get_lep_pz,get_lep_e.  
Then cut 2muon event ,use 2jet2muon in Kinfit,save. 
```shell
df2=df1.Filter("Muon_px.size()==2&&P_MX.size()==1")\
.Define("l1","getLepton(Muon_px,Muon_py,Muon_pz,Muon_e)")\
.Define("higgs_M","Kinfit::fit(j2[0],j2[1],l1[0],l1[1])")\
.Snapshot("events","./data/after4cfit.root",{"j2","higgs_M","l1"})
``` 
Kinfit is finished by Kinfit::fit().Pass the amount you need to fit into Kinfit::fit. getLepton() converts data to LeptonFitObject data class.


## Example
To better understand the workflow, we provide a example in CEPC here. It is an  Electron-positron collision at 240GeV with final state two jet two muon.
Firstly,
```shell
cd YOUR_PATH_TO_CEPCSW/build.103.0.2.x86_64-centos7-gcc11-opt/_deps/rdanalysis-src/example
```
run the example.
Secondly,run
```shell
python jetcluster.py
```
Input a rootfile(2muon2jet.root)to get a rootfile(afterjet.root) after JetClustering, which have two jet two muon data.  
Finally, run afterjet.root in
```shell
python Kinfit.py
```
to do kinematic fit to two jet and two muon, output file is after4cfit.root. To check the effectiveness of the 2 analysis tools, higgs mass reconstructed by two jet is output as higgs.png.  


## Note
If you want to add function to dynamic function library ,you should add header file to “include” and add source code to “src”.Beside ,you should pay attention to the data type.With test.py hello.cc and hello.h, you can know how to add.  
The input data should be after the PFO.If selLep(int i) is error,you can use "ArborPFOs.type==13||ArborPFOs.type==-13" instead.So,you can replace ArborPFOs with PFO in your data.


