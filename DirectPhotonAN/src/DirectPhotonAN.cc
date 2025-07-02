#include "DirectPhotonAN.h"
#include "Debugger.h"

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>

// Fun4All
#include <ffaobjects/EventHeader.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllServer.h>
#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>
#include <phool/phool.h>

#include <phhepmc/PHGenIntegral.h>

// ROOT stuff
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TLorentzVector.h>
#include <TTree.h>

// For clusters and geometry
#include <calobase/RawCluster.h>
#include <calobase/RawClusterContainer.h>
#include <calobase/RawClusterUtility.h>
#include <calobase/RawTower.h>
#include <calobase/RawTowerContainer.h>
#include <calobase/RawTowerDefs.h>
#include <calobase/RawTowerGeom.h>
#include <calobase/RawTowerGeomContainer.h>

#include <calotrigger/TriggerRunInfo.h>

// Tower stuff
#include <calobase/TowerInfo.h>
#include <calobase/TowerInfoContainerv4.h>
#include <calobase/TowerInfoDefs.h>

// GL1 Information
#include <ffarawobjects/Gl1Packet.h>

// for cluster vertex correction
#include <CLHEP/Geometry/Point3D.h>

// for the vertex
#include <globalvertex/GlobalVertex.h>
#include <globalvertex/GlobalVertexMap.h>
#include <globalvertex/MbdVertex.h>
#include <globalvertex/MbdVertexMap.h>

#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4VtxPoint.h>
#include <g4main/PHG4Shower.h>
// caloEvalStack for cluster to truth matching
#include <g4eval/CaloEvalStack.h>
#include <g4eval/CaloRawClusterEval.h>

#include <mbd/MbdGeom.h>
#include <mbd/MbdPmtContainer.h>
#include <mbd/MbdPmtHit.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <phhepmc/PHHepMCGenEvent.h>
#include <phhepmc/PHHepMCGenEventMap.h>
#include <ffaobjects/RunHeader.h>

#include <HepMC/GenEvent.h>
#include <HepMC/GenVertex.h> // for GenVertex, GenVertex::part...
#pragma GCC diagnostic pop

#include <jetbase/Jetv1.h>
#include <jetbase/Jetv2.h>
#include <jetbase/JetContainer.h>



//spin database stuff
#include <uspin/SpinDBContent.h>
#include <uspin/SpinDBOutput.h>

//____________________________________________________________________________..
DirectPhotonAN::DirectPhotonAN(const std::string &name, const std::string &filename) : SubsysReco(name)
{
  if(verbosity > 1){
    std::cout << "DirectPhotonAN::DirectPhotonAN(const std::string &name) Calling ctor" << std::endl;
  }
  outputFileName = filename;
}

//____________________________________________________________________________..
DirectPhotonAN::~DirectPhotonAN()
{
  if(verbosity > 1){
      std::cout << "DirectPhotonAN::~DirectPhotonAN() Calling dtor" << std::endl;
  }
}

