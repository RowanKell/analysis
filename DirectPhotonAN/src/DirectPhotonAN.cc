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
#include <calobase/TowerInfoContainer.h>
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
  tree->Branch("trigger_prescale", trigger_prescale, "trigger_prescale[64]/F");
  tree->Branch("scaledtrigger", scaledtrigger, "scaledtrigger[64]/O");
  tree->Branch("livetrigger", livetrigger, "livetrigger[64]/O");
  tree->Branch("trigger_prescale", trigger_prescale, "trigger_prescale[64]/F");
  tree->Branch("vertexz", &vertexz, "vertexz/F");
  for (int i = 0; i < nclustercontainer; i++)
  {
    tree->Branch(Form("ncluster_%s", clusternamelist[i].c_str()), &ncluster[i], Form("ncluster_%s/I", clusternamelist[i].c_str()));
    
    tree->Branch(Form("cluster_e_array_%s", clusternamelist[i].c_str()), cluster_e_array[i], Form("cluster_e_array_%s[ncluster_%s][%d]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str(), arrayntower));
    tree->Branch(Form("cluster_adc_array_%s", clusternamelist[i].c_str()), cluster_adc_array[i], Form("cluster_adc_array_%s[ncluster_%s][%d]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str(), arrayntower));
    tree->Branch(Form("cluster_time_array_%s", clusternamelist[i].c_str()), cluster_time_array[i], Form("cluster_time_array_%s[ncluster_%s][%d]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str(), arrayntower));
    tree->Branch(Form("cluster_e_array_idx_%s", clusternamelist[i].c_str()), cluster_e_array_idx[i], Form("cluster_e_array_idx_%s[ncluster_%s][%d]/I", clusternamelist[i].c_str(), clusternamelist[i].c_str(), arrayntower));
    tree->Branch(Form("cluster_status_array_%s", clusternamelist[i].c_str()), cluster_status_array[i], Form("cluster_status_array_%s[ncluster_%s][%d]/I", clusternamelist[i].c_str(), clusternamelist[i].c_str(), arrayntower));
    tree->Branch(Form("cluster_ownership_array_%s", clusternamelist[i].c_str()), cluster_ownership_array[i], Form("cluster_ownership_array_%s[ncluster_%s][%d]/I", clusternamelist[i].c_str(), clusternamelist[i].c_str(), arrayntower));
    
    tree->Branch(Form("cluster_E_%s", clusternamelist[i].c_str()), cluster_E[i], Form("cluster_E_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_Et_%s", clusternamelist[i].c_str()), cluster_Et[i], Form("cluster_Et_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_Eta_%s", clusternamelist[i].c_str()), cluster_Eta[i], Form("cluster_Eta_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_Phi_%s", clusternamelist[i].c_str()), cluster_Phi[i], Form("cluster_Phi_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_prob_%s", clusternamelist[i].c_str()), cluster_prob[i], Form("cluster_prob_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_merged_prob_%s", clusternamelist[i].c_str()), cluster_merged_prob[i], Form("cluster_merged_prob_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_CNN_prob_%s", clusternamelist[i].c_str()), cluster_CNN_prob[i], Form("cluster_CNN_prob_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_02_%s", clusternamelist[i].c_str()), cluster_iso_02[i], Form("cluster_iso_02_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_%s", clusternamelist[i].c_str()), cluster_iso_03[i], Form("cluster_iso_03_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_04_%s", clusternamelist[i].c_str()), cluster_iso_04[i], Form("cluster_iso_04_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_emcal_%s", clusternamelist[i].c_str()), cluster_iso_03_emcal[i], Form("cluster_iso_03_emcal_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_hcalin_%s", clusternamelist[i].c_str()), cluster_iso_03_hcalin[i], Form("cluster_iso_03_hcalin_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_hcalout_%s", clusternamelist[i].c_str()), cluster_iso_03_hcalout[i], Form("cluster_iso_03_hcalout_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_60_emcal_%s", clusternamelist[i].c_str()), cluster_iso_03_60_emcal[i], Form("cluster_iso_03_60_emcal_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_60_hcalin_%s", clusternamelist[i].c_str()), cluster_iso_03_60_hcalin[i], Form("cluster_iso_03_60_hcalin_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_60_hcalout_%s", clusternamelist[i].c_str()), cluster_iso_03_60_hcalout[i], Form("cluster_iso_03_60_hcalout_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_120_emcal_%s", clusternamelist[i].c_str()), cluster_iso_03_120_emcal[i], Form("cluster_iso_03_120_emcal_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_120_hcalin_%s", clusternamelist[i].c_str()), cluster_iso_03_120_hcalin[i], Form("cluster_iso_03_120_hcalin_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_03_120_hcalout_%s", clusternamelist[i].c_str()), cluster_iso_03_120_hcalout[i], Form("cluster_iso_03_120_hcalout_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_04_emcal_%s", clusternamelist[i].c_str()), cluster_iso_04_emcal[i], Form("cluster_iso_04_emcal_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_04_hcalin_%s", clusternamelist[i].c_str()), cluster_iso_04_hcalin[i], Form("cluster_iso_04_hcalin_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iso_04_hcalout_%s", clusternamelist[i].c_str()), cluster_iso_04_hcalout[i], Form("cluster_iso_04_hcalout_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e1_%s", clusternamelist[i].c_str()), cluster_e1[i], Form("cluster_e1_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e2_%s", clusternamelist[i].c_str()), cluster_e2[i], Form("cluster_e2_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e3_%s", clusternamelist[i].c_str()), cluster_e3[i], Form("cluster_e3_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e4_%s", clusternamelist[i].c_str()), cluster_e4[i], Form("cluster_e4_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_et1_%s", clusternamelist[i].c_str()), cluster_et1[i], Form("cluster_et1_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_et2_%s", clusternamelist[i].c_str()), cluster_et2[i], Form("cluster_et2_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_et3_%s", clusternamelist[i].c_str()), cluster_et3[i], Form("cluster_et3_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_et4_%s", clusternamelist[i].c_str()), cluster_et4[i], Form("cluster_et4_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_weta_%s", clusternamelist[i].c_str()), cluster_weta[i], Form("cluster_weta_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_wphi_%s", clusternamelist[i].c_str()), cluster_wphi[i], Form("cluster_wphi_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_weta_cog_%s", clusternamelist[i].c_str()), cluster_weta_cog[i], Form("cluster_weta_cog_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_wphi_cog_%s", clusternamelist[i].c_str()), cluster_wphi_cog[i], Form("cluster_wphi_cog_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_weta_cogx_%s", clusternamelist[i].c_str()), cluster_weta_cogx[i], Form("cluster_weta_cogx_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_wphi_cogx_%s", clusternamelist[i].c_str()), cluster_wphi_cogx[i], Form("cluster_wphi_cogx_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_ietacent_%s", clusternamelist[i].c_str()), cluster_ietacent[i], Form("cluster_ietacent_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_iphicent_%s", clusternamelist[i].c_str()), cluster_iphicent[i], Form("cluster_iphicent_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_detamax_%s", clusternamelist[i].c_str()), cluster_detamax[i], Form("cluster_detamax_%s[ncluster_%s]/I", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_dphimax_%s", clusternamelist[i].c_str()), cluster_dphimax[i], Form("cluster_dphimax_%s[ncluster_%s]/I", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_nsaturated_%s", clusternamelist[i].c_str()), cluster_nsaturated[i], Form("cluster_nsaturated_%s[ncluster_%s]/I", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    
    tree->Branch(Form("cluster_e11_%s", clusternamelist[i].c_str()), cluster_e11[i], Form("cluster_e11_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e22_%s", clusternamelist[i].c_str()), cluster_e22[i], Form("cluster_e22_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e13_%s", clusternamelist[i].c_str()), cluster_e13[i], Form("cluster_e13_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e15_%s", clusternamelist[i].c_str()), cluster_e15[i], Form("cluster_e15_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e17_%s", clusternamelist[i].c_str()), cluster_e17[i], Form("cluster_e17_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e31_%s", clusternamelist[i].c_str()), cluster_e31[i], Form("cluster_e31_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e51_%s", clusternamelist[i].c_str()), cluster_e51[i], Form("cluster_e51_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e71_%s", clusternamelist[i].c_str()), cluster_e71[i], Form("cluster_e71_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e33_%s", clusternamelist[i].c_str()), cluster_e33[i], Form("cluster_e33_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e35_%s", clusternamelist[i].c_str()), cluster_e35[i], Form("cluster_e35_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e37_%s", clusternamelist[i].c_str()), cluster_e37[i], Form("cluster_e37_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e53_%s", clusternamelist[i].c_str()), cluster_e53[i], Form("cluster_e53_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e73_%s", clusternamelist[i].c_str()), cluster_e73[i], Form("cluster_e73_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e55_%s", clusternamelist[i].c_str()), cluster_e55[i], Form("cluster_e55_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e57_%s", clusternamelist[i].c_str()), cluster_e57[i], Form("cluster_e57_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e75_%s", clusternamelist[i].c_str()), cluster_e75[i], Form("cluster_e75_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e77_%s", clusternamelist[i].c_str()), cluster_e77[i], Form("cluster_e77_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_w32_%s", clusternamelist[i].c_str()), cluster_w32[i], Form("cluster_w32_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e32_%s", clusternamelist[i].c_str()), cluster_e32[i], Form("cluster_e32_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_w52_%s", clusternamelist[i].c_str()), cluster_w52[i], Form("cluster_w52_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e52_%s", clusternamelist[i].c_str()), cluster_e52[i], Form("cluster_e52_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_w72_%s", clusternamelist[i].c_str()), cluster_w72[i], Form("cluster_w72_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));
    tree->Branch(Form("cluster_e72_%s", clusternamelist[i].c_str()), cluster_e72[i], Form("cluster_e72_%s[ncluster_%s]/F", clusternamelist[i].c_str(), clusternamelist[i].c_str()));  
    }

  E_histo = new TH1F("E_histo","E_histo", 100, 0, 100);

  return Fun4AllReturnCodes::EVENT_OK;
}


//____________________________________________________________________________..
int DirectPhotonAN::InitRun(PHCompositeNode *topNode)
{
  runnumber = 0;

  RunHeader *runheader = findNode::getClass<RunHeader>(topNode, "RunHeader");
  if (!runheader)
  {
    if(verbosity > 0){
        std::cout << "DirectPhotonAN::InitRun(PHCompositeNode *topNode) Can't find runheader, reseting node tree" << std::endl;
    }
   return Fun4AllReturnCodes::RESET_NODE_TREE;
  }
  runnumber = runheader->get_RunNumber();
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
        if(verbosity > 0)
        {
            std::cout << PHWHERE << "DirectPhotonAN::process_event: " << "MbdVertexMap" << " node is missing. Skipping event #" << eventnumber << std::endl;
        }
        return Fun4AllReturnCodes::EVENT_OK;
    }
    if(vertexmap->empty()){
        if(verbosity > 0)
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
        currTowerContainer = findNode::getClass<TowerInfoContainer>(topNode, towerNodeName);
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

    for (int i = 0; i < nclustercontainer; i++)
    {
        // Skip CEMC_NO_SPLIT container if not using raw dst
        if(!useRaw && (i == 0)){continue;}
        RawClusterContainer *clusterContainer = 
            findNode::getClass<RawClusterContainer>(topNode, clusternamelist[i]);
        if(!clusterContainer)
        {
            if(verbosity > 0)
            {
                std::cout << PHWHERE << "DirectPhotonAN::process_event Missing cluster node: "
                          << clusternamelist[i] << std::endl;
                std::cout << "Aborting event" << std::endl;
            }
            return Fun4AllReturnCodes::ABORTEVENT;
        }
        ncluster[i] = 0;

        RawClusterContainer::ConstRange clusterEnd = clusterContainer->getClusters();
        RawClusterContainer::ConstIterator clusterIter;
        float maxclusterpT = -1;

        debugger->print_clustercontainer(clusterContainer);

        // Iterate over the reconstructed clusters
        for (clusterIter = clusterEnd.first; clusterIter != clusterEnd.second; ++clusterIter)
        {
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
            num_reached_ET_cut[i]++;
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
            std::cout << "Passed ET cut with ET = " << ET << std::endl;
            num_passed_ET_cut[i]++;

            // Array for storing iso energy for each different radii
            float clusteriso[nRadii];
            for (int i = 0; i < nRadii; i++)
            {
                //CHECK
                clusteriso[i] = recoCluster->get_et_iso(2 + i, false, true);
            }

            //CHECK
            //Calculate various ET values
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

            // Filling the 7x7 matrix
            if(verbosity > 3){ std::cout << "finding shower shapes in 7x7";}
            int maxieta = leadtowerindex.first;
            int maxiphi = leadtowerindex.second;
            
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
                for (int ieta = maxtowerieta - ylength; ieta <= maxtowerieta + ylength; ieta++)
                {
                    for (int iphi = maxtoweriphi - xlength; iphi <= maxtoweriphi + xlength; iphi++)
                    {
                        int mappediphi = iphi;

                        if(mappediphi < 0)
                        {
                            mappediphi += 256;
                        }

                        if(mappediphi > 256)
                        {
                            mappediphi -= 256;
                        }

                        unsigned int towerinfokey = TowerInfoDefs::encode_emcal(ieta, mappediphi);
                        TowerInfo *towerinfo = towerNodeMap["TOWERINFO_CALIB_CEMC"]->get_tower_at_key(towerinfokey);
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
            }

            // Calculating distance between phi/eta and the max values
            int detamax = 0;
            int dphimax = 0;
            int nsaturated = 0;
            std::set<unsigned int> towers_in_cluster;
            
            //Loop over the towers in current cluster
            const RawCluster::TowerMap tower_map = recoCluster->get_towermap();
            for (auto tower_iter : tower_map)
            {
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
                    int temp_ieta = ieta;
                    int temp_iphi = iphi;

                    int deta = ieta - maxieta + 3;
                    int dphi = iphi - maxiphi + 3;

                    int arraykey = deta * 7 + dphi;
                    shift_tower_index(ieta, iphi, 96, 256);
                    cluster_e_array_idx[i][ncluster[i]][arraykey] = TowerInfoDefs::encode_emcal(ieta, iphi);
                    cluster_ownership_array[i][ncluster[i]][arraykey] = 0;
                    cluster_e_array[i][ncluster[i]][arraykey] = 0;
                    cluster_adc_array[i][ncluster[i]][arraykey] = 0;
                    cluster_time_array[i][ncluster[i]][arraykey] = 0;

                    unsigned int towerinfokey = TowerInfoDefs::encode_emcal(ieta, iphi);

                    ieta = temp_ieta;
                    iphi = temp_iphi;

                    // Check if key is in the cluster
                    if(towers_in_cluster.find(towerinfokey) != towers_in_cluster.end())
                    {
                        cluster_ownership_array[i][ncluster[i]][arraykey] = 1;
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
                    cluster_status_array[i][ncluster[i]][arraykey] = (int)towerinfo->get_status();
                    cluster_e_array[i][ncluster[i]][arraykey] = towerinfo->get_energy();
                    cluster_time_array[i][ncluster[i]][arraykey] = towerinfo->get_time_float();

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

                        cluster_adc_array[i][ncluster[i]][arraykey] = towerinforaw->get_energy();
                    }

                }
            }
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

            cluster_E[i][ncluster[i]] = E;
            cluster_Et[i][ncluster[i]] = ET;
            cluster_Eta[i][ncluster[i]] = eta;
            cluster_Phi[i][ncluster[i]] = phi;
            cluster_prob[i][ncluster[i]] = prob;
            cluster_merged_prob[i][ncluster[i]] = merged_prob;
            cluster_CNN_prob[i][ncluster[i]] = CNNprob;
            cluster_iso_02[i][ncluster[i]] = clusteriso[0];
            cluster_iso_03[i][ncluster[i]] = clusteriso[1];
            cluster_iso_04[i][ncluster[i]] = clusteriso[2];
            cluster_iso_03_emcal[i][ncluster[i]] = emcalET_03 - ET;
            cluster_iso_03_hcalin[i][ncluster[i]] = ihcalET_03;
            cluster_iso_03_hcalout[i][ncluster[i]] = ohcalET_03;
            cluster_iso_03_60_emcal[i][ncluster[i]] = emcalET_03_60 - ET;
            cluster_iso_03_60_hcalin[i][ncluster[i]] = ihcalET_03_60;
            cluster_iso_03_60_hcalout[i][ncluster[i]] = ohcalET_03_60;
            cluster_iso_03_120_emcal[i][ncluster[i]] = emcalET_03_120 - ET;
            cluster_iso_03_120_hcalin[i][ncluster[i]] = ihcalET_03_120;
            cluster_iso_03_120_hcalout[i][ncluster[i]] = ohcalET_03_120;
            cluster_iso_04_emcal[i][ncluster[i]] = emcalET_04 - ET;
            cluster_iso_04_hcalin[i][ncluster[i]] = ihcalET_04;
            cluster_iso_04_hcalout[i][ncluster[i]] = ohcalET_04;
            cluster_e1[i][ncluster[i]] = showershape[8];
            cluster_e2[i][ncluster[i]] = showershape[9];
            cluster_e3[i][ncluster[i]] = showershape[10];
            cluster_e4[i][ncluster[i]] = showershape[11];
            cluster_ietacent[i][ncluster[i]] = showershape[4];
            cluster_iphicent[i][ncluster[i]] = showershape[5];
            cluster_weta[i][ncluster[i]] = weta;
            cluster_wphi[i][ncluster[i]] = wphi;
            cluster_weta_cog[i][ncluster[i]] = weta_cog;
            cluster_wphi_cog[i][ncluster[i]] = wphi_cog;
            cluster_weta_cogx[i][ncluster[i]] = weta_cogx;
            cluster_wphi_cogx[i][ncluster[i]] = wphi_cogx;
            cluster_detamax[i][ncluster[i]] = detamax;
            cluster_dphimax[i][ncluster[i]] = dphimax;
            cluster_nsaturated[i][ncluster[i]] = nsaturated;
            cluster_et1[i][ncluster[i]] = showershape[0];
            cluster_et2[i][ncluster[i]] = showershape[1];
            cluster_et3[i][ncluster[i]] = showershape[2];
            cluster_et4[i][ncluster[i]] = showershape[3];
            cluster_e11[i][ncluster[i]] = e11;
            cluster_e22[i][ncluster[i]] = e22;
            cluster_e13[i][ncluster[i]] = e13;
            cluster_e15[i][ncluster[i]] = e15;
            cluster_e17[i][ncluster[i]] = e17;
            cluster_e31[i][ncluster[i]] = e31;
            cluster_e51[i][ncluster[i]] = e51;
            cluster_e71[i][ncluster[i]] = e71;
            cluster_e33[i][ncluster[i]] = e33;
            cluster_e35[i][ncluster[i]] = e35;
            cluster_e37[i][ncluster[i]] = e37;
            cluster_e53[i][ncluster[i]] = e53;
            cluster_e73[i][ncluster[i]] = e73;
            cluster_e55[i][ncluster[i]] = e55;
            cluster_e57[i][ncluster[i]] = e57;
            cluster_e75[i][ncluster[i]] = e75;
            cluster_e77[i][ncluster[i]] = e77;
            cluster_w32[i][ncluster[i]] = w32;
            cluster_e32[i][ncluster[i]] = e32;
            cluster_w52[i][ncluster[i]] = w52;
            cluster_e52[i][ncluster[i]] = e52;
            cluster_w72[i][ncluster[i]] = w72;
            cluster_e72[i][ncluster[i]] = e72;
            /* Look into HCAL stuff
            cluster_ihcal_et[i][ncluster[i]] = ihcal_et;
            cluster_ohcal_et[i][ncluster[i]] = ohcal_et;
            cluster_ihcal_et22[i][ncluster[i]] = ihcal_et22;
            cluster_ohcal_et22[i][ncluster[i]] = ohcal_et22;
            cluster_ihcal_et33[i][ncluster[i]] = ihcal_et33;
            cluster_ohcal_et33[i][ncluster[i]] = ohcal_et33;
            cluster_ihcal_ieta[i][ncluster[i]] = ihcal_ieta;
            cluster_ihcal_iphi[i][ncluster[i]] = ihcal_iphi;
            cluster_ohcal_ieta[i][ncluster[i]] = ohcal_ieta;
            cluster_ohcal_iphi[i][ncluster[i]] = ohcal_iphi;
            */  
            ncluster[i]++;
            if (ncluster[i] > nclustermax)
            {
                if(verbosity > 0){
                    std::cout << "DirectPhotonAN::process_event: ";
                    std::cout << "ncluster exceed the max range: " << ncluster[i] << std::endl;
                }
                return Fun4AllReturnCodes::ABORTEVENT;
            }
        }
        if(verbosity > 3)
        {
                    std::cout << "DirectPhotonAN::process_event: ";
                    std::cout << "Finished with cluster container: " << clusternamelist[i] << std::endl;
        }



    }
    debugger->checkpoint5();
    // Do jets here

    // Only save event if there is at least 1 cluster
    bool saveevent = false;
    for (int i = 0; i < nclustercontainer; i++)
    {
        if(ncluster[i] > 0){saveevent = true;}
    }
    if(saveevent)
    {
        if(verbosity > 3)
        {
            std::cout << "DirectPhotonAN::process_event: ";
            std::cout << "Filling tree: "<< std::endl;
        }
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
        for (int i = 0; i < nclustercontainer; i++)
        {
            float fraction_passed_ET_cut = num_reached_ET_cut[i] > 0 ? num_passed_ET_cut[i] / num_reached_ET_cut[i]  : 0;
            std::cout << "DirectPhotonAN::End: ";
            std::cout << "Fraction of clusters in " << clusternamelist[i] << " that passed ET cut: " << fraction_passed_ET_cut << std::endl;
        }
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
  TowerInfoContainer *towercontainer = nullptr;
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
