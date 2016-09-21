#pragma once
#include <unordered_map>
/*
This class uses par_shapes.h to generate shapes and puts them into the modelbank
*/
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
class par_shapes_mesh_s;
#define g_ShapeGenerator ShapeGenerator::GetInstance()
class ShapeGenerator {
public:
	~ShapeGenerator();
	static ShapeGenerator& GetInstance();

	int GenerateModel(BASIC_SHAPE shape); //returns a model handle
	void LoadAllShapes();
private:
	int CreateModelFromMesh(par_shapes_mesh_s*  mesh);
	ShapeGenerator();
	std::unordered_map<BASIC_SHAPE, int> m_GeneratedShapes;
};