#include <kernel/chroma.h>

static size_t strlen(const char* String) {
    size_t Len = 0;
    while(String[Len] != '\0') {
        Len++;
    }
    return Len;
}

static void NumToStr(char* Buffer, size_t Num, size_t Base) {
    size_t Temp, i = 0, j = 0;

    do {
        Temp = Num % Base;
        Buffer[i++] = (Temp < 10) ? (Temp + '0') : (Temp + 'a' - 10);
    } while (Num /= Base);

    Buffer[i--] = 0;

    for(j = 0; j < i; j++, i--) {
        Temp = Buffer[j];
        Buffer[j] = Buffer[i];
        Buffer[i] = Temp;
    }

}

int SerialPrintf(const char* restrict Format, ...) {
    va_list Parameters;
    va_start(Parameters, Format);

    int CharsWritten = 0;
    size_t Base, Num;

    char BufferStr[512] = {0};

    while(*Format != '\0') {
        size_t Limit = UINT64_MAX - CharsWritten;

        if(*Format == '%') {
            if(*(++Format) == '%')
                Format++;
        

            switch(*Format) {
                case 'c':
                    Format++;

                    char c = (char) va_arg(Parameters, int);

                    WriteSerialString(&c, sizeof(c));

                    CharsWritten++;
                    break;
                case 's':
                    Format++;
                    const char* Str = va_arg(Parameters, char*);

                    size_t Len = strlen(Str);

                    if(Limit < Len)
                        return -1;

                    WriteSerialString(Str, Len);

                    CharsWritten += Len;
                    break;
                case 'd':
                case 'u':
                case 'p':
                case 'x':

                    Num = va_arg(Parameters, size_t);
                    Base = 0;

                    
                    if(*Format == 'd' || *Format == 'u') {
                        Base = 10; // Decimal & Unsigned are base 10
                    } else {
                        Base = 16; // Hex and Ptr are base 16
                    }
                    Format++;

                    NumToStr(BufferStr, Num, Base);
                    Len = strlen(BufferStr);

                    WriteSerialString(BufferStr, Len);

                    CharsWritten += Len;
                    break;
                //case 'p':
                    //uint8_t Base = 16;
                    //size_t Dest = (uintptr_t) va_arg(Parameters, void*);
                default:
                    WriteSerialString("%u", 2);
                    break;
            }

        } else {
            WriteSerialChar(*Format);
            Format++;
        }
    }

    va_end(Parameters);
    return CharsWritten;
}

