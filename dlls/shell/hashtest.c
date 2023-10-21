void main(void)
{
  unsigned int entries=0x25;
    unsigned int i, hash = 0;
   char str[]=".classes";


    for (i = 0; i < 8; i++) hash ^= str[i] + i;
    printf("%x", hash % entries);
}
