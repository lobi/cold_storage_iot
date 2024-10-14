void stradd(unsigned char *dest, unsigned char *src, unsigned int pos, unsigned int size)
{
  int i = 0;
  for (i = pos; i < (pos + size); i++)
  {
    dest[i] = src[i];
  }
}

void strc(unsigned char *dest, unsigned char *src, int size)
{
  int i = 0;
  for (i = 0; i < size; i++)
  {
    dest[i] = src[i];
  }
}