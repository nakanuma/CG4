#pragma once
#include <cmath>
#include <cassert>

class Matrix;

struct Float3
{
	float x;
	float y;
	float z;

    ///
    /// 二項演算子のオーバーロード
    /// 

    // 加算
    Float3 operator+(const Float3& other) const
    {
        return { x + other.x, y + other.y, z + other.z };
    }

    // 減算
    Float3 operator-(const Float3& other) const
    {
        return { x - other.x, y - other.y, z - other.z };
    }

    // 右辺のスカラーと乗算
    Float3 operator*(float scalar) const
    {
        return { x * scalar, y * scalar, z * scalar };
    }

    // 左辺のスカラーと乗算
    friend Float3 operator*(float scalar, const Float3& vec)
    {
        return { vec.x * scalar, vec.y * scalar, vec.z * scalar };
    }

    ///
    /// 複合代入演算子のオーバーロード
    /// 

    Float3& operator+=(const Float3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Float3& operator-=(const Float3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    // 正規化
    static Float3 Normalize(const Float3& a) {
        float length = std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
        return Float3(a.x / length, a.y / length, a.z / length);
    }

    // 線形補間
    static Float3 Lerp(const Float3& a, const Float3& b, float t)
    {
        return {
            a.x * (1.0f - t) + b.x * t,
            a.y * (1.0f - t) + b.y * t,
            a.z * (1.0f - t) + b.z * t,
        };
    }
};