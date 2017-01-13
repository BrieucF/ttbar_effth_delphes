import os
import ROOT
import sys

if len(sys.argv)<2:
    base_directory = "results/minmc_50_effRatio_2_discriBased/" # output of mischief where are the various 'replay' folders 
else :
    base_directory  = sys.argv[1]
#systematic_list = ["jec", "jjbtag", "pu", "llidiso_elidiso", "llidiso_muid", "llidiso_muiso"]
systematic_list = []

out_dir = base_directory+"/suitable_th1/"
if not os.path.isdir(out_dir):
    os.mkdir(out_dir)

# Extract the list of processes : 
nominal_file_dir = base_directory + "/replay_nominal/"
fileList = [file for file in  os.listdir(nominal_file_dir) if "_hists.root" in file] 
for file in fileList :
    print "Treating ", file
    # Nominal TH1
    nominal_rootFile = ROOT.TFile(nominal_file_dir + file, "read")
    nominal_th1 = nominal_rootFile.Get("yields")
    out_rootFile = ROOT.TFile(out_dir+file, "recreate")
    nominal_th1.Write()
    out_rootFile.Close()
    for systematic in systematic_list :
        if "llidiso" in systematic :
            suffix_up = "up"
            suffix_down = "down"
        else :
            suffix_up = "_up"
            suffix_down = "_down"

        up_systematic_dir = os.path.join(base_directory, "replay_" + systematic + suffix_up) 
        up_systematic_rootFile = ROOT.TFile(os.path.join(up_systematic_dir, file))
        up_systematic_th1 = up_systematic_rootFile.Get("yields")
        up_systematic_th1.SetTitle("yields__"+systematic+"up")
        up_systematic_th1.SetName("yields__"+systematic+"up")
        out_rootFile = ROOT.TFile(out_dir+file, "update")
        up_systematic_th1.Write()
        out_rootFile.Close()

        down_systematic_dir = os.path.join(base_directory, "replay_" + systematic + suffix_down) 
        down_systematic_rootFile = ROOT.TFile(os.path.join(down_systematic_dir, file))
        down_systematic_th1 = down_systematic_rootFile.Get("yields")
        down_systematic_th1.SetTitle("yields__"+systematic+"down")
        down_systematic_th1.SetName("yields__"+systematic+"down")
        out_rootFile = ROOT.TFile(out_dir+file, "update")
        down_systematic_th1.Write()
        out_rootFile.Close()
