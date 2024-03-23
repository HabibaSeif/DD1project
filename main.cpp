#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include<map>
#include <algorithm> 
#include <stack>

using namespace std;
// stores the wires
unordered_map<string, bool> wires;
// stores the main inputs of the Circuit
unordered_map<string, bool> inputsOfCircuit;
//stores the propagation delays in each component in the circuit
unordered_map<string, int> propDelay;
//the ofstream that will output in the 5 simulation files for each circuit
ofstream outputFile;

// this function will take the name of the file and will open the passed file and output the tracing of the outputs
void openOutputFile(string& filename) 
{
    outputFile.open(filename);
    if (!outputFile.is_open()) {
        cerr << "Failed to open stimuli file!" << endl;
        exit(1);
    }
}


// I will use this struct as a type in order to store the info of each gate that will be read from the library.lib
struct GateInfo 
{
    int numInputs;
    string evaluate;
    int delay;
};


//Ive made this class for the library.lib gates 
class GateLibrary 
{
private:
    // will store the gate name (first entry) and the gate info (second entry)
    unordered_map<string, GateInfo> gates;

public:
    //this function will be used when we read the library.lib file. Every new line we meet, we will have to add an gate to the unordered_map so we will use this function
    void addGate(const string& name,int numInputs, string evaluate, int delay) {
        gates[name] = {numInputs, evaluate, delay};
    }
//all of these are certain getters from the class that we will need in the future
    const GateInfo& getGate(const string& name) const 
    {
        return gates.at(name);
    }

string getGatexp(const string& name) const {
    return gates.at(name).evaluate;
}

int getGatedelay(const string& name) const
{
    return gates.at(name).delay;
}
// end of getters

    // Check if the library contains a gate with the given name
    // here I used this for a mere reason of tracing to make sure that the data structure of gates was filled from its call
bool containsGate(const string& name) const {
    return gates.find(name) != gates.end();
}


};



// Load gate library from file
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
            //GateType type;
            //cout << expression << endl;
            gateLibrary.addGate(gateName, numInputs, expression, delay);
        }
      
        file.close();
    } else {
        cerr << "Unable to open library file: " << filename << endl;
    }
}



// will use this struct as a type of variable to describe the components in the circuit file
struct compGate 
{
    string gateName;
    vector<string> inputs;
    string output;
};
map<string, compGate> circuitMap;
unordered_map<string, bool> inputs;
// Trim whitespace from the beginning and end of a string
string trim(const string& str) {
    auto start = find_if_not(str.begin(), str.end(), [](unsigned char c) { return isspace(c); });
    auto end = find_if_not(str.rbegin(), str.rend(), [](unsigned char c) { return isspace(c); }).base();
    return (start < end ? string(start, end) : "");
}


// reads from the circuit files and parses the information to be placed in the right data structure
pair<unordered_map<string, bool>, map<string, compGate>> parseCircuitFile(const string& filename) {
    ifstream file(filename);
    bool readInputs = false;

    if (file.is_open()) {
        string line;
      cout << line << endl;
        while (getline(file, line)) {
            if (line == "INPUTS") {
                readInputs = true;
                continue;
            } else if (line == "COMPONENTS") {
                readInputs = false;
                continue;
            }

            if (readInputs) {
                string inputName = trim(line);
                inputs[inputName] = false; // Set initial value to false
            } else {
                istringstream iss(line);
                string gateID, output, gateType;
                vector<string> gateInputs;

                getline(iss, gateID, ',');
                getline(iss, gateType, ',');
                getline(iss, output, ',');

                string input;
                while (getline(iss, input, ',')) {
                    gateInputs.push_back(trim(input));
                }

                compGate gate;
                gate.gateName = trim(gateType);
                gate.inputs = gateInputs;
                gate.output = trim(output);

                circuitMap[trim(gateID)] = gate;
              
            }
        }
        file.close();
    } else {
        cerr << "Unable to open file: " << filename << endl;
    }

    return make_pair(inputs, circuitMap);
}

void getValues(vector<string> inputs,vector<string>& values)
{
  values.resize(0);
    for(int i = 0; i < inputs.size(); i++)
      {
        if(inputsOfCircuit.find(inputs[i]) != inputsOfCircuit.end())
        {
values.push_back(to_string(inputsOfCircuit.find(inputs[i])->second));
        }
        else if(wires.find(inputs[i]) != wires.end())
        {
values.push_back(to_string(wires.find(inputs[i])->second));
        }
        else
        {
          values.push_back(to_string(0));
        }

      }
}

