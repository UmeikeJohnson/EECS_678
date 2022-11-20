#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include <iterator>
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
#define FIND_EXEC  "/bin/find"
#define GREP_EXEC  "/bin/grep"

void print_out() {
    cout << "An error occurred. You have too many arguments!" << endl;
    cout << "./Quash  --requires no additional argument(s)" << endl;
};


list<BackgroundJobs> JobWaitList;

void BackgroundJobs::printJobs() {
    for(auto it = JobWaitList.begin(); it!=JobWaitList.end(); ++it) {
        cout << "[" << (*it).job_id << "]\t" << (*it).job_pid << "\t" << (*it).job_name << endl;
    }
    return;
}
void BackgroundJobs::addJobs() {
    BackgroundJobs jobtoadd(job_id,job_pid,job_name,job_status);
    JobWaitList.push_back(jobtoadd);
    return;
}
void BackgroundJobs::removeJobs() {
    
    BackgroundJobs jobtoremove(job_id,job_pid,job_name,job_status);
    
    for(auto it = JobWaitList.begin(); it!=JobWaitList.end(); ++it) {
        if((*it).job_pid == jobtoremove.job_pid) {
            JobWaitList.erase(it);
            break;
        }
    }
    return;
}

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
    return outcmd;
}

void execute_redirections(list<string> cmd) {

    if(fork()==0) {
        vector<vector<string>> fullcommand;
        vector<int*>pipecommand;
        vector<string> args;
        vector<pid_t> waitpidList;
        vector<string> redirection;
        string op, outFile, inFile;
        int *pipes;
        bool flag = false;
        for(auto arg = cmd.begin(); arg != cmd.end(); ++arg) {
            
            if(*arg == "|" && arg != cmd.begin() && arg != cmd.end()) {
                
                if(!flag) {
                    fullcommand.push_back(args);
                    args.clear();
                }
                else
                    flag = false;
                pipes = new int[2];
                pipe(pipes);
                pipecommand.push_back(pipes);
                redirection.push_back("x");
            }
            else if(*arg == "<" && arg != cmd.begin() && arg != cmd.end()) {
                inFile = *(next(arg));
                op = *arg;
                if(!flag){
                    fullcommand.push_back(args);
                    args.clear();
                }
                redirection.push_back(op);
                flag = true;
            }

            else if(*arg == ">" && arg != cmd.begin() && arg != cmd.end()) {
                outFile = *(next(arg));
                op = *arg;
                if(!flag){
                    fullcommand.push_back(args);
                    args.clear();
                }
                redirection.push_back(op);
                flag = true;
            }

            else if(*arg == ">>" && arg != cmd.begin() && arg != cmd.end()) {
                outFile = *(next(arg));
                op = *arg;
                if(!flag){
                    fullcommand.push_back(args);
                    args.clear();
                }
                redirection.push_back(op);
                flag = true;
            }
            
            else if(*arg != ">" && *arg != "<" && *arg != ">>" && !flag) {
                args.push_back(*arg);

            }

        }
        if(!flag)
            fullcommand.push_back(args);

        int check = fullcommand.size() - redirection.size();

        if(check > 0 && check != 0) {
            for(int i=0; i<check; ++i)
                redirection.push_back("x");
        }
        
        for(int i = 0; i<fullcommand.size() ; ++i) {
            vector<string> cmdexec;
           
            cmdexec = fullcommand.at(i);

            pid_t ret;

            ret = fork();
            if (ret != 0) {
                waitpidList.push_back(ret);
            }
            if(ret == 0) {
                char *argList[args.size()+1];
                int count =0;
                for(vector<string>::iterator i=cmdexec.begin(); i!=cmdexec.end(); i++)
                {
                    argList[count] = (char *)malloc(sizeof(char)*(*i).length()+1);
                    strcpy(argList[count], (*i).c_str());
                    count++;
                }
                argList[count] = NULL;
                //pipes
                if(pipecommand.size() >= i){
                
                    if(i>0){
                        dup2(pipecommand[i-1][0], STDIN_FILENO);
                    }
                    if(i < pipecommand.size()){
                        dup2(pipecommand[i][1],STDOUT_FILENO);
                    }
                    for(int j=0; j<pipecommand.size(); j++){
                        if(j == i-1){
                            close(pipecommand[j][1]);
                        }
                        else if(j == i){
                            close(pipecommand[j][0]);
                        }
                        else
                        {
                            close(pipecommand[j][0]);
                            close(pipecommand[j][1]);
                        }
                    }
                }

                if( redirection[i] == "<") {
                    freopen(inFile.c_str(),"r",stdin); //Open text file for reading.  The stream is positioned at the beginning of the file.
                    if(execvp(cmdexec.front().c_str(), argList) == -1)
                        cerr << strerror(errno) << endl;
                    //}
                    std::exit(1);
                }
                else if (redirection[i] == ">") {
                    freopen(outFile.c_str(),"w",stdout); //Truncate file to zero length or create text file for writing.  The stream is positioned at the beginning of the file.
                        if(execvp(cmdexec.front().c_str(), argList) == -1)
                            cerr << strerror(errno) << endl;
                    std::exit(1);
                }

                else if (redirection[i] == ">>") {
                    freopen(outFile.c_str(),"a",stdout); //Open for appending (writing at end of file).  The file is created if it does not exist.  The stream is positioned at the end of the file.
                    if(execvp(cmdexec.front().c_str(), argList) == -1)
                        cerr << strerror(errno) << endl;
                    std::exit(1);
                }
                else {
                    if(execvp(cmdexec.front().c_str(), argList) == -1)
                        cerr << strerror(errno) << endl;
                    //}
                    std::exit(1);

                }
                //free dynamic memory
                for(int i=0; i<=count; ++i)
                    free(argList[count]);
                
            }
        }
        for(int i = 0; i < pipecommand.size(); ++i) {
            close(pipecommand[i][0]);
            close(pipecommand[i][1]);
        }
        waitpid(waitpidList.back(),NULL,0);
        if(pipecommand.size() != 0) {
            // cout << "deleting dynamic memory for pipes" << endl;
            delete(pipes);   //causes double free corruption when echo hey >> a.txt is called
        }
        exit(0);
    }
    else
        wait(NULL);
}

