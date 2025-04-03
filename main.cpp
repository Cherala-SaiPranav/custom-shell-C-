#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>

using namespace std;

vector<string> split(const string &line, char delimiter) {
    vector<string> tokens;
    stringstream ss(line);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void execute_command(vector<string> args) {
    vector<char*> c_args;
    for (auto &arg : args) {
        c_args.push_back(&arg[0]);
    }
    c_args.push_back(nullptr);
    execvp(c_args[0], c_args.data());
    perror("execvp");
    exit(1);
}

void execute_pipeline(vector<string> commands) {
    int prev_pipe[2] = {-1, -1};
    for (size_t i = 0; i < commands.size(); ++i) {
        int new_pipe[2];
        if (i < commands.size() - 1) pipe(new_pipe);
        
        pid_t pid = fork();
        if (pid == 0) {
            if (i > 0) {
                dup2(prev_pipe[0], STDIN_FILENO);
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
            if (i < commands.size() - 1) {
                dup2(new_pipe[1], STDOUT_FILENO);
                close(new_pipe[0]);
                close(new_pipe[1]);
            }
            vector<string> args = split(commands[i], ' ');
            execute_command(args);
        }
        if (i > 0) {
            close(prev_pipe[0]);
            close(prev_pipe[1]);
        }
        prev_pipe[0] = new_pipe[0];
        prev_pipe[1] = new_pipe[1];
    }
    while (wait(nullptr) > 0);
}

void execute_script(const string &filename) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        vector<string> commands = split(line, '|');
        execute_pipeline(commands);
    }
}

void change_directory(const string &input) {
    string path = input;
    
    // Remove leading/trailing spaces
    size_t first = path.find_first_not_of(' ');
    size_t last = path.find_last_not_of(' ');
    if (first != string::npos && last != string::npos) {
        path = path.substr(first, last - first + 1);
    }

    // Support quoted paths
    if (!path.empty() && path.front() == '"' && path.back() == '"') {
        path = path.substr(1, path.size() - 2);
    }

    if (chdir(path.c_str()) != 0) {
        perror("chdir");
    }
}

int main() {
    string input;
    while (true) {
        cout << "myshell> ";
        getline(cin, input);

        if (input == "exit") break;

        if (input.find("cd ", 0) == 0) {  // Handle cd command
            change_directory(input.substr(3));
            continue;
        }

        if (input.find("source ", 0) == 0) {
            execute_script(input.substr(7));
        } else {
            vector<string> commands = split(input, '|');
            execute_pipeline(commands);
        }
    }
    return 0;
}
