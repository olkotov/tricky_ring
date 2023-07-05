// Oleg Kotov.

#include "clock.h"
#include <Arduino.h>

float Clock::currentTime() const
{
    return millis() / 1000.0f;
}

float Clock::calculateDeltaTime()
{
    float currentTime = this->currentTime();
    m_deltaTime = currentTime - m_previousTime;
    m_previousTime = currentTime;
    
    return m_deltaTime;
}

float Clock::deltaTime() const
{
    return m_deltaTime;
}

