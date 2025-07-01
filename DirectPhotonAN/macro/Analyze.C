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

  // Working
  // std::string fnameCalib = "DST_CALO_run2pp_ana468_2024p012_v001-00051940-00038.root";
  // std::string fnameRaw = "DST_CALOFITTING_run2pp_ana468_2024p012_v001-00051940-00038.root";

  // From Jaein
  std::string fnameCalib = "/sphenix/lustre01/sphnxpro/production/run2pp/physics/ana468_2024p012_v001/DST_JETCALO/run_00047200_00047300/dst/DST_JETCALO_run2pp_ana468_2024p012_v001-00047289-00000.root";
  std::string fnameRaw = "/sphenix/lustre01/sphnxpro/production/run2pp/physics/ana468_2024p012_v001/DST_JETCALO/run_00047200_00047300/dst/DST_JET_run2pp_ana468_2024p012_v001-00047289-00000.root";

  //Not working
  // std::string fnameCalib = "DST_JET_run2pp_ana468_2024p012-00047480-00298.root";
  // std::string fnameRaw = "DST_JETCALO_run2pp_ana468_2024p012-00047480-00298.root"; // NEED A RAW DST

  //Run info
  const std::string dbtag = "2024p012";
  pair<int, int> runseg = Fun4AllUtils::GetRunSegment(fnameRaw);
  int runnumber = runseg.first;

  // NEED RAW DST
  Fun4AllInputManager *inRaw = new Fun4AllDstInputManager("DSTRaw");
  inRaw->AddFile(fnameRaw);
  se->registerInputManager(inRaw);

  Fun4AllInputManager *inCalib = new Fun4AllDstInputManager("DSTCalib");
  inCalib->AddFile(fnameCalib);
  se->registerInputManager(inCalib);

  recoConsts *rc = recoConsts::instance();
  rc -> set_StringFlag("CDB_GLOBALTAG", dbtag);
  rc -> set_uint64Flag("TIMESTAMP", runnumber);

  // Not working right now
  // Process_Calo_Calib();
  std::cout << "Analyze.C: HERE" << std::endl;

  // Filename parameter for CaloAna24 is output file
  DirectPhotonAN* an = new DirectPhotonAN("DirectPhotonAN","/sphenix/user/rowankel/analysis/DirectPhotonAN/output/AnalyzedData.root");

  an->set_verbosity(1);
  an->set_useOnnx(false);
  an->set_useRaw(true);

  std::vector<string> reqNodes = {
                                  // "TOWERINFO_CALIB_CEMC",
                                  // "TOWERINFO_CALIB_HCALIN",
                                  // "TOWERINFO_CALIB_HCALOUT",
                                  "TOWERS_CEMC",
                                  "TOWERS_HCALOUT",
                                  "TOWERS_HCALIN"};
  an->set_requiredTowerNodes(reqNodes);
  an->set_requireVertexMap(true); //DEBUGGING

  //Debugging
  Debugger *debugger = Debugger::getInstance();
  debugger->set_debugging_feature("Towers", false);
  debugger->set_debugging_feature("showershape", true);
  debugger->set_debugging_feature("Clusters", true);
  debugger->set_debugging_feature("IO", false);

  debugger->set_debugging_feature("ETCut", true);

  debugger->set_debugging_feature("checkpointInitRun", false);

  debugger->set_debugging_feature("checkpoint1", true);
  debugger->set_debugging_feature("checkpoint2", true);
  debugger->set_debugging_feature("checkpoint3", true);
  debugger->set_debugging_feature("checkpoint4", true);
  debugger->set_debugging_feature("checkpoint5", true);
  debugger->set_debugging_feature("checkpoint6", true);
  debugger->set_debugging_feature("checkpoint7", true);

  debugger->set_debugging_feature("clusterCheckpoint1", true);
  debugger->set_debugging_feature("clusterCheckpoint2", true);
  debugger->set_debugging_feature("clusterCheckpoint3", true);
  debugger->set_debugging_feature("clusterCheckpoint4", true);
  debugger->set_debugging_feature("clusterCheckpoint5", true);
  debugger->set_debugging_feature("clusterCheckpoint6", true);
  debugger->set_debugging_feature("clusterCheckpoint7", true);
  debugger->set_debugging_feature("clusterCheckpoint8", true);
  debugger->set_debugging_feature("clusterCheckpoint9", true);
  debugger->set_debugging_feature("clusterCheckpoint10", true);
  debugger->set_debugging_feature("clusterCheckpoint11", true);
  debugger->set_debugging_feature("clusterCheckpoint12", true);

  se->registerSubsystem(an);
  se->run(3);
  se->End();
  delete se;
  gSystem->Exit(0);

  
}
#endif /*MACRO_ANALYZE_C*/