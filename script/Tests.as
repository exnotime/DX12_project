
void main(){

	//void AddTest(string name, bool culling, bool async, int duration, int batchSize, int batchCount)
	//duration is in number of frames
	AddTest("testdata/Culling.dat",true, false, 1000, 1024, 1024);
	AddTest("testdata/NoCulling.dat",false, false, 1000, 1024,1024);

	string name = "testdata/Batch_";

	for(int c = 64; c <= 1024; c*=2 ){
			string testname = name + c + "_" + c + ".dat";
			AddTest(testname,true, false, 500, c, c);
	}
	//AddTest("testdata/Culling128.dat",true, false, 100, 128, 1024);
	//AddTest("testdata/Culling256.dat",true, false, 100, 256, 1024);
	//AddTest("testdata/Culling512.dat",true, false, 100, 512, 1024);
	//AddTest("testdata/Culling1024.dat",true, false, 100, 1024, 1024);

}