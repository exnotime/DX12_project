#pragma once
#define g_TestParams TestParams::GetInstance()
#include <vector>
#include <string>
struct TestData {
	std::string TestName;
	bool AsyncCompute;
	bool Culling;
	float Duration;
	int BatchSize;
	int BatchCount;
};

 class TestParams {
 public:
	bool UseCulling = false;
	int BatchSize = 1024;
	int BatchCount = 1024;
	bool Instrument = false;
	bool AsyncCompute = false;
	bool LogToFile = false;
	std::string Filename = "default.dat";
	bool Reset = false;

	static TestParams& GetInstance();

	std::vector<TestData> Tests;
 };

