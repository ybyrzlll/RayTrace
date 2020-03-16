#pragma once
#ifndef MESH_H
#define MESH_H

#include "mMath.h"
#include <vector>
#include <iostream>
#include"debugTool.h"
using namespace std;

class Mesh {
public:
	//Per vertex values
	int numVertices = 0;
	vector<Vector3f> vertices;
	vector<Vector3f> normals;
	vector<Vector3f> texels;
	vector<Vector3f> tangents;
	vector<Vector3f> biTangents;

	//Per face values
	int numFaces = 0;
	vector<Vector3f> fNormals; //Normals for the whole face
	vector<Vector3f> fGeneralEquation;//x,y,z,d=>A,B,C,D General Equation of Plane平面的一般方程
	vector<Vector3i> vertexIndices;
	vector<Vector3i> textureIndices;
	vector<Vector3i> normalsIndices;

	void buildFacet() {
		for (int i = 0; i < numFaces; i++) {
			Vector3i indices = vertexIndices[i];
			Vector3f N1 = vertices[indices.data[1]] - vertices[indices.data[0]];
			Vector3f N2 = vertices[indices.data[2]] - vertices[indices.data[0]];
			Vector3f N3 = N1.crossProduct(N2);
			N3.normalized();
			Vector3f GE= N3;
			GE.w = -vertices[indices.data[1]].x * N3.x - vertices[indices.data[1]].y * N3.y - vertices[indices.data[1]].x * N3.y;
			fGeneralEquation.push_back(GE);
			fNormals.push_back(N3);
			//showVector3(N3);
		}
	};

};

#endif