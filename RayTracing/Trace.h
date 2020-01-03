#pragma once

#include<iostream>
#include "mMath.h"
#include "Intersect.h"
#include "light.h"
#include "Shader.h"

using namespace std;

namespace Trace {
	boolean intersect_Triangle(const Ray& ray, Intersect& intersect) {

		boolean find = false;
		float minZ = FLT_MAX;

		static int count = 0;
		count++;

		for (int i = 0; i < cuboid.numFaces; i++) {
			

			////求交点
			//Vector3f GE = cuboid.fGeneralEquation[i];
			//float t = (-GE.w - GE.x * ray.pos.x - GE.y * ray.pos.y - GE.z * ray.pos.z) /
			//	(GE.x * ray.dir.x + GE.y * ray.dir.y + GE.z * ray.dir.z);
			//if (t < 0) continue;
			//Vector3f intersect_point = ray.dir * t + ray.pos;

			Vector3i* pIndices = &cuboid.vertexIndices[i];
			int index1 = pIndices->data[0], index2 = pIndices->data[1], index3 = pIndices->data[2];
			Vector3f E1 = ray.dir;
			Vector3f E2 = cuboid.vertices[index1] - cuboid.vertices[index2];
			Vector3f E3 = cuboid.vertices[index1] - cuboid.vertices[index3];
			Vector3f E4 = cuboid.vertices[index1] - ray.pos;
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
				/*if(i<8 && i!=0 && i!=1)*/
					//cout <<"count "<<count<<"  i  "<<i<<"  t"<< t << endl;

				

				float alpha = 1 - lamda - beita;
				Vector3i* nIndices = &cuboid.normalsIndices[i];

				intersect.pos = cuboid.vertices[index1] * alpha + cuboid.vertices[index2] * lamda + cuboid.vertices[index3] * beita;
				intersect.normal = cuboid.normals[nIndices->x] * alpha + cuboid.normals[nIndices->y] * lamda + cuboid.normals[nIndices->z] * beita;
				/*intersect.texel = cuboid.texels[index1] * t + cuboid.texels[index2] * lamda + cuboid.texels[index3] * beita;
				intersect.tangent = cuboid.tangents[index1] * t + cuboid.tangents[index2] * lamda + cuboid.tangents[index3] * beita;
				intersect.biTangent = cuboid.biTangents[index1] * t + cuboid.biTangents[index2] * lamda + cuboid.biTangents[index3] * beita;
				*/

				find = true;
			}
		}

		//背面剔除
		//if (intersect.normal.dot(ray.dir) > 0) {
		//	//showVector3(cuboid.fNormals[i]);
		//	find = false;
		//}

		return find;
	}

	boolean shadow(const Ray& ray, const Light* light) {
		return false;
	}

	Vector3f shade(const Ray& ray, Intersect& intersect, const Light* light) {
		//材质选择相应
		return Lambert::shade(ray, intersect, light);
	}

	Vector3f castRay(const Ray& ray) {
		Intersect intersect;
		if (intersect_Triangle(ray, intersect)) {
			//todo 遍历光源
			Vector3f res = { 0, 0, 0 };
			for (auto light : lights) 
			{
				if (shadow(ray, light)) {

				}
				if (true)//matarial
				{
					res += shade(ray, intersect, light);
				}
				//反射
				/*if (true) {
					Vector3f refl = reflection(intersect.normal, ray.dir);
					Ray ray2;
					ray2.pos = intersect.pos;
					ray2.dir = refl;
					res += castRay(ray2) * reflFactor;
				}*/

				//todo: 折射
			
				
			}
			//todo 对光进行蒙特卡洛
			return res;
		}
		else
			return { 0, 0, 0 };
	}
}

