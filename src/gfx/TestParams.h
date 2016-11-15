#pragma once
#define g_TestParams TestParams::GetInstance()
#include <queue>
#include <string>
struct TestData {
	std::string TestName;
	bool AsyncCompute;
	bool Culling;
	float Duration;
	int BatchSize;
	int BatchCount;
	int TriangleCount;
	bool FilterBackFace;
	bool FilterSmallTri;
	bool FilterFrustum;
	bool FilterOcclusion;
};

 class TestParams {
 public:
	bool Instrument = true;
	bool LogToFile = false;
	std::string Filename = "default.dat";
	bool Reset = false;
	bool FreeCamera = true;

	int FrameCounter = 0;
	TestData CurrentTest;
	std::queue<TestData> Tests;

	static TestParams& GetInstance();

	TestParams() {
		CurrentTest.AsyncCompute = false;
		CurrentTest.Culling = true;
		CurrentTest.Duration = 10000;
		CurrentTest.TestName = "default.dat";
		CurrentTest.BatchCount = 1024;
		CurrentTest.BatchSize = 1024;
		CurrentTest.TriangleCount = 1;
		CurrentTest.FilterBackFace = true;
		CurrentTest.FilterSmallTri = true;
		CurrentTest.FilterFrustum = true;
		CurrentTest.FilterOcclusion = false;
	}
 };

#define BATCH_COUNT g_TestParams.CurrentTest.BatchCount
#define BATCH_SIZE g_TestParams.CurrentTest.BatchSize
#define TRI_COUNT g_TestParams.CurrentTest.TriangleCount

