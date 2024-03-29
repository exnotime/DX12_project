#include "SubSystemSet.h"
#include <algorithm>
using namespace core;
SubSystemSet::SubSystemSet() {

}

SubSystemSet::~SubSystemSet() {
}

void SubSystemSet::AddSubSystem( SubSystem* ss, uint32_t startUpPrio, uint32_t updatePrio, uint32_t shutdownPrio) {
	SubSystemEntry sse;
	sse.ss = ss;
	
	sse.start = (startUpPrio == SUBSYSTEM_INPUT_ORDER) ? m_Entries.size() : startUpPrio;
	sse.update = (updatePrio == SUBSYSTEM_INPUT_ORDER) ? m_Entries.size() : updatePrio;
	sse.shutdown = (shutdownPrio == SUBSYSTEM_INPUT_ORDER) ? m_Entries.size() : shutdownPrio;

	m_Entries.push_back(sse);
	m_Updated = true;
}

void SubSystemSet::StartSubSystems() {
	auto sortByStartUp = [](const SubSystemEntry& lhs, const SubSystemEntry& rhs) {
		return lhs.start < rhs.start;
	};
	std::sort(m_Entries.begin(), m_Entries.end(), sortByStartUp);

	for (auto& system : m_Entries) {
		system.ss->Startup();
	}
}

void SubSystemSet::UpdateSubSystems(const double deltaTime) {
	if (m_Updated) {
		auto sortByUpdate = [](const SubSystemEntry& lhs, const SubSystemEntry& rhs) {
			return lhs.update < rhs.update;
		};
		std::sort(m_Entries.begin(), m_Entries.end(), sortByUpdate);
		m_Updated = false;
	}

	for (auto& system : m_Entries) {
		system.ss->Update(deltaTime);
	}
}

void SubSystemSet::ShutdownSubSystems() {
	auto sortByShutdown = [](const SubSystemEntry& lhs, const SubSystemEntry& rhs) {
		return lhs.shutdown < rhs.shutdown;
	};
	std::sort(m_Entries.begin(), m_Entries.end(), sortByShutdown);

	for (auto& system : m_Entries) {
		system.ss->Shutdown();
		delete system.ss;
	}
	m_Entries.clear();
}