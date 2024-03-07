#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    // Open the file
    ifstream inputFile("circuits.circ");

    // Check if the file is opened successfully
    if (!inputFile.is_open()) {
        cout << "Error opening the file." << endl;
        return 1;
    }

    string line;
    // Read and output each line of the file
    while (getline(inputFile, line)) {
        cout << line << endl;
    }

    // Close the file
    inputFile.close();

    return 0;
}
