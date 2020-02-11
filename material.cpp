#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI* traceUI;

#include <glm/gtx/io.hpp>
#include <iostream>
#include "../fileio/images.h"

using namespace std;
extern bool debugMode;

Material::~Material()
{
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene* scene, const ray& r, const isect& i, int depth, double index1) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
	// You will need to call both distanceAttenuation() and
	// shadowAttenuation()
	// somewhere in your code in order to compute shadows and light falloff.
	// if( debugMode )
	// 	std::cout << "Debugging Phong code..." << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	
	

	glm::dvec3 kA = ka(i);
	glm::dvec3 iScene = scene->ambient();
	glm::dvec3 result(kA*iScene);
	glm::dvec3 p = (r.getPosition() + i.getT() * r.getDirection()) + i.getN() * RAY_EPSILON;
	glm::dvec3 n = glm::normalize(i.getN());

	// std::cout << "material " << i.getT() << std::endl;
	
	// std::cout<< p[0] << " " << p[1] << " " << p[2] << std::endl;

	for ( const auto& pLight : scene->getAllLights() )
	{
	             // pLight has type unique_ptr<Light>
		 
		glm::dvec3 kD = kd(i);
		glm::dvec3 kS = ks(i);
		glm::dvec3 iIn = pLight.get()->getColor();
		
		// LIGHT ATTENUATION
		double lightAttenuation = pLight.get()->distanceAttenuation(p);
		iIn *= lightAttenuation;
		
		// SHADOW ATTENUATION
		glm::dvec3 shadowAttenuation = pLight.get()->shadowAttenuation(r, p);
		iIn *= shadowAttenuation;
		
		
		// std::cout << Refl() << std::endl;
		// if (Refl()) {
		// 	glm::dvec3 kR = kr(i);
		// 	glm::dvec3 kR_max = specular_max * kR;
		// 	*result += kR_max * iIn;
		// } 267, 257
		
		glm::dvec3 v = glm::normalize(-(r.getDirection()));
		glm::dvec3 l = glm::normalize(-(pLight->getDirection(p)));

		glm::dvec3 ref = glm::normalize(l - n * glm::dot(n, l) * 2.0);
		double defuse_max = glm::max(-glm::dot(l, n), 0.0);
		glm::dvec3 kD_max = defuse_max * kD;

		

		double specularDot = glm::dot(v, ref);
		double specular_max = pow(glm::max(specularDot, 0.0), shininess(i));
		glm::dvec3 kS_max = specular_max * kS;

		glm::dvec3 kS_kD = kS_max + kD_max;

		
		result += iIn * kS_kD;
		// result += iIn;
		
		// glm::dvec3 shadowAttenuation = pLight.get()->shadowAttenuation(r, i.getP());
		// *result *= shadowAttenuation;
	}
	// if (depth >= 0) {
	// 	// std::cout << ks(i)[0] << " " << ks(i)[1] << " " << ks(i)[2] << std::endl;
	// 	if (Refl() || Trans() || Recur() || Spec() || Both()) {
	// 		glm::dvec3 camVector = glm::normalize(r.getDirection());
	// 		glm::dvec3 ref = glm::normalize((camVector) - 2.0 * -(glm::dot((camVector), n)) * n);;
	// 		isect iReflect;
	// 		ray rayReflect(p, ref, glm::dvec3(1,1,1), ray::REFLECTION);
	// 		if(scene->intersect(rayReflect, iReflect)) {
	// 			*result += kr(iReflect) * shade(scene, rayReflect, iReflect, depth-1);
	// 		}
	// 	}
	// }

	if (Recur()) {
		if (depth > 0) {
			glm::dvec3 aoeu(0,0,0);
			glm::dvec3 win = r.getDirection();
			if (Refl()) {
				
				glm::dvec3 wref(glm::normalize(win - n * glm::dot(n, win) * 2.0));
				ray reflRay(p, wref, glm::dvec3(0,0,0), ray::REFLECTION);
				isect reflIsect;
				if (scene->intersect (reflRay, reflIsect)) {
					aoeu += kr(i) * shade(scene, reflRay, reflIsect, depth-1, index1);
				}
				// return aoeu;
			}
			if (Trans()) {
				// std::cout << Trans() << std::endl;
				double theta1 = glm::acos(glm::dot(win, i.getN()));
				glm::dvec3 wref;
				double index2 = index(i);
				if (theta1 <= RAY_EPSILON || index1 == index2) {
					wref = win;
				} else {
					double theta2 = index1/index2 * glm::sin(theta1);
					if (theta2 > 1) {
						wref = glm::normalize(win - n * glm::dot(n, win) * 2.0);
					} else {
						glm::dvec3 n2 = -i.getN();
						wref = n2*glm::cos(theta2);
					}
				}
				ray refrRay(p, wref, glm::dvec3(1,1,1), ray::REFRACTION);
				isect refrIsect;
				if (scene->intersect(refrRay, refrIsect)) {
					aoeu += kt(i) * shade(scene, refrRay, refrIsect, depth-1, index2);
				}

			}
			return aoeu;
		
		}
	}
	
	result += ke(i);
	return result;
}

TextureMap::TextureMap(string filename)
{
	data = readImage(filename.c_str(), width, height);
	if (data.empty()) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2& coord) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
	// What this function should do is convert from
	// parametric space which is the unit square
	// [0, 1] x [0, 1] in 2-space to bitmap coordinates,
	// and use these to perform bilinear interpolation
	// of the values.

	return glm::dvec3(1, 1, 1);
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.

	return glm::dvec3(1, 1, 1);
}

glm::dvec3 MaterialParameter::value(const isect& is) const
{
	if (0 != _textureMap)
		return _textureMap->getMappedValue(is.getUVCoordinates());
	else
		return _value;
}

double MaterialParameter::intensityValue(const isect& is) const
{
	if (0 != _textureMap) {
		glm::dvec3 value(
		        _textureMap->getMappedValue(is.getUVCoordinates()));
		return (0.299 * value[0]) + (0.587 * value[1]) +
		       (0.114 * value[2]);
	} else
		return (0.299 * _value[0]) + (0.587 * _value[1]) +
		       (0.114 * _value[2]);
}
