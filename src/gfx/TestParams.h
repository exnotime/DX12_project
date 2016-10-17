#pragma once

#define g_TestParams TestParams::GetInstance()

 class TestParams {
 public:
	bool UseCulling = false;
	int BatchSize = 512;
	bool Instrument = false;

	static TestParams& GetInstance();
 };

