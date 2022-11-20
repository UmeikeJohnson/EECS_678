#include "Functions.h"


Runner::Runner()
{}

Runner::~Runner()
{
	
}


bool Runner::set(std::string varName, std::string assignExp)
{
	char * newVar = new char[varName.length() + assignExp.length()+1];
	string envEntry = varName.append("=").append(string(assignExp));
	const char * envEntryC = envEntry.c_str();
	strcpy(newVar, envEntryC );
	/*
	if(varName == "PATH"){
		strcpy(newVar,string("PATH=").append(string(assignExp)).c_str() );
	}
	else if(varName == "HOME"){
		strcpy(newVar,string("HOME=").append(string(assignExp)).c_str() );
	}
	*/
		
	//TODO allow more variables?

	if (putenv(newVar) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool Runner::setPath(std::string newPath)
{
	list<string> newPathList;
	size_t pos=0;
	while( (pos = newPath.find(':')) != string::npos)
	{
		newPathList.push_back(newPath.substr(0, pos));
		newPath = newPath.substr(pos+1);
	}

	char * newVar = new char[newPath.length()+1];
	strcpy(newVar,string("PATH=").append(string(newPath)).c_str() );

	if (putenv(newVar) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}

}
std::string Runner::getPath()
{
	return string(getenv("PATH"));
} 

void Runner::executeFunc(list<string> args, std::string exec, bool foreground)//IDK which exec to use
{
	int exOrd[2];

	pid_t toEx;


	if (pipe(exOrd) != 0)
	{
		std::cerr<<"Could not execute: Pipe Failure!\n";
		return;
	}

	toEx = fork();
	if (toEx == 0)
	{
		close(exOrd[1]);

		char *argList[args.size()+1];
		int count =0;
		for(list<string>::iterator i=args.begin(); i!=args.end(); i++)
		{
			argList[count] = (char *)malloc(sizeof(char)*(*i).length()+1);
			strcpy(argList[count], (*i).c_str());
			count++;
		}
		argList[count] = NULL;
		if(exec[0] == '.' && exec[1]=='/')
		{
			if(execv(exec.c_str(), argList) == -1)
			{
				cout<<strerror(errno)<<"\n";
			}
		}
		else {
			if(execvp(exec.c_str(), argList) == -1)
			{
				cout<<strerror(errno)<<"\n";
			}
		}
		
		read(exOrd[0], NULL, 1);//on failure wait for parent to finish allocating entry
		close(exOrd[0]);

		for(--count; count>=0; count--)
		{
			delete argList[count];
		}
		exit(1);

	}
	else
	{
		if(foreground)
		{
			close(exOrd[1]);
			close(exOrd[0]);
			int status;
			if(waitpid(toEx, &status, 0) == 0)
			{
				fprintf(stderr, "Process %d encountered an error. Error: %s", toEx, strerror(status));
			}
		}
		else
		{
			

			string name ;
			for(list<string>::iterator i=args.begin(); i!=args.end(); i++)
			{
				name = name + " " + (*i);
			}
			
			Job newJob(name, Job::RUNNING,toEx, jobList.back().m_id+1 );
			pthread_mutex_lock(&jobListMutex);
			jobList.push_back(newJob);
			pthread_mutex_unlock(&jobListMutex);
			cout<<"["<<newJob.m_id<<"] " << newJob.m_pid <<" running in background\n";
			close(exOrd[1]);
			close(exOrd[0]);
		}
	}
	
}

void Runner::execStatement(list<string> args, bool foreground){
	vector<vector<string>> complete;//stores vector of commands to be pipeed
	vector<vector<int*>> redirDups;//stores vector of dups to be done within each process
	vector<string> singleCommand;
	vector<int*> singleDupList;
	vector<pid_t> waitList; //pid_t's to wait for

	vector<int*> ipc;
	
	bool alreadyPushedCmd = false;
	for(auto arg = args.begin(); arg!=args.end(); arg++){
		if(*arg == "|"){
			if(!alreadyPushedCmd){
				complete.push_back(singleCommand);
				singleCommand.clear();
			}
			else{
				alreadyPushedCmd = false;
			}
			int * pipee = new int[2];
			pipe(pipee);
			ipc.push_back(pipee);
			redirDups.push_back(singleDupList);
			singleDupList.clear();
		}
		else if(*arg == "<" && arg != args.end() && arg != args.begin()){//single command is full
			string inpFile = *(next(arg));
			
			complete.push_back(singleCommand);
			singleCommand.clear();
			int inpFd = open(inpFile.c_str(), O_RDWR);
			if(inpFd == -1){
				cout<<strerror(errno)<<"\n";
			}
			int * newDup = new int[2];
			newDup[0]=inpFd;
			newDup[1] = 0;
			singleDupList.push_back(newDup);
			alreadyPushedCmd = true;
		}
		else if(*arg == ">" && arg != args.end() && arg != args.begin()){
			string outFile = *(next(arg));
			complete.push_back(singleCommand);
			singleCommand.clear();
			int outFd = open(outFile.c_str(), O_RDWR|O_CREAT, 0666);
			if(outFd == -1){
				cout<<strerror(errno)<<"\n";
			}
			int * newDup = new int[2];
			newDup[0]=outFd;
			newDup[1] = 1;
			singleDupList.push_back(newDup);
			alreadyPushedCmd = true;
		}
		else if(*arg != ">" && *arg != "<" && *arg != "|" && !alreadyPushedCmd){
			singleCommand.push_back(*arg);
		}
	}
	redirDups.push_back(singleDupList);
	if(!alreadyPushedCmd){
		complete.push_back(singleCommand);
	}


	//fork for loop
	for(int i=0; i<complete.size(); i++)//TODO BACKGROUND exec of pipe stmnt
	{
		vector<string> execArgs = complete.at(i);
		pid_t uniFork;

		uniFork = fork();
		waitList.push_back(uniFork);
		if(uniFork == 0)
		{
			char *argList[execArgs.size()+1];
			int count =0;
			for(vector<string>::iterator i=execArgs.begin(); i!=execArgs.end(); i++)
			{
				argList[count] = (char *)malloc(sizeof(char)*(*i).length()+1);
				strcpy(argList[count], (*i).c_str());
				count++;
			}
			argList[count] = NULL;

			if(ipc.size() >= i){//pipees
				if(i>0){
					dup2(ipc[i-1][0], 0);
				}
				if(i < ipc.size()){
					dup2(ipc[i][1],1);
				}
				for(int j=0; j<ipc.size(); j++){
					if(j == i-1){
						close(ipc[j][1]);
					}
					else if(j == i){
						close(ipc[j][0]);
					}
					else
					{
						close(ipc[j][0]);
						close(ipc[j][1]);
					}
				}
			}
			for(auto& i : argList) {
				std::cout << "argList: " << i << "execArgs: " << execArgs[0] << endl;
			}
			//redir input from redirDup fd
			vector<int*> & myDups = redirDups.at(i);//0 -> i for itr
			for(auto & dupl : myDups){
				dup2(dupl[0], dupl[1]);
			}
			std::cout << "after dup\n" << endl;
			if(execArgs[0][0] == '.' && execArgs[0][1]=='/')
			{
			//	cout << "execArgs[0]" << execArgs[0].c_str() << endl;
				if(execv(execArgs[0].c_str(), argList) == -1)
				{
					cout<<strerror(errno)<<"\n";
				}
			}
			else {
				std::cout << "execArgs[0]" << execArgs[0].c_str() << endl;
				if(execvp(execArgs[0].c_str(), argList) == -1)
				{
					cout<<strerror(errno)<<"\n";
				}
			}
			exit(1);//could not exec
		}
	}
	for(int j=0; j<ipc.size(); j++){
		close(ipc[j][0]);
		close(ipc[j][1]);
	}
	waitpid(waitList.back(), NULL, 0);
}

void Runner::changeDir(string where)
{
	if(chdir(where.c_str()) == -1)
	{
		cerr<<strerror(errno);
	}
}
string Runner::get_env(string var)
{
	char * varVal = getenv(var.c_str());
	if(varVal != NULL)
	{
		return string(varVal) + "\n";
	}
	else{
		return "\n";
	}
}
list<Job>& Runner::getJobList(){
	return jobList;
}

