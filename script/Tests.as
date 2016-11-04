
void main(){

	//void AddTest(string name, bool culling, bool async, int duration, int batchSize, int batchCount)
	//duration is in number of frames
		
	AddTest("testdata/Culling.dat",true, false, 10000, 1024, 1024);
	AddTest("testdata/NoCulling.dat",false, false, 10000, 1024,1024);
	
	string name = "testdata/Batch_";

	for(int c = 128; c <= 1024; c*=2 ){
			string testname = name + c + "_" + c + ".dat";
			AddTest(testname,true, false, 5000, c, c);
	}
}