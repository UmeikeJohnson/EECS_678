/*Execs built in functions*/
#include "Job.h"
//#include "PreDefs.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <iostream>

#include <climits>
#include <cstring>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>

using namespace std;


class Runner{
	public:
	Runner();
	~Runner();

	//Function execution
	void executeFunc(list<string> args, string, bool someBool);

	//setting environment variables
	bool set(std::string varName, std::string assignExp);
	bool setPath(std::string newPath);
	std::string getPath();

	//redirection functions
	void execStatement(list<string> args, bool);

	void changeDir(string where);

	string get_env(string var);


	//JobList
	list<Job>& getJobList(); 

	pthread_mutex_t jobListMutex;

	private:
	unordered_map<string, string> vars;
	//unordered_map<pid_t, Job> jobList;

	list<Job> jobList;



	//friend void catch_dead_child(int signum);
	//friend void *waiter_clean_up(void *);
	//friend void *waiter(void *param);

};
