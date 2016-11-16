
void main(){
	//print("Spawning Level\n");
	//Sponza scene with bunny
/*	*/
	//SpawnPlayer(vec3(28.75f, 42.03f, -3.47f), vec3(0.5f, 1.8f, 0.3f), quat(-0.627f, 0.187f, -0.724f, -0.216f));
	//int bunny = LoadModel("assets/models/bunny/bunny.obj");
	//int level = LoadModel("assets/models/sponza/sponza.obj");
	//int occluder = LoadModel("assets/models/sponza/SponzaOccluder.obj");
	//SpawnLevelObjectWithOccluder(level, occluder, vec3(0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(1));
	//SpawnLevelObject(bunny, vec3(0, 1.1f, 10), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(0.5f, 0.6f, 0.5f, 1));
	//AddSplinePoint(vec3(57.678, 7.215, 0.239));
	//AddSplinePoint(vec3(1.131, 4.717, 0.998));
	//AddSplinePoint(vec3(-61.093, 5.442, 0.995));
	//AddSplinePoint(vec3(-63.344, 13.235, -19.568));
	//AddSplinePoint(vec3( 55.313, 9.693, -23.701));
	//AddSplinePoint(vec3(56.236, 10.359, 21.343));
	//AddSplinePoint(vec3(-60.501, 11.631, 25.242));
	//AddSplinePoint(vec3(-60.469, 8.995, 1.860));
	//AddSplinePoint(vec3(14.201, 34.598, 2.302));
	//AddSplinePoint(vec3(66.118, 32.442, 21.352));
	//AddSplinePoint(vec3(-65.885, 31.746, 26.149));

	//San miguel scene
	
	SpawnPlayer(vec3(-4.47f, 3.02f, -17.73f), vec3(0.5f, 1.8f, 0.3f), quat(-0.93f, 0.05f, -0.37f, -0.02f));
	int level = LoadModel("D:/san_miguel/sanMiguel.obj");
	int occluder = LoadModel("D:/san_miguel/sanMiguelOcc.obj");
	SpawnLevelObjectWithOccluder(level,occluder, vec3(0, 0, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(2.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	AddSplinePoint(vec3(-4.5f, 2.8f, -18.0f));
	AddSplinePoint(vec3(-22.0f, 2.8f,-18.0f));
	AddSplinePoint(vec3(-22.0f, 2.8f, -40.0f));
	AddSplinePoint(vec3(0.5f, 2.8f, -40.0f));
	AddSplinePoint(vec3(2.1f, 2.8f, -17.0f));
	AddSplinePoint(vec3(-8.75f, 2.8f, -17.0f));
	ResizeFrameBuffer(vec2(1920,1080) * 2.0f);

	//rungholt
	//SpawnPlayer(vec3(0.0f, 20.0f, 0.0f), vec3(0.5f, 1.8f, 0.3f), quat(1.0f, 0.0f, 0.0f, 0.0f));
	//level = LoadModel("assets/models/rungholt/rungholt.obj");
	//SpawnLevelObject(level, vec3(0, 0, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
}
