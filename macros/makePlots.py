#!/usr/bin/env python

import sys
import ROOT
ROOT.gROOT.LoadMacro("AtlasStyle.C") 
ROOT.SetAtlasStyle()
import math

#dirnames = ["", "Photon", "Electron"]
dirnames = ["", "Photon"]

inputs = {}
#inputs["sc_with_cells"] = ("TestHistograms_sc_cells.root", ROOT.kGreen)
inputs["one_cluster"] = ("TestHistograms_one.root", ROOT.kRed)
inputs["one_cluster_phi"] = ("TestHistograms_one_phi.root", ROOT.kCyan)
inputs["one_cluster_5"] = ("TestHistograms_one_5.root", ROOT.kMagenta)
#inputs["sc_noemf10GeV"] = ("TestHistograms_sc_noemf2.root", ROOT.kCyan)
#inputs["sc_emf"] = ("TestHistograms_sc_subset.root", ROOT.kBlue)
inputs["superclusters"] = ("TestHistograms_sc.root", ROOT.kBlue) # no pileup now
inputs["superclusters_phi"] = ("TestHistograms_sc_phi.root", ROOT.kGreen) # no pileup now
inputs["superclusters_5"] = ("TestHistograms_sc_5.root", ROOT.kYellow+2) # no pileup now
inputs["sliding_windows"] = ("TestHistograms_sw.root", ROOT.kGreen)
#inputs["sw+toposeeded"] = ("TestHistograms_sw_toposeed.root", ROOT.kMagenta)
inputs["electrons"] = ("TestHistograms_electrons.root", ROOT.kMagenta)


import random, string

digit = 0
def incrementDigit():
   global digit
   digit += 1
   return digit

def randomword(length):
   return ''.join(random.choice(string.lowercase) for i in range(length))

def NormalizedErrors(tprofin):
   axis = tprofin.GetXaxis()
   binsArray = axis.GetXbins().GetArray()
   newtprof = ROOT.TH1F(randomword(15), "Error;"+axis.GetTitle()+";(Norm.) RMS Error" , tprofin.GetNbinsX(), binsArray)
   for i in range(tprofin.GetNbinsX() + 1):
      if tprofin.GetBinLowEdge(i) == 1.37:
         # ignore
         errval = 0
         errerr = 0
      else:
         print "Low edge", tprofin.GetBinLowEdge(i), "bool", tprofin.GetBinLowEdge(i) == 1.37, "error",
         errval = tprofin.GetBinError(i)/(tprofin.GetBinContent(i) + 1) if tprofin.GetBinContent(i) != 1 else 0
         print errval
         errerr = errval/math.sqrt(tprofin.GetBinEntries(i)) if tprofin.GetBinEntries(i) != 0 else 0
      newtprof.SetBinContent(i, errval)
      newtprof.SetBinError(i, errerr)
   return newtprof

def overlayPlots(inpts):
    if len(inpts) < 1:
        print "ERROR: inpts needs to have at least length one"
        sys.exit(1)
        

    # lets open all the files
    files = []
    colors = []
    for inp in inpts:
        print "Current input",inp
        fnd = inputs[inp]
        files.append(ROOT.TFile(fnd[0]))
        colors.append(fnd[1])

    for dirname in dirnames:
        files[0].cd(dirname)

        histnames = []

        for key in ROOT.gDirectory.GetListOfKeys():
            #print "key", key
            if not key.IsFolder():
                #print "not folder"
                if dirname == "":
                    histnames.append(key.GetName())
                else:
                    histnames.append(dirname + "/" + key.GetName())

        for histname in histnames:
            print "histname",histname

            hists = [fl.Get(histname) for fl in files]

            #print "hists",hists
            #hist = hists[0].Clone()

            if 'NumCells' in histname:
               # profiles = [hist.ProfileY("_pfy", 1, -1, "s") for hist in hists]
               profiles = map(lambda x: x.ProfileY(randomword(15), 1, -1, "s"), hists)
               #print "profiles",profiles
               [hist.GetXaxis().SetRangeUser(0, 1000) for hist in profiles]
               printHisto(histname, profiles, inpts, colors)
            elif '3D' in histname:
               # also print errors
               printHisto(histname, hists, inpts, colors)
               #errors = map(NormalizedErrors, hists)
               #printHisto(histname+"_err", errors, inpts, colors)
            else:
               printHisto(histname, hists, inpts, colors)


def printHisto(histname, hists, inpts, colors):
    #print "inpts", inpts
    #print "colors", colors
    c_paper = ROOT.TCanvas()
    #legend = ROOT.TLegend(0.19, 0.61, 0.47, 0.92)
    #legend = ROOT.TLegend(0.19, 0.75, 0.47, 0.92)
    if '3D' in histname:
       legend = ROOT.TLegend(0.69, 0.25, 0.95, 0.35)
    else:
       legend = ROOT.TLegend(0.19, 0.75, 0.35, 0.92)
    legend.SetFillColor(0)
    legend.SetBorderSize(0)
    legend.SetTextSize(0.038)

    isMu = 'EResolution' in histname and "_mu" in histname;
    #isRes = 'EResolution' in histname and "_mu" not in histname and "3D" not in histname;
    isEta = 'Eta' in histname and "3D" not in histname;
    #isEtruth = "Etruth" in histname and "3D" in histname

    #print "iterate overs",range(len(hists))

    for i in range(len(hists)):
        hists[i].SetLineColor(colors[i])            
        #if isRes:
        #    hists[i].SetAxisRange(-1.1,0.6)
        if isEta:
            hists[i].Rebin(2)
        #if isEtruth:
        #    print "isEtruth, histname", histname
        #    hists[i].SetAxisRange(0, 900)
        legend.AddEntry(hists[i], inpts[i], "l")
        # if isMu: 
        #     hists[i].Fit("pol1")
        #     hists[i].GetFunction("pol1").SetLineColor(colors[i])
        if i == 0:
           hists[i].SetLineWidth(4)
           hists[i].Draw()
        else:
           hists[i].Draw("same")

        # if isMu: 
        #     hists[i].Fit("pol1")
        #     hists[i].GetFunction("pol1").SetLineColor(colors[i])

    legend.Draw()
    c_paper.Print(histname + ".pdf")

if __name__ == "__main__":
    overlayPlots(sys.argv[1:])

