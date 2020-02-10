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
glm::dvec3 Material::shade(Scene* scene, const ray& r, const isect& i) const
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
	glm::dvec3 *iA = new glm::dvec3(kA[0] * iScene[0], kA[1] * iScene[1], kA[2] * iScene[2]);
	glm::dvec3 *result = iA;
	glm::dvec3 p = (r.getPosition() + i.getT() * r.getDirection()) + RAY_EPSILON*i.getN();
	// std::cout<< p[0] << " " << p[1] << " " << p[2] << std::endl;
	

	for ( const auto& pLight : scene->getAllLights() )
	{
	             // pLight has type unique_ptr<Light>
		 
		glm::dvec3 kD = kd(i);
		glm::dvec3 kS = ks(i);
		glm::dvec3 iIn = pLight.get()->getColor();
		
		glm::dvec3 v = glm::normalize(-(r.getDirection()));
		glm::dvec3 n = glm::normalize(i.getN());
		glm::dvec3 l = glm::normalize(-(pLight.get()->getDirection(i.getP())));
		
		double lightAttenuation = pLight.get()->distanceAttenuation(p);
		iIn *= lightAttenuation;
		// std::cout << lightAttenuation << std::endl;
		glm::dvec3 shadowAttenuation = pLight.get()->shadowAttenuation(r, p);
		// std::cout << shadowAttenuation[0] << " " << shadowAttenuation[1] << " " << shadowAttenuation[2] << std::endl;
		iIn *= shadowAttenuation;

		glm::dvec3 ref = glm::normalize((l) - 2.0 * -(glm::dot((l), n)) * n);
		double defuse_max = glm::max(-(glm::dot(l, n)), 0.0);
		glm::dvec3 kD_max = defuse_max * kD;
		double specular_max = pow(glm::max(glm::dot(v, ref), 0.0), shininess(i));
		glm::dvec3 kS_max = specular_max * kS;
		glm::dvec3 kD_kS = kD_max + kS_max;
		glm::dvec3 *kD_kS_iIn = new glm::dvec3(kD_kS[0] * iIn[0], kD_kS[1] * iIn[1], kD_kS[2] * iIn[2]);
		*result += *kD_kS_iIn;
		// glm::dvec3 shadowAttenuation = pLight.get()->shadowAttenuation(r, i.getP());
		// *result *= shadowAttenuation;
	}
	
	return *result;
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
