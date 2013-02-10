/*****************************************************************************************

Forever War - a NetHack-like FPS

Copyright (C) 2008 Thomas Schöps

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, see <http://www.gnu.org/licenses/>.

*****************************************************************************************/

#ifndef _POINT_3_H_
#define _POINT_3_H_

namespace Ogre
{
	class Vector3;
}

/// A point in 3-dimensional space with integer coordinates
class Point3
{
public:
	int x;
	int y;
	int z;

	Point3() {}
	Point3(int nx, int ny, int nz) :
			x(nx), y(ny), z(nz) {};
	Point3(Ogre::Vector3& vec) :
			x(vec.x + 0.5f), y(vec.y + 0.5f), z(vec.z + 0.5f) {};

    inline int& operator [] ( const size_t i )
    {
        assert( i < 3 );

        return *(&x+i);
    }

	inline Point3& operator = ( const Point3& rkVector )
	{
		x = rkVector.x;
		y = rkVector.y;
		z = rkVector.z;

		return *this;
	}

	inline Point3& operator = ( const int fScaler )
	{
		x = fScaler;
		y = fScaler;
		z = fScaler;

		return *this;
	}

	inline bool operator == ( const Point3& rkVector ) const
	{
		return ( x == rkVector.x && y == rkVector.y && z == rkVector.z );
	}

	inline bool operator != ( const Point3& rkVector ) const
	{
		return ( x != rkVector.x || y != rkVector.y || z != rkVector.z );
	}

        // arithmetic operations
	inline Point3 operator + ( const Point3& rkVector ) const
	{
		return Point3(x + rkVector.x, y + rkVector.y, z + rkVector.z);
	}

	inline Point3 operator - ( const Point3& rkVector ) const
	{
		return Point3(x - rkVector.x, y - rkVector.y, z - rkVector.z);
	}

	inline Point3 operator * ( const float fScalar ) const
	{
		return Point3(x * fScalar, y * fScalar, z * fScalar);
	}

	inline Point3 operator * ( const Point3& rhs) const
	{
		return Point3(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	inline Point3 operator / ( const float fScalar ) const
	{
		assert( fScalar != 0.0 );

		float fInv = 1.0 / fScalar;

		return Point3(x * fInv, y * fInv, z * fInv);
	}

	inline Point3 operator / ( const Point3& rhs) const
	{
		return Point3(x / rhs.x, y / rhs.y, z / rhs.z);
	}

	inline const Point3& operator + () const
	{
		return *this;
	}

	inline Point3 operator - () const
	{
		return Point3(-x, -y, -z);
	}

        // overloaded operators to help Point3
	inline friend Point3 operator * ( const float fScalar, const Point3& rkVector )
	{
		return Point3(fScalar * rkVector.x,
					   fScalar * rkVector.y,
		fScalar * rkVector.z);
	}

	inline friend Point3 operator / ( const float fScalar, const Point3& rkVector )
	{
		return Point3(fScalar / rkVector.x,
					   fScalar / rkVector.y,
		fScalar / rkVector.z);
	}

	inline friend Point3 operator + (const Point3& lhs, const int rhs)
	{
		return Point3(lhs.x + rhs,
					   lhs.y + rhs,
		lhs.z + rhs);
	}

	inline friend Point3 operator + (const float lhs, const Point3& rhs)
	{
		return Point3(lhs + rhs.x,
					   lhs + rhs.y,
		lhs + rhs.z);
	}

	inline friend Point3 operator - (const Point3& lhs, const int rhs)
	{
		return Point3(lhs.x - rhs,
					   lhs.y - rhs,
		lhs.z - rhs);
	}

	inline friend Point3 operator - (const float lhs, const Point3& rhs)
	{
		return Point3(lhs - rhs.x,
					   lhs - rhs.y,
		lhs - rhs.z);
	}

    // arithmetic updates
	inline Point3& operator += ( const Point3& rkVector )
	{
		x += rkVector.x;
		y += rkVector.y;
		z += rkVector.z;

		return *this;
	}

	inline Point3& operator += ( const int fScalar )
	{
		x += fScalar;
		y += fScalar;
		z += fScalar;
		return *this;
	}

	inline Point3& operator -= ( const Point3& rkVector )
	{
		x -= rkVector.x;
		y -= rkVector.y;
		z -= rkVector.z;

		return *this;
	}

	inline Point3& operator -= ( const int fScalar )
	{
		x -= fScalar;
		y -= fScalar;
		z -= fScalar;
		return *this;
	}

	inline Point3& operator *= ( const float fScalar )
	{
		x *= fScalar;
		y *= fScalar;
		z *= fScalar;
		return *this;
	}

	inline Point3& operator *= ( const Point3& rkVector )
	{
		x *= rkVector.x;
		y *= rkVector.y;
		z *= rkVector.z;

		return *this;
	}

	inline Point3& operator /= ( const float fScalar )
	{
		assert( fScalar != 0.0 );

		float fInv = 1.0 / fScalar;

		x *= fInv;
		y *= fInv;
		z *= fInv;

		return *this;
	}

	inline Point3& operator /= ( const Point3& rkVector )
	{
		x /= rkVector.x;
		y /= rkVector.y;
		z /= rkVector.z;

		return *this;
	}

	inline float length () const
	{
		return Math::Sqrt( x * x + y * y + z * z );
	}

	inline float squaredLength () const
	{
		return x * x + y * y + z * z;
	}

	inline float distance(const Point3& rhs) const
	{
		return (*this - rhs).length();
	}

	inline float squaredDistance(const Point3& rhs) const
	{
		return (*this - rhs).squaredLength();
	}

	inline float dotProduct(const Point3& vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z;
	}

	inline float absDotProduct(const Point3& vec) const
	{
		return Math::Abs(x * vec.x) + Math::Abs(y * vec.y) + Math::Abs(z * vec.z);
	}
};

#endif
