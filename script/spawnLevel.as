
int teapot, cube, dragon, lucina, sphere, bunny, level;
bool start = true;
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
	if(start){
		//level = LoadModel("E:/sponza/sponza.obj");
		teapot = LoadModel("assets/models/teapot/teapot.obj");
		//cube = LoadModel("assets/models/cube/cube.obj");
		//dragon = LoadModel("assets/models/dragon/dragon.obj");
		//lucina = LoadModel("assets/models/LucinaResource/Lucina_Posed.obj");
		//sphere = LoadModel("assets/models/sphere/sphere.dae");
		//bunny = LoadModel("assets/models/bunny/bunny.obj");
		start = false;
	}
	print("Spawning Level\n");
	SpawnPlayer(vec3(0,0,0), vec3(0.5f, 1.8f, 0.3f));
	SpawnLevelObject(teapot, vec3(70, 20, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(1.0f), vec4(0, 0.4f, 0, 1));
	//SpawnLevelObject(bunny, vec3(0, 10, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f));

	SpawnPhysicsShape(int(CUBE), vec3(0, -1.6f, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(200,1,200), vec4(1.0f), 0.0f);
	SpawnPhysicsShape(int(CUBE), vec3(22, 10, 10), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.1f, 0.1f, 60.0f), vec4(1.0f) , 0.0f);

	for(int i = 0; i < SHAPE_COUNT; ++i){
		SpawnPhysicsShape(i, vec3( -50 + i * 10, 30, 110), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(3), vec4(0.5f, 0.2f, 0.7f, 1.0f), 100.0f);
	}

	float size = 3.0f;
	for(int x = 0; x < size; x++){
		for(int y = 0; y < size; y++){
			SpawnPhysicsShape(0, vec3(x + 22 + (x * 0.5f), 50, y + (y * 0.2f)), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(20,0.5f,0.5f), vec4(x / size, 1.0f - (y / size), 1.0f - (y / size) - (x / size), 1.0f), x + y + 1);
		}
	}
}