#include "Matrix.h"
#include <math.h>
#include <stdexcept>

Matrix::Matrix()
{
	r[0][0] = 1.0f;
	r[0][1] = 0.0f;
	r[0][2] = 0.0f;
	r[0][3] = 0.0f;

	r[1][0] = 0.0f;
	r[1][1] = 1.0f;
	r[1][2] = 0.0f;
	r[1][3] = 0.0f;

	r[2][0] = 0.0f;
	r[2][1] = 0.0f;
	r[2][2] = 1.0f;
	r[2][3] = 0.0f;

	r[3][0] = 0.0f;
	r[3][1] = 0.0f;
	r[3][2] = 0.0f;
	r[3][3] = 1.0f;
}

Matrix::Matrix(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	r[0][0] = m00;
	r[0][1] = m01;
	r[0][2] = m02;
	r[0][3] = m03;

	r[1][0] = m10;
	r[1][1] = m11;
	r[1][2] = m12;
	r[1][3] = m13;

	r[2][0] = m20;
	r[2][1] = m21;
	r[2][2] = m22;
	r[2][3] = m23;

	r[3][0] = m30;
	r[3][1] = m31;
	r[3][2] = m32;
	r[3][3] = m33;
}

Matrix Matrix::operator-() const
{
	Matrix result;
	float temp[4][8] = {};

	float a;

	// 一時行列にコピー
	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			temp[i][j] = r[i][j];

			if (i == j)temp[i][4 + j] = 1;
		}
	}

	for (int32_t k = 0; k < 4; k++) {
		a = 1 / temp[k][k];

		for (int32_t j = 0; j < 8; j++) {
			temp[k][j] *= a;
		}

		for (int32_t i = 0; i < 4; i++) {
			if (i == k) {
				continue;
			}

			a = -temp[i][k];

			for (int32_t j = 0; j < 8; j++) {
				temp[i][j] += temp[k][j] * a;
			}
		}
	}

	for (int32_t i = 0; i < 4; i++) {
		for (int32_t j = 0; j < 4; j++) {
			result.r[i][j] = temp[i][4 + j];
		}
	}
	return result;
}

Matrix Matrix::operator+(const Matrix& m) const
{
	return Matrix(
		r[0][0] + m.r[0][0],
		r[0][1] + m.r[0][1],
		r[0][2] + m.r[0][2],
		r[0][3] + m.r[0][3],

		r[1][0] + m.r[1][0],
		r[1][1] + m.r[1][1],
		r[1][2] + m.r[1][2],
		r[1][3] + m.r[1][3],

		r[2][0] + m.r[2][0],
		r[2][1] + m.r[2][1],
		r[2][2] + m.r[2][2],
		r[2][3] + m.r[2][3],

		r[3][0] + m.r[3][0],
		r[3][1] + m.r[3][1],
		r[3][2] + m.r[3][2],
		r[3][3] + m.r[3][3]
	);
}

Matrix Matrix::operator-(const Matrix& m) const
{
	return Matrix(
		r[0][0] - m.r[0][0],
		r[0][1] - m.r[0][1],
		r[0][2] - m.r[0][2],
		r[0][3] - m.r[0][3],

		r[1][0] - m.r[1][0],
		r[1][1] - m.r[1][1],
		r[1][2] - m.r[1][2],
		r[1][3] - m.r[1][3],

		r[2][0] - m.r[2][0],
		r[2][1] - m.r[2][1],
		r[2][2] - m.r[2][2],
		r[2][3] - m.r[2][3],

		r[3][0] - m.r[3][0],
		r[3][1] - m.r[3][1],
		r[3][2] - m.r[3][2],
		r[3][3] - m.r[3][3]
	);
}

