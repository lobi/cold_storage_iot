void stradd(unsigned char *dest, unsigned char *src, int pos, int size)
{
  int i;
  for (i = 0; i < size; i++)
  {
    dest[pos] = src[i];
    pos++;
  }
}

// copy
void strc(unsigned char *dest, unsigned char *src, int size)
{
  int i;
  for (i = 0; i < size; i++)
  {
    dest[i] = src[i];
  }
}

// reset
void strrst(unsigned char *dest, int size)
{
	int i = 0;
  for (i = 0; i < size; i++)
  {
    dest[i] = '\0';
  }
}

// get command value
void getRxVal(unsigned char *src, unsigned char *dest, int len)
{
  // sample str: 003:05
  int i = 4; // value alway start from 4
  int i2;
  for (i2 = 0; i2 < len; i2++)
  {
    dest[i2] = src[i + i2];
  }
}

// convert char array to integer
int ca2i(unsigned char *ptr, int size)
{
  int i;
  int r = 0;
  for (i = 0; i < size; i++) {
    if (ptr[i] < '0' || ptr[i] > '9') {
        // Handle non-numeric characters
        continue;
    }
    r = r * 10 + (ptr[i] - '0');
  }
  return r;
}