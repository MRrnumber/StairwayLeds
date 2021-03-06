#include <limits.h>
#include "PatternEscalatorRainbow.h"
#include "Arduino.h"
#include "globals.h"
#include "makeColor.h"
#include "printbuf.h"

PatternEscalatorRainbow::PatternEscalatorRainbow(int mainLuminosity, int delayFirst, int delayLast, int hueMultiplier, unsigned int delayBetweenPhases) :
  _mainLuminosity(mainLuminosity), _delayFirst(delayFirst), _delayLast(delayLast), _hueMultiplier(hueMultiplier), _delayBetweenPhases(delayBetweenPhases)
{
}

void PatternEscalatorRainbow::_animate(direction_t direction)
{
    int phase = 1;
    float humanPositionOffset = 0;
    int shift = random(0, 360);
    int delayNow = _delayFirst;
    int delayStepPerLed = (_delayFirst - _delayLast) / (float)(NBLEDS * 2);
    unsigned long lastPhaseChange = 0;
    while (1)
    {
      ++this->_iterations;
      float humanPosition = (this->elapsed() / 1000.0 * _humanWalkingSpeed) - humanPositionOffset;
      float humanPositionLed = humanPosition * _ledsPerMeter;
      float humanPositionLedTotal = humanPositionLed + ((phase - 1) * NBLEDS);
      delayNow = _delayFirst - (int)(delayStepPerLed * humanPositionLedTotal);
      if (delayNow < 0 || delayNow > 1000)
      {
        delayNow = 0;
      }

      shift++;
      
      for (int i = 0; i < NBLEDS; i++)
      {
        int luminosity = 0;
        int hue = (shift + i) % 360;
        if (phase == 1)
        {
          if      (humanPositionLed <  i    ) { luminosity = 0; }
          else if (humanPositionLed >= i + 1) { luminosity = _mainLuminosity; }
          else
          {
            luminosity = _mainLuminosity * (humanPositionLed - (int)humanPositionLed);
            dbg3("phase=%d humanPos=%.1f humanPosLedTot=%.1f humanPosLed=%.1f delay=%d led.%d=%d", phase, humanPosition, humanPositionLedTotal, humanPositionLed, delayNow, i, luminosity);
          }
        }
        else if (phase == 2)
        {
          luminosity = _mainLuminosity;
        }
        else if (phase == 3)
        {
          if      (humanPositionLed >  i    ) { luminosity = 0; }
          else if (humanPositionLed <= i - 1) { luminosity = _mainLuminosity; }
          else
          {
            luminosity = _mainLuminosity * (1 - (humanPositionLed - (int)humanPositionLed));
            dbg3("phase=%d humanPos=%.1f humanPosLedTot=%.1f humanPosLed=%.1f delay=%d led.%d=%d", phase, humanPosition, humanPositionLedTotal, humanPositionLed, delayNow, i, luminosity);
          }
        }
        setPix(direction, LEDS_OFFSET, i, makeColor( (hue * _hueMultiplier) % 360, 100, luminosity));
      }
      
      leds.show();

      if (phase == 1 && humanPositionLed > NBLEDS)
      {
        phase = 2;
        humanPositionOffset = humanPosition;
        lastPhaseChange = this->elapsed();
      }
      else if (phase == 2)
      {
        if (this->elapsed() - lastPhaseChange > _delayBetweenPhases)
        {
          phase = 3;
          humanPositionOffset += humanPosition;
        }
      }
      else if (phase == 3 && humanPositionLed > NBLEDS)
      {
        break;
      }

      // FIXME find a better way that this, we're lowering the fps :/
      delay(delayNow);
    }
}

