#include <dej/kernel.h>
#include <dej/io.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <dej/percpu.h>




lock_t com1_lock = {
    .held = false,
    .holding_cpu = -1
};



const char g_HexChars[] = "0123456789abcdef";
static void printf_unsigned(unsigned long long number, int radix)
{
    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do
    {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = g_HexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        putc(buffer[pos]);
}

void printf_signed(long long number, int radix)
{
    if (number < 0)
    {
        putc('-');
        printf_unsigned(-number, radix);
    }
    else printf_unsigned(number, radix);
}

#define LOGF_STATE_NORMAL         0
#define LOGF_STATE_LENGTH         1
#define LOGF_STATE_LENGTH_SHORT   2
#define LOGF_STATE_LENGTH_LONG    3
#define LOGF_STATE_SPEC           4

#define LOGF_LENGTH_DEFAULT       0
#define LOGF_LENGTH_SHORT_SHORT   1
#define LOGF_LENGTH_SHORT         2
#define LOGF_LENGTH_LONG          3
#define LOGF_LENGTH_LONG_LONG     4




void vlog(const char * fmt, va_list args){

    int state = LOGF_STATE_NORMAL;
    int length = LOGF_LENGTH_DEFAULT;
    int radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt)
    {
        switch (state)
        {
            case LOGF_STATE_NORMAL:
                switch (*fmt)
                {
                    case '%':   state = LOGF_STATE_LENGTH;
                                break;
                    default:    putc(*fmt);
                                break;
                }
                break;

            case LOGF_STATE_LENGTH:
                switch (*fmt)
                {
                    case 'h':   length = LOGF_LENGTH_SHORT;
                                state = LOGF_STATE_LENGTH_SHORT;
                                break;
                    case 'l':   length = LOGF_LENGTH_LONG;
                                state = LOGF_STATE_LENGTH_LONG;
                                break;
                    default:    goto PRINTF_STATE_SPEC_;
                }
                break;

            case LOGF_STATE_LENGTH_SHORT:
                if (*fmt == 'h')
                {
                    length = LOGF_LENGTH_SHORT_SHORT;
                    state = LOGF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case LOGF_STATE_LENGTH_LONG:
                if (*fmt == 'l')
                {
                    length = LOGF_LENGTH_LONG_LONG;
                    state = LOGF_STATE_SPEC;
                }
                else goto PRINTF_STATE_SPEC_;
                break;

            case LOGF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch (*fmt)
                {
                    case 'c':   putc((char)va_arg(args, int));
                                break;

                    case 's':
                                puts(va_arg(args, const char*));
                                break;

                    case '%':   putc('%');
                                break;

                    case 'd':
                    case 'i':   radix = 10; sign = true; number = true;
                                break;

                    case 'u':   radix = 10; sign = false; number = true;
                                break;

                    case 'X':
                    case 'x':
                    case 'p':   radix = 16; sign = false; number = true;
                                break;

                    case 'o':   radix = 8; sign = false; number = true;
                                break;

                    // ignore invalid spec
                    default:    break;
                }

                if (number)
                {
                    if (sign)
                    {
                        switch (length)
                        {
                        case LOGF_LENGTH_SHORT_SHORT:
                        case LOGF_LENGTH_SHORT:
                        case LOGF_LENGTH_DEFAULT:     printf_signed(va_arg(args, int), radix);
                                                        break;

                        case LOGF_LENGTH_LONG:        printf_signed(va_arg(args, long), radix);
                                                        break;

                        case LOGF_LENGTH_LONG_LONG:   printf_signed(va_arg(args, long long), radix);
                                                        break;
                        }
                    }
                    else
                    {
                        switch (length)
                        {
                        case LOGF_LENGTH_SHORT_SHORT:
                        case LOGF_LENGTH_SHORT:
                        case LOGF_LENGTH_DEFAULT:     printf_unsigned(va_arg(args, unsigned int), radix);
                                                        break;

                        case LOGF_LENGTH_LONG:        printf_unsigned(va_arg(args, unsigned  long), radix);
                                                        break;

                        case LOGF_LENGTH_LONG_LONG:   printf_unsigned(va_arg(args, unsigned  long long), radix);
                                                        break;
                        }
                    }
                }

                // reset state
                state = LOGF_STATE_NORMAL;
                length = LOGF_LENGTH_DEFAULT;
                radix = 10;
                sign = false;
                number = false;
                break;
        }

        fmt++;
    }

}

void LogStr(const char * s){
    aquire_lock(&com1_lock);
    while (*s++){

        if (*s == '\n') putc('\r');
        putc(*s);
    }
    unlock_lock(&com1_lock);
}

void LogfStr(const char * s, ...){
    aquire_lock(&com1_lock);

    va_list args;

    va_start(args, s);
    vlog(s, args);
    va_end(args);

    unlock_lock(&com1_lock);
}
void LogStrEarly(const char *s);
void LogfStrEarly(const char * s, ...);
void LogRaw(void * buffer, size_t len){
    aquire_lock(&com1_lock);

    uint8_t * out = buffer;
    uint16_t i = 0;
    while (len){
        putc(out[i]);
        len--;
        i++;
    }

    unlock_lock(&com1_lock);
    return;
}
