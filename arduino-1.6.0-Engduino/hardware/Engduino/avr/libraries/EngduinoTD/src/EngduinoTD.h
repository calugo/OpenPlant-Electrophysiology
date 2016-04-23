/* EngduinoTD - Engduino TouchDevelop support */

#ifndef __ENGDUINOTD_H__
#define __ENGDUINOTD_H__

#include <Arduino.h>
#include <Engduino.h>
#include <EngduinoLEDs.h>
#include <EngduinoButton.h>
#include <EngduinoAccelerometer.h>
#include <EngduinoThermistor.h>

class TD_Color
{
public:
        byte r, g, b, a;
        TD_Color(byte r, byte g, byte b, byte a)
        {
                this->r = r;
                this->g = g;
                this->b = b;
                this->a = a;
        }
        float A() { return a / 255.0; }
        float R() { return r / 255.0; }
        float G() { return g / 255.0; }
        float B() { return b / 255.0; }
};

class TD_Time
{
public:
        static void sleep(float seconds)
        {
                delay(seconds * 1000);
        }
};

class TD_Math
{
public:
        static float random(float max) { return ::random(max); }
        static float random_range(float min, float max) { return ::random(min, max + 1); }
};

class TD_Math_
{
public:
        static float random(float max) { return ::random(max); }
        static float random_range(float min, float max) { return ::random(min, max + 1); }
};

class TD_Colors
{
public:
        static TD_Color random()
        {
                return TD_Color(::random(256), ::random(256), ::random(256), 255);
        }

        static TD_Color black() { return TD_Color(0, 0, 0, 255); }
        static TD_Color white() { return TD_Color(255, 255, 255, 255); }
        static TD_Color red() { return TD_Color(255, 0, 0, 255); }
        static TD_Color green() { return TD_Color(0, 255, 0, 255); }
        static TD_Color blue() { return TD_Color(0, 0, 255, 255); }
        static TD_Color yellow() { return TD_Color(255, 255, 0, 255); }
        static TD_Color magenta() { return TD_Color(255, 0, 255, 255); }
        static TD_Color cyan() { return TD_Color(0, 255, 255, 255); }
        static TD_Color purple() { return TD_Color(128, 0, 128, 255); }
		static TD_Color orange() { return TD_Color(255, 102, 00, 255); }

        static byte toByte(float f) { return constrain(f * 255.0, 0, 255); }

        static TD_Color from_rgb(float r, float g, float b) {
                return TD_Color(toByte(r), toByte(g), toByte(b), 255);
        }
        static TD_Color from_argb(float a, float r, float g, float b) {
                return TD_Color(toByte(r), toByte(g), toByte(b), toByte(a));
        }
        //static TD_Color from_hsb(float h, float s, float b) { return from_ahsb(1.0, h, s, b); }
        //static TD_Color from_ahsb(float a, float h, float s, float b);
};

class TD_Vector3
{
public:
        float _x, _y, _z;
        TD_Vector3(float x, float y, float z)
        {
                _x = x;
                _y = y;
                _z = z;
        }

        float x() { return _x; }
        float y() { return _y; }
        float z() { return _z; }

        TD_Vector3 scale(float s)
        {
                return TD_Vector3(s * _x, s * _y, s * _z);
        }
};

class TD_Senses
{
public:
        static TD_Vector3 acceleration_quick()
        {
                float buf[3];
                EngduinoAccelerometer.xyz(buf);
                // match the experience on the phone
                return TD_Vector3(buf[1], -buf[0], buf[2]);
        }
        static TD_Vector3 acceleration_smooth() { return acceleration_quick(); }
        static TD_Vector3 acceleration_stable() { return acceleration_quick(); }
};


class TDLIB_Engduino
{
public:
        static void init()
        {
        }

        static void set_LED(int num, TD_Color c)
        {
                EngduinoLEDs.setLED(num, c.r/16, c.g/16, c.b/16);
        }

        static void set_all_LEDs(TD_Color c)
        {
                EngduinoLEDs.setAll(c.r/16, c.g/16, c.b/16);
        }

        static float millis() { return ::millis(); }
        static float micros() { return ::micros(); }
        static void delay(float num) { ::delay(num); }
        static bool button_pressed() { return EngduinoButton.isPressed(); }
        static bool button_was_pressed() { return EngduinoButton.wasPressed(); }
        static TD_Vector3 acceleration() { return TD_Senses::acceleration_quick(); }
        static float temperature() { return EngduinoThermistor.temperature(); }
};


class EngduinoTDClass 
{
	private:

	public:
		EngduinoTDClass();
		void     begin();
		void     end();
};

extern EngduinoTDClass EngduinoTD;

#endif
