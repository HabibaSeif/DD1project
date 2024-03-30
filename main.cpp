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
#include <queue>
using namespace std;
//!!!PLEASE BEWARE THAT ANY UNCESSARY COUTS ARE FOR TRACING AND FOR THE JUSTIFICATION OF THE CODE WHILE QUESTIONING!!!
//stores the prop delay
unordered_map<string, int> propDelay;
//the ofstream that will output in the 5 simulation files for each circuit
ofstream outputFile;
//that includes both wires and outputs and hold boolean values
unordered_map<string, bool> wireMap;

//start----------openOutputFile-------------
void openOutputFile(string& filename) 
{
    outputFile.open(filename);
    if (!outputFile.is_open()) {
        cerr << "Failed to open stimuli file!" << endl;
        exit(1);
    }
}
//decripiton: takes the name of the file and will open the passed file and output the tracing of the outputs
//end----------openOutputFile-------------

//start----------Gate data structure storage-------------
struct GateInfo 
{
    int numInputs;
    string evaluate;
    int delay;
};
class GateLibrary 
{
private:
    // will store the gate name (first entry) and the gate info (second entry)
    unordered_map<string, GateInfo> gates;

public:
    //this function will be used when we read the library.lib file. Every new line we meet, we will 
    //have to add an gate to the unordered_map so we will use this function
    void addGate(const string& name,int numInputs, string evaluate, int delay) 
    {
        gates[name] = {numInputs, evaluate, delay};
    }

    const GateInfo& getGate(const string& name) const 
    {
        return gates.at(name);
    }

  string getGatexp(const string& name) const 
  {
      return gates.at(name).evaluate;
  }

  int getGatedelay(const string& name) const
  {
      return gates.at(name).delay;
  }
  void reset()
  {
      gates.clear();
  }
// end of getters

    // Check if the library contains a gate with the given name
    // here I used this for a mere reason of tracing to make sure that the data structure of gates was filled from its call
  bool containsGate(const string& name) const 
  {
      return gates.find(name) != gates.end();
  }


};
//description:this is the main data structure that has all the info of the gates in the library.lib file
//end----------Gate data structure storage-------------

//start----------loading function-------------
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
//description:this function will load the library.lib file and will add the gates to the unordered_map
//end----------loading function-------------



//start----------Circuit structure storage-------------
struct compGate 
{
    string gateName;
    vector<string> inputs;
    string output;
};
//description: stores the components of the circuit in the map
//end----------Circuit structure storage-------------

//stores each gate and its components
map<string, compGate> circuitMap;
//general main inputs of the circuits
unordered_map<string, bool> inputs;

//start------------trim--------------------------------
string trim(const string& str) 
{
    auto start = find_if_not(str.begin(), str.end(), [](unsigned char c) { return isspace(c); });
    auto end = find_if_not(str.rbegin(), str.rend(), [](unsigned char c) { return isspace(c); }).base();
    return (start < end ? string(start, end) : "");
}
//description trims white space when parseCircuitFile is used
//end------------trim--------------------------------


