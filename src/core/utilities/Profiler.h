#pragma once
#include "Timer.h"
#include <unordered_map>
#include <stdio.h>
class Profiler {
public:
	Profiler();
	~Profiler();
	void StartFrame();
	void Step(const std::string& stepName);
	void EndFrame();
	void Print();
	void Reset();
private:
	std::unordered_map<std::string, double> m_TimeSteps;
	Timer m_Timer;
	FILE* m_File;
};