Matrix Matrix::operator*(const Matrix& m) const
{
	return Matrix(
		r[0][0] * m.r[0][0] + r[0][1] * m.r[1][0] + r[0][2] * m.r[2][0] + r[0][3] * m.r[3][0],
		r[0][0] * m.r[0][1] + r[0][1] * m.r[1][1] + r[0][2] * m.r[2][1] + r[0][3] * m.r[3][1],
		r[0][0] * m.r[0][2] + r[0][1] * m.r[1][2] + r[0][2] * m.r[2][2] + r[0][3] * m.r[3][2],
		r[0][0] * m.r[0][3] + r[0][1] * m.r[1][3] + r[0][2] * m.r[2][3] + r[0][3] * m.r[3][3],

		r[1][0] * m.r[0][0] + r[1][1] * m.r[1][0] + r[1][2] * m.r[2][0] + r[1][3] * m.r[3][0],
		r[1][0] * m.r[0][1] + r[1][1] * m.r[1][1] + r[1][2] * m.r[2][1] + r[1][3] * m.r[3][1],
		r[1][0] * m.r[0][2] + r[1][1] * m.r[1][2] + r[1][2] * m.r[2][2] + r[1][3] * m.r[3][2],
		r[1][0] * m.r[0][3] + r[1][1] * m.r[1][3] + r[1][2] * m.r[2][3] + r[1][3] * m.r[3][3],

		r[2][0] * m.r[0][0] + r[2][1] * m.r[1][0] + r[2][2] * m.r[2][0] + r[2][3] * m.r[3][0],
		r[2][0] * m.r[0][1] + r[2][1] * m.r[1][1] + r[2][2] * m.r[2][1] + r[2][3] * m.r[3][1],
		r[2][0] * m.r[0][2] + r[2][1] * m.r[1][2] + r[2][2] * m.r[2][2] + r[2][3] * m.r[3][2],
		r[2][0] * m.r[0][3] + r[2][1] * m.r[1][3] + r[2][2] * m.r[2][3] + r[2][3] * m.r[3][3],

		r[3][0] * m.r[0][0] + r[3][1] * m.r[1][0] + r[3][2] * m.r[2][0] + r[3][3] * m.r[3][0],
		r[3][0] * m.r[0][1] + r[3][1] * m.r[1][1] + r[3][2] * m.r[2][1] + r[3][3] * m.r[3][1],
		r[3][0] * m.r[0][2] + r[3][1] * m.r[1][2] + r[3][2] * m.r[2][2] + r[3][3] * m.r[3][2],
		r[3][0] * m.r[0][3] + r[3][1] * m.r[1][3] + r[3][2] * m.r[2][3] + r[3][3] * m.r[3][3]
	);
}

Matrix& Matrix::operator+=(const Matrix& m)
{
	r[0][0] += m.r[0][0];
	r[0][1] += m.r[0][1];
	r[0][2] += m.r[0][2];
	r[0][3] += m.r[0][3];

	r[1][0] += m.r[1][0];
	r[1][1] += m.r[1][1];
	r[1][2] += m.r[1][2];
	r[1][3] += m.r[1][3];

	r[2][0] += m.r[2][0];
	r[2][1] += m.r[2][1];
	r[2][2] += m.r[2][2];
	r[2][3] += m.r[2][3];

	r[3][0] += m.r[3][0];
	r[3][1] += m.r[3][1];
	r[3][2] += m.r[3][2];
	r[3][3] += m.r[3][3];

	return *this;
}

Matrix& Matrix::operator-=(const Matrix& m)
{
	r[0][0] -= m.r[0][0];
	r[0][1] -= m.r[0][1];
	r[0][2] -= m.r[0][2];
	r[0][3] -= m.r[0][3];

	r[1][0] -= m.r[1][0];
	r[1][1] -= m.r[1][1];
	r[1][2] -= m.r[1][2];
	r[1][3] -= m.r[1][3];

	r[2][0] -= m.r[2][0];
	r[2][1] -= m.r[2][1];
	r[2][2] -= m.r[2][2];
	r[2][3] -= m.r[2][3];

	r[3][0] -= m.r[3][0];
	r[3][1] -= m.r[3][1];
	r[3][2] -= m.r[3][2];
	r[3][3] -= m.r[3][3];

	return *this;
}

Matrix& Matrix::operator*=(const Matrix& m)
{
	*this = *this * m;
	return *this;
}

Matrix Matrix::Identity()
{
	return Matrix();
}

Matrix Matrix::Inverse(Matrix m)
{
	return -m;
}

Matrix Matrix::Transpose(const Matrix& m)
{
	Matrix result;

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.r[i][j] = m.r[j][i];
		}
	}

	return result;
}

