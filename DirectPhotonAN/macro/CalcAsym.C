#include <cmath>

#include <uspin/SpinDBOutput.h>
#include <uspin/SpinDBInput.h>

R__LOAD_LIBRARY(libuspin.so)

void CalcAsym(const std::string infile = "../output/AnalyzedData.root", int storenumber = 0, int runnumber = 47396)
{
    TFile *f = new TFile(infile.c_str());

    TString beam[2] = {"Blue", "Yellow"};

    //DEFINE HISTOS HERE

    //             GET SPIN PATTERN                  //
    //===============================================//
    TTree *tree = (TTree*)f->Get("tree");
    int bunchnumber;
    tree->SetBranchAddress("bunchnumber", &bunchnumber);
    int nentries = tree->GetEntries();

    unsigned int qa_level = 0xffff;
    SpinDBOutput spin_out("sphnxrc");
    SpinDBOutput spin_cont;
    spin_out.StoreDBContent(runnumber, runnumber, qa_level);
    spin_out.GetDBContentStore(spin_cont, runnumber);

    int crossingshift = spin_cont.GetCrossingShift();

    std::cout << "crossing shift: " << crossingshift std::endl;
    int bspinpat[120] = {0};
    int yspinpat[120] = {0};
    for (int i = 0; i < 120; i++)
    {
        bspinpat[i] = spin_cont.GetSpinPatternBlue(i);
        yspinpat[i] = spin_cont.GetSpinPatternYellow(i);

    }

    std::cout << "nentries: " << nentries << std::endl;

    for (int i = 0; i < nentries; i++)
    {
        //check: do we need clock offset like Devon had?
        tree->GetEntry(i);
        int sphenix_cross = (bunchnumber + crossingshift) % 120;

        int bspin = bspinpat[sphenix_cross];
        int yspin = yspinpat[sphenix_cross];

        
    }


}