#pragma once

#include<iostream>
#include "mMath.h"
#include "Obj.hpp"
#include "Global.h"
#include "Camera.hpp"

using namespace std;

namespace Trace {


	boolean intersect_Triangle(const Ray& ray, Intersection& intersection, Obj* object, float& minZ) {

		boolean find = false;
		Mesh* mesh = object->mesh;

		for (int i = 0; i < mesh->numFaces; i++) {

			Vector3i* pIndices = &mesh->vertexIndices[i];
			int index1 = pIndices->data[0], index3 = pIndices->data[1], index2 = pIndices->data[2];
			Vector3f P1 = matrix_mul(object->transform, matrix_mul(object->rotation, mesh->vertices[index1])),
				P2 = matrix_mul(object->transform, matrix_mul(object->rotation, mesh->vertices[index2])),
				P3 = matrix_mul(object->transform, matrix_mul(object->rotation, mesh->vertices[index3]));
			Vector3f E1 = ray.dir,
				E2 = P1 - P2,
				E3 = P1 - P3,
				E4 = P1 - ray.pos;
			float D1 = E4.x * E2.y * E3.z + E2.x * E3.y * E4.z + E3.x * E4.y * E2.z
				- E3.x * E2.y * E4.z - E2.x * E4.y * E3.z - E4.x * E3.y * E2.z;
			float D2 = E1.x * E4.y * E3.z + E4.x * E3.y * E1.z + E3.x * E1.y * E4.z
				- E3.x * E4.y * E1.z - E4.x * E1.y * E3.z - E1.x * E3.y * E4.z;
			float D3 = E1.x * E2.y * E4.z + E2.x * E4.y * E1.z + E4.x * E1.y * E2.z
				- E4.x * E2.y * E1.z - E2.x * E1.y * E4.z - E1.x * E4.y * E2.z;
			float D = E1.x * E2.y * E3.z + E2.x * E3.y * E1.z + E3.x * E1.y * E2.z
				- E3.x * E2.y * E1.z - E2.x * E1.y * E3.z - E1.x * E3.y * E2.z;
			float t = D1 / D;
			float lamda = D2 / D;
			float beita = D3 / D;
			//minZ约束离光线最近的
			if (t > c2z&& 0 <= lamda && lamda <= 1 && 0 < beita && beita <= 1 && lamda + beita <= 1 && t < minZ) {

				minZ = t;

				float alpha = 1 - lamda - beita;
				Vector3i* nIndices = &mesh->normalsIndices[i];

				intersection.pos = P1 * alpha + P2 * lamda + P3 * beita;
				intersection.normal = matrix_mul(object->rotation, mesh->normals[nIndices->x]) * alpha
					+ matrix_mul(object->rotation, mesh->normals[nIndices->y]) * beita 
					+ matrix_mul(object->rotation, mesh->normals[nIndices->z]) * lamda;
				/*intersect.texel = 
				intersect.tangent 
				intersect.biTangent = 
				*/

				find = true;
				//return true;
				/*if(twoFace >1)
					cout << "！！！！count " << count << "  i  " << i << "  t" << t << endl;*/
			}
		}

		return find;
	}


	boolean intersect(const Ray& ray, Intersection& intersection, Obj** object) {
		float minZ = FLT_MAX;
		boolean res = false;
		for (auto obj : objs) {
			if (!obj->boundingBox || obj->boundingBox->intersect(ray))
			{
				if (intersect_Triangle(ray, intersection, obj, minZ)) {
					*object = (Obj*)obj;
					res = true;
				}
			}
		}
		return res;
	}

	boolean shadowRay(Intersection& intersection, const Light* light) {
		Vector3f dir = light->pos - intersection.pos;
		Ray ray(intersection.pos, dir);//+ Vector3f(0, 0.01, 0)
		Intersection intersection2;
		Obj* object = nullptr;
		if (intersect(ray, intersection2, &object)) {
			// || intersection2.pos == intersection.pos
			/*if (intersection.pos.y>0) {
				showVector3(intersection.pos);
				showVector3(intersection2.pos);
				cout << "-----" << endl;
			}*/
			return true;
		}
		else
			return false;
	}

	//Vector3f shade(const Ray& ray, Intersection& intersection, const Light* light) {
	//	//材质选择相应  { 255,255,255 };//
	//	return intersection.obj->matarial->shader->shade(ray, intersection, light);
	//}

	Vector3f castRay(const Ray& ray, int times) {
		Intersection intersection;
		Obj* object = nullptr;
		Vector3f res = { 0, 0, 0 };
		if (intersect(ray, intersection, &object)) {
			//todo 遍历光源
			for (auto light : lights)
			{
				//阴影射线
				//if (shadowRay(intersection, light)) {
				//	//res += shade(ray, intersection, light);
				//}
				//else 
				if (true)//matarial
				{
					res += object->shader->shade(ray, intersection, light, object->matarial);//(ray, intersection, light);
				}

			}

			//间接光照

			//反射
			if (times < reflTimes && true) {
				Vector3f refl = reflection(intersection.normal, ray.dir);
				Ray ray2(intersection.pos, refl);
				Vector3f temp = castRay(ray2, ++times) * object->matarial->reflFactor;
				res += temp;
			}

			//todo: 折射
		}
		//todo 对光进行蒙特卡洛
		return res;
	}

	Ray generateRay(float j, float i, Camera& camera) {
		Ray ray;
		ray.pos = camera.pos;

		ray.dir = camera.pos_begin + camera.rightFactor * ((float)i / (float)window_width)
			- camera.downFactor * ((float)j / (float)window_height) - ray.pos;
		ray.dir.normalized();

		//showVector3(ray.pos);
		//showVector3(ray.dir);

		return ray;
	}
}

