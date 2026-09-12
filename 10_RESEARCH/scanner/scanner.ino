#include <Wire.h>

void setup()
{
    Wire.begin();
    Serial.begin(9600);

    Serial.println("I2C Scanner");
}

void loop()
{
    byte error;
    int devices = 0;

    for (byte address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);// Sends a start condition and the address
        error = Wire.endTransmission();

        if (error == 0)// If there is no error, then a device is present at that address
        {
            Serial.print("Device found at 0x");

            if (address < 16)
                Serial.print("0");

            Serial.println(address, HEX);
            devices++;
        }

    }

    if (devices == 0)
        Serial.println("No I2C devices found.");

    else
        Serial.println("Scan complete.");

    delay(3000);
}