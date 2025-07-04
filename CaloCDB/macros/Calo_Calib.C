#ifndef CALO_CALIB_H
#define CALO_CALIB_H

#include <caloreco/CaloTowerCalib.h>
#include <caloreco/CaloTowerStatus.h>
#include <caloreco/RawClusterBuilderTemplate.h>
#include <caloreco/RawClusterDeadHotMask.h>
#include <caloreco/RawClusterPositionCorrection.h>
#include <fun4all/Fun4AllBase.h>

R__LOAD_LIBRARY(libcalo_reco.so)

void Process_Calo_Calib()
{
  Fun4AllServer *se = Fun4AllServer::instance();

  std::string run = "47892";
  std::string base_path = "/gpfs02/sphenix/user/anarde/calo-cdb/calo-cdb/output/"+run+"_ana437_2024p007";

  //////////////////////////////
  // set statuses on raw towers

  std::cout << "status setters" << std::endl;
  CaloTowerStatus *statusEMC = new CaloTowerStatus("CEMCSTATUS");
  statusEMC->set_detector_type(CaloTowerDefs::CEMC);
  statusEMC->set_time_cut(1);
  statusEMC->set_directURL_time(base_path+"/CEMC_meanTime_ana437_2024p007_"+run+".root");
  statusEMC->set_directURL_chi2(base_path+"/CEMC_hotTowers_fracBadChi2_ana437_2024p007_"+run+".root");
  statusEMC->Verbosity(Fun4AllBase::VERBOSITY_MAX);
  se->registerSubsystem(statusEMC);

  CaloTowerStatus *statusHCalIn = new CaloTowerStatus("HCALINSTATUS");
  statusHCalIn->set_detector_type(CaloTowerDefs::HCALIN);
  statusHCalIn->set_time_cut(2);
  statusHCalIn->set_directURL_time(base_path+"/HCALIN_meanTime_ana437_2024p007_"+run+".root");
  statusHCalIn->set_directURL_chi2(base_path+"/HCALIN_hotTowers_fracBadChi2_ana437_2024p007_"+run+".root");
  statusHCalIn->Verbosity(Fun4AllBase::VERBOSITY_MAX);
  se->registerSubsystem(statusHCalIn);

  CaloTowerStatus *statusHCALOUT = new CaloTowerStatus("HCALOUTSTATUS");
  statusHCALOUT->set_detector_type(CaloTowerDefs::HCALOUT);
  statusHCALOUT->set_time_cut(2);
  statusHCALOUT->set_directURL_time(base_path+"/HCALOUT_meanTime_ana437_2024p007_"+run+".root");
  statusHCALOUT->set_directURL_chi2(base_path+"/HCALOUT_hotTowers_fracBadChi2_ana437_2024p007_"+run+".root");
  statusHCALOUT->Verbosity(Fun4AllBase::VERBOSITY_MAX);
  se->registerSubsystem(statusHCALOUT);

  ////////////////////
  // Calibrate towers
  std::cout << "Calibrating EMCal" << std::endl;
  CaloTowerCalib *calibEMC = new CaloTowerCalib("CEMCCALIB");
  calibEMC->set_detector_type(CaloTowerDefs::CEMC);
  se->registerSubsystem(calibEMC);

  std::cout << "Calibrating OHcal" << std::endl;
  CaloTowerCalib *calibOHCal = new CaloTowerCalib("HCALOUT");
  calibOHCal->set_detector_type(CaloTowerDefs::HCALOUT);
  se->registerSubsystem(calibOHCal);

  std::cout << "Calibrating IHcal" << std::endl;
  CaloTowerCalib *calibIHCal = new CaloTowerCalib("HCALIN");
  calibIHCal->set_detector_type(CaloTowerDefs::HCALIN);
  se->registerSubsystem(calibIHCal);

  std::cout << "Calibrating ZDC" << std::endl;
  CaloTowerCalib *calibZDC = new CaloTowerCalib("ZDC");
  calibZDC->set_detector_type(CaloTowerDefs::ZDC);
  se->registerSubsystem(calibZDC);

  //////////////////
  // Clusters

  std::cout << "Building clusters" << std::endl;
  RawClusterBuilderTemplate *ClusterBuilder = new RawClusterBuilderTemplate("EmcRawClusterBuilderTemplate");
  ClusterBuilder->Detector("CEMC");
  ClusterBuilder->set_threshold_energy(0.030f);  // for when using basic calibration
  std::string emc_prof = getenv("CALIBRATIONROOT");
  emc_prof += "/EmcProfile/CEMCprof_Thresh30MeV.root";
  ClusterBuilder->LoadProfile(emc_prof);
  ClusterBuilder->set_UseTowerInfo(1);  // to use towerinfo objects rather than old RawTower
  // se->registerSubsystem(ClusterBuilder);

  // currently NOT included!
  //std::cout << "Applying Position Dependent Correction" << std::endl;
  //RawClusterPositionCorrection *clusterCorrection = new RawClusterPositionCorrection("CEMC");
  //clusterCorrection->set_UseTowerInfo(1);  // to use towerinfo objects rather than old RawTower
 // se->registerSubsystem(clusterCorrection);

}

#endif
