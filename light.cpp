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
	
	// IMPLEMENT SHADOW ATTENUATION FOR TRANSLUCENT OBJECTS

	isect i;
	glm::dvec3 l = getDirection(p);
	ray lightRay(p, l, glm::dvec3(1,1,1), ray::SHADOW);
	// std::cout << p[0] << " " << p[1] << " " << p[2] << std::endl;
	if(thisscene->intersect(lightRay, i)) {
		return glm::dvec3(0,0,0);
		
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
	double distance = glm::distance(P, position);
	// std::cout << constantTerm << " " << linearTerm << " " << quadraticTerm << std::endl;
	// std::cout << distance << " ";
	// double attenC = (constantTerm + linearTerm*RAY_EPSILON + quadraticTerm*(glm::pow(RAY_EPSILON, 2)));
	double atten = glm::pow((constantTerm + linearTerm*distance + quadraticTerm*(glm::pow(distance, 2))), -1);
	//  atten = attenC/atten;
	// std::cout << atten << " " << attenC << std::endl;
	if (atten < 0) atten = 0;
	atten = glm::min(atten, 1.0);
	
	return atten;
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getPosition() const
{
	return position;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.


	// IMPLEMENT SHADOW ATTENUATION FOR TRANSLUCENT OBJECTS


	isect i;
	glm::dvec3 l = getDirection(p);
	ray lightRay(p, l, glm::dvec3(1,1,1), ray::SHADOW);
	// std::cout << p[0] << " " << p[1] << " " << p[2] << std::endl;
	if(thisscene->intersect(lightRay, i)) {
		glm::dvec3 iPos = p + i.getT() * l;
		if (!i.getMaterial().Trans()) {
			if (glm::distance(p, iPos) < glm::distance(p, position))
			return glm::dvec3(0,0,0);
		}
		
		
		
		glm::dvec3 aoeu(1,1,1);
		if(i.getMaterial().Trans()) {
			// std::cout << "trans ";
			// glm::dvec3 iNorm = i.getN();
			glm::dvec3 kT = i.getMaterial().kt(i);
			// glm::dvec3 kT= i.getMaterial().kt(i);
			
			double n1 = 0;
			double t = i.getT();
			glm::dvec3 p2 = p + t * l;
			lightRay = ray(p2, l, glm::dvec3(1,1,1), ray::SHADOW);
			i = isect();
			
			t = i.getT();
			p2 = p2 + t * l;
			lightRay = ray(p2, l, glm::dvec3(1,1,1), ray::SHADOW);
			i = isect();
			bool intersect = (scene->intersect(lightRay, i));
			t = i.getT();
			// while (t < RAY_EPSILON) {
			// 	i = isect();
			// 	p2 = p2 + t * l;
			// 	lightRay = ray(p2, l, glm::dvec3(1,1,1), ray::SHADOW);
			// 	i = isect();
			// 	scene->intersect(lightRay, i);
			// }
			std::cout << t << " " << std::endl;
			aoeu = glm::dvec3(glm::pow(kT[0], t), glm::pow(kT[1], t), glm::pow(kT[2], t));
			
			// p2 = p2 + t * l;
			// lightRay = ray(p2, l, glm::dvec3(1,1,1), ray::SHADOW);
			// i = isect();
			// bool intersect = (scene->intersect(lightRay, i));
			// t = i.getT();
			// while (t < RAY_EPSILON) {
			// 	i = isect();
			// 	p2 = p2 + t * l;
			// 	lightRay = ray(p2, l, glm::dvec3(1,1,1), ray::SHADOW);
			// 	i = isect();
			// 	intersect = scene->intersect(lightRay, i);
			// }
			return aoeu;
		}
		// std::cout << p_distance << " " << ip_distance << std::endl;
		
	}
	

	// Implement Shading Contribution for nonshadows
	
	return glm::dvec3(1,1,1);
}

#define VERBOSE 0

