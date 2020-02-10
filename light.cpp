#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>


using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3& P) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


glm::dvec3 DirectionalLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	// isect i;
	// glm::dvec3 l = glm::normalize(orientation - p);
	// ray lightRay(p, l, glm::dvec3(1,1,1), ray::SHADOW);  
	// if(thisscene->intersect(lightRay, i)) {
		// return glm::dvec3(0.5,0.5,0.5);
	// }
	isect i;
	glm::dvec3 l = getDirection(p);
	ray lightRay(p, l, glm::dvec3(1,1,1), ray::SHADOW);
	// std::cout << p[0] << " " << p[1] << " " << p[2] << std::endl;
	if(thisscene->intersect(lightRay, i)) {
		// isect j;
		// glm::dvec3 l2 = glm::normalize(i.getP() - p);
		// ray lightRay2(i.getP(), l2, ray::SHADOW);
		// bool intersect = thisscene->intersect(lightRay2, j);
		double it = i.getT();
		if (it > RAY_EPSILON) {
			return glm::dvec3(0,0,0);
		}
		
		// std::cout << p_distance << " " << ip_distance << std::endl;
		
	}
	return glm::dvec3(1,1,1);
}

glm::dvec3 DirectionalLight::getColor() const
{
	return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3& P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3& P) const
{

	// YOUR CODE HERE
	
	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, we assume no attenuation and just return 1.0
	// glm::dvec3 rayFromLight = P - position;
	// std::cout << P[0] << " " << P[1] << " " << P[2] << std::endl;
	double distance = glm::abs(glm::distance(P, position));
	// std::cout << constantTerm << " " << linearTerm << " " << quadraticTerm << std::endl;
	// std::cout << distance << " ";
	double atten = glm::pow((constantTerm + linearTerm*distance + quadraticTerm*(glm::pow(distance, 2))), -1);
	// std::cout << atten << std::endl;
	if (atten < 0) atten = 0;
	atten = glm::min(atten, 1.0);
	return atten;
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	isect i;
	glm::dvec3 l = getDirection(p);
	ray lightRay(p, l, glm::dvec3(1,1,1), ray::SHADOW);
	// std::cout << p[0] << " " << p[1] << " " << p[2] << std::endl;
	if(thisscene->intersect(lightRay, i)) {
		
		double it = i.getT();
		if (it > RAY_EPSILON) {
			return glm::dvec3(0,0,0);
		}
		
		// std::cout << p_distance << " " << ip_distance << std::endl;
		
	}
	return glm::dvec3(1,1,1);
}

#define VERBOSE 0