//____________________________________________________________________________..
int DirectPhotonAN::Init(PHCompositeNode *topNode)
{
  if(verbosity > 1)
  {
    std::cout << "DirectPhotonAN::Init(PHCompositeNode *topNode) Initializing" << std::endl;
  }
  if(useOnnx)
  {
    onnxmodule = onnxSession(m_modelPath);
  }
  fout = new TFile(outputFileName.c_str(), "RECREATE");

  tree = new TTree("tree", "tree");
  tree->Branch("runnumber", &runnumber, "runnumber/I");
  tree->Branch("eventnumber", &eventnumber, "eventnumber/I");
  tree->Branch("bunchnumber", &bunchnumber, "bunchnumber/I");
  tree->Branch("trigger_prescale", trigger_prescale, "trigger_prescale[64]/F");
  tree->Branch("scaledtrigger", scaledtrigger, "scaledtrigger[64]/O");
  tree->Branch("livetrigger", livetrigger, "livetrigger[64]/O");
  tree->Branch("trigger_prescale", trigger_prescale, "trigger_prescale[64]/F");
  tree->Branch("vertexz", &vertexz, "vertexz/F");

  tree->Branch("bspin", &bspin, "bspin/F");
  tree->Branch("yspin", &yspin, "yspin/F");
  tree->Branch("lumiUpYellow", &lumiUpYellow, "lumiUpYellow/F");
  tree->Branch("lumiDownYellow", &lumiDownYellow, "lumiDownYellow/F");
  tree->Branch("lumiUpBlue", &lumiUpBlue, "lumiUpBlue/F");
  tree->Branch("lumiDownBlue", &lumiDownBlue, "lumiDownBlue/F");
  tree->Branch("polBlue", &polBlue, "polBlue/F");
  tree->Branch("polYellow", &polYellow, "polYellow/F");
  tree->Branch("crossingAngle", &crossingAngle, "crossingAngle/F");

tree->Branch(Form("ncluster_%s", clustername.c_str()), &ncluster, Form("ncluster_%s/I", clustername.c_str()));

  tree->Branch(Form("cluster_e_array_%s", clustername.c_str()), cluster_e_array, Form("cluster_e_array_%s[ncluster_%s][%d]/F", clustername.c_str(), clustername.c_str(), arrayntower));
  tree->Branch(Form("cluster_adc_array_%s", clustername.c_str()), cluster_adc_array, Form("cluster_adc_array_%s[ncluster_%s][%d]/F", clustername.c_str(), clustername.c_str(), arrayntower));
  tree->Branch(Form("cluster_time_array_%s", clustername.c_str()), cluster_time_array, Form("cluster_time_array_%s[ncluster_%s][%d]/F", clustername.c_str(), clustername.c_str(), arrayntower));
  tree->Branch(Form("cluster_e_array_idx_%s", clustername.c_str()), cluster_e_array_idx, Form("cluster_e_array_idx_%s[ncluster_%s][%d]/I", clustername.c_str(), clustername.c_str(), arrayntower));
  tree->Branch(Form("cluster_status_array_%s", clustername.c_str()), cluster_status_array, Form("cluster_status_array_%s[ncluster_%s][%d]/I", clustername.c_str(), clustername.c_str(), arrayntower));
  tree->Branch(Form("cluster_ownership_array_%s", clustername.c_str()), cluster_ownership_array, Form("cluster_ownership_array_%s[ncluster_%s][%d]/I", clustername.c_str(), clustername.c_str(), arrayntower));
  
  tree->Branch(Form("cluster_E_%s", clustername.c_str()), cluster_E, Form("cluster_E_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_Et_%s", clustername.c_str()), cluster_Et, Form("cluster_Et_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_Eta_%s", clustername.c_str()), cluster_Eta, Form("cluster_Eta_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_Phi_%s", clustername.c_str()), cluster_Phi, Form("cluster_Phi_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_prob_%s", clustername.c_str()), cluster_prob, Form("cluster_prob_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_merged_prob_%s", clustername.c_str()), cluster_merged_prob, Form("cluster_merged_prob_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_CNN_prob_%s", clustername.c_str()), cluster_CNN_prob, Form("cluster_CNN_prob_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_02_%s", clustername.c_str()), cluster_iso_02, Form("cluster_iso_02_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_%s", clustername.c_str()), cluster_iso_03, Form("cluster_iso_03_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_04_%s", clustername.c_str()), cluster_iso_04, Form("cluster_iso_04_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_emcal_%s", clustername.c_str()), cluster_iso_03_emcal, Form("cluster_iso_03_emcal_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_hcalin_%s", clustername.c_str()), cluster_iso_03_hcalin, Form("cluster_iso_03_hcalin_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_hcalout_%s", clustername.c_str()), cluster_iso_03_hcalout, Form("cluster_iso_03_hcalout_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_60_emcal_%s", clustername.c_str()), cluster_iso_03_60_emcal, Form("cluster_iso_03_60_emcal_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_60_hcalin_%s", clustername.c_str()), cluster_iso_03_60_hcalin, Form("cluster_iso_03_60_hcalin_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_60_hcalout_%s", clustername.c_str()), cluster_iso_03_60_hcalout, Form("cluster_iso_03_60_hcalout_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_120_emcal_%s", clustername.c_str()), cluster_iso_03_120_emcal, Form("cluster_iso_03_120_emcal_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_120_hcalin_%s", clustername.c_str()), cluster_iso_03_120_hcalin, Form("cluster_iso_03_120_hcalin_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_03_120_hcalout_%s", clustername.c_str()), cluster_iso_03_120_hcalout, Form("cluster_iso_03_120_hcalout_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_04_emcal_%s", clustername.c_str()), cluster_iso_04_emcal, Form("cluster_iso_04_emcal_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_04_hcalin_%s", clustername.c_str()), cluster_iso_04_hcalin, Form("cluster_iso_04_hcalin_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iso_04_hcalout_%s", clustername.c_str()), cluster_iso_04_hcalout, Form("cluster_iso_04_hcalout_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e1_%s", clustername.c_str()), cluster_e1, Form("cluster_e1_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e2_%s", clustername.c_str()), cluster_e2, Form("cluster_e2_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e3_%s", clustername.c_str()), cluster_e3, Form("cluster_e3_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e4_%s", clustername.c_str()), cluster_e4, Form("cluster_e4_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_et1_%s", clustername.c_str()), cluster_et1, Form("cluster_et1_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_et2_%s", clustername.c_str()), cluster_et2, Form("cluster_et2_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_et3_%s", clustername.c_str()), cluster_et3, Form("cluster_et3_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_et4_%s", clustername.c_str()), cluster_et4, Form("cluster_et4_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_weta_%s", clustername.c_str()), cluster_weta, Form("cluster_weta_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_wphi_%s", clustername.c_str()), cluster_wphi, Form("cluster_wphi_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_weta_cog_%s", clustername.c_str()), cluster_weta_cog, Form("cluster_weta_cog_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_wphi_cog_%s", clustername.c_str()), cluster_wphi_cog, Form("cluster_wphi_cog_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_weta_cogx_%s", clustername.c_str()), cluster_weta_cogx, Form("cluster_weta_cogx_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_wphi_cogx_%s", clustername.c_str()), cluster_wphi_cogx, Form("cluster_wphi_cogx_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_ietacent_%s", clustername.c_str()), cluster_ietacent, Form("cluster_ietacent_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_iphicent_%s", clustername.c_str()), cluster_iphicent, Form("cluster_iphicent_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_detamax_%s", clustername.c_str()), cluster_detamax, Form("cluster_detamax_%s[ncluster_%s]/I", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_dphimax_%s", clustername.c_str()), cluster_dphimax, Form("cluster_dphimax_%s[ncluster_%s]/I", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_nsaturated_%s", clustername.c_str()), cluster_nsaturated, Form("cluster_nsaturated_%s[ncluster_%s]/I", clustername.c_str(), clustername.c_str()));
  
  tree->Branch(Form("cluster_e11_%s", clustername.c_str()), cluster_e11, Form("cluster_e11_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e22_%s", clustername.c_str()), cluster_e22, Form("cluster_e22_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e13_%s", clustername.c_str()), cluster_e13, Form("cluster_e13_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e15_%s", clustername.c_str()), cluster_e15, Form("cluster_e15_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e17_%s", clustername.c_str()), cluster_e17, Form("cluster_e17_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e31_%s", clustername.c_str()), cluster_e31, Form("cluster_e31_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e51_%s", clustername.c_str()), cluster_e51, Form("cluster_e51_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e71_%s", clustername.c_str()), cluster_e71, Form("cluster_e71_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e33_%s", clustername.c_str()), cluster_e33, Form("cluster_e33_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e35_%s", clustername.c_str()), cluster_e35, Form("cluster_e35_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e37_%s", clustername.c_str()), cluster_e37, Form("cluster_e37_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e53_%s", clustername.c_str()), cluster_e53, Form("cluster_e53_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e73_%s", clustername.c_str()), cluster_e73, Form("cluster_e73_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e55_%s", clustername.c_str()), cluster_e55, Form("cluster_e55_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e57_%s", clustername.c_str()), cluster_e57, Form("cluster_e57_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e75_%s", clustername.c_str()), cluster_e75, Form("cluster_e75_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e77_%s", clustername.c_str()), cluster_e77, Form("cluster_e77_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_w32_%s", clustername.c_str()), cluster_w32, Form("cluster_w32_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e32_%s", clustername.c_str()), cluster_e32, Form("cluster_e32_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_w52_%s", clustername.c_str()), cluster_w52, Form("cluster_w52_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e52_%s", clustername.c_str()), cluster_e52, Form("cluster_e52_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_w72_%s", clustername.c_str()), cluster_w72, Form("cluster_w72_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));
  tree->Branch(Form("cluster_e72_%s", clustername.c_str()), cluster_e72, Form("cluster_e72_%s[ncluster_%s]/F", clustername.c_str(), clustername.c_str()));  


  E_histo = new TH1F("E_histo","E_histo", 100, 0, 100);

  std::cout << "Finished Init()" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}


//____________________________________________________________________________..
int DirectPhotonAN::InitRun(PHCompositeNode *topNode)
{

  Debugger *debugger = Debugger::getInstance();
  debugger->checkpointInitRun();
  runnumber = 0;

  RunHeader *runheader = findNode::getClass<RunHeader>(topNode, "RunHeader");
  if (!runheader)
  {
    if(verbosity > 0){
        std::cout << "DirectPhotonAN::InitRun(PHCompositeNode *topNode) Can't find runheader, resetting node tree" << std::endl;
    }
   return Fun4AllReturnCodes::RESET_NODE_TREE;
  }
  runnumber = runheader->get_RunNumber();

  
//   int spinDB_status = getSpinInfo();
//   if(spinDB_status) 
//   {
//     std::cout << "Error: spinDB_Status: " << spinDB_status << std::endl;
//     std::cout << "Aborting run" << std::endl;
//     return Fun4AllReturnCodes::ABORTRUN;
//   }

  if(verbosity > 1){
    std::cout << "DirectPhotonAN::InitRun(PHCompositeNode *topNode) Initializing for Run " << runnumber << std::endl;
  }
  return Fun4AllReturnCodes::EVENT_OK;
}



int DirectPhotonAN::process_event(PHCompositeNode *topNode)
{

    Debugger *debugger = Debugger::getInstance();
    debugger->checkpoint1();
    if(verbosity > 1)
    {
        std::cout << "DirectPhotonAN::process_event(PHCompositeNode *topNode) Processing event."<< std::endl;
    }
    if(verbosity > 3)
    {
        std::cout << "DirectPhotonAN::process_event: " << "Saving eventnumber" << std::endl;
    }

    EventHeader *eventheader = findNode::getClass<EventHeader>(topNode, "EventHeader");
    if(eventheader)
    {
        if(eventheader->isValid()){ eventnumber = eventheader->get_EvtSequence(); }
    }
    else
    {
        eventnumber = -1;
        if(verbosity > 0){
            std::cout << "DirectPhotonAN::process_event: No valid EventHeader found. Event Number set to -1" << std::endl;
        }
    }
    debugger->print_eventNumber(eventnumber);
    if(verbosity > 2 && eventnumber >= 0)
    {
        std::cout << "DirectPhotonAN::process_event(PHCompositeNode *topNode) Processing event " << eventnumber << std::endl;
    }
  // What we need to do in process event:
  // Extract EMCal tower info
  // Extract EMCal cluster info
  // Reconstruct photons
  // Save info in ttree, maybe some histos?
  
  //Trigger logic

    if(verbosity > 2)
    {
        std::cout << "DirectPhotonAN::process_event: " << "Processing gl1PacketInfo" << std::endl;
    }

    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
    /*                Process Gl1Packet                         */
    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
    Gl1Packet *gl1PacketInfo = findNode::getClass<Gl1Packet>(topNode, "GL1Packet");
    if(!gl1PacketInfo && verbosity > 0)
    {
        std::cout << PHWHERE << "DirectPhotonAN::process_event: " << "Gl1Packet" << " node is missing. Output related to this node will be empty" << std::endl;
    }
    if(gl1PacketInfo)
    {
        // Get spins
        bunchnumber = gl1PacketInfo->getBunchNumber();
        sphenixBunch = (bunchnumber + crossingShift)%NBUNCHES;
        bspin = spinPatternBlue[sphenixBunch];
        yspin = spinPatternYellow[sphenixBunch];

        uint64_t triggervec = gl1PacketInfo->getScaledVector();
        uint64_t triggervecraw = gl1PacketInfo->getLiveVector();

        // Record which triggers are set
        for (int i = 0; i < 64; i++)
        {
            // Is the ith bit flipped?
            bool trig_decision = ((triggervec & 0x1U) == 0x1U);
            bool trig_decision_raw = ((triggervecraw & 0x1U) == 0x1U);

            scaledtrigger[i] = trig_decision;
            livetrigger[i] = trig_decision_raw;
            
            nscaledtrigger[i] += int(trig_decision);
            nlivetrigger[i] += int(trig_decision_raw);

            // Increment to have LSB now the i+1th bit
            triggervec = (triggervec >> 1U);
            triggervecraw = (triggervecraw >> 1U);

            //Check: do I need initscaler and currentscaler raw/live/scaled
        }

    }

    // Save the scaledown
    TriggerRunInfo *trigRunInfo = findNode::getClass<TriggerRunInfo>(topNode, "TriggerRunInfo");
    for (int i = 0; i < 32; i++)
    {
        trigger_prescale[i] = trigRunInfo->getPrescaleByBit(i);
    }
    debugger->checkpoint2();
    if(scaledtrigger[29] || scaledtrigger[30] || scaledtrigger[31]){num_photon_trigger_passed++;}

    vertexz = -9999;
    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
    /*             Process MbdPmtContainer                      */
    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

    //Can implement Minimum bias detector module if needed in future

    /*
    if(verbosity > 3)
    {
        std::cout << "DirectPhotonAN::process_event: " << "Processing MbdPmtContainer" << std::endl;
    }
    MbdPmtContainer *mbdtow = findNode::getClass<MbdPmtContainer>(topNode, "MbdPmtContainer");
    if(mbdtow)
    {
    }
    else
    {
        if(verbosity > 0){
            std::cout << "DirectPhotonAN::process_event: " << "MbdPmtContainer node is missing." << std::endl;
        }
    }
    */ 

    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
    /*                Process MbdVertexMap                      */
    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

    MbdVertexMap *vertexmap = findNode::getClass<MbdVertexMap>(topNode, "MbdVertexMap");
    if(! vertexmap)
    {
        if(require_vertexmap)
        {
            if(verbosity > 0)
            {
                std::cout << PHWHERE << "DirectPhotonAN::process_event: " << "MbdVertexMap" << " node is missing. Skipping event #" << eventnumber << std::endl;
            }
            return Fun4AllReturnCodes::EVENT_OK;
        }
        else
        {
            if(verbosity > 0)
            {
                std::cout << PHWHERE << "DirectPhotonAN::process_event: " << "MbdVertexMap" << " node is missing. Setting vtx z to 0 for event #" << eventnumber << std::endl;
            }
            vertexz = 0;
        }
    }
    else
    {
        if(vertexmap->empty()){
            if(verbosity > 3) //This happens a lot...
            {
                std::cout << PHWHERE << "DirectPhotonAN::process_event: " << "vertexmap" << " is empty. Skipping event #" << eventnumber << std::endl;
            }
        return Fun4AllReturnCodes::EVENT_OK;
        }
        MbdVertex *vtx = vertexmap->begin()->second;
        if(!vtx)
        {
            if(verbosity > 0)
            {
                std::cout << PHWHERE << "DirectPhotonAN::process_event: " << "MbdVertex" << " is null. Skipping event #" << eventnumber << std::endl;
            }
            return Fun4AllReturnCodes::EVENT_OK;
        }
        vertexz = vtx->get_z();
        if(vertexz != vertexz)
        {
            if(verbosity > 0)
            {
                std::cout << PHWHERE << "DirectPhotonAN::process_event: " << "vertexz" << " is nan. Skipping event #" << eventnumber << std::endl;
            }
            return Fun4AllReturnCodes::EVENT_OK;
        }
    }
    if(abs(vertexz) > vertexz_cut)
    {
        if(verbosity > 0)
        {
            std::cout << PHWHERE << "DirectPhotonAN::process_event: " << "vertexz" << " failed cut. Skipping event #" << eventnumber << std::endl;
        }
        return Fun4AllReturnCodes::EVENT_OK;
    }
    debugger->checkpoint3();


    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
    /*             Process Calo Towers and Clusters             */
    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
    if(verbosity > 2)
    {
        std::cout << "DirectPhotonAN::process_event: " << "Beginning processing Calo Towers and Clusters" << std::endl;
    }
    // geom nodes:
    geomEM = findNode::getClass<RawTowerGeomContainer>(topNode, "TOWERGEOM_CEMC");
    geomIH = findNode::getClass<RawTowerGeomContainer>(topNode, "TOWERGEOM_HCALIN");
    geomOH = findNode::getClass<RawTowerGeomContainer>(topNode, "TOWERGEOM_HCALOUT");
    
    debugger->print_requiredTowerNodes(requiredTowerNodes);
    // Handle all TowerNodes here rather than one by one
    for(const auto& [towerNodeName, requirement] : requiredTowerNodes){
        currTowerContainer = findNode::getClass<TowerInfoContainerv4>(topNode, towerNodeName);
        if(!currTowerContainer)
        {
            // Can't process event without required nodes
            if(requirement)
            {
                if(verbosity > 0)
                {
                    std::cout << "DirectPhotonAN::process_event Could not locate tower node " 
                        << towerNodeName << std::endl;
                    std::cout << "Aborting event #" << eventnumber << std::endl;
                }
                return Fun4AllReturnCodes::ABORTEVENT;
            }
            // Fill map with null pointers so that we can handle later
            else
            {
                towerNodeMap.insert({towerNodeName, NULL});
            }
        }
        else
        {
            // Keep track of valid nodes with valid ptr (and invalid with null ptr)
            towerNodeMap.insert({towerNodeName, currTowerContainer});
        }
    }

    if(! (geomEM && geomIH && geomOH))
    {
        if(verbosity > 0)
        {
            std::cout << PHWHERE << "DirectPhotonAN::process_event - missing tower geometry node, aborting event" 
                << std::endl;
        }
        return Fun4AllReturnCodes::ABORTEVENT;
    }
    debugger->checkpoint4();


    RawClusterContainer *clusterContainer = 
        findNode::getClass<RawClusterContainer>(topNode, clustername);
    if(!clusterContainer)
    {
        if(verbosity > 0)
        {
            std::cout << PHWHERE << "DirectPhotonAN::process_event Missing cluster node: "
                        << clustername << std::endl;
            std::cout << "Aborting event" << std::endl;
        }
        return Fun4AllReturnCodes::ABORTEVENT;
    }
    ncluster = 0;
    debugger->clusterCheckpoint1();
    RawClusterContainer::ConstRange clusterEnd = clusterContainer->getClusters();
    RawClusterContainer::ConstIterator clusterIter;
    float maxclusterpT = -1;

    debugger->print_clustercontainer(clusterContainer);

    // Iterate over the reconstructed clusters
    for (clusterIter = clusterEnd.first; clusterIter != clusterEnd.second; ++clusterIter)
    {
        debugger->clusterCheckpoint2();
        // Pull cluster properties
        RawCluster *recoCluster = clusterIter->second;
        float prob = recoCluster->get_prob();
        float merged_prob = recoCluster->get_merged_cluster_prob();

        CLHEP::Hep3Vector vertex(0,0,vertexz);

        float ecalib = 1.00;
        float E = recoCluster->get_energy() * ecalib;

        float phi = RawClusterUtility::GetAzimuthAngle(*recoCluster, vertex);
        float eta = RawClusterUtility::GetPseudorapidity(*recoCluster, vertex);
        float ET = E / cosh(eta);

        E_histo->Fill(E);
        num_reached_ET_cut++;
        debugger->clusterCheckpoint3();
        if(ET < clusterpTmin)
        {
            if(verbosity > 2)
            {
                std::cout << "DirectPhotonAN::process_event: "
                            << "Skipping cluster with ET = " << ET <<" < " << clusterpTmin << std::endl;
            }
            continue; // Skip clusters with low ET
        }
        //DEBUGGING
        debugger->print_ETCut(ET);
        num_passed_ET_cut++;

        // Array for storing iso energy for each different radii
        float clusteriso[nRadii];
        for (int i = 0; i < nRadii; i++)
        {
            //CHECK
            clusteriso[i] = recoCluster->get_et_iso(2 + i, false, true);
        }
        debugger->clusterCheckpoint4();

        //CHECK
        //Calculate various ET values
        // eta: eta of cluster
        // phi : phi of cluster
        //                 calculateET(eta, phi, dR,  layer, min_E)
        float emcalET_04 = calculateET(eta, phi, 0.4, 0, -10.0);
        float ihcalET_04 = calculateET(eta, phi, 0.4, 1, -10.0);
        float ohcalET_04 = calculateET(eta, phi, 0.4, 2, -10.0);

        float emcalET_03 = calculateET(eta, phi, 0.3, 0, -10.0);
        float ihcalET_03 = calculateET(eta, phi, 0.3, 1, -10.0);
        float ohcalET_03 = calculateET(eta, phi, 0.3, 2, -10.0);

        float emcalET_03_60 = calculateET(eta, phi, 0.3, 0, 0.06);
        float ihcalET_03_60 = calculateET(eta, phi, 0.3, 1, 0.06);
        float ohcalET_03_60 = calculateET(eta, phi, 0.3, 2, 0.06);

        float emcalET_03_120 = calculateET(eta, phi, 0.3, 0, 0.12);
        float ihcalET_03_120 = calculateET(eta, phi, 0.3, 1, 0.12);
        float ohcalET_03_120 = calculateET(eta, phi, 0.3, 2, 0.12);

        if(ET > maxclusterpT)
        {
            maxclusterpT = ET;
        }
        debugger->clusterCheckpoint5();

        //CHECK
        std::vector<float> showershape = recoCluster->get_shower_shapes(0.070);
        // Skip valid clusters
        if(showershape.size() == 0)
        {
            if(verbosity > 2){
                std::cout << "DirectPhotonAN::process_event: "
                            << "Skipping cluster with no shower shape info" << std::endl;
            }
            continue;
        }
        debugger->print_showershape(showershape);
        
        std::pair<int,int> leadtowerindex = recoCluster->get_lead_tower();

        debugger->clusterCheckpoint6();
        // Filling the 7x7 matrix
        if(verbosity > 3){ std::cout << "finding shower shapes in 7x7";}
        int maxieta = leadtowerindex.first;
        int maxiphi = leadtowerindex.second;
        

        /*
        int maxtowerieta = maxieta;
        int maxtoweriphi = maxiphi;

        // Preparing inputs for classifier
        float CNNprob = -1;
        std::vector<float> input;
        // resize to inputDimx x inputDimy
        int vectorSize = inputDimx * inputDimy;
        input.resize(vectorSize, 0);

        if (ET > 0)
        {
            int xlength = int((inputDimx - 1) / 2);
            int ylength = int((inputDimy - 1) / 2);
            if (maxtowerieta - ylength < 0 || maxtowerieta + ylength >= 96)
            {

                if(verbosity > 3){
                    std::cout << "DirectPhotonAN::process_event: maxtowerieta out of range, maxtowerieta = " << maxtowerieta << std::endl;
                    std::cout << "Continuing..." << std::endl;
                }
                continue;
            }
            debugger->clusterCheckpoint7();
            for (int ieta = maxtowerieta - ylength; ieta <= maxtowerieta + ylength; ieta++)
            {
                for (int iphi = maxtoweriphi - xlength; iphi <= maxtoweriphi + xlength; iphi++)
                {
                    int mappediphi = iphi;

                    if(mappediphi < 0)
                    {
                        mappediphi += 256;
                    }

                    if(mappediphi >= 256)
                    {
                        mappediphi -= 256;
                    }
                    unsigned int towerinfokey = TowerInfoDefs::encode_emcal(ieta, mappediphi);
                    TowerInfo *towerinfo = towerNodeMap["TOWERINFO_CALIB_CEMC_RETOWER"]->get_tower_at_key(towerinfokey);
                    if (!towerinfo)
                    {
                        if(verbosity > 3)
                        {
                            std::cout << "DirectPhotonAN::process_event: "
                                        << "No towerinfo for tower key " << towerinfokey << std::endl;
                            std::cout << "ieta: " << ieta << " iphi" << mappediphi << std::endl;
                        }
                        continue;
                    }
                    int index = (ieta - maxtowerieta + ylength) * inputDimx + iphi - maxtoweriphi + xlength;
                    input.at(index) = towerinfo->get_energy();

                }
            }
        }
        if(useOnnx)
        {
            std::vector<float> probresult = onnxInference(onnxmodule, input, 1, inputDimx, inputDimy, inputDimz, outputDim);
            CNNprob = probresult[0];
        }
        else
        {
            CNNprob = -1;
        }*/
        debugger->clusterCheckpoint8();

        /*
        // DEBUGGING
        std::cout << "DEBUGGING: recoCluster towermap: (key, eta, phi)";
        const RawCluster::TowerMap tm = recoCluster->get_towermap();
        for (auto tower_iter : tm)
        {
            RawTowerDefs::keytype tower_key = tower_iter.first;
            float eta = RawTowerDefs::decode_index1(tower_key);
            float phi = RawTowerDefs::decode_index2(tower_key);
            
            std::cout << "\t(" << tower_key << ", " << eta << ", " <<phi << ")" << std::endl;
        }

        std::cout << "DEBUGGING: TOWERINFO_CALIB_CEMC_RETOWER: (tower key, eta, phi), (key, eta, phi)" << std::endl;
        TowerInfoContainerv4 *tfcemc = towerNodeMap["TOWERINFO_CALIB_CEMC"];
        float ntowers = tfcemc->size();
        for (unsigned int channel = 0; channel < ntowers; channel++)
        {

            unsigned int towerkey = tfcemc->encode_key(channel);
            int ieta = tfcemc->getTowerEtaBin(towerkey);
            int iphi = tfcemc->getTowerPhiBin(towerkey);
            RawTowerDefs::CalorimeterId caloid = RawTowerDefs::CalorimeterId::CEMC;
            RawTowerDefs::keytype key = RawTowerDefs::encode_towerid(caloid, ieta, iphi);
            RawTowerGeom *tower_geom = geomEM->get_tower_geometry(key);
            double this_phi = tower_geom->get_phi();
            double this_eta = getTowerEta(tower_geom, 0, 0, vertexz);
            if(ieta > 82 && ieta < 85 && iphi > 122 && iphi < 125)
            {
                std::cout << "\t(" << towerkey << ", " << ieta << ", " << iphi << "), (" << key << ", " << this_eta << ", " << this_phi << ")" << std::endl;
            }
        }
        */





        // Calculating distance spread between the highest energy tower and the other towers
        int detamax = 0;
        int dphimax = 0;
        int nsaturated = 0;
        std::set<unsigned int> towers_in_cluster;
        
        //Loop over the towers in current cluster
        const RawCluster::TowerMap tower_map = recoCluster->get_towermap();
        for (auto tower_iter : tower_map)
        {
            debugger->clusterCheckpoint9();
            RawTowerDefs::keytype tower_key = tower_iter.first;

            float eta = RawTowerDefs::decode_index1(tower_key);
            float phi = RawTowerDefs::decode_index2(tower_key);

            unsigned int towerinfokey = TowerInfoDefs::encode_emcal(eta,phi);
            TowerInfo *towerinfo = (towerNodeMap["TOWERINFO_CALIB_CEMC"])->get_tower_at_key(towerinfokey);
            towers_in_cluster.insert(towerinfokey);

            if(towerinfo)
            {
                if (towerinfo->get_isSaturated()){nsaturated++;}
            }
            
            // Calculate the angular distance with wrapping
            int totalphibins = 256;
            auto dphiwrap = [totalphibins](float towerphi, float maxiphi)
            {
                float idphi = towerphi - maxiphi;
                float idphiwrap = totalphibins - std::abs(idphi);
                if (std::abs(idphiwrap) < std::abs(idphi))
                {
                    return (idphi > 0) ? -idphiwrap : idphiwrap;
                }
                return idphi;
            };
            float deta = eta - maxieta;
            float dphi = dphiwrap(phi, maxiphi);

            if(abs(deta) > detamax) {detamax = abs(deta);}
            if(abs(dphi) > dphimax) {dphimax = abs(dphi);}
        }
        debugger->clusterCheckpoint10();

        float avg_eta = showershape[4] + 0.5;
        float avg_phi = showershape[5] + 0.5;

        // Find center of cluster
        maxieta = std::floor(avg_eta);
        maxiphi = std::floor(avg_phi);

        // Skip the cluster if too close to the edge
        if(maxieta < 3 || maxieta > 92) 
        {
            if(verbosity > 3){
                std::cout << "DirectPhotonAN::process_event: maxieta out of range, maxieta = " << maxieta << std::endl;
                std::cout << "Continuing..." << std::endl;
            }
            continue;
        }

        float E77[7][7] = {0};
        int E77_ownership[7][7] = {0};

        // Loop over 7 by 7 phi/eta bins around cluster center
        for (int ieta = maxieta - 3; ieta < maxieta + 4; ieta++)
        {
            for (int iphi = maxiphi - 3; iphi < maxiphi + 4; iphi++)
            {
                debugger->clusterCheckpoint11();
                int temp_ieta = ieta;
                int temp_iphi = iphi;

                int deta = ieta - maxieta + 3;
                int dphi = iphi - maxiphi + 3;

                int arraykey = deta * 7 + dphi;
                shift_tower_index(ieta, iphi, 96, 256);
                cluster_e_array_idx[ncluster][arraykey] = TowerInfoDefs::encode_emcal(ieta, iphi);
                cluster_ownership_array[ncluster][arraykey] = 0;
                cluster_e_array[ncluster][arraykey] = 0;
                cluster_adc_array[ncluster][arraykey] = 0;
                cluster_time_array[ncluster][arraykey] = 0;

                unsigned int towerinfokey = TowerInfoDefs::encode_emcal(ieta, iphi);

                ieta = temp_ieta;
                iphi = temp_iphi;

                // Check if key is in the cluster
                if(towers_in_cluster.find(towerinfokey) != towers_in_cluster.end())
                {
                    cluster_ownership_array[ncluster][arraykey] = 1;
                    E77_ownership[ieta - maxieta + 3][iphi - maxiphi + 3] = 1;
                }
                if(ieta < 0 || ieta > 95) 
                {
                    if(verbosity > 3){
                        std::cout << "DirectPhotonAN::process_event: ieta out of range, ieta = " << ieta << std::endl;
                        std::cout << "Continuing..." << std::endl;
                    }
                    continue;
                }
                TowerInfo *towerinfo = towerNodeMap["TOWERINFO_CALIB_CEMC"]->get_tower_at_key(towerinfokey);

                if(! towerinfo)
                {
                    if(verbosity > 3)
                    {
                        std::cout << "DirectPhotonAN::process_event: ";
                        std::cout << "No towerinfo for tower key " << towerinfokey << std::endl;
                        std::cout << "ieta: " << ieta << " iphi " << iphi << std::endl;
                        std::cout << "Continuing..." << std::endl;
                    }
                    continue;
                }

                //Save tower status
                cluster_status_array[ncluster][arraykey] = (int)towerinfo->get_status();
                cluster_e_array[ncluster][arraykey] = towerinfo->get_energy();
                cluster_time_array[ncluster][arraykey] = towerinfo->get_time_float();

                if(!towerinfo->get_isGood())
                {
                    if(verbosity > 3)
                    {
                        std::cout << "DirectPhotonAN::process_event: ";
                        std::cout << "tower info not good; tower E: " << towerinfo->get_energy()
                        << " is hot: "<<towerinfo->get_isHot()<< " is bad chi2: "
                        <<towerinfo->get_isBadChi2()<< " is saturated: "<<towerinfo->get_isSaturated()<<std::endl;
                        std::cout << "Continuing..." << std::endl;
                    }
                    continue;
                }
                E77[ieta - maxieta + 3][iphi - maxiphi + 3] = towerinfo->get_energy() > m_shower_shape_min_tower_E ? towerinfo->get_energy() : 0;
                
                // Only get the raw tower info if we have raw DST available
                if(useRaw)
                {
                    TowerInfo *towerinforaw = towerNodeMap["TOWERS_CEMC"]->get_tower_at_key(towerinfokey);

                    if(! towerinforaw)
                    {
                        if(verbosity > 3)
                        {
                            std::cout << "DirectPhotonAN::process_event: ";
                            std::cout << "No towerinforaw for tower key " << towerinfokey << std::endl;
                            std::cout << "ieta: " << ieta << " iphi " << iphi << std::endl;
                            std::cout << "Continuing..." << std::endl;
                        }
                        continue;
                    }

                    cluster_adc_array[ncluster][arraykey] = towerinforaw->get_energy();
                }

            }
        }
        debugger->clusterCheckpoint12();
        float e11 = E77[3][3];
        float e22 = showershape[8] + showershape[9] + showershape[10] + showershape[11];
        float e13 = 0;
        float e15 = 0;
        float e17 = 0;
        float e31 = 0;
        float e51 = 0;
        float e71 = 0;
        float e33 = 0;
        float e35 = 0;
        float e37 = 0;
        float e53 = 0;
        float e73 = 0;
        float e55 = 0;
        float e57 = 0;
        float e75 = 0;
        float e77 = 0;
        float w32 = 0;
        float e32 = 0;
        float w52 = 0;
        float e52 = 0;
        float w72 = 0;
        float e72 = 0;
        float weta = 0;
        float wphi = 0;
        float weta_cog = 0;
        float wphi_cog = 0;
        float weta_cogx = 0;
        float wphi_cogx = 0;
        float Eetaphi = 0;
        float shift_eta = avg_eta - std::floor(avg_eta) - 0.5;
        float shift_phi = avg_phi - std::floor(avg_phi) - 0.5;
        float cog_eta = 3 + shift_eta;
        float cog_phi = 3 + shift_phi;
        int signphi = (avg_phi - std::floor(avg_phi)) > 0.5 ? 1 : -1;
        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < 7; j++)
            {
            int di = abs(i - 3);
            int dj = abs(j - 3);
            float di_float = i - cog_eta;
            float dj_float = j - cog_phi;
            // safe gard, we already did this while populating the matrix
            if (E77[i][j] < m_shower_shape_min_tower_E)
                E77[i][j] = 0;
            // if owned by the tower
            if (E77_ownership[i][j] == 1)
            {
                weta += E77[i][j] * di * di;
                wphi += E77[i][j] * dj * dj;
                weta_cog += E77[i][j] * di_float * di_float;
                wphi_cog += E77[i][j] * dj_float * dj_float;
                Eetaphi += E77[i][j];
                if (i != 3 || j != 3)
                {
                weta_cogx += E77[i][j] * di_float * di_float;
                wphi_cogx += E77[i][j] * dj_float * dj_float;
                }
            }
            e77 += E77[i][j];
            if (di <= 1 && (dj == 0 || j == (3 + signphi)))
            {
                w32 += E77[i][j] * (i - 3) * (i - 3);
                e32 += E77[i][j];
            }
            if (di <= 2 && (dj == 0 || j == (3 + signphi)))
            {
                w52 += E77[i][j] * (i - 3) * (i - 3);
                e52 += E77[i][j];
            }
            if (di <= 3 && (dj == 0 || j == (3 + signphi)))
            {
                w72 += E77[i][j] * (i - 3) * (i - 3);
                e72 += E77[i][j];
            }
            if (di <= 0 && dj <= 1)
            {
                e13 += E77[i][j];
            }
            if (di <= 0 && dj <= 2)
            {
                e15 += E77[i][j];
            }
            if (di <= 0 && dj <= 3)
            {
                e17 += E77[i][j];
            }
            if (di <= 1 && dj <= 0)
            {
                e31 += E77[i][j];
            }
            if (di <= 2 && dj <= 0)
            {
                e51 += E77[i][j];
            }
            if (di <= 3 && dj <= 0)
            {
                e71 += E77[i][j];
            }
            if (di <= 1 && dj <= 1)
            {
                e33 += E77[i][j];
            }
            if (di <= 1 && dj <= 2)
            {
                e35 += E77[i][j];
            }
            if (di <= 1 && dj <= 3)
            {
                e37 += E77[i][j];
            }
            if (di <= 2 && dj <= 1)
            {
                e53 += E77[i][j];
            }
            if (di <= 3 && dj <= 1)
            {
                e73 += E77[i][j];
            }
            if (di <= 2 && dj <= 2)
            {
                e55 += E77[i][j];
            }
            if (di <= 2 && dj <= 3)
            {
                e57 += E77[i][j];
            }
            if (di <= 3 && dj <= 2)
            {
                e75 += E77[i][j];
            }
            }
        }
        w32 = e32 > 0 ? w32 / e32 : 0;
        w52 = e52 > 0 ? w52 / e52 : 0;
        w72 = e72 > 0 ? w72 / e72 : 0;

        weta = Eetaphi > 0 ? weta / Eetaphi : 0;
        wphi = Eetaphi > 0 ? wphi / Eetaphi : 0;

        weta_cog = Eetaphi > 0 ? weta_cog / Eetaphi : 0;
        wphi_cog = Eetaphi > 0 ? wphi_cog / Eetaphi : 0;

        weta_cogx = Eetaphi > 0 ? weta_cogx / Eetaphi : 0;
        wphi_cogx = Eetaphi > 0 ? wphi_cogx / Eetaphi : 0;

        
        // Calculate the energy behind the cluster
        /* LOOK INTO HCAL STUFF
        std::vector<int> ihcal_tower = find_closest_hcal_tower(eta, phi, geomIH, ihcalTowerContainer, vertexz, true);
        std::vector<int> ohcal_tower = find_closest_hcal_tower(eta, phi, geomOH, ohcalTowerContainer, vertexz, false);

        float ihcal_et = 0;
        float ohcal_et = 0;
        float ihcal_et22 = 0;
        float ohcal_et22 = 0;
        float ihcal_et33 = 0;
        float ohcal_et33 = 0;

        int ihcal_ieta = ihcal_tower[0];
        int ihcal_iphi = ihcal_tower[1];
        float ihcalEt33[3][3] = {0};

        int ohcal_ieta = ohcal_tower[0];
        int ohcal_iphi = ohcal_tower[1];
        float ohcalEt33[3][3] = {0};

        // need to calculate ET from eta, sin(theta) =  sech(eta)
        std::cout << "ihcal_eta: " << ihcal_tower[0] << " ihcal_phi: " << ihcal_tower[1] << std::endl;

        for (int ieta = ihcal_ieta - 1; ieta <= ihcal_ieta + 1; ieta++)
        {
            for (int iphi = ihcal_iphi - 1; iphi <= ihcal_iphi + 1; iphi++)
            {
            int temp_ieta = ieta;
            int temp_iphi = iphi;
            shift_tower_index(ieta, iphi, 24, 64);
            // std::cout<<"ieta: "<<ieta<<" iphi: "<<iphi<<std::endl;
            if (ieta < 0)
            {
                ieta = temp_ieta;
                iphi = temp_iphi;
                continue;
            }
            unsigned int towerinfokey = TowerInfoDefs::encode_hcal(ieta, iphi);
            TowerInfo *towerinfo = towerNodeMap["TOWERINFO_CALIB_HCALIN"]->get_tower_at_key(towerinfokey);
            const RawTowerDefs::keytype key = RawTowerDefs::encode_towerid(RawTowerDefs::CalorimeterId::HCALIN, ieta, iphi);

            ieta = temp_ieta;
            iphi = temp_iphi;
            if (!towerinfo)
            {
                std::cout << "No towerinfo for tower key " << towerinfokey << std::endl;
                std::cout << "ieta: " << ieta << " iphi: " << iphi << std::endl;
                continue;
            }
            if (!(towerinfo->get_isGood()))
                continue;
            RawTowerGeom *tower_geom = geomIH->get_tower_geometry(key);
            if (!tower_geom)
            {
                std::cout << "No tower geometry for tower key " << key << std::endl;
                continue;
            }
            float energy = towerinfo->get_energy();
            float eta = getTowerEta(tower_geom, 0, 0, m_vertex);
            float sintheta = 1 / cosh(eta);
            float Et = energy * sintheta;

            ihcalEt33[ieta - ihcal_ieta + 1][iphi - ihcal_iphi + 1] = Et;
            }
        }
        std::cout << "ohcal_eta: " << ohcal_tower[0] << " ohcal_phi: " << ohcal_tower[1] << std::endl;
        for (int ieta = ohcal_ieta - 1; ieta <= ohcal_ieta + 1; ieta++)
        {
            for (int iphi = ohcal_iphi - 1; iphi <= ohcal_iphi + 1; iphi++)
            {
            int temp_ieta = ieta;
            int temp_iphi = iphi;
            shift_tower_index(ieta, iphi, 24, 64);
            if (ieta < 0)
            {
                ieta = temp_ieta;
                iphi = temp_iphi;
                continue;
            }
            unsigned int towerinfokey = TowerInfoDefs::encode_hcal(ieta, iphi);
            TowerInfo *towerinfo = towerNodeMap["TOWERINFO_CALIB_HCALOUT"]->get_tower_at_key(towerinfokey);
            const RawTowerDefs::keytype key = RawTowerDefs::encode_towerid(RawTowerDefs::CalorimeterId::HCALOUT, ieta, iphi);
            ieta = temp_ieta;
            iphi = temp_iphi;
            if (!towerinfo)
            {
                std::cout << "No towerinfo for tower key " << towerinfokey << std::endl;
                std::cout << "ieta: " << ieta << " iphi: " << iphi << std::endl;
                continue;
            }

            RawTowerGeom *tower_geom = geomOH->get_tower_geometry(key);
            float energy = towerinfo->get_energy();
            float eta = getTowerEta(tower_geom, 0, 0, m_vertex);
            float sintheta = 1 / cosh(eta);
            float Et = energy * sintheta;

            ohcalEt33[ieta - ohcal_ieta + 1][iphi - ohcal_iphi + 1] = Et;
            }
        }
        // std::cout<<"ihcal_et33: "<<ihcalEt33[1][1]<<" ohcal_et33: "<<ohcalEt33[1][1]<<std::endl;
        ihcal_et = ihcalEt33[1][1];
        ohcal_et = ohcalEt33[1][1];

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
            ihcal_et33 += ihcalEt33[i][j];
            ohcal_et33 += ohcalEt33[i][j];
            if (i == 1 || j == 1 + ihcal_tower[2])
            {
                if (j == 1 || i == 1 + ihcal_tower[3])
                {
                ihcal_et22 += ihcalEt33[i][j];
                }
            }
            if (i == 1 || j == 1 + ohcal_tower[2])
            {
                if (j == 1 || i == 1 + ohcal_tower[3])
                {
                ohcal_et22 += ohcalEt33[i][j];
                }
            }
            }
        }*/

        cluster_E[ncluster] = E;
        cluster_Et[ncluster] = ET;
        cluster_Eta[ncluster] = eta;
        cluster_Phi[ncluster] = phi;
        cluster_prob[ncluster] = prob;
        cluster_merged_prob[ncluster] = merged_prob;
        // cluster_CNN_prob[ncluster] = CNNprob;
        cluster_iso_02[ncluster] = clusteriso[0];
        cluster_iso_03[ncluster] = clusteriso[1];
        cluster_iso_04[ncluster] = clusteriso[2];
        cluster_iso_03_emcal[ncluster] = emcalET_03 - ET;
        cluster_iso_03_hcalin[ncluster] = ihcalET_03;
        cluster_iso_03_hcalout[ncluster] = ohcalET_03;
        cluster_iso_03_60_emcal[ncluster] = emcalET_03_60 - ET;
        cluster_iso_03_60_hcalin[ncluster] = ihcalET_03_60;
        cluster_iso_03_60_hcalout[ncluster] = ohcalET_03_60;
        cluster_iso_03_120_emcal[ncluster] = emcalET_03_120 - ET;
        cluster_iso_03_120_hcalin[ncluster] = ihcalET_03_120;
        cluster_iso_03_120_hcalout[ncluster] = ohcalET_03_120;
        cluster_iso_04_emcal[ncluster] = emcalET_04 - ET;
        cluster_iso_04_hcalin[ncluster] = ihcalET_04;
        cluster_iso_04_hcalout[ncluster] = ohcalET_04;
        cluster_e1[ncluster] = showershape[8];
        cluster_e2[ncluster] = showershape[9];
        cluster_e3[ncluster] = showershape[10];
        cluster_e4[ncluster] = showershape[11];
        cluster_ietacent[ncluster] = showershape[4];
        cluster_iphicent[ncluster] = showershape[5];
        cluster_weta[ncluster] = weta;
        cluster_wphi[ncluster] = wphi;
        cluster_weta_cog[ncluster] = weta_cog;
        cluster_wphi_cog[ncluster] = wphi_cog;
        cluster_weta_cogx[ncluster] = weta_cogx;
        cluster_wphi_cogx[ncluster] = wphi_cogx;
        cluster_detamax[ncluster] = detamax;
        cluster_dphimax[ncluster] = dphimax;
        cluster_nsaturated[ncluster] = nsaturated;
        cluster_et1[ncluster] = showershape[0];
        cluster_et2[ncluster] = showershape[1];
        cluster_et3[ncluster] = showershape[2];
        cluster_et4[ncluster] = showershape[3];
        cluster_e11[ncluster] = e11;
        cluster_e22[ncluster] = e22;
        cluster_e13[ncluster] = e13;
        cluster_e15[ncluster] = e15;
        cluster_e17[ncluster] = e17;
        cluster_e31[ncluster] = e31;
        cluster_e51[ncluster] = e51;
        cluster_e71[ncluster] = e71;
        cluster_e33[ncluster] = e33;
        cluster_e35[ncluster] = e35;
        cluster_e37[ncluster] = e37;
        cluster_e53[ncluster] = e53;
        cluster_e73[ncluster] = e73;
        cluster_e55[ncluster] = e55;
        cluster_e57[ncluster] = e57;
        cluster_e75[ncluster] = e75;
        cluster_e77[ncluster] = e77;
        cluster_w32[ncluster] = w32;
        cluster_e32[ncluster] = e32;
        cluster_w52[ncluster] = w52;
        cluster_e52[ncluster] = e52;
        cluster_w72[ncluster] = w72;
        cluster_e72[ncluster] = e72;
        /* Look into HCAL stuff
        cluster_ihcal_et[ncluster] = ihcal_et;
        cluster_ohcal_et[ncluster] = ohcal_et;
        cluster_ihcal_et22[ncluster] = ihcal_et22;
        cluster_ohcal_et22[ncluster] = ohcal_et22;
        cluster_ihcal_et33[ncluster] = ihcal_et33;
        cluster_ohcal_et33[ncluster] = ohcal_et33;
        cluster_ihcal_ieta[ncluster] = ihcal_ieta;
        cluster_ihcal_iphi[ncluster] = ihcal_iphi;
        cluster_ohcal_ieta[ncluster] = ohcal_ieta;
        cluster_ohcal_iphi[ncluster] = ohcal_iphi;
        */  
        ncluster++;
        if (ncluster > nclustermax)
        {
            if(verbosity > 0){
                std::cout << "DirectPhotonAN::process_event: ";
                std::cout << "ncluster exceed the max range: " << ncluster << std::endl;
            }
            return Fun4AllReturnCodes::ABORTEVENT;
        }
    }
    if(verbosity > 3)
    {
                std::cout << "DirectPhotonAN::process_event: ";
                std::cout << "Finished with cluster container: " << clustername << std::endl;
    }



    
    debugger->checkpoint5();
    // Do jets here

    // Only save event if there is at least 1 cluster
    bool saveevent = false;
    if(ncluster > 0){saveevent = true;}
    if(saveevent)
    {
        if(verbosity > 3)
        {
            std::cout << "DirectPhotonAN::process_event: ";
            std::cout << "Filling tree: "<< std::endl;
        }
        tree->Fill();
    }
    else
    {
        if(verbosity > 1)
        {
            std::cout << "DirectPhotonAN::process_event: ";
            std::cout << "No clusters found, skipping event." << std::endl;
        }
    }
    debugger->checkpoint6();
    return Fun4AllReturnCodes::EVENT_OK;


}

