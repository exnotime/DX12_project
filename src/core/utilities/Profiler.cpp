#include "Profiler.h"
#include "../../gfx/TestParams.h"
Profiler::Profiler() {
#ifdef PRINT_TO_FILE
	m_File = fopen((g_TestParams.Directory + "/CPU_Timings.dat").c_str(), "w");
#endif
	
}

Profiler::~Profiler() {

}


void Profiler::StartFrame() {
	m_Timer.Reset();
	m_TimeSteps.clear();
}

void Profiler::Step(const std::string& stepName) {
	if (m_TimeSteps.find(stepName) != m_TimeSteps.end()) {
		m_TimeSteps[stepName] += m_Timer.Tick();
	} else {
		m_TimeSteps[stepName] = m_Timer.Tick();
	}
}

void Profiler::EndFrame() {
	m_TimeSteps["End"] = m_Timer.Reset();
}

void Profiler::Print() {
#ifndef SILENT_LOG
	printf("\n");
	for (auto& step : m_TimeSteps) {
		printf("CPU TimeStep %s: %3.3f ms\n", step.first.c_str(), step.second * 1000.0);
	}
#endif
#ifdef PRINT_TO_FILE
	fprintf(m_File, "%f\n", m_TimeSteps["End"] * 1000.0);
#endif
}

void Profiler::Reset() {
#ifdef PRINT_TO_FILE
	fclose(m_File);
	m_File = fopen((g_TestParams.Directory + "/CPU_Timings.dat").c_str(), "w");
#endif
}