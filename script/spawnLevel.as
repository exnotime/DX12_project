
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
	//print("Spawning Level\n");
	//Sponza scene with bunny
	//SpawnPlayer(vec3(28.75f, 42.03f, -3.47f), vec3(0.5f, 1.8f, 0.3f), quat(-0.627f, 0.187f, -0.724f, -0.216f));
	//bunny = LoadModel("assets/models/bunny/bunny.obj");
	//level = LoadModel("assets/models/sponza/sponza.obj");
	//occluder = LoadModel("assets/models/sponza/SponzaOccluder.obj");
	//SpawnLevelObjectWithOccluder(level, occluder, vec3(0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(1));
	//SpawnLevelObject(bunny, vec3(0, 3.0f, 10), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(0.5f, 0.6f, 0.5f, 1));
	//SpawnLevelObject(bunny, vec3(-10, 2.0f, 10), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(0.5f, 0.6f, 0.5f, 1));
	//SpawnLevelObject(bunny, vec3(-20, 2.0f, 10), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(0.5f, 0.6f, 0.5f, 1));
	//SpawnLevelObject(bunny, vec3(-30, 2.0f, 10), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(0.5f, 0.6f, 0.5f, 1));
	//SpawnLevelObject(bunny, vec3(0, 2.0f, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(0.5f, 0.6f, 0.5f, 1));
	//SpawnLevelObject(bunny, vec3(-10, 2.0f, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(0.5f, 0.6f, 0.5f, 1));
	//SpawnLevelObject(bunny, vec3(-20, 2.0f, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(0.5f, 0.6f, 0.5f, 1));
	//SpawnLevelObject(bunny, vec3(-30, 2.0f, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.05f), vec4(0.5f, 0.6f, 0.5f, 1));
	
	//San miguel scene
	//SpawnPlayer(vec3(-4.47f, 3.02f, -17.73f), vec3(0.5f, 1.8f, 0.3f), quat(-0.93f, 0.05f, -0.37f, -0.02f));
	//level = LoadModel("E:/san_miguel/sanMiguel.obj");
	//SpawnLevelObject(level, vec3(0, 0, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(2.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));

	//rungholt
	SpawnPlayer(vec3(0.0f, 20.0f, 0.0f), vec3(0.5f, 1.8f, 0.3f), quat(1.0f, 0.0f, 0.0f, 0.0f));
	level = LoadModel("assets/models/rungholt/rungholt.obj");
	//occluder = LoadModel("assets/models/rungholt/rungholt.obj");
	SpawnLevelObject(level, vec3(0, 0, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f));

}
