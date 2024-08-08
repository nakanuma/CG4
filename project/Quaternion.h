#pragma once
#include "Float3.h"

class Quaternion
{
public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	// コンストラクタ
	Quaternion() = default;
	Quaternion(float x, float y, float z, float w) : w(w), x(x), y(y), z(z) {}

	///
	///	二項演算子のオーバーロード
	/// 

	// 加算
	Quaternion operator+(const Quaternion& other) const;
	// 減算
	Quaternion operator-(const Quaternion& other) const;
	// 右辺のスカラーと乗算
	Quaternion operator*(float scalar) const;
	// 左辺のスカラーと乗算
	friend Quaternion operator*(float scalar, const Quaternion& quat);

	///
	///	複合代入演算子のオーバーロード
	/// 

	Quaternion& operator+=(const Quaternion& other);
	Quaternion& operator-=(const Quaternion& other);

	// 任意軸回転を表すQuaternionの生成
	static Quaternion MakeRotateAxisAngleQuaternion(const Float3& axis, float angle);
	// 球面線形補間
	static Quaternion Slerp(const Quaternion& a, const Quaternion& b, float t);
};

