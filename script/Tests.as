
void main(){

	//void AddTest(string name, bool culling, int duration, int triangleCount, int batchSize, int batchCount, vec2 framebuffersize, bool instrument, bvec4 filters)
	//duration is in number of frames
	//two tests with the same duration will match the image rendered each frame.
	//topdir will be replaced by a shell script
	string topDir = "#REPLACE_THIS_DIR" + "/";

	AddTest(topDir + "WarmUp",true, 5000, 1, 1024, 1024, vec2(1920,1080), false, bvec4(true));

	AddTest(topDir + "Culling",true, 10000, 1, 1024, 1024, vec2(1920,1080), true, bvec4(true));
	AddTest(topDir + "NoCulling",false, 10000, 1, 1024,1024, vec2(1920,1080), false, bvec4(true));

	AddTest(topDir + "2Tri",true, 3000, 2, 1024, 1024, vec2(1920,1080), true, bvec4(true));
	AddTest(topDir + "3Tri",true, 3000, 3, 1024, 1024, vec2(1920,1080), true, bvec4(true));
	AddTest(topDir + "4Tri",true, 3000, 4, 1024, 1024, vec2(1920,1080), true, bvec4(true));
}