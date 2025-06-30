#ifndef MACRO_ANALYZE_C
#define MACRO_ANALYZE_C


#include <directphotonan/DirectPhotonAN.h>
#include <directphotonan/Debugger.h>

#include <Calo_Calib.C> // Need both DST and DST raw for this

#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllUtils.h>


#include <phool/recoConsts.h>
#include <phool/PHCompositeNode.h>
#include <ffaobjects/RunHeader.h>


R__LOAD_LIBRARY(libfun4all.so);
R__LOAD_LIBRARY(libDirectPhotonAN.so);

void Analyze(){
  Fun4AllServer* se = Fun4AllServer::instance();

  /*
Calib file:
  TOP (PHCompositeNode)/
    DST (PHCompositeNode)/
        MBD (PHCompositeNode)/
          MbdOut (IO,MbdOutV2)
          MbdPmtContainer (IO,MbdPmtContainerV1)
        GLOBAL (PHCompositeNode)/
          MbdVertexMap (IO,MbdVertexMapv1)
          GlobalVertexMap (IO,GlobalVertexMapv1)
        GL1 (PHCompositeNode)/
          GL1Packet (IO,Gl1Packetv2)
        ZDC (PHCompositeNode)/
          Zdcinfo (IO,Zdcinfov2)
          TOWERINFO_CALIB_ZDC (IO,TowerInfoContainerv4)
        CEMC (PHCompositeNode)/
          TOWERINFO_CALIB_CEMC (IO,TowerInfoContainerv4)
          CLUSTERINFO_CEMC (IO,RawClusterContainer)
        Sync (IO,SyncObjectv1)
        EventHeader (IO,EventHeaderv1)
        HCALIN (PHCompositeNode)/
          TOWERINFO_CALIB_HCALIN (IO,TowerInfoContainerv4)
        HCALOUT (PHCompositeNode)/
          TOWERINFO_CALIB_HCALOUT (IO,TowerInfoContainerv4)
    RUN (PHCompositeNode)/
        MBD (PHCompositeNode)/
          MbdGeom (IO,MbdGeomV1)
        CEMC (PHCompositeNode)/
          TOWERGEOM_CEMC (IO,RawTowerGeomContainer_Cylinderv1)
        HCALIN (PHCompositeNode)/
          TOWERGEOM_HCALIN (IO,RawTowerGeomContainer_Cylinderv1)
        HCALOUT (PHCompositeNode)/
          TOWERGEOM_HCALOUT (IO,RawTowerGeomContainer_Cylinderv1)
        TriggerRunInfo (IO,TriggerRunInfov1)
        RunHeader (IO,RunHeaderv1)
        Flags (IO,FlagSavev1)
        CdbUrl (IO,CdbUrlSavev1)
    PAR (PHCompositeNode)/

  */

  std::string fnameCalib = "/home/rowan/sPHENIX/data/DST_CALO_run2pp_ana450_2024p009-00048746-00000.root";
  std::string fnameRaw = ""; // NEED A RAW DST

  // NEED RAW DST
  // Fun4AllInputManager *inRaw = new Fun4AllDstInputManager("DSTRaw");
  // inRaw->AddFile(fnameRaw);
  // se->registerInputManager(inRaw);

  Fun4AllInputManager *inCalib = new Fun4AllDstInputManager("DSTCalib");
  inCalib->AddFile(fnameCalib);
  se->registerInputManager(inCalib);

  // Process_Calo_Calib();

  // Filename parameter for CaloAna24 is output file
  DirectPhotonAN* an = new DirectPhotonAN("DirectPhotonAN","/home/rowan/sPHENIX/analysis/DirectPhotonAN/output/AnalyzedData.root");

  an->set_verbosity(0);
  an->set_useOnnx(false);
  an->set_useRaw(false);

  std::vector<string> reqNodes = {"TOWERINFO_CALIB_CEMC","TOWERINFO_CALIB_HCALIN","TOWERINFO_CALIB_HCALOUT"};
  an->set_requiredTowerNodes(reqNodes);

  std::pair<int,int> seg = Fun4AllUtils::GetRunSegment(fnameCalib);
  int runnumber = seg.first;

  recoConsts *rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER",runnumber);

  //Debugging
  Debugger *debugger = Debugger::getInstance();
  debugger->set_debugging_feature("Towers", false);
  debugger->set_debugging_feature("showershape", false);
  debugger->set_debugging_feature("Clusters", false);
  debugger->set_debugging_feature("IO", false);

  debugger->set_debugging_feature("checkpoint1", true);
  debugger->set_debugging_feature("checkpoint2", true);
  debugger->set_debugging_feature("checkpoint3", true);
  debugger->set_debugging_feature("checkpoint4", true);
  debugger->set_debugging_feature("checkpoint5", true);
  debugger->set_debugging_feature("checkpoint6", true);
  debugger->set_debugging_feature("checkpoint7", true);

  se->registerSubsystem(an);
  se->run(20);
  se->End();
  delete se;
  gSystem->Exit(0);

  
}
#endif /*MACRO_ANALYZE_C*/