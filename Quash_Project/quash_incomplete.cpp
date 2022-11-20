#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include "Jobs.h"
#include <vector>

using namespace std;

#define MAX_LIMIT 1024
#define LS_EXEC  "/bin/ls"
#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

void print_out() {
    cout << "An error occurred. You have too many arguments!" << endl;
    cout << "./Quash  --requires no additional argument(s)" << endl;
};

list<string> cmdParser(string cmd) {
    list<string> outcmd;
    size_t npos=0;
    string intcmd;

    if(cmd.front() == '#') {
        outcmd.push_back("");
    }

    for(size_t pos=0;pos<=cmd.size();pos+=intcmd.size()+1){
        
        while(pos<cmd.size() && cmd[pos] == ' ')
            ++pos;
        
        if(cmd=="") {
            outcmd.push_back(intcmd);
            break;
        }

        npos=cmd.find(' ',pos+1);
        if(npos==string::npos)
            intcmd = cmd.substr(pos);
        else
            intcmd = cmd.substr(pos,npos-pos);

        if(intcmd.front() == '#')
            break;
        outcmd.push_back(intcmd);

        while(pos<cmd.size() && cmd[pos] == ' ')
            ++pos;
    }

    /*list<string>::iterator it;
    for (it=outcmd.begin(); it!=outcmd.end(); ++it)
        cout << *it << endl;*/
    return outcmd;
}

void execute_redirections(list<string> cmd) {
    int fd1[2],fd2[2],fd3[2];
    //pipe(fd1);
    //pipe(fd2);
    //pipe(fd3);
    vector<vector<string>> fullcommand;
    vector<int*>pipecommand;
    vector<string> args;
    string op, outFile, inFile;
    bool flag = false;
    for(auto arg = cmd.begin(); arg != cmd.end(); ++arg) {
        
        if(*arg == "|" && arg != cmd.begin() && arg != cmd.end()) {
            // int fd[2];
            // int *pipes = (int *)pipe(fd);
            // pipecommand.push_back(pipes);
            // fullcommand.push_back(args);
            // args.clear();
            // if(flag)
            //     flag = false;
        }
        else if(*arg == "<" && arg != cmd.begin() && arg != cmd.end()) {
            inFile = *(next(arg));
            op = *arg;
            if(fd1[0] = open(inFile.c_str(),O_RDWR) == -1) {
                cerr << strerror(errno) << endl;
            }
            fd1[1] = 0;
            flag = true;
        }

        else if(*arg == ">" && arg != cmd.begin() && arg != cmd.end()) {
            outFile = *(next(arg));
            op = *arg;
            if(fd2[0] = open(outFile.c_str(), O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO) == -1) {
                cerr << strerror(errno) << endl;
            }
            flag = true;
            fd2[1] = 1;
        }

        else if(*arg == ">>" && arg != cmd.begin() && arg != cmd.end()) {
            outFile = *(next(arg));
            op = *arg;
            if(fd3[0] = open(outFile.c_str(), O_RDWR | O_APPEND) == -1) {
                cerr << strerror(errno) << endl;
            } 

            flag = true;
            fd3[1] = 1;
        }
        
        else if(*arg != ">" && *arg != "<" && *arg != ">>" && !flag) {
            args.push_back(*arg);
        }

    }

    /*list<string>::iterator it;
    for (it=args.begin(); it!=args.end(); ++it)
        cout << *it << endl;*/
    if(fork()==0) {

        char *argList[args.size()+1];
        int count =0;
        for(vector<string>::iterator i=args.begin(); i!=args.end(); i++)
        {
            argList[count] = (char *)malloc(sizeof(char)*(*i).length()+1);
            strcpy(argList[count], (*i).c_str());
            count++;
        }
        argList[count] = NULL;
        //cout << "argList: " << *(argList+1) << endl;

        /*for(auto& i : argList) {
            std::cout << "argList: " << i << endl;
        }*/
        if( op == "<") {
            //dup2(fd1[0],fd1[1]);
            freopen(inFile.c_str(),"r",stdin); //Open text file for reading.  The stream is positioned at the beginning of the file.
            if(cmd.front()[0]=='.' && cmd.front()[1]=='/') {
                if(execv(cmd.front().c_str(), argList) == -1)
                    cerr << strerror(errno) << endl;
                }
            else{
                if(execvp(cmd.front().c_str(), argList) == -1)
                    cerr << strerror(errno) << endl;
            }
            exit(1);
        }
        else if (op == ">") {
            // dup2(fd2[0],fd2[1]);
            freopen(outFile.c_str(),"w",stdout); //Truncate file to zero length or create text file for writing.  The stream is positioned at the beginning of the file.
            if(cmd.front()[0]=='.' && cmd.front()[1]=='/') {
                if(execv(cmd.front().c_str(), argList) == -1)
                    cerr << strerror(errno) << endl;
                }
            else{
                if(execvp(cmd.front().c_str(), argList) == -1)
                    cerr << strerror(errno) << endl;
            }
            /*ofstream WriteFile(outFile);
            ifstream ReadFile(*(argList+1));
            string line;
            while(getline(ReadFile, line)) {
                WriteFile << line << endl;
            }*/
            exit(1);
        }

        else if (op == ">>") {
            // dup2(fd3[0],fd3[1]);

            freopen(outFile.c_str(),"a",stdout); //Open for appending (writing at end of file).  The file is created if it does not exist.  The stream is positioned at the end of the file.
            if(cmd.front()[0]=='.' && cmd.front()[1]=='/') {
                if(execv(cmd.front().c_str(), argList) == -1)
                    cerr << strerror(errno) << endl;
                }
            else{
                if(execvp(cmd.front().c_str(), argList) == -1)
                    cerr << strerror(errno) << endl;
            }
            
            /*ofstream WriteFile;
            WriteFile.open(outFile, ios::out | ios::app);
            ifstream ReadFile(*(argList+1));
            string line;
            while(getline(ReadFile, line)) {
                WriteFile << line << endl;
            }*/
            exit(0);
        }
        free(argList[count]);
        
    }
    else {
        
        wait(NULL);
        
    }
}

