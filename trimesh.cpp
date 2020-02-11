#include "trimesh.h"
#include <assert.h>
#include <float.h>
#include <string.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

using namespace std;

Trimesh::~Trimesh()
{
	for (auto m : materials)
		delete m;
	for (auto f : faces)
		delete f;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3& v)
{
	vertices.emplace_back(v);
}

void Trimesh::addMaterial(Material* m)
{
	materials.emplace_back(m);
}

void Trimesh::addNormal(const glm::dvec3& n)
{
	normals.emplace_back(n);
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c)
{
	int vcnt = vertices.size();

	if (a >= vcnt || b >= vcnt || c >= vcnt)
		return false;

	TrimeshFace* newFace = new TrimeshFace(
	        scene, new Material(*this->material), this, a, b, c);
	newFace->setTransform(this->transform);
	if (!newFace->degen)
		faces.push_back(newFace);
	else
		delete newFace;

	// Don't add faces to the scene's object list so we can cull by bounding
	// box
	return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char* Trimesh::doubleCheck()
{
	if (!materials.empty() && materials.size() != vertices.size())
		return "Bad Trimesh: Wrong number of materials.";
	if (!normals.empty() && normals.size() != vertices.size())
		return "Bad Trimesh: Wrong number of normals.";

	return 0;
}

bool Trimesh::intersectLocal(ray& r, isect& i) const
{
	bool have_one = false;
	//generateNormals();
	for (auto face : faces) {
		isect cur;
		if (face->intersectLocal(r, cur)) {
			if (!have_one || (cur.getT() < i.getT())) {
				i = cur;
				have_one = true;
			}
		}
	}
	if (!have_one)
		i.setT(1000.0);
	return have_one;
}

bool TrimeshFace::intersect(ray& r, isect& i) const
{
	return intersectLocal(r, i);
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray& r, isect& i) const
{
	// YOUR CODE HERE
	//
	// FIXME: Add ray-trimesh intersection
	//cout << "before distance" << endl;
	glm::dvec3 v0 = parent->vertices[ids[0]];
	glm::dvec3 v1 = parent->vertices[ids[1]];
	glm::dvec3 v2 = parent->vertices[ids[2]];

	glm::dvec3 a_normal = parent->normals[ids[0]];
	glm::dvec3 b_normal = parent->normals[ids[1]];
	glm::dvec3 c_normal = parent->normals[ids[2]];

	glm::dvec3 normal = this->normal;
	glm::dvec3 orig = r.getPosition();
	glm::dvec3 dir = r.getDirection();

	glm::dvec3 v0v1 = v1 - v0;
	glm::dvec3 v0v2 = v2 - v0;
	glm::dvec3 pvec = glm::cross(dir, v0v2);
	double det = glm::dot(v0v1, pvec);

	if (det < 0) return false;
	if (glm::abs(det) < RAY_EPSILON) return false;

	double invDet = 1/det;

	glm::dvec3 tvec = orig - v0;
	double u = glm::dot(tvec , pvec) * invDet;
	if (u < 0 || u > 1) return false;
	// if (u < RAY_EPSILON) u = RAY_EPSILON;
	// if (u > 1-RAY_EPSILON) u = 1-RAY_EPSILON;

	glm::dvec3 qvec = glm::cross(tvec, v0v1);
	double v = glm::dot(dir, qvec) * invDet;
	if (v < 0 || u + v > 1) return false;
	// if (v < RAY_EPSILON) v = RAY_EPSILON;
	// if (v > 1-RAY_EPSILON) v = 1-RAY_EPSILON;
	
	double t = glm::dot(v0v2, qvec) * invDet;
	double w = 1 - u - v;
	// if (w < RAY_EPSILON) w = RAY_EPSILON;
	// if (w > 1-RAY_EPSILON) w = 1-RAY_EPSILON;

	if (parent->vertNorms) {
		glm::dvec3 lin_interpolate = (w * a_normal + u * b_normal + v * c_normal);
		glm::dvec3 renormalized_interpolate = glm::normalize(lin_interpolate);
		i.setN(renormalized_interpolate);
	} else {
		i.setN(normal);
	}

	i.setT(t);
	if (t < RAY_EPSILON) return false;
	i.setBary(u, v, w);
	i.setUVCoordinates(glm::dvec2(u, v));
	// std::cout << "trimesh " << t << std::endl;
	// Material m = *parent->materials[ids[0]];
	if (parent->materials.empty()) {
		i.setMaterial(*materials); 
	} else {
		Material m0 = *parent->materials[ids[0]];
		Material m1 = *parent->materials[ids[1]];
		Material m2 = *parent->materials[ids[2]];
		Material* newM = new Material();
		newM->setEmissive(w*m0.ke(i) + u*m1.ke(i) + v*m2.ke(i));
		newM->setAmbient(w*m0.ka(i) + u*m1.ka(i) + v*m2.ka(i));
		newM->setSpecular(w*m0.ks(i) + u*m1.ks(i) + v*m2.ks(i));
		newM->setDiffuse(w*m0.kd(i) + u*m1.kd(i) + v*m2.kd(i));
		newM->setReflective(w*m0.kr(i) + u*m1.kr(i) + v*m2.kr(i));
		newM->setTransmissive(w*m0.kt(i) + u*m1.kt(i) + v*m2.kt(i));
		newM->setShininess(w*m0.shininess(i) + u*m1.shininess(i) + v*m2.shininess(i));
		newM->setIndex(w*m0.index(i) + u*m1.index(i) + v*m2.index(i));
		i.setMaterial(*newM);
	}

	r.setIntersect(orig + dir*i.getT());
	
	// double distance = (glm::dot((a_coords - origin),(normal)) / glm::dot((direction),(normal)));
	// //cout << "distance: " << distance << endl;
	// if (distance <= 0) return false;

	// glm::dvec3 p_coords = origin + distance*direction;
	

	// glm::dvec3 vab = (b_coords - a_coords);
	// glm::dvec3 vca = (a_coords - c_coords);
	// glm::dvec3 vbc = (c_coords - b_coords);
	// glm::dvec3 vap = (p_coords - a_coords);
	// glm::dvec3 vcp = (p_coords - c_coords);
	// glm::dvec3 vbp = (p_coords - b_coords);

	// glm::dvec3 a_cross = glm::cross(vab,vap);
	// glm::dvec3 b_cross = glm::cross(vbc,vbp);
	// glm::dvec3 c_cross = glm::cross(vca,vcp);
	// glm::dvec3 whole_cross = glm::cross(vab,-(vca));


	// double a_const = glm::dot((a_cross),normal);
	// double b_const = glm::dot((b_cross),normal);
	// double c_const = glm::dot((c_cross),normal);

	// double a_area = sqrt(glm::dot(a_cross,a_cross));
	// double b_area = sqrt(glm::dot(b_cross,b_cross)); 
	// double c_area = sqrt(glm::dot(c_cross,c_cross)); 
	// double whole_area = sqrt(glm::dot(whole_cross,whole_cross));

	// double u = c_area / whole_area;
	// double v = a_area / whole_area;
	// double w = b_area / whole_area;

	//cout << 123 << endl;
	// if(a_const >= 0 && b_const >= 0 && c_const >= 0){
	// 	glm::dvec3 lin_interpolate = (w * a_normal + u * b_normal + v * c_normal);
	// 	//cout << sqrt(glm::dot(lin_interpolate, lin_interpolate)) << endl;
	// 	glm::dvec3 renormalized_interpolate = glm::normalize(lin_interpolate);
	// 	i.setT(distance);
	// 	i.setBary(u, v, w);
	// 	i.setN(renormalized_interpolate);
	// 	i.setMaterial(*(this->material));
	// 	r.setIntersect(p_coords);
	// 	// glm::dvec3 p = i.getP();
	// 	// std::cout<< p[0] << " " << p[1] << " " << p[2] << std::endl;
	// 	//cout << true << endl;
	// 	return true;
	// }
	//cout << false << endl;
	return true;
}

// Once all the verts and faces are loaded, per vertex normals can be
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals()
{
	int cnt = vertices.size();
	normals.resize(cnt);
	std::vector<int> numFaces(cnt, 0);

	for (auto face : faces) {
		glm::dvec3 faceNormal = face->getNormal();

		for (int i = 0; i < 3; ++i) {
			normals[(*face)[i]] += faceNormal;
			++numFaces[(*face)[i]];
		}
	}

	for (int i = 0; i < cnt; ++i) {
		if (numFaces[i])
			normals[i] /= numFaces[i];
	}

	vertNorms = true;
}

