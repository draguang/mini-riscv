#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  va_list args;
  va_start(args,fmt);
  int count = 0;
  while(*fmt)
  {
    if(*fmt=='%')
    {
      fmt++;
      switch(*fmt)
      {
        case 'd':
        {
          int num=va_arg(args,int);
          if(num<0)
          {
            putch('-');
            count++;
            num = -num;
          }
          int divisor = 1;
          while(num/divisor>9)  divisor*=10;
          while(divisor!=0)
          {
            putch('0'+(num/divisor));
            
            num = num%divisor;
            divisor/=10;
            count++;
          }
          break;
        }
        case 'c':
        {
          char chr = (char)va_arg(args,int);
          putch(chr);
          count++;
          break;
        }
        case 's':
        {
          char *start = va_arg(args,char*);
          if (!start) start = "(null)";
          while(*start)
          {
            putch(*start);
            start++;
            count++;
          }
          break;
        }
        case '%':
        {
          putch('%');
          count++;
          break;
        }
        default:
        {
          putch('%');
          putch(*fmt);
          count+=2;
          break;
        }
      }
      fmt++;
    }
    else
    {
      putch(*fmt);
      count++;
    }
    fmt++;
  }
  va_end(args);
  return count;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int int_to_str(char*out,int*position,int num)
{
  char buf[16];
  int start_pos = *position;
  int count = 0;
  do{
    buf[count++] = '0'+(num%10);
    num/=10;
  }while(num>0);
  while(count-->0)
  {
    out[*position++] = buf[count];
  }
  return *position - start_pos;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args,fmt);
  int count = 0;
  int pos = 0;
  while(*fmt)
  {
    if(*fmt == '%')
    {
      fmt++;
      switch(*fmt)
      {
        case 'c':
        {
          char chr = (char)va_arg(args,int);
          out[pos++] = chr;
          count++;
          break;
        }
        case 's':
        {
          char *str = va_arg(args,char*);
          while(*str)
          {
            out[pos++] = *str++;
            count++;
          }
          break;
        }
        case 'd':
        {
          int res = va_arg(args,int);
          count+=int_to_str(out,&pos,res);
          break;
        }
        default:
        {
          out[pos++] = *fmt;
          count++;
          break;
        }
      }
    }
    else
    {
      out[pos++] = *fmt;
      count++;
    }
    fmt++;
  }
  out[pos] = '\0';
  va_end(args);
  return count;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
