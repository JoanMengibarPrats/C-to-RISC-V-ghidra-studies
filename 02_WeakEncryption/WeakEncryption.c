/* Hashing Algorithm */

#include <stdio.h>

int get_bitmask(char* key)
{
	int hash =0;
	while(*key)
	{
		hash+=(*key++);
	}
	return hash;
}

int encrypt(int c, int bitmask)
{
	return c ^ bitmask;
}

int main(int arc, char* argv[])
{
	int c;
	int bitmask;

	bitmask = get_bitmask(argv[1]);

	while ((c = getchar()) != EOF)
		putchar(encrypt(c, bitmask));

	return 0;
}
