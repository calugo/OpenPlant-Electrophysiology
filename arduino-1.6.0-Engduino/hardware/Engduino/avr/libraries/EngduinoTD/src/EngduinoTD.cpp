/* Library files for TouchDevelop -> Engduino export.  */

#include "EngduinoTD.h"

EngduinoTDClass::EngduinoTDClass()
{
}

void EngduinoTDClass::begin() 
{
}

void EngduinoTDClass::end() 
{
}

EngduinoTDClass EngduinoTD = EngduinoTDClass();


/*
TD_Color TD_Colors::from_ahsb(float a, float h, float s, float b)
{
            var r = brightness;
            var g = brightness;
            var b = brightness;
            if (saturation !== 0) {
                var max = brightness;
                var dif = brightness * saturation / 255;
                var min = brightness - dif;

                var h = hue * 360 / 255;

                if (h < 60) {
                    r = max;
                    g = h * dif / 60 + min;
                    b = min;
                }
                else if (h < 120) {
                    r = -(h - 120) * dif / 60 + min;
                    g = max;
                    b = min;
                }
                else if (h < 180) {
                    r = min;
                    g = max;
                    b = (h - 120) * dif / 60 + min;
                }
                else if (h < 240) {
                    r = min;
                    g = -(h - 240) * dif / 60 + min;
                    b = max;
                }
                else if (h < 300) {
                    r = (h - 240) * dif / 60 + min;
                    g = min;
                    b = max;
                }
                else if (h <= 360) {
                    r = max;
                    g = min;
                    b = -(h - 360) * dif / 60 + min;
                }
                else {
                    r = 0;
                    g = 0;
                    b = 0;
                }
            }
			

}
*/

