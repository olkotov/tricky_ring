// Oleg Kotov.

#pragma once

/**
* Represents a game clock.
***********************************************************************************/

class Clock
{    
public:

    // Gets the current time since the start of the game, in seconds.
    float currentTime() const;
	
	// Calculate the delta time.
	float calculateDeltaTime();
    
	// Gets the elapsed time between the start of the previous and current frames, in seconds.
	float deltaTime() const;

private:

	// The previous frame time.
	float m_previousTime = 0.0f;

	// The delta time between previous and current frames.
	float m_deltaTime = 0.0f;
};

