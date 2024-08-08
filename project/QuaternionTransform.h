#pragma once
#include "Float3.h"
#include "Quaternion.h"
#include "Matrix.h"

class QuaternionTransform
{
public:
	Float3 scale;
	Quaternion rotate;
	Float3 translate;

	Matrix MakeAffineMatrix();
};

