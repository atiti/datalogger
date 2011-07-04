typedef struct
{
  const char *name;
  int value;
} configuration;


static char* rstrip(char *s) {
  char *p = s + strlen(s);
  while (p > s && isspace(*--p))
    *p = '\0';
  return s;
}

static char* lskip(const char *s) {
  while (*s && isspace(*s))
    s++;
  return (char *)s;
}

