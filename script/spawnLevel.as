void main(){
	float NaN = 0x7fc00000; //used to set the lookat to follow the spline instead of looking at the look at position
	//Sponza scene with bunny, teapot and dragon
#if SPONZA
	SpawnPlayer(vec3(28.75f, 42.03f, -3.47f), vec3(0.5f, 1.8f, 0.3f), quat(-0.627f, 0.187f, -0.724f, -0.216f));

	int level = LoadModel("big_assets/sponza/sponza.obj");
	int occluder = LoadModel("big_assets/sponza/SponzaOccluder.obj");

	int bunny = LoadModel("assets/models/bunny/bunny.obj");
	int dragon = LoadModel("assets/models/dragon/dragon.obj");
	int teapot = LoadModel("assets/models/teapot/teapot.obj");
	SpawnLevelObjectWithOccluder(level, occluder, vec3(0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(1));
	SpawnLevelObjectWithOccluder(bunny,bunny, vec3(0, 1.9f, 2), quat(0.0f, 0.0f, 1.0f, 0.0f), vec3(0.05f), vec4(0.2f, 0.6f, 0.5f, 1));
	SpawnLevelObjectWithOccluder(dragon,dragon, vec3(0, 0.2f, -5), quat(0.0f, 0.0f, 1.0f, 0.0f), vec3(1.0f), vec4(0.7f, 0.2f, 0.1f, 1));
	SpawnLevelObjectWithOccluder(teapot,teapot, vec3(-10, 0.2f, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(1.5f), vec4(0.5f, 0.9f, 0.3f, 1));
	AddSplinePoint(vec3( 50.678, 37.215 ,  -2.239));
	AddSplinePoint(vec3( 1.131 , 8.717 ,  2.998));
	AddSplinePoint(vec3(-50.093, 8.442 ,  0.995));
	AddSplinePoint(vec3(-50.344, 13.235, -19.568));
	AddSplinePoint(vec3( 50.313, 9.693 , -23.701));
	AddSplinePoint(vec3( 50.236, 10.359,  21.343));
	AddSplinePoint(vec3(-50.501, 11.631,  25.242));
	AddSplinePoint(vec3(-50.469, 8.995 ,  -1.860));
	AddSplinePoint(vec3( 14.201, 34.598,  3.302));
	AddSplinePoint(vec3( 50.118, 32.442,  21.352));
	AddSplinePoint(vec3(-50.885, 31.746,  26.149));
	SetLookatPos(vec3(0, 8, -2));
#endif
	//San miguel scene
#if SAN_MIGUEL
	SpawnPlayer(vec3(-4.47f, 3.02f, -17.73f), vec3(0.5f, 1.8f, 0.3f), quat(-0.93f, 0.05f, -0.37f, -0.02f));
	int level = LoadModel("big_assets/san_miguel/sanMiguelTest.obj");
	int occluder = LoadModel("big_assets/san_miguel/sanMiguelOcc.obj");
	SpawnLevelObjectWithOccluder(level,occluder, vec3(0, 0, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(2.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	AddSplinePoint(vec3(-4.5f, 2.8f, -18.0f));
	AddSplinePoint(vec3(-22.0f, 2.8f,-18.0f));
	AddSplinePoint(vec3(-22.0f, 2.8f, -42.0f));
	AddSplinePoint(vec3(0.5f, 2.8f, -42.0f));
	AddSplinePoint(vec3(2.1f, 2.8f, -17.0f));
	AddSplinePoint(vec3(-8.75f, 2.8f, -17.0f));
	SetLookatPos(vec3(-12.0f, 2.8f, -30.0f));
#endif
	//rungholt
#if RUNGHOLT
	SpawnPlayer(vec3(0.0f, 20.0f, 0.0f), vec3(0.5f, 1.8f, 0.3f), quat(1.0f, 0.0f, 0.0f, 0.0f));
	int level = LoadModel("big_assets/rungholt/rungholt.obj");
	int occluder = LoadModel("big_assets/rungholt/rungholt_occ.obj");
	SpawnLevelObjectWithOccluder(level, occluder, vec3(0, 0, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	AddSplinePoint(vec3(234.0f, 33.5f, -197.0f));
	AddSplinePoint(vec3(-3.0f, 37.5f, -179.0f));
	AddSplinePoint(vec3(-93.0f, 36.5f, -60.0f));
	AddSplinePoint(vec3(-93.0f, 36.5f, 135.0f));
	AddSplinePoint(vec3(-192.0f, 36.5f, 205.0f));
	SetLookatPos(vec3(NaN));
#endif
}
