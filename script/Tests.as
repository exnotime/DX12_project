
void main(){

	//void AddTest(string name, bool culling, bool async, int duration, int batchSize, int batchCount)
	//duration is in number of frames
		
	AddTest("testdata/Culling.dat",true, 10000, 1024, 1024, vec2(1920,1080));
	AddTest("testdata/NoCulling.dat",false, 10000, 1024,1024, vec2(1920,1080));
}