#include "ShapeGenerator.h"
#include "ModelBank.h"
#define PAR_SHAPES_IMPLEMENTATION
#include <par_shapes.h>

ShapeGenerator::ShapeGenerator() {

}

ShapeGenerator::~ShapeGenerator() {

}

ShapeGenerator& ShapeGenerator::GetInstance() {
	static ShapeGenerator m_Instance;
	return m_Instance;
}

int ShapeGenerator::GenerateModel(BASIC_SHAPE shape) {
	if (m_GeneratedShapes.find(shape) != m_GeneratedShapes.end()) {
		return m_GeneratedShapes[shape];
	}
	par_shapes_mesh_s* mesh;
	switch (shape) {
	case CUBE:
		mesh = par_shapes_create_cube();
		//unweld to create per vertex normals
		par_shapes_unweld(mesh, true);
		par_shapes_compute_normals(mesh);
		//scale and translate into unit cube
		par_shapes_scale(mesh, 2, 2, 2);
		par_shapes_translate(mesh, -1, -1, -1);
		break;
	case PLANE:
		mesh = par_shapes_create_plane(1,1);
		//unweld to create per vertex normals
		par_shapes_unweld(mesh, true);
		par_shapes_compute_normals(mesh);
		break;
	case SPHERE_SUBDIV:
		mesh = par_shapes_create_subdivided_sphere(5);
		break;
	case SPHERE_PARA:
		mesh = par_shapes_create_parametric_sphere(32, 8);
		break;
	case CYLINDER:
	{
		mesh = par_shapes_create_cylinder(64, 1);
		par_shapes_translate(mesh, 0, 0, -0.5f);
		par_shapes_scale(mesh, 1, 1, 2.0f);

		glm::vec3 topNormal = glm::vec3(0, 0, 1);
		glm::vec3 axis = glm::vec3(1, 0, 0);

		par_shapes_mesh_s* topDisc = par_shapes_create_disk(1.0f, 64, &topNormal[0], &topNormal[0]);
		par_shapes_merge(mesh, topDisc);
		par_shapes_rotate(topDisc, 3.14f, &axis[0]);
		par_shapes_merge(mesh, topDisc);
		par_shapes_free_mesh(topDisc);
		break;
	}
	case CAPSULE:
	{
		mesh = par_shapes_create_cylinder(30, 3);
		par_shapes_translate(mesh, 0, 0, -0.5f);
		par_shapes_scale(mesh, 1, 1, 2.0f);

		par_shapes_mesh_s* topSphere = par_shapes_create_parametric_sphere(32, 8);
		par_shapes_mesh_s* bottomSphere = par_shapes_create_parametric_sphere(32, 8);

		glm::vec3 axis = glm::vec3(1, 0, 0);
		par_shapes_rotate(topSphere, 3.14f, &axis[0]);
		par_shapes_rotate(bottomSphere, 3.14f, &axis[0]);

		par_shapes_translate(topSphere, 0, 0, 1.0f);
		par_shapes_translate(bottomSphere, 0, 0, -1.0f);

		par_shapes_merge(mesh, topSphere);
		par_shapes_merge(mesh, bottomSphere);
		par_shapes_free_mesh(topSphere);
		par_shapes_free_mesh(bottomSphere);
		break;
	}

	case DONUT:
		mesh = par_shapes_create_torus(32, 32, 0.5f);
		break;
	case OCTOHEDRON:
		mesh = par_shapes_create_octohedron();
		//unweld to create per vertex normals
		par_shapes_unweld(mesh, true);
		par_shapes_compute_normals(mesh);
		break;
	case TETRAHEDRON:
		mesh = par_shapes_create_tetrahedron();
		//unweld to create per vertex normals
		par_shapes_unweld(mesh, true);
		par_shapes_compute_normals(mesh);
		break;
	case DODECAHEDRON:
		mesh = par_shapes_create_dodecahedron();
		//unweld to create per vertex normals
		par_shapes_unweld(mesh, true);
		par_shapes_compute_normals(mesh);
		break;
	case ICOSAHEDRON:
		mesh = par_shapes_create_icosahedron();
		//unweld to create per vertex normals
		par_shapes_unweld(mesh, true);
		par_shapes_compute_normals(mesh);
		break;
	case ROCK:
		mesh = par_shapes_create_rock(1233456789, 4);
		par_shapes_unweld(mesh, true);
		par_shapes_compute_normals(mesh);
		break;
	default:
		return -1;
		break;
	}
	//check for normals
	if (!mesh->normals) {
		printf("No normals was generated for the mesh\n");
		return -1;
	}
	m_GeneratedShapes[shape] = CreateModelFromMesh(mesh);
	par_shapes_free_mesh(mesh);
	return m_GeneratedShapes[shape];
}

int ShapeGenerator::CreateModelFromMesh(par_shapes_mesh_s*  mesh) {
	//create vertices
	std::vector<Vertex> vertices;
	Vertex vertex;
	UINT texIndex = 0;
	for (int i = 0; i < mesh->npoints * 3; i += 3) {
		vertex.Position = glm::vec3(mesh->points[i], mesh->points[i + 1], mesh->points[i + 2]);
		glm::vec3 normal = glm::vec3(mesh->normals[i], mesh->normals[i + 1], mesh->normals[i + 2]);
		glm::vec3 c1 = glm::cross(normal, glm::vec3(0.0, 0.0, 1.0));
		glm::vec3 c2 = glm::cross(normal, glm::vec3(0.0, 1.0, 0.0));
		glm::vec3 tangent;
		if (glm::length(c1) > glm::length(c2)) {
			tangent = c1;
		}
		else {
			tangent = c2;
		}
		vertex.Normal = glm::vec3(glm::normalize(normal));
		vertex.Tangent = glm::vec3(glm::normalize(tangent));
		//most shapes dont have texture coordinates so just create tex coords as if it was a sphere and hope for the best :)
		if (!mesh->tcoords) {
			glm::vec2 uv;
			uv.x = glm::dot(glm::vec3(vertex.Normal), glm::vec3(1, 0, 0)) * 0.5 + 0.5;
			uv.y = glm::dot(glm::vec3(vertex.Normal), glm::vec3(0, 1, 0)) * 0.5 + 0.5;
			vertex.TexCoord = glm::vec2(uv);
		}
		else {
			vertex.TexCoord = glm::vec2(mesh->tcoords[texIndex], mesh->tcoords[texIndex + 1]);
			texIndex += 2;
		}
		vertices.push_back(vertex);
	}

	std::vector<UINT> indices;
	for (int i = 0; i < mesh->ntriangles * 3; i += 3) {
		indices.push_back(mesh->triangles[i]);
		indices.push_back(mesh->triangles[i + 2]);
		indices.push_back(mesh->triangles[i + 1]);
	}
	int modelhandle = g_ModelBank.CreateCustomModel(&vertices, &indices);
	return modelhandle;
}

void ShapeGenerator::LoadAllShapes() {
	for (int i = 0; i < SHAPE_COUNT; ++i) {
		GenerateModel((BASIC_SHAPE)i);
	}
}