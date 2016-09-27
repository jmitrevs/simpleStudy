#!/usr/bin/env python

import sys
import ROOT
#ROOT.gROOT.LoadMacro("AtlasStyle.C") 
#ROOT.SetAtlasStyle()
import math
import csv

MIN_ENTRIES = 20
FIT1_LOW = 1.5
FIT1_HIGH = 3
FIT2_LOW = 1.5
FIT2_HIGH = 3
TAIL = 2

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

def fitPlots(inpts):
    if len(inpts) != 1:
        print "ERROR: need to give exactly one input file."
        sys.exit(1)
        

    with open(inpts[0] + ".csv", "wb") as outfl: 

       outcsv = csv.writer(outfl)
       #legend
       outcsv.writerow(["suffix", "|eta|", "Etruth", "mean", "rms", 
                        "fit const", "const error",
                        "fit mean", "mean error",
                        "fit sigma", "sigma error",
                        "norm. sigma", "norm. sigma error",
                        "Fraction tail", "Tail error"
                        ])


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
               # print "histname",histname

               hists = [fl.Get(histname) for fl in files]

               if '3D' in histname:
                  # also print errors
                  fitHisto(histname, hists, outcsv)

               #ignore all others

def fitHisto(histname, hists, outcsv):
   """ 
   Fit the input histgram, and output results in text. For now only one input hist is supporte.
   (though left the old mechanism of giving a list of hists)
   """

  
   hist = hists[0]
   parsedName = histname.split("_")
   suffix = parsedName[0][20:]
   eta = float(parsedName[2])
   Etruth = int(parsedName[4])

   fitresults = []

   #print "Looking at histogram with suffix", suffix, "eta", eta, "Etruth", Etruth, "Entries = ", hist.GetEntries()

   if hist.GetEntries() >= MIN_ENTRIES:
      # actually output the fit
      c_paper = ROOT.TCanvas()
      mean = hist.GetMean()
      rms = hist.GetRMS()
      print "   Mean =", mean, "rms =", rms
      hist.Draw()
      fr1 = hist.Fit("gaus", "LS", "", mean - FIT1_LOW*rms, mean + FIT1_HIGH*rms)
      fit1mean = fr1.Parameter(1)
      fit1sigma = fr1.Parameter(2)
      fr2 = hist.Fit("gaus", "LS", "", fit1mean - FIT2_LOW*fit1sigma, fit1mean + FIT2_HIGH*fit1sigma)
      fit2mean = fr2.Parameter(1)
      fit2sigma = fr2.Parameter(2)
      fitresults = [fr2.Parameter(0), fr2.ParError(0), 
                    fit2mean, fr2.ParError(1), 
                    fit2sigma, fr2.ParError(2),
                    fit2sigma/(fit2mean + 1), fr2.ParError(2)/(fit2mean + 1)]

      c_paper.Print(histname + "_fit.pdf")

      ntot = hist.GetEntries()
      #calcualte tails, first low
      hist.GetXaxis().SetRangeUser(-1.1,fit2mean - TAIL*fit2sigma)
      tail = hist.Integral()
      hist.GetXaxis().SetRangeUser(fit2mean + TAIL*fit2sigma, 1.1)
      tail += hist.Integral()

      fracTail = tail/ntot
      tailError = math.sqrt(tail)/ntot # not correct

      outcsv.writerow([suffix, eta, Etruth, mean, rms] + fitresults + [fracTail, tailError])

   else:
      outcsv.writerow([suffix, eta, Etruth])


if __name__ == "__main__":
    fitPlots(sys.argv[1:])

