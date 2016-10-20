#pragma once

#define g_TestParams TestParams::GetInstance()

 class TestParams {
 public:
	bool UseCulling = false;
	int BatchSize = 1024;
	int BatchCount = 1024;
	bool Instrument = false;
	bool AsyncCompute = false;

	static TestParams& GetInstance();
 };

