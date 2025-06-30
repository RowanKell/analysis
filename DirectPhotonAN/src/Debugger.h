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
        void print_requiredTowerNodes(const std::map<std::string, bool>& requiredTowerNodes);
        void print_clustercontainer(RawClusterContainer* clusterContainer);
        void print_showershape(std::vector<float>& vec);

        void checkpoint1();
        void checkpoint2();
        void checkpoint3();
        void checkpoint4();
        void checkpoint5();
        void checkpoint6();
        void checkpoint7();

        void set_debugging_feature(const std::string& feature, bool status) {feature_map[feature] = status;}

    private:
        static Debugger* instance;

        std::map<std::string, bool> feature_map = 
        {
            {"Towers", false},
            {"Clusters", false},
            {"showershape", false},
            {"IO", false}
        };

        void _print_requiredTowerNodes(const std::map<std::string, bool>& requiredTowerNodes);
        void _print_clustercontainer(RawClusterContainer* clusterContainer);
        void _print_vector(std::vector<float>& vec);

        void _checkpoint(std::string num);

        ClassDef(Debugger, 1) // ROOT class definition for dictionary generation
};

#endif // Debugger_H