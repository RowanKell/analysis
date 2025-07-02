#ifndef Debugger_H
#define Debugger_H

#include <string>
#include <map>
#include <TObject.h>  // Add this line for ClassDef macro

class RawClusterContainer;


class Debugger
{
    public:
        Debugger() = default;
        virtual ~Debugger(){};
        static Debugger* getInstance();

        void print_eventNumber(int eventNum);

        void print_requiredTowerNodes(const std::map<std::string, bool>& requiredTowerNodes);
        void print_clustercontainer(RawClusterContainer* clusterContainer);
        void print_showershape(std::vector<float>& vec);
        void print_ETCut(float ET);

        void checkpointInitRun();
        void checkpoint1();
        void checkpoint2();
        void checkpoint3();
        void checkpoint4();
        void checkpoint5();
        void checkpoint6();
        void checkpoint7();


        void clusterCheckpoint1();
        void clusterCheckpoint2();
        void clusterCheckpoint3();
        void clusterCheckpoint4();
        void clusterCheckpoint5();
        void clusterCheckpoint6();
        void clusterCheckpoint7();
        void clusterCheckpoint8();
        void clusterCheckpoint9();
        void clusterCheckpoint10();
        void clusterCheckpoint11();
        void clusterCheckpoint12();

        void set_debugging_feature(const std::string& feature, bool status) {feature_map[feature] = status;}

    private:
        static Debugger* instance;

        std::map<std::string, bool> feature_map = 
        {
            {"PrintEventNumber", false},
            {"Towers", false},
            {"Clusters", false},
            {"showershape", false},
            {"IO", false},
            {"ETCut", false},
            {"checkpoint1", false},
            {"checkpoint2", false},
            {"checkpoint3", false},
            {"checkpoint4", false},
            {"checkpoint5", false},
            {"checkpoint6", false},
            {"checkpoint7", false},
            {"clusterCheckpoint1", false},
            {"clusterCheckpoint2", false},
            {"clusterCheckpoint3", false},
            {"clusterCheckpoint4", false},
            {"clusterCheckpoint5", false},
            {"clusterCheckpoint6", false},
            {"clusterCheckpoint7", false},
            {"clusterCheckpoint8", false},
            {"clusterCheckpoint9", false},
            {"clusterCheckpoint10", false},
            {"clusterCheckpoint11", false},
            {"clusterCheckpoint12", false},
            {"checkpointInitRun", false}
        };

        void _print_eventNumber(int eventNum);

        void _print_currentModule(const std::string currModule);
        void _print_ETCut(float ET);

        void _print_requiredTowerNodes(const std::map<std::string, bool>& requiredTowerNodes);
        void _print_clustercontainer(RawClusterContainer* clusterContainer);
        void _print_vector(std::vector<float>& vec);

        void _checkpoint(std::string num);

        void _clusterCheckpoint(std::string num);

        ClassDef(Debugger, 1) // ROOT class definition for dictionary generation
};

#endif // Debugger_H