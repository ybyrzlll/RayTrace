#pragma once

#include<iostream>
#include "mMath.h"
#include "Intersect.h"
#include "light.h"
#include "Shader.h"

using namespace std;

namespace Trace {


	boolean intersect_Triangle(const Ray& ray, Intersection& intersection, Mesh* mesh) {

		boolean find = false;
		float minZ = FLT_MAX;

		for (int i = 0; i < mesh->numFaces; i++) {
			

			////求交点
			//Vector3f GE = cuboid.fGeneralEquation[i];
			//float t = (-GE.w - GE.x * ray.pos.x - GE.y * ray.pos.y - GE.z * ray.pos.z) /
			//	(GE.x * ray.dir.x + GE.y * ray.dir.y + GE.z * ray.dir.z);
			//if (t < 0) continue;
			//Vector3f intersect_point = ray.dir * t + ray.pos;

			Vector3i* pIndices = &mesh->vertexIndices[i];
			int index1 = pIndices->data[0], index3 = pIndices->data[1], index2 = pIndices->data[2];
			Vector3f E1 = ray.dir;
			Vector3f E2 = mesh->vertices[index1] - mesh->vertices[index2];
			Vector3f E3 = mesh->vertices[index1] - mesh->vertices[index3];
			Vector3f E4 = mesh->vertices[index1] - ray.pos;
			float D1 = E4.x * E2.y * E3.z + E2.x * E3.y * E4.z + E3.x * E4.y * E2.z
				- E3.x * E2.y * E4.z - E2.x * E4.y * E3.z - E4.x * E3.y * E2.z;
			float D2 = E1.x * E4.y * E3.z + E4.x * E3.y * E1.z + E3.x * E1.y * E4.z
				- E3.x * E4.y * E1.z - E4.x * E1.y * E3.z - E1.x * E3.y * E4.z;
			float D3 = E1.x * E2.y * E4.z + E2.x * E4.y * E1.z + E4.x * E1.y * E2.z
				- E4.x * E2.y * E1.z - E2.x * E1.y * E4.z - E1.x * E4.y * E2.z;
			float D = E1.x * E2.y * E3.z + E2.x * E3.y * E1.z + E3.x * E1.y * E2.z
				- E3.x * E2.y * E1.z - E2.x * E1.y * E3.z - E1.x * E3.y * E2.z;
			float t = D1;
			float lamda = D2 / D;
			float beita = D3 / D;
			//t<minZ 深度剔除 t<minZ &&
			if ( t > c2z&& c2z < lamda && lamda < 1 && c2z < beita && beita < 1 && lamda + beita < 1) {
				
				minZ = t;
				
				float alpha = 1 - lamda - beita;
				Vector3i* nIndices = &mesh->normalsIndices[i];

				intersection.pos = mesh->vertices[index1] * alpha + mesh->vertices[index2] * lamda + mesh->vertices[index3] * beita;
				intersection.normal = mesh->normals[nIndices->x] * alpha + mesh->normals[nIndices->y] * lamda + mesh->normals[nIndices->z] * beita;
				/*intersect.texel = cuboid.texels[index1] * t + cuboid.texels[index2] * lamda + cuboid.texels[index3] * beita;
				intersect.tangent = cuboid.tangents[index1] * t + cuboid.tangents[index2] * lamda + cuboid.tangents[index3] * beita;
				intersect.biTangent = cuboid.biTangents[index1] * t + cuboid.biTangents[index2] * lamda + cuboid.biTangents[index3] * beita;
				*/

				//find = true;
				return true;
				/*if(twoFace >1)
					cout << "！！！！count " << count << "  i  " << i << "  t" << t << endl;*/
			}
		}

		return find;
	}


	boolean intersect(const Ray& ray, Intersection& intersection) {
		for (Mesh* mesh : meshs) {
			//TODO: 对比深度
			if (intersect_Triangle(ray, intersection, mesh))
				return true;
		}
		return false;
	}

	boolean shadow(const Ray& ray, const Light* light) {
		return false;
	}

	Vector3f shade(const Ray& ray, Intersection& intersection, const Light* light) {
		//材质选择相应  { 255,255,255 };//
		return Lambert::shade(ray, intersection, light);
	}

	Vector3f castRay(const Ray& ray, int times) {
		Intersection intersection;
		if (intersect(ray, intersection)) {
			//todo 遍历光源
			Vector3f res = { 0, 0, 0 };
			for (auto light : lights) 
			{
				if (shadow(ray, light)) {

				}
				if (true)//matarial
				{
					res += shade(ray, intersection, light);
				}
				//反射
				if (times< reflTimes && true) {
					Vector3f refl = reflection(intersection.normal, ray.dir);
					Ray ray2;
					ray2.pos = intersection.pos;
					ray2.dir = refl;
					res += castRay(ray2, ++times) * reflFactor;
				}

				//todo: 折射
			
				
			}
			//todo 对光进行蒙特卡洛
			return res;
		}
		else
			return { 0, 0, 0 };
	}
}

