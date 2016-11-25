
void main(){

	//void AddTest(string name, bool culling, int duration, int triangleCount, int batchSize, int batchCount, vec2 framebuffersize, bool instrument, bvec4 filters)
	//duration is in number of frames
	//two tests with the same duration will match the image rendered each frame.
	//topdir will be replaced by a shell script
	string topDir = "#REPLACE_THIS_DIR" + "/";
	//warm up test since it takes a little while for the solution to reach its full speed on some GPUs
	AddTest(topDir + "WarmUp",true, 5000, 1, 1024, 1024, vec2(1920,1080), false, bvec4(true));
	//pure performance test
	AddTest(topDir + "Culling",true, 5000, 1, 1024, 1024, vec2(1920,1080), false, bvec4(true));
	AddTest(topDir + "NoCulling",false, 5000, 1, 1024,1024, vec2(1920,1080), false, bvec4(true));
	//multi triangle test
	AddTest(topDir + "1Tri",true, 5000, 1, 1024, 1024, vec2(1920,1080), true, bvec4(true));
	AddTest(topDir + "2Tri",true, 5000, 2, 1024, 1024, vec2(1920,1080), true, bvec4(true));
	AddTest(topDir + "3Tri",true, 5000, 3, 1024, 1024, vec2(1920,1080), true, bvec4(true));
	AddTest(topDir + "4Tri",true, 5000, 4, 1024, 1024, vec2(1920,1080), true, bvec4(true));
	//test the effectiveness of the different filters
	AddTest(topDir + "Backface",true, 2000, 1, 1024, 1024, vec2(1920,1080), true, bvec4(true, false, false, false));
	AddTest(topDir + "SmallTris",true, 2000, 1, 1024, 1024, vec2(1920,1080), true, bvec4(false, true, false, false));
	AddTest(topDir + "Frustum",true, 2000, 1, 1024, 1024, vec2(1920,1080), true, bvec4(false, false, true, false));
	AddTest(topDir + "Occlusion",true, 2000, 1, 1024, 1024, vec2(1920,1080), true, bvec4(false, false, false, true));
	AddTest(topDir + "AllFilters",true, 2000, 1, 1024, 1024, vec2(1920,1080), true, bvec4(true));
	//test different batch sizes
	AddTest(topDir + "BatchSize128", true, 5000, 1, 128, 1024, vec2(1920,1080), false, bvec4(true));
	AddTest(topDir + "BatchSize256", true, 5000, 1, 256, 1024, vec2(1920,1080), false, bvec4(true));
	AddTest(topDir + "BatchSize512", true, 5000, 1, 512, 1024, vec2(1920,1080), false, bvec4(true));
	AddTest(topDir + "BatchSize1024", true, 5000, 1, 1024, 1024, vec2(1920,1080), false, bvec4(true));
	//test different batch counts
	AddTest(topDir + "BatchCount128", true, 5000, 1, 1024, 128, vec2(1920,1080), false, bvec4(true));
	AddTest(topDir + "BatchCount256", true, 5000, 1, 1024, 256, vec2(1920,1080), false, bvec4(true));
	AddTest(topDir + "BatchCount512", true, 5000, 1, 1024, 512, vec2(1920,1080), false, bvec4(true));
	AddTest(topDir + "BatchCount1024", true, 5000, 1, 1024, 1024, vec2(1920,1080), false, bvec4(true));
	AddTest(topDir + "BatchCount2048", true, 5000, 1, 1024, 2048, vec2(1920,1080), false, bvec4(true));
	AddTest(topDir + "BatchCount4096", true, 5000, 1, 1024, 4096, vec2(1920,1080), false, bvec4(true));
	//test resolution changes on small tri effectiveness
	AddTest(topDir + "Resolution270", true, 2000, 1, 1024, 1024, vec2(480,270), true, bvec4(false, true, false, false));
	AddTest(topDir + "Resolution540", true, 2000, 1, 1024, 1024, vec2(960,540), true, bvec4(false, true, false, false));
	AddTest(topDir + "Resolution1080", true, 2000, 1, 1024, 1024, vec2(1920,1080), true, bvec4(false, true, false, false));
	AddTest(topDir + "Resolution2160", true, 2000, 1, 1024, 1024, vec2(3840,2160), true, bvec4(false, true, false, false));
}