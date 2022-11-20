#include <iostream>
#include <list>
#include <cassert>

#include "Job.h"
#include "Functions.h"
//#include "PreDefs.h"

using namespace std;
static Runner executor;


void updateJobList(void);

void catch_dead_child(int signum)
{
	#ifdef DEBUG
	cout<<"SigChld handler called\n";
	#endif
	if(wait(NULL) != -1)
	{
		updateJobList();
	}
	//return;
}

typedef struct waiterArgs {
	int exOrd[2];
	sigset_t toUnmask;
} waiterArgs;

void *waiter(void *param)//will handle all signals
{

	#ifdef DEBUG
	cout<<"Waiter created\n";
	
	#endif
	int sig;
	
	waiterArgs myArgs = *(waiterArgs *)param;
	pthread_sigmask(SIG_UNBLOCK, &myArgs.toUnmask, NULL);

	struct sigaction sa;
	memset((void *)&sa, 0, sizeof(sa));
	sa.sa_handler = catch_dead_child;
	sigaction(SIGCHLD, &sa, NULL);
	
	
	write(myArgs.exOrd[1], (void *)&myArgs.toUnmask, sizeof(sigset_t));

	while(1){
		
		//sigwait(&myArgs.toUnmask, &sig);//eats signal
		sleep(UINT_MAX);
		#ifdef DEBUG
		cout<<"Waiter awakened" <<endl;
		#endif
	}
	
}

void updateJobList(){
	list<Job> & curr = executor.getJobList();
	pthread_mutex_lock(&executor.jobListMutex);
	for(auto i=curr.begin(); i!=curr.end(); )
	{
		if((*i).mySt == Job::TERMINATED)
		{
			i=curr.erase(i);
		}
		else{
			i++;
		}
	}
	int status;
	for(auto i=curr.begin(); i!=curr.end(); i++)
	{
		#ifdef DEBUG
		if(waitpid((*i).m_pid, NULL ,WNOHANG) == -1)
		{
			cout<<"["<<(*i).m_id<<"] "<<(*i).m_pid<<" mystery job " << (*i).myName<<"\n";
			cout<<strerror(errno)<<"\n";
		}
		#endif
		if(waitpid((*i).m_pid, &status ,WNOHANG|WCONTINUED) != 0)//TODO handle signals from kill
		{
			cout<<"["<<(*i).m_id<<"] "<<(*i).m_pid<<" finished " << (*i).myName<<"\n";
			#ifdef DEBUG
			if(WIFEXITED(status))
			{
				cout<<"Child terminated normally\n";
			}
			if(WIFSIGNALED(status))
			{
				cout<<"Child received signal:"<<WTERMSIG(status)<< "\n";
			}
			#endif
			curr.erase(i);
			break;
		}
		/*
		if(waitpid((*i).m_pid, &status ,WNOHANG|WUNTRACED|WCONTINUED) != 0)//TODO handle signals from kill
		{
			cout<<"["<<(*i).m_id<<"] "<<(*i).m_pid<<" finished " << (*i).myName<<"\n";
			#ifdef DEBUG
			if(WIFEXITED(status))
			{
				cout<<"Child terminated normally\n";
			}
			if(WIFSIGNALED(status))
			{
				cout<<"Child terminated by signal:"<<WTERMSIG(status)<< "\n";
			}
			#endif
			curr.erase(i);
			break;
		}
		*/
	}
	for(auto i=curr.begin(); i!=curr.end(); i++)
	{
		if(waitpid((*i).m_pid, NULL ,WNOHANG) != 0)
		{
			(*i).mySt = Job::TERMINATED;
			cout<<"["<<(*i).m_id<<"] "<<(*i).m_pid<<" finished " << (*i).myName<<"\n";
		}
	}
	pthread_mutex_unlock(&executor.jobListMutex);

}

void printUsage(){
	printf("Usage: ./Quash -- Interactive\n");
	printf("Usage: ./Quash < cmdFile -- Read from Stored File\n");
}

