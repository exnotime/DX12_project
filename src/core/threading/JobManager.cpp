#include "JobManager.h"

JobManager::JobManager() {

}
JobManager::~JobManager() {
	//close threads
	for (int i = 0; i < m_ThreadCount; i++) {
		m_Signals[i] = JOB_EXIT;
		m_Threads[i].join();
	}
	delete[] m_Signals;
}

JobManager& JobManager::GetInstance() {
	static JobManager m_Manager;
	return m_Manager;
}

void JobManager::Init(uint32_t numWorkerThreads) {
	m_ThreadCount = numWorkerThreads;
	m_Threads.resize(numWorkerThreads);
	m_Signals = new std::atomic_int[numWorkerThreads];
	for (int i = 0; i < m_ThreadCount; i++) {
		m_Signals[i] = 0;
		m_Threads[i] = std::thread(JobThread,i);
	}
	for (int i = 0; i < JOB_TYPE_COUNT; i++) {
		m_Fences[i] = 0;
	}
}

void JobManager::EnqueueJob(const Job job) {
	m_JobQueueMutex.lock();
	m_Jobs.push(job);
	m_Fences[job.Type]++;
	m_JobQueueMutex.unlock();
}

void JobManager::EnqueueJobs(std::vector<Job> jobs) {
	m_JobQueueMutex.lock();
	for (auto& job : jobs) {
		m_Jobs.push(job);
		m_Fences[job.Type]++;
	}
	m_JobQueueMutex.unlock();
}

bool JobManager::HasJobs() {
	m_JobQueueMutex.lock();
	bool status = !m_Jobs.empty();
	m_JobQueueMutex.unlock();
	return status;
}

bool JobManager::GetNextJob(Job& outJob) {
	m_JobQueueMutex.lock();
	bool status = !m_Jobs.empty();
	if (status) {
		outJob = m_Jobs.front();
		m_Jobs.pop();
	}
	m_JobQueueMutex.unlock();
	return status;
}

bool JobManager::ShouldThreadClose(int threadID) {
	return m_Signals[threadID] != JOB_EXIT;
}

void JobManager::WaitForJobType(int type) {
	while (m_Fences[type] != 0) {
		std::this_thread::yield();
	}
}

void JobManager::SignalCompletion(int type) {
	m_Fences[type]--;
}

void JobThread(int threadID) {
	Job j;
	while (g_JobManager.ShouldThreadClose(threadID)) {
		if (g_JobManager.GetNextJob(j)) {
			if (j.JobFunction(j.Args) == JOB_ERROR) {
				printf("Error with Job thread %d\n", threadID);
			}
			g_JobManager.SignalCompletion(j.Type);
		}
	}
	
}