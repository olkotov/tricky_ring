// Oleg Kotov

#pragma once

class Vector2
{
public:

	Vector2();
	Vector2( float x, float y );

public:

	Vector2 operator+( const Vector2& vec ) const;
	Vector2 operator+=( const Vector2& vec );

	Vector2 operator-( const Vector2& vec ) const;
	Vector2 operator-=( const Vector2& vec );

	Vector2 operator*( const float scale ) const;
	Vector2 operator*=( const float scale );

public:

	float dot( const Vector2& vec ) const;

	float length() const;
	float lengthSquared() const;

	Vector2 normalize();

	static Vector2 reflect( const Vector2& direction, const Vector2& normal );
	static Vector2 lerp( const Vector2& start, const Vector2& end, float amount );
	static Vector2 rotate( const Vector2& direction, float angle );

public:

	float x;
	float y;
};

