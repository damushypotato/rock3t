#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = 10;

Adafruit_MPU6050 mpu;

char filename[] = "LOG0000.CSV";

bool serial_only = false;

bool createNewFile();
void printData(sensors_event_t a);
void writeData(sensors_event_t a);

void setup(void)
{
    Serial.begin(115200);

    if (!mpu.begin())
    {
        Serial.println("Failed to find MPU6050 chip");
        while (1)
        {
            delay(10);
        }
    }

    if (!SD.begin(chipSelect))
    {
        Serial.println("Card failed, or not present, switching to serial output");
        serial_only = true;
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);

    if (serial_only)
    {
        return;
    }

    for (uint8_t i = 0; i < 10000; i++)
    {
        filename[3] = i / 1000 + '0';
        filename[4] = (i % 1000) / 100 + '0';
        filename[5] = (i % 100) / 10 + '0';
        filename[6] = i % 10 + '0';
        if (!SD.exists(filename))
        {
            break;
        }

        if (i == 9999)
        {
            Serial.println("Couldnt create file. switching to serial output");
            serial_only = true;
            return;
        }
    }

    File dataFile = SD.open(filename, FILE_WRITE);

    if (!dataFile)
    {
        Serial.println("error opening file");
        while (1)
        {
            delay(10);
        }
    }

    dataFile.println("Timestamp, Ax, Ay, Az");

    dataFile.close();

    Serial.print("Logging to: ");
    Serial.println(filename);
}

void loop()
{
    sensors_event_t a;
    mpu.getAccelerometerSensor()->getEvent(&a);

    if (serial_only)
    {
        printData(a);
    }
    else
    {
        writeData(a);
    }
}

void printData(sensors_event_t a)
{
    Serial.print("X: ");
    Serial.print(a.acceleration.x);
    Serial.print(" Y: ");
    Serial.print(a.acceleration.y);
    Serial.print(" Z: ");
    Serial.print(a.acceleration.z);
    Serial.println(" m/s^2");
}

void writeData(sensors_event_t a)
{
    File dataFile = SD.open(filename, O_CREAT | O_WRITE | O_APPEND);

    if (dataFile)
    {
        dataFile.print(micros());
        dataFile.print(", ");
        dataFile.print(a.acceleration.x);
        dataFile.print(", ");
        dataFile.print(a.acceleration.y);
        dataFile.print(", ");
        dataFile.println(a.acceleration.z);

        dataFile.close();
    }
    else
    {
        Serial.println("error opening file");
    }
}