//__________________________________________________________..
int DirectPhotonAN::End(PHCompositeNode *topNode)
{
    //DEBUGGING
    if(verbosity > 3)
    {
        float fraction_passed_ET_cut = num_reached_ET_cut > 0 ? num_passed_ET_cut / num_reached_ET_cut  : 0;
        std::cout << "DirectPhotonAN::End: ";
        std::cout << "Fraction of clusters in " << clustername << " that passed ET cut: " << fraction_passed_ET_cut << std::endl;
    
    }
    if(verbosity > 1)
    {
        std::cout << "DirectPhotonAN::End: ";
        std::cout << "Writing output file: " << outputFileName << std::endl;
    }
    fout->cd();
    fout->Write();
    fout->Close();


    Debugger* debugger = Debugger::getInstance();
    debugger->checkpoint7();

    //DEBUGGING
    std::cout << "num_photon_trigger_passed: " << num_photon_trigger_passed << std::endl;
    return Fun4AllReturnCodes::EVENT_OK;
}


void DirectPhotonAN::set_requiredTowerNodes(std::vector<std::string> nodeNames)
{
    for (const auto& currName : nodeNames)
    {
        requiredTowerNodes[currName] = true;
    }
}

float DirectPhotonAN::calculateET(float eta, float phi, float dR, int layer, float min_E) // layer: 0 EMCal, 1 IHCal, 2 OHCal
{
  float ET = 0;
  RawTowerGeomContainer *geomcontainer = nullptr;
  TowerInfoContainerv4 *towercontainer = nullptr;
  // TowerInfoContainer *waveformcontainer = nullptr;
  RawTowerDefs::CalorimeterId caloid = RawTowerDefs::CalorimeterId::CEMC;

  if (layer == 0)
  {
    geomcontainer = geomEM;

    // for debug
    // std::string towerNodeName = "WAVEFORM_CEMC";
    // waveformcontainer = findNode::getClass<TowerInfoContainer>(topNodeptr, towerNodeName);
    towercontainer = towerNodeMap["TOWERINFO_CALIB_CEMC"];

    caloid = RawTowerDefs::CalorimeterId::CEMC;
  }
  else if (layer == 1)
  {
    geomcontainer = geomIH;
    towercontainer = towerNodeMap["TOWERINFO_CALIB_HCALIN"];
    caloid = RawTowerDefs::CalorimeterId::HCALIN;
  }
  else if (layer == 2)
  {
    geomcontainer = geomOH;
    towercontainer = towerNodeMap["TOWERINFO_CALIB_HCALOUT"];
    caloid = RawTowerDefs::CalorimeterId::HCALOUT;
  }
  else
  {
    std::cout << "Invalid layer" << std::endl;
    return ET;
  }
  float ntowers = towercontainer->size();
  for (unsigned int channel = 0; channel < ntowers; channel++)
  {
    TowerInfo *tower = towercontainer->get_tower_at_channel(channel);
    if (!tower)
    {
        if(verbosity > 3)
        {
            std::cout << "DirectPhotonAN::Process_event: no tower found at channel " << channel << "; continuing" << std::endl; 
        }
      continue;
    }
    if (tower->get_isGood() == false)
    {
        if(verbosity > 3)
        {
            std::cout << "DirectPhotonAN::Process_event: tower found at channel " << channel << " is not good; continuing" << std::endl; 
        }
      continue;
    }

    unsigned int towerkey = towercontainer->encode_key(channel);
    int ieta = towercontainer->getTowerEtaBin(towerkey);
    int iphi = towercontainer->getTowerPhiBin(towerkey);
    RawTowerDefs::keytype key = RawTowerDefs::encode_towerid(caloid, ieta, iphi);
    RawTowerGeom *tower_geom = geomcontainer->get_tower_geometry(key);
    double this_phi = tower_geom->get_phi();
    double this_eta = getTowerEta(tower_geom, 0, 0, vertexz);

    if (layer == 0)
    {
      if (tower->get_energy() < -10)
      {
        std::cout << "!!!!!!!!!!!!!!!!energy: " << tower->get_energy() << " chi2: " << tower->get_chi2() << " ieta: " << ieta << " iphi: " << iphi << " eta: " << this_eta << " phi: " << this_phi << std::endl;
        std::cout << "!!!!!!!!!!!!!!!!!is_ZS: " << tower->get_isZS() << " is_good: " << tower->get_isGood() << std::endl;
        /*
        TowerInfo *waveform = waveformcontainer->get_tower_at_channel(channel);
        std::cout<<"!!!!!!!!!!!!!!!! ";
        int nsamples = 16;
        for(int i = 0; i<nsamples; i++){
          std::cout<<waveform->get_waveform_value(i)<<" ";
        }
        std::cout<<std::endl;
        */
      }
    }

    if (deltaR(eta, this_eta, phi, this_phi) < dR)
    {
      if(tower->get_energy() > min_E)
      {
        ET += tower->get_energy() / cosh(this_eta);
      }
    }
  }

  return ET;
}


