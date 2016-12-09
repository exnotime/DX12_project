#pragma once
#define g_TestParams TestParams::GetInstance()
#include <queue>
#include <string>
#include <glm/glm.hpp>
struct TestData {
	std::string TestName;
	glm::vec2 FrameBufferSize;
	bool Culling;
	float Duration;
	int BatchSize;
	int BatchCount;
	int TriangleCount;
	bool Instrument;
	bool FilterBackFace;
	bool FilterSmallTri;
	bool FilterFrustum;
	bool FilterOcclusion;
};

 class TestParams {
 public:
	std::string Directory = "testdata/Default";
	std::string Level = "SPONZA";
	bool Reset = false;
	int FrameCounter = 0;
	TestData CurrentTest;
	std::queue<TestData> Tests;

	static TestParams& GetInstance();

	TestParams() {
		CurrentTest.Culling = false;
		CurrentTest.Instrument = false;
		CurrentTest.Duration = 10000;
		CurrentTest.TestName = "testdata/Default";
		CurrentTest.BatchSize = 1024;
		CurrentTest.BatchCount = 1024;
		CurrentTest.TriangleCount = 1;
		CurrentTest.FilterBackFace = true;
		CurrentTest.FilterSmallTri = true;
		CurrentTest.FilterFrustum = true;
		CurrentTest.FilterOcclusion = true;
		CurrentTest.FrameBufferSize = glm::vec2(1600, 900);
	}
 };

//#define DO_TESTING
//#define PRINT_TO_FILE
//#define SILENT_LOG

#define FREE_CAMERA

#define BATCH_COUNT g_TestParams.CurrentTest.BatchCount
#define BATCH_SIZE g_TestParams.CurrentTest.BatchSize
#define TRI_COUNT g_TestParams.CurrentTest.TriangleCount

