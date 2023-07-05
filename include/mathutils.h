// Oleg Kotov

#pragma once

class Vector2;

// extern const float TWO_PI;
extern const float Deg2Rad;
extern const float Rad2Deg;

float lerp( float start, float end, float amount );

bool getRandomBool();
float getRandomNumber();
int getRandomNumberInRange( int min, int max );
float getRandomNumberInRange( float min, float max );
Vector2 getRandomUnitVector();

