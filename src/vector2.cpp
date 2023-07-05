// Oleg Kotov

#include "vector2.h"
#include "mathutils.h"

#include <math.h>

Vector2::Vector2()
{
	x = 0.0f;
	y = 0.0f;
}

Vector2::Vector2( float x, float y )
{
	this->x = x;
	this->y = y;
}

Vector2 Vector2::operator+( const Vector2& vec ) const
{
	Vector2 result;

	result.x = x + vec.x;
	result.y = y + vec.y;

	return result;
}

Vector2 Vector2::operator+=( const Vector2& other )
{
    *this = *this + other;
    return *this;
}

Vector2 Vector2::operator-( const Vector2& vec ) const
{
	Vector2 result;

	result.x = x - vec.x;
	result.y = y - vec.y;

	return result;
}

Vector2 Vector2::operator-=( const Vector2& other )
{
    *this = *this - other;
    return *this;
}

Vector2 Vector2::operator*( const float scale ) const
{
    return Vector2( x * scale, y * scale );
}

Vector2 Vector2::operator*=( const float scale )
{
    *this = *this * scale;
    return *this;
}

float Vector2::dot( const Vector2& vec ) const
{
	return ( x * vec.x ) + ( y * vec.y );
}

float Vector2::length() const
{
    return sqrtf( x * x + y * y );
}

float Vector2::lengthSquared() const
{
    return x * x + y * y;
}

Vector2 Vector2::normalize()
{
	float lengthSq = lengthSquared();

    if ( lengthSq > 0.0f )
    {
        float invLength = 1.0f / sqrtf( lengthSq );
        
        x *= invLength;
        y *= invLength;
    }

	return *this;
}

Vector2 Vector2::reflect( const Vector2& direction, const Vector2& normal )
{
	float factor = -2.0f * normal.dot( direction ); // dot( normal, direction );
	return Vector2( factor * normal.x + direction.x, factor * normal.y + direction.y );
}

Vector2 Vector2::lerp( const Vector2& start, const Vector2& end, float amount )
{
	Vector2 result;
    
    result.x = start.x + ( end.x - start.x ) * amount;
    result.y = start.y + ( end.y - start.y ) * amount;

    return result;
}

Vector2 Vector2::rotate( const Vector2& direction, float angle )
{
    Vector2 result;

    float angleRadians = angle * Deg2Rad;

    // use rotation matrix
    result.x = direction.x * cosf( angleRadians ) - direction.y * sinf( angleRadians );
    result.y = direction.x * sinf( angleRadians ) + direction.y * cosf( angleRadians );

    return result;
}

