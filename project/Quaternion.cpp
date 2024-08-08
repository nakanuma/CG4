#include "Quaternion.h"
#include "cmath"

Quaternion Quaternion::operator+(const Quaternion& other) const
{
    return { x + other.x, y + other.y, z + other.z, w + other.w };
}

Quaternion Quaternion::operator-(const Quaternion& other) const
{
    return { x - other.x, y - other.y, z - other.z, w - other.w };
}

Quaternion Quaternion::operator*(float scalar) const
{
    return { x * scalar, y * scalar, z * scalar, w * scalar };
}

Quaternion& Quaternion::operator+=(const Quaternion& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

Quaternion& Quaternion::operator-=(const Quaternion& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

Quaternion Quaternion::MakeRotateAxisAngleQuaternion(const Float3& axis, float angle)
{
    Float3 normalizedAxis = Float3::Normalize(axis);
    float halfAngle = angle * 0.5f;
    float sinHalfAngle = std::sin(halfAngle);

    float x = normalizedAxis.x * sinHalfAngle;
    float y = normalizedAxis.y * sinHalfAngle;
    float z = normalizedAxis.z * sinHalfAngle;
    float w = std::cos(halfAngle);

    return Quaternion(x, y, z, w);
}

Quaternion Quaternion::Slerp(const Quaternion& a, const Quaternion& b, float t)
{
    // コサイン角度の計算
    float dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

    // 補間のためにクォータニオンを反転
    const float DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD) {
        // クォータニオンが非常に近い場合は、線形補間を使用
        Quaternion result = a + t * (b - a);
        float len = std::sqrt(result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w);
        return result * (1.0f / len);
    }

    // コサイン角度の範囲内に収める
    if (dot < 0.0f) {
        dot = -dot;
        Quaternion bNegated = { -b.x, -b.y, -b.z, -b.w };
        return Slerp(a, bNegated, t);
    }

    // 角度の計算
    float theta_0 = std::acos(dot);
    float theta = theta_0 * t;

    // 補間クォータニオンの計算
    Quaternion c = b - a * dot;
    float len_c = std::sqrt(c.x * c.x + c.y * c.y + c.z * c.z + c.w * c.w);
    c = c * (1.0f / len_c);

    Quaternion result = a * std::cos(theta) + c * std::sin(theta);
    return result;
}

Quaternion operator*(float scalar, const Quaternion& quat)
{
    return { quat.x * scalar, quat.y * scalar, quat.z * scalar, quat.w * scalar };
}
