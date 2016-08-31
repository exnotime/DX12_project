#pragma once
#include <vector>
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#define g_JobManager JobManager::GetInstance()
#define JOB_SUCCESS 0
#define JOB_ERROR -1
#define JOB_EXIT 1

enum JOB_TYPES {
	TEST,
	JOB_TYPE_COUNT
};

struct Job {
	int Type;
	void* Args;
	std::function<int(void*)> JobFunction;
};

void JobThread(int threadID);

class JobManager {
public:
	~JobManager();
	static JobManager& GetInstance();
	void Init(uint32_t numWorkerThreads);
	void EnqueueJob(const Job job);
	void EnqueueJobs(std::vector<Job> jobs);
	bool HasJobs();
	bool GetNextJob(Job& outJob);
	bool ShouldThreadClose(int threadID);
	void WaitForJobType(int type);
	void SignalCompletion(int type);
private:
	JobManager();
	uint32_t m_ThreadCount = 1;
	std::vector<std::thread> m_Threads;
	std::atomic_int* m_Signals;
	std::atomic_int m_Fences[JOB_TYPE_COUNT];
	std::mutex m_JobQueueMutex;
	std::queue<Job> m_Jobs;
};


