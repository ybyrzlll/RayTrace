#pragma once

struct Matrix4//[列][行]
{
	float m[4][4];
	Matrix4() {
		for (int rows = 0; rows < 4; ++rows)
			for (int cols = 0; cols < 4; ++cols)
				m[rows][cols] = 0;
	};
	Matrix4 operator* (Matrix4& rhs) {
		//Matrix dimensions
		Matrix4 results;
		int n = 4;
		for (int rows = 0; rows < n; ++rows) {
			for (int cols = 0; cols < n; ++cols) {
				float total = 0;
				//total value of multiplication with all submultiplications added together
				//sub represents submultiplications in the actual matrix multiplication
				//For a nxn matrix you have n submultiplications
				for (int sub = 1; sub < n + 1; ++sub) {
					int rowLhs = rows; //row ind left matrix
					int colLhs = sub - 1; //col ind left matrix
					int rowRhs = sub - 1; //row ind right matrix
					int colRhs = cols; //col ind right matrix

					total += this->m[rowLhs][colLhs] * rhs.m[rowRhs][colRhs];
				}
				results.m[rows][cols] = total;
			}
		}
		return results;
	}
}; 

// y = x * m 能够用于自身
static Vector3f  matrix_mul(const Matrix4& m, const Vector3f& x) {
	float X = x.x, Y = x.y, Z = x.z, W = x.w;
	Vector3f y;
	y.x = X * m.m[0][0] + Y * m.m[1][0] + Z * m.m[2][0] + W * m.m[3][0];
	y.y = X * m.m[0][1] + Y * m.m[1][1] + Z * m.m[2][1] + W * m.m[3][1];
	y.z = X * m.m[0][2] + Y * m.m[1][2] + Z * m.m[2][2] + W * m.m[3][2];
	y.w = X * m.m[0][3] + Y * m.m[1][3] + Z * m.m[2][3] + W * m.m[3][3];
	return y;
}

// 旋转矩阵
static void matrix_set_rotate(Matrix4* m, Vector3f dir, float theta) {
	float x = dir.x, y = dir.y, z = dir.z;
	float qsin = (float)sin(theta * 0.5f);
	float qcos = (float)cos(theta * 0.5f);
	Vector3f vec{ x, y, z };
	float w = qcos;
	vec.normalized();
	x = vec.x * qsin;
	y = vec.y * qsin;
	z = vec.z * qsin;
	m->m[0][0] = 1 - 2 * y * y - 2 * z * z;
	m->m[1][0] = 2 * x * y - 2 * w * z;
	m->m[2][0] = 2 * x * z + 2 * w * y;
	m->m[0][1] = 2 * x * y + 2 * w * z;
	m->m[1][1] = 1 - 2 * x * x - 2 * z * z;
	m->m[2][1] = 2 * y * z - 2 * w * x;
	m->m[0][2] = 2 * x * z - 2 * w * y;
	m->m[1][2] = 2 * y * z + 2 * w * x;
	m->m[2][2] = 1 - 2 * x * x - 2 * y * y;
	m->m[0][3] = m->m[1][3] = m->m[2][3] = 0.0f;
	m->m[3][0] = m->m[3][1] = m->m[3][2] = 0.0f;
	m->m[3][3] = 1.0f;
}

