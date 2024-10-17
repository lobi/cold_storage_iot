// Pin connection
sbit DHT11_PIN = P1^7;

// Function declarations
void delay_ms(unsigned int);
void DHT11_Start();
bit DHT11_CheckResponse();
unsigned char DHT11_ReadByte();

// Main function
void Dht_Reflect(void)
{
  unsigned char humidity_integer, temperature_integer;
  char sbuf2[2];
  DHT11_Start();

  if (DHT11_CheckResponse())
  {
    clearLine(0);
    displayText("dht..");

    humidity_integer = DHT11_ReadByte();
    temperature_integer = DHT11_ReadByte();
    // Read the remaining 3 bytes and discard them
    DHT11_ReadByte();
    DHT11_ReadByte();
    DHT11_ReadByte();

    // Process the read data (e.g., display on an LCD)
    clearLine(0);
    displayText("Hum: ");
    sprintf(sbuf2, "%d", humidity_integer);
    displayText(sbuf2);

    clearLine(0);
    displayText("Temp: ");
    sprintf(sbuf2, "%d", temperature_integer);
    displayText(sbuf2);

    // Add a delay before the next reading
    //delay_ms(2000);
  }
}


// Function to start DHT11 communication
void DHT11_Start() {
  DHT11_PIN = 0;
  delay_ms(18);
  DHT11_PIN = 1;
  delay_ms(20);
  DHT11_PIN = 1;
}

// Function to check DHT11 response
bit DHT11_CheckResponse() {
  bit response = 0;
  delay_ms(1);
  if (!DHT11_PIN)
  {
    delay_ms(1);
    if (DHT11_PIN)
    {
      response = 1;
    }
  }
  while (DHT11_PIN)
    ; // Wait until DHT11 releases the bus
  return response;
}

// Function to read a byte from DHT11
unsigned char DHT11_ReadByte() {
  unsigned char i, byte = 0;
  for (i = 0; i < 8; i++)
  {
    while (!DHT11_PIN)
      ; // Wait for the pin to go high
    delay_ms(1);
    if (DHT11_PIN)
    {
      byte |= (1 << (7 - i));
      while (DHT11_PIN)
        ; // Wait for the pin to go low
    }
  }
  return byte;
}
