#ifndef JOBS
#define JOBS

#include <string>
//#include <vector>
using namespace std;


class BackgroundJobs {
    public:
        
        enum status {RUNNING, TERMINATED};

        BackgroundJobs() {
            job_id = -1;
            job_pid = -1;
            job_name = "";
            job_status = TERMINATED;
        }

        BackgroundJobs(int id, pid_t pid, string name, BackgroundJobs::status st) {
            job_id = id;
            job_pid = pid;
            job_name = name;
            job_status = st;
        }

        ~BackgroundJobs(){

        }
        int job_id;
        pid_t job_pid;
        string job_name;
        status job_status;
        void printJobs(void);
        void addJobs(void);
        void removeJobs(void);
};
#endif