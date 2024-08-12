#include "QuaternionTransform.h"

Matrix QuaternionTransform::MakeAffineMatrix()
{
	Matrix result = Matrix::Identity();

	// SRTの順番で行列を生成してかける
	result *= Matrix::Scaling({ scale.x, scale.y, scale.z });
	result *= Matrix::QuaternionToRotation({ rotate.x, rotate.y, rotate.z, rotate.w });
	result *= Matrix::Translation({ translate.x, translate.y, translate.z });

	return result;
}
