#pragma once

#include <cmath>

typedef unsigned char UInt8;        //!< An unsigned 8-bit integer value
typedef unsigned short UInt16;      //!< An unsigned 16-bit integer value
typedef unsigned long UInt32;       //!< An unsigned 32-bit integer value
typedef unsigned long long UInt64;  //!< An unsigned 64-bit integer value
typedef signed char SInt8;          //!< A signed 8-bit integer value
typedef signed short SInt16;        //!< A signed 16-bit integer value
typedef signed long SInt32;         //!< A signed 32-bit integer value
typedef signed long long SInt64;    //!< A signed 64-bit integer value
typedef float Float32;              //!< A 32-bit floating point value
typedef double Float64;             //!< A 64-bit floating point value

inline UInt32 Extend16(UInt32 in)
{
	return (in & 0x8000) ? (0xFFFF0000 | in) : in;
}

inline UInt32 Extend8(UInt32 in)
{
	return (in & 0x80) ? (0xFFFFFF00 | in) : in;
}

inline UInt16 Swap16(UInt16 in)
{
	return ((in >> 8) & 0x00FF) |
	       ((in << 8) & 0xFF00);
}

inline UInt32 Swap32(UInt32 in)
{
	return ((in >> 24) & 0x000000FF) |
	       ((in >> 8) & 0x0000FF00) |
	       ((in << 8) & 0x00FF0000) |
	       ((in << 24) & 0xFF000000);
}

inline void SwapFloat(float* in)
{
	UInt32* temp = (UInt32*)in;

	*temp = Swap32(*temp);
}

const float kFloatEpsilon = 0.0001f;

inline bool FloatEqual(float a, float b)
{
	float magnitude = a - b;
	if (magnitude < 0)
		magnitude = -magnitude;
	return magnitude < kFloatEpsilon;
}

class Vector2
{
public:
	Vector2() {}
	Vector2(const Vector2& in)
	{
		x = in.x;
		y = in.y;
	}
	Vector2(float inX, float inY)
	{
		x = inX;
		y = inY;
	}
	~Vector2() {}

	void Set(float inX, float inY)
	{
		x = inX;
		y = inY;
	}
	void SetX(float inX) { x = inX; }
	void SetY(float inY) { y = inY; }
	void Get(float* outX, float* outY)
	{
		*outX = x;
		*outY = y;
	}
	float GetX(void) { return x; }
	float GetY(void) { return y; }

	void Normalize(void)
	{
		float mag = Magnitude();
		x /= mag;
		y /= mag;
	}
	float Magnitude(void) { return sqrt(x * x + y * y); }

	void Reverse(void)
	{
		float temp = -x;
		x = -y;
		y = temp;
	}

	void Scale(float scale)
	{
		x *= scale;
		y *= scale;
	}

	void SwapBytes(void)
	{
		SwapFloat(&x);
		SwapFloat(&y);
	}

	Vector2& operator+=(const Vector2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
	Vector2& operator-=(const Vector2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}
	Vector2& operator*=(float rhs)
	{
		x *= rhs;
		y *= rhs;
		return *this;
	}
	Vector2& operator/=(float rhs)
	{
		x /= rhs;
		y /= rhs;
		return *this;
	}

	float x;
	float y;
};

inline Vector2 operator+(const Vector2& lhs, const Vector2& rhs)
{
	return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
};

inline Vector2 operator-(const Vector2& lhs, const Vector2& rhs)
{
	return Vector2(lhs.x - rhs.x, lhs.y - rhs.y);
};

inline Vector2 operator*(const Vector2& lhs, float rhs)
{
	return Vector2(lhs.x * rhs, lhs.y * rhs);
};

inline Vector2 operator/(const Vector2& lhs, float rhs)
{
	return Vector2(lhs.x / rhs, lhs.y / rhs);
};

inline bool MaskCompare(void* lhs, void* rhs, void* mask, UInt32 size)
{
	UInt8* lhs8 = (UInt8*)lhs;
	UInt8* rhs8 = (UInt8*)rhs;
	UInt8* mask8 = (UInt8*)mask;

	for (UInt32 i = 0; i < size; i++)
		if ((lhs8[i] & mask8[i]) != (rhs8[i] & mask8[i]))
			return false;

	return true;
}

class Vector3
{
public:
	Vector3() {}
	Vector3(const Vector3& in)
	{
		x = in.x;
		y = in.y;
		z = in.z;
	}
	Vector3(float inX, float inY, float inZ)
	{
		x = inX;
		y = inY;
		z = inZ;
	}
	~Vector3() {}

	void Set(float inX, float inY, float inZ)
	{
		x = inX;
		y = inY;
		z = inZ;
	}
	void Get(float* outX, float* outY, float* outZ)
	{
		*outX = x;
		*outY = y;
		*outZ = z;
	}

	void Normalize(void)
	{
		float mag = Magnitude();
		x /= mag;
		y /= mag;
		z /= mag;
	}
	float Magnitude(void) { return sqrt(x * x + y * y + z * z); }

	void Scale(float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
	}

	void SwapBytes(void)
	{
		SwapFloat(&x);
		SwapFloat(&y);
		SwapFloat(&z);
	}

	Vector3& operator+=(const Vector3& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
	Vector3& operator-=(const Vector3& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}
	Vector3& operator*=(const Vector3& rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}
	Vector3& operator/=(const Vector3& rhs)
	{
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		return *this;
	}

	union
	{
		struct
		{
			float x, y, z;
		};
		float d[3];
	};
};

inline Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

inline Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

inline Vector3 operator*(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

inline Vector3 operator/(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
}
