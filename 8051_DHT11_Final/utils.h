void stradd(unsigned char *dest, unsigned char *src, unsigned int pos)
{
	int i;
	for (i = pos; i < sizeof(dest) && i < (sizeof(src) + pos); i++)
	{
		dest[i] = src[i];
	}
}