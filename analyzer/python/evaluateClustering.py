#! /bin/env python
from sklearn.cluster import KMeans
from sklearn.externals import joblib
import ROOT
from root_numpy import root2array, rec2array, tree2array

import os, sys
import pickle, pprint
import gc

import yaml

import argparse

parser = argparse.ArgumentParser(description='Replay a tree on a new input file')
parser.add_argument('config')

args = parser.parse_args()

n_clusters = list(range(2,21))
cluster_model_files = {}
for n_cluster in n_clusters:
    cluster_model_files[str(n_cluster)] = "/home/fynu/bfrancois/playWithScikit/models/cluster_"+str(n_cluster)+".pkl"

branch_names = ["pp_tt_llbb_tfJetAllEta_minLog_weight", "pp_Z_llbb_simple_tfJetAllEta_minLog_weight", "twplus_tfJetAllEta_minLog_weight", "twminus_tfJetAllEta_minLog_weight", "pp_zz_llbb_simple_tfJetAllEta_minLog_weight", "pp_zh_llbb_simple_tfJetAllEta_minLog_weight"]
condition =  ""
#for var in branch_names :
#    condition += "(" + var + "< 10000)&&"
#condition = condition[:-2]


configuration = {}
# Parse configuration
with open(args.config) as f:
    configuration = yaml.load(f)
    with open(configuration["analysis"]["trained_tree"]) as g:
        tree = pickle.Unpickler(g).load()

        # Clear old input files
        tree.cfg.procCfg = {}

        #root = tree.firstBox
        #mvaReader = TMVAReplayer.TMVAReplayer(configuration, root)
        #mvaReader.run()
        for datasets_key in configuration["datasets"].keys():
            print "Treating dataset :", datasets_key
            datasets = configuration["datasets"][datasets_key]
            inRootFileName = datasets['path'][0]
            # Load rootFile in scikit friendly form
            print "Treating ", inRootFileName
            inRootFile = ROOT.TFile(inRootFileName)
            tree = inRootFile.Get("t")
            features_to_feed_clustering = tree2array(tree, branch_names, condition)
            features_to_feed_clustering = rec2array(features_to_feed_clustering)
            # Output file with TH1s
            outRootFileName =  os.path.join("clustering_rootFiles",datasets['path'][0].split("/")[-1])
            outRootFile = ROOT.TFile(outRootFileName, "recreate")
            clusters_th1 = {}
            for n_cluster in n_clusters:
                print "     Nb cluster ", n_cluster

                #create TH1
                clusters_th1[str(n_cluster)] = ROOT.TH1F(str(n_cluster) + " clusters", str(n_cluster) + " clusters", n_cluster, 0, n_cluster)
                # Load clustering model
                cluster_model = joblib.load(cluster_model_files[str(n_cluster)])
                events_clusters = cluster_model.predict(features_to_feed_clustering)
                print tree.GetEntries()
                entry_in_clusters_assignment = 0
                for i in xrange(0,tree.GetEntries()):
                    entry = tree.GetEntry(i)
                    #process = True
                    #for var in branch_names:
                    #    if getattr(tree, var) > 10000 :
                    #        print "Skip event ", i ," due to ", var , " ",  getattr(tree, var) 
                    #        print "Event number : ", tree.event_event
                    #        process = False
                    #        sys.exit()
                    #if not process :
                    #    continue
                    
                    # Need TFormula evaluation
                    #clusters_th1[str(n_cluster)].Fill(float(events_clusters[i]), datasets['evtweight'])
                    #print "Entry ", i, " : ", events_clusters[entry_in_clusters_assignment] 
                    clusters_th1[str(n_cluster)].Fill(float(events_clusters[entry_in_clusters_assignment]))
                    entry_in_clusters_assignment += 1
                clusters_th1[str(n_cluster)].Write()
            outRootFile.Close()
            print outRootFileName, " written." 
            #outRootFile = ROOT.TFile()
            #mvaReader = TMVAReplayer.TMVAReplayer(configuration, root, {datasets_key : datasets})
            #mvaReader.run()
            #del mvaReader
            #del datasets
            #gc.collect()