void execute_commands(list<string> cmd) {

    if(cmd.size()==1 && cmd.front() =="exit" || cmd.size()==1 && cmd.front() =="quit") {
            cout << endl << "Exiting the shell . . ." << endl;
            std::exit(0);
    }

    else if(cmd.front() == "jobs" && cmd.size() == 1) {
        BackgroundJobs jobList;
        jobList.printJobs();
    } 


    else if (cmd.front() == "kill" && cmd.size() == 3) {
        pid_t pid = stoi(cmd.back());
        int signal = stoi(*(++cmd.begin()));
        BackgroundJobs myJob;
        myJob.job_pid = pid;
        for(auto &it : JobWaitList){
            
            if(it.job_pid == pid) {
                
                myJob.job_id = it.job_id;
                myJob.job_name = it.job_name;
                myJob.job_status = BackgroundJobs::TERMINATED;
                break;
            }
        }
        myJob.removeJobs();
        if(pid != -1 && kill(pid,SIGTSTP)==-1) {
            cerr << strerror(errno) << endl;
        }
               

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
                    path.append(subword.c_str());
                }
            }
            if(chdir(path.c_str())!=-1){
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
    }

    else if(cmd.front() == "ls") {

        std::list<string> args;
        bool redirections = false;
        for(auto& arg : cmd) {
            args.push_back(arg);
            if(arg == "<" || arg == ">" || arg == ">>" || arg == "|") {
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
                execl(LS_EXEC, "ls", (*(++cmd.begin())).c_str(), (char*)NULL);
            }
            else if (cmd.size()==3)
                execl(LS_EXEC, "ls", (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(), (char*)NULL);
            std::exit(0);
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
            if(arg == "<" || arg == ">" || arg == ">>" || arg == "|") {
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
                execl(GREP_EXEC, GREP_EXEC, (*(++cmd.begin())).c_str(), (char*)NULL);
            }
            else if (cmd.size()==3)
                execl(GREP_EXEC, GREP_EXEC, (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(),  (char*)NULL);
            else if (cmd.size()==4)
                    execl(GREP_EXEC, GREP_EXEC, (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(), (*(++(++(++cmd.begin())))).c_str(), (char*)NULL);
            std::exit(0);
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
            if(arg == "<" || arg == ">" || arg == ">>" || arg == "|") {
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
                execl(FIND_EXEC, FIND_EXEC, (*(++cmd.begin())).c_str(), (char*)NULL);
            }
            else if (cmd.size()==3)
                execl(FIND_EXEC, FIND_EXEC, (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(),  (char*)NULL);
            
            else if (cmd.size()==4)
                execl(FIND_EXEC, FIND_EXEC, (*(++cmd.begin())).c_str(), (*(++(++cmd.begin()))).c_str(), (*(++(++(++cmd.begin())))).c_str(), (char*)NULL);
            
            std::exit(1);
            }
        else {
            wait(NULL);
        }
    }

    else if (cmd.front() == "cat") {
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
    }
    
    else if(cmd.front() == "echo") {

        std::list<string> args;
        bool redirections = false;
        for(auto& arg : cmd) {
            args.push_back(arg);
            if(arg == "<" || arg == ">" || arg == ">>" || arg == "|") {
                redirections = true;
            }
        }

        if(redirections) {
            execute_redirections(cmd);
            return;
        }

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
                    if (word.find("/",pos) != string::npos){
                        npos = word.find("/",pos);
                        subword = word.substr(pos,npos);
                        env = getenv(subword.c_str());
                        env = strcat(env,word.substr(npos).c_str());
                    }
                    else {
                        env = getenv(word.c_str());
                    }
                    cout << env;
                }
                else {
                    cout << *it << " ";
                }
            }
            cout << endl;
        }
    }

    else if (cmd.front() == "export" && cmd.size() == 2) {
        size_t npos = 0, pos=0;
        string name, value, word = *(++(cmd.begin()));
        char* env;
        char buf[MAX_LIMIT];
        npos = word.find("=",pos);
        name = word.substr(pos,npos);

        if (word.find("$",npos+1) != string::npos) {
            if(word.substr(npos+2)=="PWD")
                value = getcwd(buf,MAX_LIMIT);
            else
                value = getenv(word.substr(npos+2).c_str());
        }
        else {
            value = word.substr(npos+1);
        }

        if(setenv(name.c_str(),value.c_str(),1)!= 0)
            cerr << strerror(errno) << endl;
    }

    else {
        std::list<string> args;
        bool redirections = false;
        for(auto& arg : cmd) {
            args.push_back(arg);
            if(arg == "<" || arg == ">" || arg == ">>" || arg == "|") {
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
                }
                else if (cmd.front()=="") {
                }
                else if(execvp(cmd.front().c_str(), argList) == -1)
				    cout<<strerror(errno)<<"\n";
            }
            std::exit(1);
            
        }
        else {
            wait(NULL);
        }
        //free dynamic memory in argList
        for(int i=0; i<=count;i++)
            free(argList[i]);
    }

}


