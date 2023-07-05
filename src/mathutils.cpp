// Oleg Kotov

#include "mathutils.h"
#include "vector2.h"

#define _USE_MATH_DEFINES
#include <math.h> // M_PI, sin, cos
#include <stdlib.h> // random

const float _TWO_PI = 2.0f * M_PI;
const float Deg2Rad = M_PI / 180.0f;
const float Rad2Deg = 180.0f / M_PI;

float lerp( float start, float end, float amount )
{
    return start + ( end - start ) * amount;
}

bool getRandomBool()
{
    return rand() % 2 == 0;
}

float getRandomNumber()
{
    return rand() / (float)RAND_MAX;
}

int getRandomNumberInRange( int min, int max )
{
    return min + rand() % ( max - min + 1 );
}

float getRandomNumberInRange( float min, float max )
{
    return rand() / (float)RAND_MAX * ( max - min ) + min;
}

Vector2 getRandomUnitVector()
{
    Vector2 result;

    float angle = getRandomNumberInRange( -_TWO_PI, _TWO_PI );

    result.x = cosf( angle );
    result.y = sinf( angle );

    return result;
}