void execute_commands(list<string> cmd) {

    if(cmd.size()==1 && cmd.front() =="exit" || cmd.size()==1 && cmd.front() =="quit") {
            cout << endl << "Exiting the shell . . ." << endl;
            fflush(stdout);
            exit(0);
    }

    else if(cmd.front()=="cd" && cmd.size()<=2) {
        string env, path="HOME";
        char buf[MAX_LIMIT];
        if(cmd.size()==2) {
            path = *(++cmd.begin());
            //interprete "$" as an environment variable
            if(path.front()=='$'){
                if (path.find('/')==string::npos) {
                    path.replace(0,1,"");
                    path = getenv(path.c_str());
                }
                else {
                    int n = path.find('/',0);
                    string subword = path.substr(n);
                    path = path.substr(0,n);
                    path.replace(0,1,"");
                    path = getenv(path.c_str());
                    //path = strcat(temp,subword.c_str());
                    path.append(subword.c_str());
                }
            }
            if(chdir(path.c_str())!=-1){
                //update the PWD environment variable
                env = getcwd(buf,MAX_LIMIT);

                setenv("PWD",env.c_str(),1);
            }
            else
                cerr << strerror(errno) << endl;
        }
        else {
            if(chdir(getenv(path.c_str()))!=-1) {
                //update the PWD environment variable
                env = getcwd(buf,MAX_LIMIT);

                setenv("PWD",env.c_str(),1);
            }
            else
                cerr << strerror(errno) << endl;
        }
    }

    else if(cmd.front() == "pwd" && cmd.size()==1) {
        char *working_dir=get_current_dir_name();
        cout << working_dir << endl;
        free(working_dir);
        //cout << getenv("PWD");
    }

    else if(cmd.front() == "printenv") {
        if(cmd.size() == 1){
            //cout << "Usage: printenv %&  eg. PWD, HOME, PATH etc . . ." << endl;
        }
        else
            cout << getenv((*(++cmd.begin())).c_str()) << endl;
    }

    else if(cmd.front() == "ls") {

        std::list<string> args;
        bool redirections = false;
        for(auto& arg : cmd) {
            args.push_back(arg);
            if(arg == "<" || arg == ">" || arg == ">>") {
                redirections = true;
            }
        }

        if(redirections) {
            execute_redirections(cmd);
            return;
        }

        if(fork()==0) {
            if(cmd.size()==1)
                execl(LS_EXEC, "ls", (char*)NULL);
            else if (cmd.size() == 2) {
                //cout << *(++cmd_list.begin()) << endl << *(++(++cmd_list.begin())) << endl;
                execl(LS_EXEC, "ls", (*(++cmd.begin())).c_str(), (char*)NULL);
            }
            else if (cmd.size()==3)
                execl(LS_EXEC, "ls", (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(),  (char*)NULL);
            exit(0);
        }
        else {
            wait(NULL);
        }
    }

    else if(cmd.front() == "grep") {

        std::list<string> args;
        bool redirections = false;
        for(auto& arg : cmd) {
            args.push_back(arg);
            if(arg == "<" || arg == ">" || arg == ">>") {
                redirections = true;
            }
        }

        if(redirections) {
            execute_redirections(cmd);
            return;
        }

        if(fork()==0) {
            if(cmd.size()==1)
                execl(GREP_EXEC, GREP_EXEC, (char*)NULL);
            else if (cmd.size() == 2) {
                //cout << *(++cmd_list.begin()) << endl << *(++(++cmd_list.begin())) << endl;
                execl(GREP_EXEC, GREP_EXEC, (*(++cmd.begin())).c_str(), (char*)NULL);
            }
            else if (cmd.size()==3)
                execl(GREP_EXEC, GREP_EXEC, (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(),  (char*)NULL);
            else if (cmd.size()==4)
                    execl(GREP_EXEC, GREP_EXEC, (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(), (*(++(++(++cmd.begin())))).c_str(), (char*)NULL);
            exit(0);
        }
        else {
            wait(NULL);
        }
    }

    else if(cmd.front() == "find") {

        std::list<string> args;
        bool redirections = false;
        for(auto& arg : cmd) {
            args.push_back(arg);
            if(arg == "<" || arg == ">" || arg == ">>") {
                redirections = true;
            }
        }

        if(redirections) {
            execute_redirections(cmd);
            return;
        }

        if(fork()==0) {
            if(cmd.size()==1)
                    execl(FIND_EXEC, FIND_EXEC, (char*)NULL);
                else if (cmd.size() == 2) {
                    //cout << *(++cmd_list.begin()) << endl << *(++(++cmd_list.begin())) << endl;
                    execl(FIND_EXEC, FIND_EXEC, (*(++cmd.begin())).c_str(), (char*)NULL);
                }
                else if (cmd.size()==3)
                    execl(FIND_EXEC, FIND_EXEC, (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(),  (char*)NULL);
                
                else if (cmd.size()==4)
                    execl(FIND_EXEC, FIND_EXEC, (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(), (*(++(++(++cmd.begin())))).c_str(), (char*)NULL);
                
                exit(0);
            }
        else {
            wait(NULL);
        }
    }

    /*else if (cmd.front() == "cat") {
        if (cmd.size() == 2) {
            ifstream Readfile(cmd.back());
            string line;
            while(getline(Readfile, line)) {
                cout << line << endl;
            }
        }
        else if (cmd.size() == 1) {
        }
        std::list<string> args;
        bool redirections = false;
        for(auto& arg : cmd) {
            args.push_back(arg);
            if(arg == "<" || arg == ">" || arg == ">>") {
                redirections = true;
            }
        }

        if(redirections) {
            execute_redirections(cmd);
            return;
        }
    }*/
    else if(cmd.front() == "echo") {
        list<string>::iterator it;
        size_t npos = 0, pos=0;
        string word, subword;
        int n=0;
        char* env;
        char buf[MAX_LIMIT];

        if(cmd.size()==1){
        }

        else {

            for (auto& it : cmd )  {
                  //cout << "content: " << it << endl;
                while((it).find("\'",pos) != string::npos) {
                    n = (it).find("\'");
                    (it).replace(n,1,"");
                }
                while((it).find("\"",pos) != string::npos) {
                    n = (it).find("\"");
                    (it).replace(n,1,"");
                }
            }
            for(it=++(cmd.begin()); it != cmd.end(); ++it) {
                npos = (*it).find("$",pos);
                if(npos != string::npos) {
                    word = (*it).substr(pos+1);
                    /*if(word == "PWD") {
                        //env = get_current_dir_name();
                        env = getcwd(buf,MAX_LIMIT);
                        //cout << env;
                        //fflush(stdout);
                        //free(env);
                    }
                    else if (word.find("PWD",pos) != string::npos && word.find("/",pos) != string::npos) {
                        npos = word.find("/",pos);
                        //env = get_current_dir_name();
                        env = getcwd(buf,MAX_LIMIT);
                        env = strcat(env,word.substr(npos).c_str());
                        //cout << env;
                        //fflush(stdout);
                        //free(env);
                    }
                    if (word.find("PWD",pos) == string::npos && word.find("/",pos) != string::npos){*/
                    if (word.find("/",pos) != string::npos){
                        npos = word.find("/",pos);
                        subword = word.substr(pos,npos);
                        env = getenv(subword.c_str());
                        env = strcat(env,word.substr(npos).c_str());
                        //cout << env;
                    }
                    else {
                        env = getenv(word.c_str());
                        //cout << env;
                    }
                    cout << env;
                }
                else {
                    cout << *it << " ";
                }
            }
            cout << endl;
        }
        //free(env);
    }

    else if (cmd.front() == "export" && cmd.size() == 2) {
        size_t npos = 0, pos=0;
        string name, value, word = *(++(cmd.begin()));
        char* env;
        char buf[MAX_LIMIT];
        npos = word.find("=",pos);
        name = word.substr(pos,npos);
        //value = word.substr(npos+1);

        if (word.find("$",npos+1) != string::npos) {
            if(word.substr(npos+2)=="PWD")
                value = getcwd(buf,MAX_LIMIT);
            else
                value = getenv(word.substr(npos+2).c_str());
            //env = strcat(name,value.c_str());
            //env = new char[name.length()+value.length()+1];
            //strcpy(env,name.append(string("=").append(value)).c_str());
            //value = string("=").append(value);
        }
        else {
            value = word.substr(npos+1);
            //env = new char[value.length()+name.length()+1];
            //strcpy(env,name.append(string("=").append(value)).c_str());
            //value = string("=").append(value);
        }

        //if(putenv(env)==-1)
        //    cerr << strerror(errno);
        //cout << env << endl;
        //cout << putenv(env) << endl;
        //free(env);
        if(setenv(name.c_str(),value.c_str(),1)!= 0)
            cerr << strerror(errno) << endl;
    }

    else if (cmd.front() == "kill" && cmd.size() == 3) {
        pid_t pid = stoi(*(cmd.end()));
        int signal = stoi(*(++cmd.begin()));
        kill(pid,signal);
    }

    else {
        std::list<string> args;
        bool redirections = false;
        for(auto& arg : cmd) {
            args.push_back(arg);
            if(arg == "<" || arg == ">" || arg == ">>") {
                redirections = true;
            }
        }

        if(redirections) {
            execute_redirections(cmd);
            return;
        }
        
        char *argList[args.size()+1];
		int count =0;
		for(list<string>::iterator i=args.begin(); i!=args.end(); i++)
		{
			argList[count] = (char *)malloc(sizeof(char)*(*i).length()+1);
			strcpy(argList[count], (*i).c_str());
			count++;
		}
		argList[count] = NULL;

        if(fork()==0) {
            if(cmd.front()[0]=='.' && cmd.front()[1]=='/') {
                if(execv(cmd.front().c_str(), argList) == -1)
                    cerr << strerror(errno) << endl;
            }
            else {
                if (cmd.front() == "cat" && cmd.size() == 1) {
                    //exit(0);
                }
                else if (cmd.front()=="") {
                    //exit(0);
                }
                else if(execvp(cmd.front().c_str(), argList) == -1)
				    cout<<strerror(errno)<<"\n";
            }
            free(argList[count]);
            exit(1);
            
        }
        else {
            wait(NULL);
        }
    }

}

int main(int argc, char *argv[]) {

    char input[MAX_LIMIT];
    string usercmd;
    list<string>cmdlist;

    if(argc != 1) {
        print_out();
        exit(1);
    }

    cout << "Welcome to Quash . . .\n" << endl;

    if(isatty(STDIN_FILENO)) {
        cout << "[Quash]$ ";
    }
    while(getline(cin,usercmd)) {

        cmdlist = cmdParser(usercmd);

        execute_commands(cmdlist);

        if(isatty(STDIN_FILENO)) {
            cout << "[Quash]$ ";
        }
    }
    //cout << "I'm exiting . . ." << endl;
    return 0;

}
