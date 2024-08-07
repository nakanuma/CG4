#pragma once
class Quaternion
{
public:
	float x, y, z, w;

	// コンストラクタ
	Quaternion(float x, float y, float z, float w) : w(w), x(x), y(y), z(z) {}
};

