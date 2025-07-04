// Debugging class that provides methods to print debugging information,
// where the debugging object is a singleton, and the methods can
// be called from anywhere in the codebase.
// The class also has methods to turn on certain debugging modes that can
// be used to control the verbosity of the output. The debugging object
// only prints information if the verbosity is set to a level that
// allows it to do so.

#include "Debugger.h"

#include <iostream>
#include <calobase/RawClusterContainer.h>
#include <calobase/RawCluster.h>


Debugger* Debugger::instance = nullptr;

Debugger* Debugger::getInstance() {
    if (instance == nullptr) {
        instance = new Debugger();
    }
    return instance;
}

// Method to print the required tower nodes and their status
void Debugger::_print_requiredTowerNodes(const std::map<std::string, bool>& requiredTowerNodes)
{
        //Debugging:
        std::cout << "DEBUGGING: requiredTowerNodes: " << std::endl;
        for(auto it = requiredTowerNodes.cbegin(); it != requiredTowerNodes.cend(); ++it)
        {
            std::cout << it->first << " " << it->second << std::endl;
        }
}

void Debugger::print_requiredTowerNodes(const std::map<std::string, bool>& requiredTowerNodes)
{
    if (feature_map["Towers"]) {
        _print_requiredTowerNodes(requiredTowerNodes);
    }
}


void Debugger::_print_clustercontainer(RawClusterContainer* clusterContainer)
{
    if (clusterContainer) 
    {
        RawClusterContainer::ConstRange clusterEnd = clusterContainer->getClusters();
        RawClusterContainer::ConstIterator clusterIter;
        int num_clusters = 0;
        std::cout << "DEBUGGING: Printing clusterContainer:" << std::endl;
        for (clusterIter = clusterEnd.first; clusterIter != clusterEnd.second; ++clusterIter)
        {
            RawCluster *recoCluster = clusterIter->second;
            std::cout << "  Cluster ID: " << recoCluster->get_id()<< ", Energy: " << recoCluster->get_energy() << std::endl;
            num_clusters++;
        }
        std::cout << "DEBUGGING: ClusterContainer contained total of " << num_clusters << " clusters." << std::endl;
    } else {
        std::cout << "DEBUGGING: ClusterContainer is null." << std::endl;
    }
}

void Debugger::print_clustercontainer(RawClusterContainer* clusterContainer)
{
    if (feature_map["Clusters"]) {
        _print_clustercontainer(clusterContainer);
    }
}

void Debugger::_print_vector(std::vector<float>& vec)
{
    for (const auto& val : vec) {
        std::cout << "\t" << val << " ";
    }
    std::cout << std::endl;
}


void Debugger::print_showershape(std::vector<float>& vec)
{
    if (feature_map["showershape"]) {
        std::cout << "DEBUGGING: Printing showershape vector:";
        _print_vector(vec);
    }
}

void Debugger::print_ETCut(float ET)
{
    if (feature_map["ETCut"]) {
        _print_ETCut(ET);
    }
}

void Debugger::_print_ETCut(float ET)
{
    std::cout << "DEBUGGING: Passed ET cut with ET = " << ET <<std::endl;
}

void Debugger::_checkpoint(std::string num)
{
    std::cout << "DEBUGGING: Reached checkpoint #" << num << std::endl;
}

void Debugger::checkpoint1() {if(feature_map["checkpoint1"]){_checkpoint("1");}}
void Debugger::checkpoint2() {if(feature_map["checkpoint2"]){_checkpoint("2");}}
void Debugger::checkpoint3() {if(feature_map["checkpoint3"]){_checkpoint("3");}}
void Debugger::checkpoint4() {if(feature_map["checkpoint4"]){_checkpoint("4");}}
void Debugger::checkpoint5() {if(feature_map["checkpoint5"]){_checkpoint("5");}}
void Debugger::checkpoint6() {if(feature_map["checkpoint6"]){_checkpoint("6");}}
void Debugger::checkpoint7() {if(feature_map["checkpoint7"]){_checkpoint("7");}}

void Debugger::_print_currentModule(const std::string currModule)
{
    std::cout << "DEBUGGING: Starting module \"" << currModule << "\"" << std::endl;
}

void Debugger::checkpointInitRun() {if(feature_map["checkpointInitRun"]){_print_currentModule("InitRun");}}
