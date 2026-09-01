void setup()
{
    Serial.begin(9600);
    pinMode(11,OUTPUT);
}

void loop()
{
    if (Serial.available() > 0)
    {
        byte header = Serial.read();

        if (header == 0xAA)
        {
            while (Serial.available() < 3)
            {
            }

            byte highByte = Serial.read();
            byte lowByte = Serial.read();
            byte receivedChecksum = Serial.read();

            byte calculatedChecksum = highByte ^ lowByte;

            if (calculatedChecksum == receivedChecksum)
            {
                uint16_t ADC_value = ((uint16_t)highByte << 8) | lowByte;

                Serial.print("ADC = ");
                Serial.println(ADC_value);


                if(ADC_value < 2000)
                {
                    digitalWrite(11, HIGH);
                }
                else
                {
                    digitalWrite(11, LOW);
                }
            }
            else
            {
                Serial.println("Checksum Error");
            }
        }
    }
}