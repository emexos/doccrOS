#ifndef STRING_H
#define STRING_H

void str_copy(char *dest, const char *src);
void str_append(char *dest, const char *src);
void str_append_uint(char *dest, unsigned int num);
int str_len(const char *str);
void print_str(const char *str, unsigned int color);

#endif
