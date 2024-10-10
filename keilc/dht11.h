sbit DHT11 = P1^7; /* Connect DHT11 Sensor Pin to P2.1 Pin */
char I_RH, D_RH, I_Temp, D_Temp;

void timer_delay20ms()		/* Timer0 delay function */
{
	TMOD = 0x01;
	TH0 = 0xB8;				/* Load higher 8-bit in TH0 */
	TL0 = 0x0C;				/* Load lower 8-bit in TL0 */
	TR0 = 1;					/* Start timer0 */
	while(TF0 == 0);	/* Wait until timer0 flag set */
	TR0 = 0;					/* Stop timer0 */
	TF0 = 0;					/* Clear timer0 flag */
}

void timer_delay30us() /* Timer0 delay function */
{
	TMOD = 0x01;			/* Timer0 mode1 (16-bit timer mode) */
	TH0 = 0xFF;				/* Load higher 8-bit in TH0 */
	TL0 = 0xF1;				/* Load lower 8-bit in TL0 */
	TR0 = 1;					/* Start timer0 */
	while(TF0 == 0);	/* Wait until timer0 flag set */
	TR0 = 0;					/* Stop timer0 */
	TF0 = 0;					/* Clear timer0 flag */
}

void Dht_Request() /* Microcontroller send start pulse or request */
{
  DHT11 = 0;    /* set to low pin */
  timer_delay20ms(); /* wait for 20ms */
  DHT11 = 1;    /* set to high pin */
}

void Dht_Response() /* receive response from DHT11 */
{
  while (DHT11 == 1)
  {
  }
  while (DHT11 == 0)
  {
  }
  while (DHT11 == 1)
  {
  }
}

int Dht_ReceiveData() /* receive data */
{
  int q, c = 0;
  for (q = 0; q < 8; q++)
  {
    while (DHT11 == 0) /* check received bit 0 or 1 */
    {
    }
    timer_delay30us();
    if (DHT11 == 1) /* if high pulse is greater than 30ms */
    {
      c = (c << 1) | (0x01); /* then its logic HIGH */
    }
    else /* otherwise its logic LOW */
    {
      c = (c << 1);
    }

    while (DHT11 == 1)
    {
    }
  }
  return c;
}

void Dht_UpdateCurrentMetricsToEeprom()
{
  char crrt[2];
  Dht_Request();  /* send start pulse */
  Dht_Response(); /* receive response */

  I_RH = Dht_ReceiveData();   /* store first eight bit in I_RH */
  D_RH = Dht_ReceiveData();   /* store next eight bit in D_RH */
  I_Temp = Dht_ReceiveData(); /* store next eight bit in I_Temp */
  D_Temp = Dht_ReceiveData(); /* store next eight bit in D_Temp */

  
  //sprintf(ccrt_tem, "%ld", I_Temp);
  sprintf(crrt, "%d", I_RH);
  DA_SetHumidity(&I_RH);
  sprintf(crrt, "%ld", I_Temp);
  DA_SetTemperature(crrt);

  // validate to make sure it saved
  //DA_GetHumidity(&crrt_hum);
  //DA_GetTemperature(&ccrt_tem);
}