int main(int argc, char *argv[]) {

    char input[MAX_LIMIT];
    string usercmd;
    list<string>cmdlist;
    int jobid = 0;

    if(argc != 1) {
        print_out();
        std::exit(1);
    }

    cout << "Welcome to Quash . . .\n" << endl;

    if(isatty(STDIN_FILENO)) {
        cout << "[Quash]$ ";
    }

    while(getline(cin,usercmd)) {

        cmdlist = cmdParser(usercmd);
    
        if(cmdlist.back() == "&") {
            jobid++;

            pid_t ret = fork();
                       
            if(ret == 0) {
                BackgroundJobs myJob;
                myJob.job_id = jobid;
                myJob.job_pid = getpid();
                myJob.job_name = usercmd;
                myJob.job_status = BackgroundJobs::RUNNING;
                cout << "Background job started:  [" << myJob.job_id << "]\t" << myJob.job_pid << '\t' << myJob.job_name;
                fflush(stdout);
                cmdlist.pop_back();
                execute_commands(cmdlist);
                cout << "\nCompleted:  [" << myJob.job_id << "]\t" << myJob.job_pid << '\t' << myJob.job_name;
                fflush(stdout);
                kill(getpid(),SIGTSTP);
            }

            else {
                BackgroundJobs JobItem;
                JobItem.job_id = jobid;
                JobItem.job_name = usercmd;
                JobItem.job_pid = ret;
                JobItem.addJobs();
            } 
            
        }
        else {
            execute_commands(cmdlist);
            if(isatty(STDIN_FILENO)) {
                cout << "[Quash]$ ";
            }
        }
    }
    return 0;

}
