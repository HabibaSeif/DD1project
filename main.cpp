#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

using namespace std;
// This code is divded into two parts, first we store the gates with their info in a map, and the second part builds the circuit by adding the gates and wires to it
// A class to store the gates
enum class GateType { AND, OR, NAND, NOT, XOR, NOR };
// struct to store each gate type, input,the evaluation "representation" of the inputs which each gate gets, and the delay time and 
struct GateInfo {
    GateType type;
    int numInputs;
    string evaluate;
    int delay;
};
//this class stores the information of the gate in the steuct in the form of unordered map and it is private beacuse user cannot accesss the stored data of each gate or change it
class GateLibrary {
private:
    unordered_map<string, GateInfo> gates;
//this function is for adding building the circuit and pushing each gate in it which all its information
public:
    void addGate(const string& name, GateType type, int numInputs, string evaluate, int delay) {
        gates[name] = {type, numInputs, evaluate, delay};
    }
// to retrive the inromration of the gate
    const GateInfo& getGate(const string& name) const {
        return gates.at(name);
    }
// checks if the gate with the specified information exists in the data, gives true if found, false otherwise
    bool containsGate(const string& name) const {
        return gates.find(name) != gates.end();
    }
// print all the gates with their information
    void printGates() const {
        cout << "Gate Library:" << endl;
        for (const auto& entry : gates) {
            const GateInfo& gate = entry.second;
            cout << "Gate: " << entry.first << endl;
            cout << "Type: " << static_cast<int>(gate.type) << endl;
            cout << "Num Inputs: " << gate.numInputs << endl;
            cout << "Expression: " << gate.evaluate << endl;
            cout << "Delay: " << gate.delay << endl;
            cout << endl;
        }
    }
};
// this is the second part for building the circuit
// this function opens the .lib file and reads the data of each gate 
void loadGateLibrary(const string& filename, GateLibrary& gateLibrary) {
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string gateName, numInputsStr, expression, delayStr;
            getline(ss, gateName, ',');
            getline(ss, numInputsStr, ',');
            getline(ss, expression, ',');
            getline(ss, delayStr);
            int numInputs = stoi(numInputsStr);
            int delay = stoi(delayStr);
            GateType type;
            if (gateName == "AND") {
                type = GateType::AND;
            } else if (gateName == "OR") {
                type = GateType::OR;
            } else if (gateName == "NAND") {
                type = GateType::NAND;
            } else if (gateName == "NOT") {
                type = GateType::NOT;
            } else if (gateName == "XOR") {
                type = GateType::XOR;
            } else if (gateName == "NOR") {
                type = GateType::NOR;
            } else {
                cerr << "Invalid gate name: " << gateName << endl;
                continue;
            }
            gateLibrary.addGate(gateName, type, numInputs, expression, delay);
        }
        file.close();
    } else {
        cerr << "Unable to open library file: " << filename << endl;
    }
}
// class for building the cirucit with all the inputs and outputs stored in two separate unordered maps, we used "bool" eith the inputs and outputs to represent the digitial signals ture or false
class Circuit {
public:
    unordered_map<string, bool> inputs;
    unordered_map<string, bool> outputs;
    unordered_map<string, string> wires; // Mapping wire names to gate output names
    GateLibrary& gateLibrary;


    Circuit(GateLibrary& lib) : gateLibrary(lib) {} 

    void setInput(const string& inputName, bool value) {   // Assign the values of inputs of each gate
        inputs[inputName] = value;
    }

    bool getOutput(const string& outputName) const {   //retrive the output value of specific gate or wire
        return outputs.at(outputName);
    }
// Update the value of the gate output based on the inputs
    void simulate() {
        for (const auto& entry : wires) {
            string wireName = entry.first;
            string gateName = entry.second;
            const GateInfo& gate = gateLibrary.getGate(gateName); // wire loops over the gates in the .lib file to set the wire to its specific gate

            if (gate.type == GateType::NOT) {
                outputs[wireName] = !inputs[gate.evaluate];
            } else if (gate.type == GateType::AND) { // gets each gate information from library
                bool result = true;                   // AND gate 
                stringstream ss(gate.evaluate);
                string inputName;
                while (getline(ss, inputName, ',')) {
                    result = result && inputs[inputName];
                }
                outputs[wireName] = result;
            } else if (gate.type == GateType::OR) { // OR gate input info
                bool result = false;
                stringstream ss(gate.evaluate);
                string inputName;
                while (getline(ss, inputName, ',')) {
                    result = result || inputs[inputName];
                }
                outputs[wireName] = result;
            } else if (gate.type == GateType::NAND) { //NAND gate input info
                bool result = true;
                stringstream ss(gate.evaluate);
                string inputName;
                while (getline(ss, inputName, ',')) {
                    result = result && inputs[inputName];
                }
                outputs[wireName] = !result;
            } else {
                cerr << "Unsupported gate type." << endl;    // if the wire doesn't match with any gate from .lib file the rogram will give an error messeage
            }
        }
    }

    void connectWire(const string& wireName, const string& outputName) {
        wires[wireName] = outputName;      // connect the wire to a specific output 
    }
};
// Stimulates a circuits from a text file to be in a digital logic circuit format
void loadCircuit(const string& filename, Circuit& circuit) {
    ifstream file(filename); // opens the text file and reads the components of the provided circuit line by line
    if (file.is_open()) {
        string line;
        string section;
        while (getline(file, line)) {
            if (line == "INPUTS" || line == "COMPONENTS") {
                section = line;
                continue;
            }

            stringstream ss(line);
            string gateName, gateType, output, input1, input2; // take each gate name, its inputs and outputs
            getline(ss, gateName, ',');
            getline(ss, gateType, ',');
            getline(ss, output, ',');
            if (gateType == "NOT") {   // the gates are divided into two categories: the NOT gate which takes one input, and any other gate which takes two inputs
                getline(ss, input1);
                circuit.connectWire(output, input1);  // connect the output wire based on the gate and the input
            } else {
                getline(ss, input1, ',');
                getline(ss, input2);
                circuit.connectWire(output, gateName);
                circuit.setInput(input1, false); // Assume inputs are initially false
                circuit.setInput(input2, false);
            }
        }
        file.close();
    } else {
        cerr << "Unable to open circuit file: " << filename << endl; // error messeage if the file is not opened
    }
}

int main() {
    GateLibrary gateLibrary;
    loadGateLibrary("library.lib", gateLibrary);   // call the loadgate function to get the info from the library file

    Circuit circuit(gateLibrary);
    loadCircuit("circuit.circ", circuit);      // load circuit file which contain the test circuit

    // Simulate the circuit
    circuit.simulate();

    // Output the results
    for (const auto& entry : circuit.outputs) {
        cout << "Output " << entry.first << ": " << entry.second << endl;
    }

    return 0;
}