double DirectPhotonAN::getTowerEta(RawTowerGeom *tower_geom, double vx, double vy, double vz)
{
  float r;
  if (vx == 0 && vy == 0 && vz == 0)
  {
    r = tower_geom->get_eta();
  }
  else
  {
    double radius = sqrt((tower_geom->get_center_x() - vx) * (tower_geom->get_center_x() - vx) + (tower_geom->get_center_y() - vy) * (tower_geom->get_center_y() - vy));
    double theta = atan2(radius, tower_geom->get_center_z() - vz);
    r = -log(tan(theta / 2.));
  }
  return r;
}

void DirectPhotonAN::printSetBits(uint64_t n)
  {
      for (int i = 0; i < 64; i++) {
          if (n & (1ULL << i)) {
              std::cout << i << " ";
          }
      }
      std::cout << std::endl;
  }

    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
    /*                        SPIN INFO                         */
    /*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
/*
int DirectPhotonAN::getSpinInfo()
{
    
    int spinDB_status = 0;
    unsigned int qa_level = 0xffff;
    SpinDBContent spin_cont;
    SpinDBOutput spin_out("phnxrc");

    spin_out.StoreDBContent(runnumber,runnumber,qa_level);
    spin_out.GetDBContentStore(spin_cont,runnumber);

    crossingShift = spin_cont.GetCrossingShift();
    if (crossingShift == -999 && verbosity > 0)
    {
        std::cout << "Warning: found crossing shift = -999 from Spin DB." << std::endl;
        crossingShift = 0;
    }

    // Get beam polarizations
    float bpolerr, ypolerr;
    spin_cont.GetPolarizationBlue(0, polBlue, bpolerr);
    spin_cont.GetPolarizationYellow(0, polYellow, ypolerr);

    for (int i = 0; i < NBUNCHES; i++)
    {
        spinPatternBlue[i] = spin_cont.GetSpinPatternBlue(i);
        spinPatternYellow[i] = spin_cont.GetSpinPatternYellow(i);

        gl1pScalers[i] = spin_cont.GetScalerMbdNoCut(i);

        int bsp = spinPatternBlue[i];
        int ysp = spinPatternYellow[i];
        if (bsp == 1) {lumiUpBlue += gl1pScalers[i];}
        if (bsp == -1) {lumiDownBlue += gl1pScalers[i];}
        if (ysp == 1) {lumiUpYellow += gl1pScalers[i];}
        if (ysp == 1) {lumiDownYellow += gl1pScalers[i];}
    }

    crossingAngle = spin_cont.GetCrossAngle();

    if(crossingAngle < -0.75)
    {
	    crossingAngleIntended = -1.5;
    }
    else if (crossingAngle > 0.75)
    {
        crossingAngleIntended = 1.5;
    }
    else
    {
        crossingAngleIntended = 0;
    }

    if (spinPatternYellow[0] == -999) 
    {
        spinDB_status = 1;
        std::cout << "ERROR spinDB: spinPatternYellow[0] is -999" << std::endl;
    }
    if (lumiUpBlue == 0) 
    {
        spinDB_status = 2;
        std::cout << "ERROR spinDB: lumiUpBlue is 0" << std::endl;
    }
    int badRunFlag = spin_cont.GetBadRunFlag();
    if (badRunFlag) 
    {
        spinDB_status = 3;
        std::cout << "ERROR spinDB: badRunFlag true" << std::endl;
    }
    
    return spinDB_status;
}*/