Matrix Matrix::PerspectiveFovLH(float fov, float aspectRatio, float nearZ, float farZ)
{
	Matrix result = Matrix();

	result.r[1][1] = 1 / tanf(fov / 2);
	result.r[0][0] = result.r[1][1] / aspectRatio;
	result.r[2][2] = farZ / (farZ - nearZ);
	result.r[3][2] = (farZ * -nearZ) / (farZ - nearZ);
	result.r[2][3] = 1;
	result.r[3][3] = 0;

	return result;
}

Matrix Matrix::Orthographic(float width, float height, float nearClip, float farClip)
{
	return Matrix(
		2.0f / width, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / -height, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f / (farClip - nearClip), 0.0f,
		-1, 1, nearClip / (nearClip - farClip), 1.0f // 左上を原点にする
	);
}

Matrix Matrix::Scaling(Float3 scale)
{
	Matrix ret = Matrix();
	ret.r[0][0] = scale.x;
	ret.r[1][1] = scale.y;
	ret.r[2][2] = scale.z;

	return ret;
}

Matrix Matrix::Translation(Float3 translation)
{
	Matrix ret = Matrix();
	ret.r[3][0] = translation.x;
	ret.r[3][1] = translation.y;
	ret.r[3][2] = translation.z;

	return ret;
}

Matrix Matrix::RotationX(float rad)
{
	return Pitch(rad);
}

Matrix Matrix::RotationY(float rad)
{
	return Yaw(rad);
}

Matrix Matrix::RotationZ(float rad)
{
	return Roll(rad);
}

Matrix Matrix::Pitch(float rad)
{
	Matrix result = Matrix();

	result.r[1][1] = cosf(rad);
	result.r[2][1] = -sinf(rad);
	result.r[1][2] = sinf(rad);
	result.r[2][2] = cosf(rad);

	return result;
}

Matrix Matrix::Yaw(float rad)
{
	Matrix result = Matrix();

	result.r[0][0] = cosf(rad);
	result.r[0][2] = -sinf(rad);
	result.r[2][0] = sinf(rad);
	result.r[2][2] = cosf(rad);

	return result;
}

Matrix Matrix::Roll(float rad)
{
	Matrix result = Matrix();

	result.r[0][0] = cosf(rad);
	result.r[1][0] = -sinf(rad);
	result.r[0][1] = sinf(rad);
	result.r[1][1] = cosf(rad);

	return result;
}

Matrix Matrix::RotationRollPitchYaw(float roll, float pitch, float yaw)
{
	Matrix result = Matrix::Identity() * Roll(roll) * Pitch(pitch) * Yaw(yaw);
	return result;
}

Matrix Matrix::QuaternionToRotation(Quaternion q)
{
	Matrix result;

	result.r[0][0] = (q.w * q.w) + (q.x * q.x) - (q.y * q.y) - (q.z * q.z);
	result.r[0][1] = 2.0f * ((q.x * q.y) + (q.w * q.z));
	result.r[0][2] = 2.0f * ((q.x * q.z) - (q.w * q.y));
	result.r[0][3] = 0.0f;

	result.r[1][0] = 2.0f * ((q.x * q.y) - (q.w * q.z));
	result.r[1][1] = (q.w * q.w) - (q.x * q.x) + (q.y * q.y) - (q.z * q.z);
	result.r[1][2] = 2.0f * ((q.y * q.z) + (q.w * q.x));
	result.r[1][3] = 0.0f;

	result.r[2][0] = 2.0f * ((q.x * q.z) + (q.w * q.y));
	result.r[2][1] = 2.0f * ((q.y * q.z) - (q.w * q.x));
	result.r[2][2] = (q.w * q.w) - (q.x * q.x) - (q.y * q.y) + (q.z * q.z);
	result.r[2][3] = 0.0f;

	result.r[3][0] = 0.0f;
	result.r[3][1] = 0.0f;
	result.r[3][2] = 0.0f;
	result.r[3][3] = 1.0f;

	return result;
}

Matrix Matrix::MakeAffine(const Float3& scale, const Quaternion& rotate, const Float3 translate)
{
	Matrix result = Matrix();

	result *= Matrix::Scaling({ scale.x, scale.y, scale.z });
	result *= Matrix::QuaternionToRotation({ rotate.x, rotate.y, rotate.z, rotate.w });
	result *= Matrix::Translation({ translate.x, translate.y, translate.z });

	return result;
}
