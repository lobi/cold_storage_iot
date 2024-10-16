void Ext_int_Init()				
{
	EA  = 1;		/* Enable global interrupt */
	ES = 1;  		/* Enable serial interrupt */			
}

void UART_Init()
{
	TMOD = 0x20;					/* Timer 1, 8-bit auto reload mode */
	TH1 = 0xFD;						/* Load value for 9600 baud rate */
	SCON = 0x50;					/* Mode 1, reception enable, Asynchronous mode, 8-bit data and 1-stop bit */
	TR1 = 1;							/* Start timer 1 */
}

void UART_TxChar(char tx_data)
{
	SBUF = tx_data;				/* Load char in SBUF register */
	while (TI==0);				/* Wait until stop bit transmit */
	TI = 0;								/* Clear TI flag */
}

char UART_RxChar(void)
{
	while(RI==0); // wait till the data is r eceived
	RI=0; // clear receive Interrupt Flag for next cycle
	return(SBUF); // return the received char
}

void UART_TxString(char *str)
{
	int i;
	for(i=0;str[i]!=0;i++)/* Send each char of string till the NULL */
	{
		UART_TxChar(str[i]);	/* Call transmit data function */
	}
	UART_TxChar('/');// also send the indicator of ending
}

void UART_TxStr(char *pointer, int length)
{
	int i;
	for(i=0; i < length; i++)/* Send each char of string till the NULL */
	{
		UART_TxChar(pointer[i]);	/* Call transmit data function */
	}
}

int UART_RXString(char *ptr_string)
{
	char ch;
	int str_len = 0;
	while(1)
	{
		ch = UART_RxChar();
		//UART_TxChar(ch);
		if((ch=='\r') || (ch=='\n') || (ch=='/')) //read till enter key is pressed
		{                             //once enter key is pressed null terminate the string
			ptr_string[str_len]=0;           //and break the loop
			break;
		}
		
		ptr_string[str_len] = ch;
		str_len++;
	}
	
	return str_len;
}

void Serial_ISR() interrupt 4    
{
	//P1 = SBUF;		/* Give received data on port 1 */
	RI = 0;			/* Clear RI flag */
}