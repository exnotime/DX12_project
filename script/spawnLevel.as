
int teapot, cube, dragon, lucina, sphere, bunny, level, occluder;
enum BASIC_SHAPE {
	CUBE,
	PLANE,
	SPHERE_SUBDIV,
	SPHERE_PARA,
	CYLINDER,
	CAPSULE,
	DONUT,
	OCTOHEDRON,
	TETRAHEDRON,
	DODECAHEDRON,
	ICOSAHEDRON,
	ROCK,
	SHAPE_COUNT
};

void main(){
	//sponza
	level = LoadModel("assets/models/sponza/sponza.obj");
	occluder = LoadModel("assets/models/sponza/SponzaOccluder.obj");
	//san miguel
	//level = LoadModel("E:/san_miguel/sanMiguel.obj");

	teapot = LoadModel("assets/models/teapot/teapot.obj");
	//cube = LoadModel("assets/models/cube/cube.obj");
	dragon = LoadModel("assets/models/dragon/dragon.obj");
	//lucina = LoadModel("assets/models/LucinaResource/Lucina_Posed.obj");
	//sphere = LoadModel("assets/models/sphere/sphere.dae");
	//bunny = LoadModel("assets/models/bunny/bunny.obj");
	print("Spawning Level\n");
	SpawnPlayer(vec3(0,0,0), vec3(0.5f, 1.8f, 0.3f));
	SpawnLevelObject(teapot, vec3(20, 5, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.3f), vec4(0, 0.4f, 0, 1));

	SpawnLevelObject(dragon, vec3(0, 10, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(2.0f), vec4(1, 0, 0, 1));

	SpawnLevelObjectWithOccluder(level, occluder, vec3(0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(1));

	SpawnLevelObject(level, vec3(0, 0, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(2.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	
	//SpawnPhysicsShape(int(CUBE), vec3(0, -1.5f, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(100,1,100), vec4(1.0f), 0.0f);
	/*
	SpawnPhysicsShape(int(CUBE), vec3(22, 10, 10), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.1f, 0.1f, 60.0f), vec4(1.0f) , 0.0f);
	*/
	for(int i = 0; i < SHAPE_COUNT; ++i){
		SpawnPhysicsShape(i, vec3( -20 + i * 5, 30, 20), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(1), vec4(0.5f, 0.2f, 0.7f, 1.0f), 0.0f);
	}
	
	/*
	float size = 3.0f;
	for(int x = 0; x < size; x++){
		for(int y = 0; y < size; y++){
			SpawnPhysicsShape(0, vec3(x + 22 + (x * 0.5f), 50, y + (y * 0.2f)), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(20,0.5f,0.5f), vec4(x / size, 1.0f - (y / size), 1.0f - (y / size) - (x / size), 1.0f), x + y + 1);
		}
	}
	*/
}