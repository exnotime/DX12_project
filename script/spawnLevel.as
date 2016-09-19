
int teapot, cube, dragon, lucina, sphere;
bool start = true;
void main(){
	if(start){
		teapot = LoadModel("assets/models/teapot/teapot.obj");
		cube = LoadModel("assets/models/cube/cube.obj");
		dragon = LoadModel("assets/models/dragon/dragon.obj");
		//lucina = LoadModel("assets/models/LucinaResource/Lucina_Posed.obj");
		sphere = LoadModel("assets/models/sphere/sphere.dae");
		start = false;
	}
	print("Spawning Level\n");
	SpawnPlayer(vec3(0,0,0), vec3(0.5f, 1.8f, 0.3f));

	SpawnLevelObject(teapot, vec3(50, 0, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(1.0f), vec4(0, 0.2f, 0, 1));
	SpawnLevelObject(dragon, vec3(0, 0, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(20.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f));

	SpawnLevelObject(sphere, vec3(50, 0, -20), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(2.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f));
	SpawnLevelObject(sphere, vec3(0, 0, -20), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f));

	
	SpawnPhysicsObject(cube, vec3(0, -10, 0), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(100,1,100), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
	SpawnPhysicsObject(cube, vec3(22, 10, 10), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(0.1f, 0.1f, 60.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);

	float size = 1.0f;
	for(int x = 0; x < size; x++){
		for(int y = 0; y < size; y++){
			SpawnPhysicsObject(cube, vec3(x + 20, 50, y), quat(1.0f, 0.0f, 0.0f, 0.0f), vec3(20,0.5f,0.5f), vec4(x / size, 1.0f - (y / size), 1.0f - (y / size) - (x / size), 1.0f), 2.0f);
		}
	}
	*/
}