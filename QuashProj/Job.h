#ifndef JOB
#define JOB
#include <string>
using namespace std;
class Job{
	public:

	enum status {RUNNING, WAITING, TERMINATED};	

	Job(){mySt = TERMINATED; myName=""; m_pid=-1; m_id=-1;}

	Job(string name, Job::status st, int pid, int id){
		mySt = st;
		m_pid = pid;
		myName = name;
		m_id = id;
	}

	status mySt;
	string myName;
	int m_pid;
	int m_id;
};
#endif
