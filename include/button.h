// Oleg Kotov

#pragma once

#include <arduino.h>

class Button
{
public:
    
    Button( byte pin )
    {
        m_pin = pin;
        pinMode( m_pin, INPUT_PULLUP );
    }

    bool click()
    {
        bool btnState = digitalRead( m_pin );
        
        if ( !btnState && !m_flag && millis() - m_timer >= 100 )
        {
            m_flag = true;
            m_timer = millis();
            return true;
        }

        if ( !btnState && m_flag && millis() - m_timer >= 500 )
        {
            m_timer = millis();
            return true;
        }

        if ( btnState && m_flag )
        {
            m_flag = false;
            m_timer = millis();
        }

        return false;
    }

private:
    
    byte m_pin;
    uint32_t m_timer;
    bool m_flag;
};