//start------------parsing circuit----------------
pair<unordered_map<string, bool>, map<string, compGate>> parseCircuitFile(const string& filename) 
{
    ifstream file(filename);
    bool readInputs = false;

    if (file.is_open()) 
    {
        string line;
        cout << line << endl;
        while (getline(file, line)) 
        {
            if (line == "INPUTS") 
            {
                readInputs = true;
                continue;
            } else if (line == "COMPONENTS") 
            {
                readInputs = false;
                continue;
            }

            if (readInputs) 
            {
                string inputName = trim(line);
                inputs[inputName] = false; // Set initial value to false
            } else 
            {
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
    } 
    else 
    {
        cerr << "Unable to open file: " << filename << endl;
    }

  cout << "Circuit Map Contents:" << endl;
  for (const auto& item : circuitMap) 
  {
      cout << "Gate Name: " << item.second.gateName << endl;
      cout << "  Inputs: ";
      for (const auto& input : item.second.inputs) {
          cout << input << " ";
      }
      cout << endl;
      cout << "  Output: " << item.second.output << endl;
  }
   cout << "Inputs Contents:" << endl;
    for (const auto& input : inputs) 
    {
        cout << "Input Name: " << input.first << ", Value: " << (input.second ? 1 : 0) << endl;
    }
    return make_pair(inputs, circuitMap);
}
// description: reads from the circuit files and parses the information to be placed in the right data structure
//end------------parsing circuit----------------

//start---------------getting values of the inputs vector in comp Gate desc-------------
void getValues(vector<string> inputsofgate,vector<string>& values)
{
  values.resize(0);
  for(int i = 0; i < inputsofgate.size(); i++)
    {
      cout << inputsofgate[i] << endl;
    }

  for (const auto& component : inputsofgate) 
  {
      auto it = inputs.find(component);
      if (it != inputs.end()) {
          std::cout << "Input found in inputs: " << component << " with input value: " << it->second << std::endl;
          values.push_back(std::to_string(it->second));
      }

      auto it2 = wireMap.find(component);
      if (it2 != wireMap.end()) {
          std::cout << "Input found in wireMap: " << component << " with input value: " << it2->second << std::endl;
          values.push_back(std::to_string(it2->second));
      }
  }

}
//end---------------getting values of the inputs vector in comp Gate desc-------------

//start----------structure of storing clock---------------------------------------------
struct Stimulus 
{
    int time;
    string name;
    bool value;
};
//end----------structure of storing clock---------------------------------------------

// start--------Define a comparison function for the priority queue-------------------
struct CompareStimulus 
{
    bool operator()(const Stimulus& lhs, const Stimulus& rhs) const {
        // Change the comparison based on propagation delay
        return lhs.time > rhs.time; // This will create a min heap
    }
};
//end--------Define a comparison function for the priority queue-------------------

priority_queue<Stimulus, vector<Stimulus>, CompareStimulus> stimuli;

//start-----------trimming used when readStimuliFromFile function is called----------------
string trim2(const string& str) 
{
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    if (start == string::npos) {
        return ""; // Empty string if the input string contains only whitespace
    }
    return str.substr(start, end - start + 1);
}
//end-----------trimming used when readStimuliFromFile function is called----------------

//start-----------readStimuliFromFile function----------------
priority_queue<Stimulus, vector<Stimulus>, CompareStimulus> readStimuliFromFile(const string& fileName) {
    priority_queue<Stimulus, vector<Stimulus>, CompareStimulus> stimuli;

    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Unable to open file: " << fileName << endl;
        return stimuli;
    }

    string line;
    while (getline(file, line)) {
        // Trim whitespace from the beginning and end of the line
        line = trim2(line);
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
        // Parse the value as an integer
        stimulus.value = stoi(line.substr(pos + 1)); // Assuming it's 0 or 1
        stimuli.push(stimulus);
    }

    file.close();
    return stimuli;
}
//end-----------readStimuliFromFile function----------------

//start-----------replaceVariables function----------------
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
//description: takes the expression and replaces the variables with bools
//end-----------replaceVariables function----------------

//strat--------------precedence funcition-------------------
int precedence(char op) {
    if (op == '~') return 3; // ~ has highest precedence
    if (op == '&') return 2;
    if (op == '|') return 1;
    return 0; // Default precedence
}
//description//after replacing the expression we use a stack in order to evaluate the expression with the values of the inputs
//end-------------precedence function-------------


//start-------------evaluate function----------------
bool evaluateBooleanExpression(const string& expr) {
    stack<bool> operands;
    stack<char> operators;

    for (char c : expr) {
        if (c == ' ') continue; // Skip whitespace

        if (c == '0' || c == '1') 
        {
            operands.push(c == '1');
        } else if (c == '(') 
        {
            operators.push(c);
        } else if (c == ')') 
        {
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
                }
            }
            if (!operators.empty()) operators.pop(); // Pop '('
        } else {
            while (!operators.empty() && precedence(c) <= precedence(operators.top())) {
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
                }
            }
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
        }
    }

    if (operands.empty()) {
        throw invalid_argument("Invalid expression");
    }

    return operands.top();
}
unordered_map<string, bool> populateWireMap(const map<string, compGate>& circuitMap) 
{
    unordered_map<string, bool> wireMap;

    // Iterate through the gates in the circuit map
    for (const auto& item : circuitMap) {
        // Extract the wire name from the output of each gate
        string wireName = item.second.output;
        // Check if the wire name starts with "w"
        if (wireName.size() > 0 && wireName[0] == 'w') {
            // Add the output wire to the wire map with an initial value of false
            wireMap[wireName] = false;
        }
    }

    return wireMap;
}
//description evaluates boolean expression after replacement
//end -------------evaluate function----------------

