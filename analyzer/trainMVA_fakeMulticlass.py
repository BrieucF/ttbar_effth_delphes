import sys, os, copy
from tmvaTools import * 

sys.path.append("./python/")
from utils import PConfig, getEntriesEffentriesYieldTuple

cfgFile = sys.argv[1]

myConfig = PConfig(cfgFile)

#print myConfig.mvaCfg
#print myConfig.procCfg

procList = ["DY","TT","ZZ", "tbarWp", "tWm", "ZH"]

# SAMPLES FOR THE TRAINING
trainingFiles = {}
discriList = []
for proc in procList : 
    trainingFiles[proc] = {
            "files" : myConfig.procCfg[proc]["path"],
            "relativeWeight" : myConfig.procCfg[proc]["xsection"]/myConfig.procCfg[proc]["genevents"],
            }
    discriList.append(myConfig.procCfg[proc]["weightname"][0])

spectatorList = []
cut = "pp_Z_llbb_simple_tfJetAllEta_minLog_weight < 60 && pp_tt_llbb_tfJetAllEta_minLog_weight < 60 && pp_zz_llbb_simple_tfJetAllEta_minLog_weight < 60 && twplus_tfJetAllEta_minLog_weight < 60 && twminus_tfJetAllEta_minLog_weight < 60 && pp_zh_llbb_simple_tfJetAllEta_minLog_weight < 60"
MVAmethods = ['kBDT']  #'BDTG', 'MLP', 'FDA_GA', 'PDEFoam']
weightExpr = "jjbtag*pu*abs(event_weight)*trigeff*llidiso"
#weightExpr = "jjbtag*pu*trigeff*llidiso"

if __name__ == '__main__':
    for proc in procList :
        tempSigs = {}
        tempBkgs = copy.copy(trainingFiles)
        del tempBkgs[proc]
        tempSigs[proc] = trainingFiles[proc]
        label = "BDT_%s_vs_All_relativeWeight_absEvtWeight"%proc
        trainMVA(tempBkgs, tempSigs, discriList, cut, weightExpr, MVAmethods, spectatorList, label)

