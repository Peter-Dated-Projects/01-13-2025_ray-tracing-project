#include <iostream>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>

#include <unordered_map>

int main() {

    // map to store child processes
    std::unordered_map<int, pid_t> childProcesses;
    int pipefd[3][2];

    // first stage is batch create 3 children
    for (int i = 0; i < 3; i++) {
        // create 3 children with pipes

        // create a pipe
        if (pipe(pipefd[i]) == -1) {
            std::cerr << "Error: pipe failed" << std::endl;
            return 1;
        }

        // fork a child process
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << " Error: fork failed" << std::endl;
        }

        else if(pid == 0) {
            // child process - was created successfully
            {
                // close read end
                close(pipefd[i][0]);

                // do processes
                std::cout << "Child process " << i << " created" << std::endl;
                sleep(5);
                
                // create message + send it through pipe
                const char* message = ("Hello from child " + std::to_string(i)).c_str();
                write(pipefd[i][1], message, strlen(message) + 1); // +1 for null termin..

                // close write end
                close(pipefd[i][1]);

                // exit child process
                std::cout << "Child process " << i << " finished" << std::endl;
                exit(0);
            }
        } else {
            // parent process -- store it in the map
            childProcesses[i] = pid;
        }
    }


    // wait for children to all finish
    for (auto& entry : childProcesses) {
        int cId = entry.first;
        pid_t cPid = entry.second;
        int status;
        waitpid(cPid, &status, 0);
                std::cout << "Child ID " << cId
                  << " with PID " << cPid 
                  << " has finished.\n";
    }


    // output all messages from children
    for (int i = 0; i < 3; i++) {
        // close write end
        close(pipefd[i][1]);

        // read from pipe
        char buffer[100];
        read(pipefd[i][0], buffer, 100);
        std::cout << "Message from child " << i << ": " << buffer << std::endl;

        // close read end
        close(pipefd[i][0]);
    }
    
    return 0;
}