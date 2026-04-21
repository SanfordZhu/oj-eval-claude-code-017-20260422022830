#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

// Simple test program
int main() {
    // Test 1: Basic commands
    cout << "Testing basic commands..." << endl;

    // We'll create a simple test that pipes commands to our program
    // For now, just check that the executable exists
    system("ls -la code");

    // Test 2: Try running with simple input
    const char* test_input = "clean\nexit\n";
    FILE* fp = fopen("test_input.txt", "w");
    fputs(test_input, fp);
    fclose(fp);

    cout << "Running test..." << endl;
    system("./code < test_input.txt");

    return 0;
}