//start------------resert-------------------------
    void resetDataStructures() 
{
        propDelay.clear();
        inputs.clear();
        circuitMap.clear();
        wireMap.clear();
}
//description // we call this function in order to clear all the data structures before moving to another circuit file
//end------------- reset -------------------------
priority_queue<Stimulus, vector<Stimulus>, CompareStimulus> cQ2;

//start------------main function-------------------
void trace(string outputfileName, string stimuliname, string circuitname)
{
  GateLibrary gateLibrary;
  vector<string> vec;
  pair<unordered_map<string, bool>, map<string, compGate>> parsedData;
  loadGateLibrary("library.lib", gateLibrary);
  string filename = circuitname;
  string outputfilename = outputfileName;
  openOutputFile(outputfilename);
  stimuli = readStimuliFromFile(stimuliname);
  parsedData = parseCircuitFile(filename);
  map<string, compGate> circuitMap = parsedData.second;
  wireMap = populateWireMap(circuitMap);
  cout << "Wire Map Contents:" << endl;
  for (const auto& item : wireMap) 
  {
      cout << "Wire Name: " << item.first << ", Value: " << (item.second ? 1 : 0) << endl;
  }
  priority_queue<Stimulus, vector<Stimulus>, CompareStimulus> copyQueue = stimuli;
  //map<string, int> propDelay;
  Stimulus s;
  s.time = 0;
  //starting of the simulation
  
  while (!copyQueue.empty()) 
  {
          Stimulus stimulus = copyQueue.top();
          string name = stimulus.name;
          int time = stimulus.time;
          copyQueue.pop();
          cout << "Time: " << stimulus.time << ", Name: " << stimulus.name << ", Value: " << 
          stimulus.value << endl;
          Stimulus stimulus2 = copyQueue.top();
          for (auto& input : inputs) 
          {
            if (" "+input.first == stimulus.name) 
              {
                cout << "Here" << endl;
                auto it = propDelay.find(stimulus.name);
                if (it != propDelay.end())
                {
                  it->second = stimulus.time;
                  cQ2.push(stimulus);
                }
                else
                {
                    propDelay.insert({stimulus.name, stimulus.time});
                    cQ2.push(stimulus);
                }
              }
            }
            for (auto& input : inputs) 
            {
                //cout << input.first <<"------"<< stimulus.name << endl;
                if (" "+input.first == stimulus.name) 
                {
                    if(input.second != stimulus.value)
                    {
                    input.second = stimulus.value; // Update value in inputs map
                    //propDelay.insert({input.first, stimulus.time});
                    }
                    break; // Exit the loop once the value is updated
                }
            }
            cout << time << " " << stimulus.time << endl;
            if(time == stimulus2.time && name != stimulus2.name)
            {
              continue;
            }
        
            for (const auto& entry : circuitMap) 
            {
              // checking if the gate is available in the gate library
              if (gateLibrary.containsGate(entry.second.gateName))
              {
                //cout << "Gate is available in the library" << endl;
                //gateLibrary.printGateTypeDescription(gatenumber);
                getValues(entry.second.inputs,vec);
              }
              else
              {
                cout << "Gate is not available in the library" << endl;
              }
        
              string expression = gateLibrary.getGatexp(entry.second.gateName);
              cout << "checking " << entry.second.gateName << endl;
              cout << vec.size();
              for(int i = 0; i < vec.size(); i++)
                {
                  cout << vec[i] << " ";
                }
              cout << endl;
              cout << "Replacing" << endl;
              cout << expression << endl;
              cout << replaceVariables(expression, vec);
              cout << "Bool after replacing";
              bool a = evaluateBooleanExpression(replaceVariables(expression, vec));
              cout << a << endl;
              s.name = entry.second.output;
              for (auto& item : wireMap) 
              {
                cout << item.first <<  "------"<<  entry.second.output << endl;
                
                 if(item.first == entry.second.output)
                 {
                   cout << item.first << " " << item.second << " " << a << endl;
                    if(item.second != a)
                    {
                      item.second = a;
                      s.value = a;
                      int gateDelay = gateLibrary.getGatedelay(entry.second.gateName);
                      s.time = gateDelay;
                      //cout << stimulus.time << " " << gateDelay << endl;
                      std::cout << "PropDelay entries:" << std::endl;
                      for (const auto& entry : propDelay) 
                      {
                          std::cout << "Gate: " << entry.first << ", Delay: " << entry.second <<std::endl;
                      }
                      cout << "PUSH PUSH " << item.first << endl;
                      int total_delay = 0; // Variable to store the total propagation delay
                      for(int i = 0; i < entry.second.inputs.size(); i++)
                        {
                          cout << entry.second.inputs[i] << endl;
                        }
        
                      string A;
                      // Loop over propDelay using range-based for loop
                      for (const std::string& component : entry.second.inputs) 
                      {
                          // Check if the component exists in propDelay
                        
                        if(component[0] != 'w')
                        {
                        A = " "+component;
                        }
                        else
                        {
                          A = component;
                        }
                        auto it = propDelay.find(A);
                        if (it != propDelay.end()) 
                        {
                              // Add the corresponding propagation delay to the total
                        total_delay += it->second;
                        } else 
                        {
                          std::cerr << "Warning: No corresponding entry found in propDelay for component '" << A << "'" << std::endl;
                        }
                      }
                      s.time += total_delay;
                      cout << "TIME " << s.time << endl;
                      for (auto& wire : wireMap) 
                      {
                        cout << item.first << " "<< wire.first << endl;
                          if (item.first == wire.first) 
                            {
                              cout << "here" << endl;
                              auto it = propDelay.find(item.first);
                              if (it != propDelay.end())
                              {
                                  // Update the value associated with stimulus.name
                                  it->second = s.time;
                              }
                              else
                              {
                                  // Insert a new entry into propDelay
                                  propDelay.insert({item.first, s.time});
                              }
                            }
                      }
                 
                      cQ2.push(s);
                      copyQueue.push(s);
                      std::cout << "PropDelay entries:" << std::endl;
                      for (const auto& entry : propDelay) 
                      {
                        std::cout << "Gate: " << entry.first << ", Delay: " << entry.second << 
                        std::endl;
                      }
                    }
                  }  
                }
              }
      for (const auto& item : wireMap) 
      {
      cout << "Wire Name: " << item.first << ", Value: " << (item.second ? 1 : 0) << 
      endl;
      }
      cout<< propDelay.size();
    
      cout << "Inputs Contents:" << endl;
      for (const auto& input : inputs) 
      {
          cout << "Input Name: " << input.first << ", Value: " << (input.second ? 1 : 0) << endl;
      }
}
  cout << cQ2.size();
  while(!cQ2.empty())
  {
    outputFile << cQ2.top().time << "," << cQ2.top().name << "," << cQ2.top().value << endl;
    cout << cQ2.top().time << "," << cQ2.top().name << "," << cQ2.top().value << endl;
    cQ2.pop();
  }
  resetDataStructures();
  parsedData.first.clear();
  parsedData.second.clear();
  gateLibrary.reset();
  outputFile.close();
  
}
//description the main simululator of the five circuits
//end-------------main function-------------------
int main() 
{
  // run the simulation of the five circuits
  trace("simulation3.sim","stimuli3.stim", "circuit3.circ");
  cout << endl;
   trace("simulation2.sim","stimuli2.stim", "circuit2.circ");
   trace("simulation4.sim","stimuli4.stim", "circuit4.circ");
   trace("simulation5.sim","stimuli5.stim", "circuit5.circ");
   trace("simulation.sim","stimuli.stim", "circuit.circ");
  return 0;
}
