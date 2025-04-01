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

    if (args[0] == "cd") {
        if (args.size() < 2) {
            cerr << "cd: missing argument" << endl;
        } else {
            // If the directory has spaces, make sure the full path is passed
            string path = args[1];
            for (size_t i = 2; i < args.size(); ++i) {
                path += " " + args[i]; // Combine the rest of the arguments into one path
            }

            // Change to the specified directory
            if (chdir(path.c_str()) != 0) {
                perror("cd failed");
            }
        }
        return;
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