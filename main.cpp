#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

vector<string> parseInput (string input){
    vector<string> tokens;
    stringstream ss(input);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

void execute_command(vector<string> &args) {
    if (args.empty()) return;

    if (args[0] == "exit") {
        cout << "Exiting shell......." << endl;
        exit(0);
    }

    // execute commands
    vector<char*> c_args;
    for (auto &arg : args) c_args.push_back(&arg[0]);
    c_args.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) { 
        //child process
        if (execvp(c_args[0], c_args.data()) == -1) {
            perror("Command execution failed");
        }
        exit(EXIT_FAILURE);
    }else if (pid < 0) {
        perror("Fork failed");
    } else {
        wait(nullptr); // wait for child process to finish
    }
}

int main() {
    string input;
    while(1) {
        cout << "myshell:$ ";
        getline(cin, input);
        vector<string> args = parseInput(input);
        execute_command(args);
        }
}