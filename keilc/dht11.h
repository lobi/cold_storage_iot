sbit DHT11 = P1^7; /* Connect DHT11 Sensor Pin to P2.1 Pin */
int I_RH, D_RH, I_Temp, D_Temp, CheckSum;

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

void Dht_Rqst() /* Microcontroller send start pulse or request */
{
  DHT11 = 1;
  DHT11 = 0;         /* set to low pin */
  timer_delay20ms(); /* wait for 20ms */
  DHT11 = 1;         /* set to high pin */
}

void Dht_Rspn() /* receive response from DHT11 */
{
  while(DHT11==1);
	while(DHT11==0);
	while(DHT11==1);
}

int Dht_Rcv() /* receive data */
{
  int q, c = 0;
  for (q = 0; q < 8; q++)
  {
    while (DHT11 == 0); /* check received bit 0 or 1 */
    timer_delay30us();
    if (DHT11 == 1)          /* If high pulse is greater than 30ms */
      c = (c << 1) | (0x01); /* Then its logic HIGH */
    else                     /* otherwise its logic LOW */
      c = (c << 1);
    while (DHT11 == 1);
  }
  return c;
}

/*
Get data from DHT11 sensor
save to eeprom
*/
void Dht_Update()
{
  //unsigned char st[2] = {'\0', '\0'};
  //unsigned char sh[2] = {'\0', '\0'};
  Dht_Rqst();  /* send start pulse */
  Dht_Rspn(); /* receive response */
	
  I_RH = Dht_Rcv();     /* store first eight bit in I_RH */
  D_RH = Dht_Rcv();     /* store next eight bit in D_RH */
  I_Temp = Dht_Rcv();   /* store next eight bit in I_Temp */
  D_Temp = Dht_Rcv();   /* store next eight bit in D_Temp */
  CheckSum = Dht_Rcv(); /* store next eight bit in CheckSum */

  if ((I_RH + D_RH + I_Temp + D_Temp) != CheckSum)
  {
    clearLine(0);
    displayText("Sensor error");
    return;
  }

  // clearLine(0);
  // displayText("Sensor reading");

  // if (I_RH > 9)
  // {
  //   EepromWriteByte(I_RH/10, 7, 0);
  //   Delay_ms(4);
  //   EepromWriteByte(I_RH%10, 8, 0);
  // }
  // else
  // {
  //   EepromWriteByte('0', 5, 0);
  //   Delay_ms(4);
  //   EepromWriteByte(I_RH, 6, 0);
  // }
  //strrst(sh, 2);
  // sscanf(sh, "%02d", I_RH);
  // DA_SetHumidity(sh);
  DA_SetHumidity("35"); // hard code for testing
  Delay_ms(4);

  // if (I_Temp > 9)
  // {
  //   EepromWriteByte(I_Temp/10, 7, 0);
  //   Delay_ms(4);
  //   EepromWriteByte(I_Temp%10, 8, 0);
  // }
  // else
  // {
  //   EepromWriteByte('0', 5, 0);
  //   Delay_ms(4);
  //   EepromWriteByte(I_Temp, 6, 0);
  // }
  //strrst(st, 2);
  // sscanf(st, "%02d", I_Temp);
  // DA_SetTemperature(st);
  DA_SetTemperature("02"); // hard code for testing
}


