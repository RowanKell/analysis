// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef DirectPhotonAN_H
#define DirectPhotonAN_H

#include <fun4all/SubsysReco.h>

#include <TFile.h>
#include <TH3.h>
#include <TH2.h>
#include <TH1.h>
#include <TProfile2D.h>
#include <TTree.h>
#include <string>
#include <TLorentzVector.h>

#include <phool/onnxlib.h>

#include <iostream>

class PHCompositeNode;
class CaloEvalStack;
class CaloRawClusterEval;
class CaloTruthEval;
class RawTowerGeom;
class RawTowerGeomContainer;
class TowerInfoContainerv4;
class PHG4TruthInfoContainer;
class CaloEvalStack;


class DirectPhotonAN : public SubsysReco
{
public:
  DirectPhotonAN(const std::string &name = "DirectPhotonAN", const std::string &filename = "default.root");

  ~DirectPhotonAN() override;

  int Init(PHCompositeNode *topNode) override;

  int InitRun(PHCompositeNode *topNode) override;

  /** Called for each event.
      This is where you do the real work.
   */
  int process_event(PHCompositeNode *topNode) override;

  /// Called at the end of all processing.
  int End(PHCompositeNode *topNode) override;



  //Setters
  void set_clusterpTmin(float pTmin) { clusterpTmin = pTmin; }

  void set_particlepTmin(float pTmin) { particlepTmin = pTmin; }

  void set_verbosity(int verb) { verbosity = verb; }

  void set_requiredTowerNodes(std::vector<std::string> nodes);

  void set_requireVertexMap(bool req) {require_vertexmap = req;}

  void set_useOnnx(bool uo) {useOnnx = uo;}

  // Need raw file to use raw... need sdcc account first
  void set_useRaw(bool ur) {useRaw = ur;}


  //Declare and define some helper functions
  double getTowerEta(RawTowerGeom *tower_geom, double vx, double vy, double vz);
  inline float deltaR(float eta1, float eta2, float phi1, float phi2)
  {
    float deta = eta1 - eta2;
    float dphi = phi1 - phi2;
    if (dphi > M_PI)
      dphi -= 2 * M_PI;
    if (dphi < -1 * M_PI)
      dphi += 2 * M_PI;
    return sqrt(deta * deta + dphi * dphi);
  }
  void printSetBits(uint64_t n);

  template<typename K, typename V>
  void printMap(const std::map<K, V>& m) 
  {
      for (const auto& [key, value] : m) 
      {
          std::cout << key << ": " << value << "\n";
      }
  }

private:

  bool useRaw = true;

  // IO
  TFile *fout;
  TTree *tree;
  TH1F *E_histo{nullptr};
  std::string outputFileName{"default.root"};

  // Numbers
  int runnumber{0};
  int eventnumber{0};

  // Trigger stuff
//   std::vector<int> using_trigger_bits{24, 25, 26, 27, 36, 37, 38};

// https://github.com/sPHENIX-Collaboration/coresoftware/blob/3a263b3f60bf23c54ab89b09814bc660e2556bb5/offline/packages/trigger/TriggerRunInfoReco.cc#L101
// Photon 2: 28
// Photon 3: 29
// Photon 4: 30
// Photon 5: 31
  bool livetrigger[64] = {false};
  bool scaledtrigger[64] = {false};
  int nlivetrigger[64] = {0};
  int nscaledtrigger[64] = {0};
  float trigger_prescale[64] = {-1};

  // Vertex stuff
  float vertexz{-9999};
  float vertexz_cut{200.0};

  bool require_vertexmap{true};

  // Tower stuff
  RawTowerGeomContainer *geomEM{nullptr};
  RawTowerGeomContainer *geomIH{nullptr};
  RawTowerGeomContainer *geomOH{nullptr};

  std::map<std::string, bool> requiredTowerNodes = {
    {"TOWERS_CEMC", false}, // Need uncalibrated DST
    {"TOWERS_HCALOUT", false}, // Need uncalibrated DST
    {"TOWERS_HCALIN", false}, // Need uncalibrated DST
    {"TOWERINFO_CALIB_CEMC", false},
    {"TOWERINFO_CALIB_HCALIN", false},
    {"TOWERINFO_CALIB_HCALOUT", false}
  };

  std::map<std::string, TowerInfoContainerv4*> towerNodeMap;
  TowerInfoContainerv4 *currTowerContainer{nullptr};


  //EMCal towers

  void shift_tower_index(int &ieta, int &iphi, int maxeta, int maxphi)
  {
    if (ieta < 0)
      ieta = -1;
    if (ieta >= maxeta)
      ieta = -1;
    if (iphi < 0)
      iphi += maxphi;
    if (iphi >= maxphi)
      iphi -= maxphi;
  }

  // Clusters
  std::string clustername = "CLUSTERINFO_CEMC";
  // If useRaw false, then the 0th entry will always be empty (no CEMC_NO_SPLIT data)