struct Stimulus 
{
    int time;
    string name;
    bool value;
};

//this function will be used to read the stimuli file and store the information in a vector of stim
// in the copypropfile I will do the following, first I will update input of Circuit bool and I will update the propagation of the values in the propDelay
vector<Stimulus> readStimuliFromFile(const string& fileName) {
    vector<Stimulus> stimuli;
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Unable to open file: " << fileName << endl;
        return stimuli;
    }

    string line;
    while (getline(file, line)) {
        // Skip empty lines
        if (line.empty())
            continue;

        Stimulus stimulus;
        size_t pos = line.find(',');
        if (pos == string::npos) {
            cerr << "Invalid line format: " << line << endl;
            continue;
        }
        stimulus.time = stoi(line.substr(0, pos));
        line = line.substr(pos + 1);
        pos = line.find(',');
        if (pos == string::npos) {
            cerr << "Invalid line format: " << line << endl;
            continue;
        }
        stimulus.name = line.substr(0, pos);
        stimulus.value = (line.substr(pos + 1) == "1");
        stimuli.push_back(stimulus);
    }

    file.close();
    return stimuli;
}


//global vector for the stimuli files since it will be used in 5 different stimuli files to store the content of the stimuli files
vector<Stimulus> Stimuli;

//This function is called before updating propagation delay because it is used to add something that is read from the circuitfile component line and insert it to wires map and thus propagation delay map

void updating(const string& o, bool b) {

    // Check if the key exists in the map
    auto it = wires.find(o);
    if (it != wires.end()) {
        // Key exists, update its value
        it->second = b;
    } else {
        // Key doesn't exist, insert a new key-value pair
        wires.insert({o, b});
        propDelay.insert({o, 0});
    }
}


// takes the expression of the gates and replace i1,i2,i3 with the values of the inputs
string replaceVariables(const string& expr, const vector<string>& values) {
    string result = expr;

    // Iterate through the values vector
    for (size_t i = 0; i < values.size(); ++i) {
        // Find the variable name (e.g., i1, i2) to replace
        string variableName = "i" + to_string(i + 1);

        // Find and replace all occurrences of the variable name with the corresponding value
        size_t pos = result.find(variableName);
        while (pos != string::npos) {
            result.replace(pos, variableName.size(), values[i]);
            pos = result.find(variableName, pos + values[i].size());
        }
    }

    return result;
}
//after replacing the expression we use a stack in order to evaluate the expression with the values of the inputs
bool evaluateBooleanExpression(const string& expr) {
    stack<bool> operands;
    stack<char> operators;

    for (char c : expr) {
        if (c == ' ') continue; // Skip whitespace

        if (c == '0' || c == '1') {
            operands.push(c == '1');
        } else if (c == '(') {
            operators.push(c);
        } else if (c == ')') {
            while (!operators.empty() && operators.top() != '(') {
                char op = operators.top();
                operators.pop();

                if (op == '~') {
                    bool operand = operands.top();
                    operands.pop();
                    operands.push(!operand);
                } else if (op == '&') {
                    bool operand2 = operands.top();
                    operands.pop();
                    bool operand1 = operands.top();
                    operands.pop();
                    operands.push(operand1 && operand2);
                } else if (op == '|') {
                    bool operand2 = operands.top();
                    operands.pop();
                    bool operand1 = operands.top();
                    operands.pop();
                    operands.push(operand1 || operand2);
                } else if (op == '^') {
                    bool operand2 = operands.top();
                    operands.pop();
                    bool operand1 = operands.top();
                    operands.pop();
                    operands.push(operand1 != operand2);
                }
            }
            if (!operators.empty()) operators.pop(); // Pop '('
        } else {
            operators.push(c);
        }
    }

    while (!operators.empty()) {
        char op = operators.top();
        operators.pop();

        if (op == '~') {
            bool operand = operands.top();
            operands.pop();
            operands.push(!operand);
        } else if (op == '&') {
            bool operand2 = operands.top();
            operands.pop();
            bool operand1 = operands.top();
            operands.pop();
            operands.push(operand1 && operand2);
        } else if (op == '|') {
            bool operand2 = operands.top();
            operands.pop();
            bool operand1 = operands.top();
            operands.pop();
            operands.push(operand1 || operand2);
        } else if (op == '^') {
            bool operand2 = operands.top();
            operands.pop();
            bool operand1 = operands.top();
            operands.pop();
            operands.push(operand1 != operand2);
        }
    }

    if (operands.empty()) {
        throw invalid_argument("Invalid expression");
    }

    return operands.top();
}

