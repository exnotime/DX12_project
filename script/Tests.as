
void main(){

	//void AddTest(string name, bool culling, bool async, int duration, int batchSize, int batchCount)
	//duration is in number of frames

	AddTest("testdata/Culling.dat",true, false, 10000, 1024, 1024);
	AddTest("testdata/NoCulling.dat",false, false, 10000, 1024,1024);

}