list<string> parseArgs(string cmd){
	list<string> sep;
	string word;
	size_t pos = 0;
	size_t npos = 0;
	while (pos < cmd.size()){
		while(pos < cmd.size() && cmd[pos] == ' '){
			pos++;
		}
		if(pos == cmd.size()){
			break;
		}
		npos = cmd.find(' ', pos+1);
		if(npos == string::npos){
			word = cmd.substr(pos);
			
		}
		else
		{
			word = cmd.substr(pos, npos-pos);
		}
		sep.push_back(word);
		pos+=word.size() + 1;
		while(pos < cmd.size() && cmd[pos] == ' '){
			pos++;
		}
	}
	if(cmd == "")
	{
		#ifdef DEBUG
		cout<<"empty cmd\n";
		#endif
		sep.push_back("");
	}
	return sep;
}
void interpret(list<string>  cmd){
	#ifdef DEBUG
	cout<<"INTERPRETER: "<<cmd.front()<<"\n";
	#endif
	if(cmd.front() == ""){
	}
	else if(cmd.front() == "quit" || cmd.front() == "exit"){
		fflush(stdout);//ADD
		exit(0);
	}
	else if(cmd.front() == "cd" && cmd.size() == 2){
		executor.changeDir(*(++(cmd.begin())));
	}
	else if(cmd.front() == "jobs" && cmd.size() == 1){
		pthread_mutex_lock(&executor.jobListMutex);
		#ifdef DEBUG
		cout<<"JOBS cmd got access to list.\n";
		#endif
		list<Job> & jobList = executor.getJobList();
		for(auto job = jobList.begin(); job !=jobList.end() ; job++)
		{
			cout<<"["<<(*job).m_id<<"] " << (*job).m_pid <<(*job).myName<<"\n";
		}
		pthread_mutex_unlock(&executor.jobListMutex);
		//updateJobList();//TODO
	}
	else if(cmd.front() == "set" && cmd.size() == 2 && (*(++(cmd.begin()))).find('=') != string::npos) {

		string setExp = *(++(cmd.begin()));
		string toSet = setExp.substr(0,setExp.find('='));

		if( setExp.find('=')!=string::npos)
		{
			executor.set(toSet, setExp.substr(setExp.find('=')+1) );//TODO: Check argument to set eg. set 
		}
	}
	else if(cmd.front() == "cd" && cmd.size() <= 2)
	{
		if(cmd.size() == 2)
		{
			executor.changeDir(*(++(cmd.begin())));
		}
		else if(cmd.size() == 1)
		{
			executor.changeDir(getenv("HOME"));
		}

	}
	else if (cmd.front() == "printenv" && cmd.size() == 2)
	{
		cout<<executor.get_env(*(++(cmd.begin())));
	}
	else if(cmd.front() == "kill" && cmd.size() == 3){
		pid_t pid = -1;
		pthread_mutex_lock(&executor.jobListMutex);
		for(auto & i : executor.getJobList())
		{
			if(i.m_id == stoi(*(++cmd.begin()))){
				pid = i.m_pid;
			}
		}
		pthread_mutex_unlock(&executor.jobListMutex);
		if(pid != -1){
			if (kill(pid, stoi(*(--(cmd.end())))) == -1){
				cout<<strerror(errno)<<"\n";
			}
			//updateJobList();//TODO call signal handler when jobs receive terminating signals
		}
	}
	else
	{
		std::list<string> args;
		bool redir = false;
		for(auto & arg : cmd)
		{
			args.push_back(arg);	
		}
		for(auto & arg : cmd)
		{
			if(arg == "<" || arg == ">" || arg == "|")
			{
				redir = true;
			}
		}

		if(args.back() == "&")
		{
			args.pop_back();
			if(redir)
			{
				executor.execStatement(args, false);
			}
			else
			{
				executor.executeFunc(args, args.front(), false);
			}
			
		}
		else{
			if(redir)
			{
				executor.execStatement(args, true);
			}
			else
			{
				executor.executeFunc(args, args.front(), true);
			}
		}
	}

}
int main(int argc, char **argv/*, char **envp*/)
{
	
	if (argc != 1){
		printUsage();
		exit(1);
	}

	sigset_t toMask;//make and block sigchld mask
	bzero((void*)&toMask, sizeof(sigset_t));
	sigaddset(&toMask, SIGCHLD);
	pthread_sigmask(SIG_BLOCK, &toMask, NULL);

	waiterArgs args;//make args
	pipe(args.exOrd);
	args.toUnmask = toMask;

	pthread_t waitThread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&waitThread, &attr, waiter, (void *)&args);//make thread

	//wait for thread to be ready
	read(args.exOrd[0], NULL, sizeof(sigset_t));

	//TODO: STOP POINT IS REACHED


	char cmd[1024];
	if(isatty(STDIN_FILENO))
	{
		std::cout<<"[Quash$]:";
	}
	while(cin.getline(cmd, 1024)){
		bool redir = false;
		list<string> parsed = parseArgs(cmd);

		#ifdef DEBUG
		cout<<"INP: " << cmd<<"\n";
		cout<<"PARSER: ";
		for (auto arg : parsed){
			cout<<arg<<" ";
		}
		cout<<"\n";
		fflush(stdout);//ADD
		#endif

		interpret(parsed);


		if(isatty(STDIN_FILENO))
		{
			std::cout<<"[Quash$]:";//ADD
		}
		fflush(stdout);
	}
	
	#ifdef DEBUG
	std::cout<<"STOP POINT\n";	
	fflush(stdout);
	#endif
	
}
