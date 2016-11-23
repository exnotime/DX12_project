
void main(){

	//void AddTest(string name, bool culling, int duration, int triangleCount, int batchSize, int batchCount, vec2 framebuffersize)
	//duration is in number of frames
	//two tests with the same duration will match the image rendered each frame.
		
	AddTest("testdata/Culling",true, 10000, 1, 1024, 1024, vec2(1920,1080));
	AddTest("testdata/NoCulling",false, 10000, 1, 1024,1024, vec2(1920,1080));
}