  // Shower shape stuff
  static const int nclustermax = 10000;
  static const int arrayntower = 49;
  int cluster_e_array_idx[nclustermax][arrayntower] = {0};
  int cluster_ownership_array[nclustermax][arrayntower] = {0};
  int cluster_status_array[nclustermax][arrayntower] = {0};
  float cluster_e_array[nclustermax][arrayntower] = {0};
  float cluster_adc_array[nclustermax][arrayntower] = {0};
  float cluster_time_array[nclustermax][arrayntower] = {0};
  
  float cluster_e1[nclustermax] = {0};
  float cluster_e2[nclustermax] = {0};
  float cluster_e3[nclustermax] = {0};
  float cluster_e4[nclustermax] = {0};
  float cluster_et1[nclustermax] = {0};
  float cluster_et2[nclustermax] = {0};
  float cluster_et3[nclustermax] = {0};
  float cluster_et4[nclustermax] = {0};
  float cluster_ietacent[nclustermax] = {0};
  float cluster_iphicent[nclustermax] = {0};
  float cluster_weta[nclustermax] = {0};
  float cluster_wphi[nclustermax] = {0};
  float cluster_weta_cog[nclustermax] = {0};
  float cluster_wphi_cog[nclustermax] = {0};
  float cluster_weta_cogx[nclustermax] = {0};
  float cluster_wphi_cogx[nclustermax] = {0};
  int cluster_detamax[nclustermax] = {0};
  int cluster_dphimax[nclustermax] = {0};
  int cluster_nsaturated[nclustermax] = {0};

  float cluster_e11[nclustermax] = {0};
  float cluster_e22[nclustermax] = {0};
  float cluster_e13[nclustermax] = {0};
  float cluster_e15[nclustermax] = {0};
  float cluster_e17[nclustermax] = {0};
  float cluster_e31[nclustermax] = {0};
  float cluster_e51[nclustermax] = {0};
  float cluster_e71[nclustermax] = {0};
  float cluster_e33[nclustermax] = {0};
  float cluster_e35[nclustermax] = {0};
  float cluster_e37[nclustermax] = {0};
  float cluster_e53[nclustermax] = {0};
  float cluster_e73[nclustermax] = {0};
  float cluster_e55[nclustermax] = {0};
  float cluster_e57[nclustermax] = {0};
  float cluster_e75[nclustermax] = {0};
  float cluster_e77[nclustermax] = {0};
  float cluster_w32[nclustermax] = {0};
  float cluster_e32[nclustermax] = {0};
  float cluster_w52[nclustermax] = {0};
  float cluster_e52[nclustermax] = {0};
  float cluster_w72[nclustermax] = {0};
  float cluster_e72[nclustermax] = {0};


  int ncluster = 0;
  float cluster_E[nclustermax] = {0};
  float cluster_Et[nclustermax] = {0};
  float cluster_Eta[nclustermax] = {0};
  float cluster_Phi[nclustermax] = {0};
  float cluster_prob[nclustermax] = {0};
  float cluster_merged_prob[nclustermax] = {0};
  float cluster_CNN_prob[nclustermax] = {0};
  float cluster_iso_02[nclustermax] = {0};
  float cluster_iso_03[nclustermax] = {0};
  float cluster_iso_04[nclustermax] = {0};
  float cluster_iso_03_emcal[nclustermax] = {0};
  float cluster_iso_03_hcalin[nclustermax] = {0};
  float cluster_iso_03_hcalout[nclustermax] = {0};
  float cluster_iso_03_60_emcal[nclustermax] = {0};
  float cluster_iso_03_60_hcalin[nclustermax] = {0};
  float cluster_iso_03_60_hcalout[nclustermax] = {0};
  float cluster_iso_03_120_emcal[nclustermax] = {0};
  float cluster_iso_03_120_hcalin[nclustermax] = {0};
  float cluster_iso_03_120_hcalout[nclustermax] = {0};
  float cluster_iso_04_emcal[nclustermax] = {0};
  float cluster_iso_04_hcalin[nclustermax] = {0};
  float cluster_iso_04_hcalout[nclustermax] = {0};


  float calculateET(float eta, float phi, float dR, int layer, float min_E); // layer: 0 EMCal, 1 IHCal, 2 OHCal
  // Classifier

  Ort::Session *onnxmodule{nullptr};
  std::string m_modelPath{""};
  bool useOnnx = false;
  const int inputDimx = 5;
  const int inputDimy = 5;
  const int inputDimz = 1;
  const int outputDim = 1;


  //Number of radii to get isolation energy for
  static const int nRadii = 3;

  // Cluster cut values
  float clusterpTmin{1};

  //Particle cut values
  float particlepTmin{1};

  //ShowerShape cuts
  float m_shower_shape_min_tower_E{0.07};



  /*
  0: bare minimum printouts
  1: print on errors and exits
  2: print on current place in analysis package
  3: print on continues inside loops
  4: print out current submodule in process_event
  */
  int verbosity{2};


  // DEBUGGING TEMP
  int num_reached_ET_cut = 0;
  int num_passed_ET_cut = 0;

  int num_photon_trigger_passed = 0;


};

#endif // DirectPhotonAN_H