//here we are storing the propagation delay map from 2 other data structures(for prop delay) 1) the stim file and 2 the inputOfcircuit of map(from the stim file we will see the alligned inputOfcircuit input name and update it with the one in the stim vector)
void copyInputToPropDelay(unordered_map<string, bool>& inputsOfCircuit, unordered_map<string, int>& propDelay, vector<Stimulus>& Stim) {
    if (!Stim.empty()) {
        for (const auto& entry : Stim) {
            outputFile << entry.time << "," << entry.name << "," << entry.value << endl;
            propDelay[entry.name] = entry.time;
        }
    } else {
        cout << "Stim is empty." << endl;
    }

  for (const auto& entry : Stim) {
      auto it = inputsOfCircuit.find(entry.name);
      if (it != inputsOfCircuit.end()) 
      {
          it->second = entry.value;
      }
  }

  
  }
//Here we call this function everytime we enter a new component line in order to update the propagation delay of the inputs or the wires
void update_propagationdelay(int gateDelay, vector<string> inputs, string o)
{
  int delay = gateDelay;
  for (const auto& input : inputs) {
      if (propDelay.find(input) != propDelay.end()) {
          delay += propDelay[input];
      }
  }
  outputFile << delay << "," << o << ",";
  propDelay[o] = delay;
  return;
}

// we call this function in order to clear all the data structures before moving to another circuit file
    void resetDataStructures() 
{
        wires.clear();
        inputsOfCircuit.clear();
        propDelay.clear();
        inputs.clear();
        circuitMap.clear();
        Stimuli.clear();
}

string outputfilename;
void trace()
{
  GateLibrary gateLibrary;
      vector<string> vec;
  pair<unordered_map<string, bool>, map<string, compGate>> parsedData;
      loadGateLibrary("library.lib", gateLibrary);
      //gateLibrary.printGates();
  for(int i = 0; i < 5; i ++)
    {
      string filename;
      if(i == 0)
      {
      filename = "circuit.circ";
        outputfilename = "simulation.sim";
        openOutputFile(outputfilename);
        Stimuli = readStimuliFromFile("stimuli.stim");

      }
      else if(i==1)
      {
      filename = "circuit2.circ";
        outputfilename = "simulation2.sim";
        openOutputFile(outputfilename);
        Stimuli = readStimuliFromFile("stimuli2.stim");

      }
      else if(i==2)
      {
        filename ="circuit5.circ";
        outputfilename = "simulation5.sim";
        openOutputFile(outputfilename);
        Stimuli = readStimuliFromFile("stimuli5.stim");

      }
      else if(i==3)
      {
        filename ="circuit4.circ";
        outputfilename = "simulation4.sim";
        openOutputFile(outputfilename);
        Stimuli = readStimuliFromFile("stimuli4.stim");

      }
      else
      {
        filename = "circuit3.circ";
        outputfilename = "simulation3.sim";
        openOutputFile(outputfilename);
        Stimuli = readStimuliFromFile("stimuli3.stim");

      }
      parsedData.first.clear();  // Reset unordered_map
      parsedData.second.clear();
      parsedData = parseCircuitFile(filename);
      inputsOfCircuit = parsedData.first;
      map<string, compGate> circuitMap = parsedData.second;

      // Printing inputs

      //Printing the contents of the map
    copyInputToPropDelay(inputsOfCircuit, propDelay, Stimuli);
      
      for (const auto& entry : circuitMap) 
      {
        // checking if the gate is available in the gate library
        vec.clear();
        if (gateLibrary.containsGate(entry.second.gateName))
        {
          cout << "Gate is available in the library" << endl;
          //gateLibrary.printGateTypeDescription(gatenumber);
          getValues(entry.second.inputs,vec);
        }
        else
        {
          cout << "Gate is not available in the library" << endl;
        }

           string expression = gateLibrary.getGatexp(entry.second.gateName);


         replaceVariables(expression, vec);

        int a = evaluateBooleanExpression(replaceVariables(expression, vec));


       updating(entry.second.output,evaluateBooleanExpression(replaceVariables(expression, vec)));
   int gateDelay = gateLibrary.getGatedelay(entry.second.gateName);
           update_propagationdelay(gateDelay, entry.second.inputs, entry.second.output);
outputFile << a << endl;
      }
  resetDataStructures();
  parsedData.first.clear();  // Reset unordered_map
  parsedData.second.clear();// Reset map
  outputFile.close();
  
  }
}
int main() 
{
  trace();
  return 0;
}
