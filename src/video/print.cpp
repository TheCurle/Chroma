#include <kernel/chroma.h>
#include <kernel/video/draw.h>

/************************
 *** Team Kitty, 2020 ***
 ***     Chroma       ***
 ***********************/

#ifdef __cplusplus
extern "C" {
#endif

/* This file contains all of the String / Print related functions
 *  that are required by the core of the kernel.
 *
 * There will be a proper C++ std::string implementation in lainlib.
 *
 * This file also provides SerialPrintf.
 */

void NumToStr(char* Buffer, size_t Num, size_t Base) {
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

int SerialPrintf(const char* Format, ...) {
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
                case 'c': {
                    Format++;

                    char c = (char) va_arg(Parameters, int);

                    WriteSerialString(&c, sizeof(c));

                    CharsWritten++;
                    break;
                }
                case 's': {
                    Format++;
                    const char* Str = va_arg(Parameters, char*);

                    size_t Len = strlen(Str);

                    if(Limit < Len)
                        return -1;

                    WriteSerialString(Str, Len);

                    CharsWritten += Len;
                    break;
                }
                case 'd':
                case 'u':
                case 'p':
                case 'x': {

                    Num = va_arg(Parameters, size_t);
                    Base = 0;


                    if(*Format == 'd' || *Format == 'u') {
                        Base = 10; // Decimal & Unsigned are base 10
                    } else {
                        Base = 16; // Hex and Ptr are base 16
                    }
                    Format++;

                    NumToStr(BufferStr, Num, Base);
                    size_t Len = strlen(BufferStr);

                    WriteSerialString(BufferStr, Len);

                    CharsWritten += Len;
                    break;
                }
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

size_t ParseEnglishColor(char* Name);
size_t ParseHexColor(const char* Name, bool bgFlag);

int Printf(const char* Format, ...) {
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
                case 'c': {
                    Format++;

                    char c = (char) va_arg(Parameters, int);

                    WriteString(&c);

                    CharsWritten++;
                    break;
                }
                case 's': {
                    Format++;
                    const char* Str = va_arg(Parameters, char*);

                    size_t Len = strlen(Str);

                    if(Limit < Len)
                        return -1;

                    WriteString(Str);

                    CharsWritten += Len;
                    break;
                }
                case 'd':
                case 'u':
                case 'p':
                case 'x': {

                    Num = va_arg(Parameters, size_t);
                    Base = 0;


                    if(*Format == 'd' || *Format == 'u') {
                        Base = 10; // Decimal & Unsigned are base 10
                    } else {
                        Base = 16; // Hex and Ptr are base 16
                    }
                    Format++;

                    NumToStr(BufferStr, Num, Base);
                    size_t Len = strlen(BufferStr);

                    WriteString(BufferStr);

                    CharsWritten += Len;
                    break;
                }
                //case 'p':
                    //uint8_t Base = 16;
                    //size_t Dest = (uintptr_t) va_arg(Parameters, void*);
                default:
                    WriteString("%u");
                    break;
            }

        } else if(*Format == '\\') {
            Format++; // Skip backslash

            switch(*Format) {
                case '$': {
                    // COLOR
                    Format ++; // Skip $
                    size_t Color = 0;
                    bool bgFlag = false;

                    switch(*Format) {
                        case '[':
                            // bg
                            bgFlag = true;
                            [[fallthrough]];
                        case '{':
                            // fg
                            Format++; // [ or {

                            if(*Format == '<') {
                                Format++;
                                Color = ParseEnglishColor((char*) Format);
                            } else {
                                Color = ParseHexColor(Format, bgFlag);
                            }

                            if(bgFlag) 
                                SetBackgroundColor(Color);
                            else
                                SetForegroundColor(Color);
                            
                            while(*Format != '}')
                                Format++;
                            Format++; // }
                            break;
                    }
                    
                    break;
                }
                case 'f':
                    // FORMAT
                    break;
            }
        } else {
            WriteChar(*Format);
            Format++;
        }
    }

    va_end(Parameters);
    return CharsWritten;
}


size_t ParseEnglishColor(char* Stream) {
    if(strcmp(Stream, "red"))
        return 0xFF0000;
    if(strcmp(Stream, "green"))
        return 0xFF00;
    if(strcmp(Stream, "blue"))
        return 0xFF;
    if(strcmp(Stream, "yellow"))
        return 0xFFFF00;
    if(strcmp(Stream, "cyan"))
        return 0xFFFF;
    if(strcmp(Stream, "magenta"))
        return 0xFF00FF;
    if(strcmp(Stream, "beans"))
        return 0xAA11CC;
    if(strcmp(Stream, "forgeb"))
        return 0x1E2D42;
    if(strcmp(Stream, "forgey"))
        return 0xE0A969;
    return 0xFFFFFF;
}

size_t ParseHexColor(const char* Stream, bool bgFlag) {
    size_t val = 0;
    char c;

    while ((*Stream != (bgFlag ? ']' : '}')) && (c = *(++Stream))) {
        char v = ((c & 0xF) + (c >> 6)) | ((c >> 3) & 0x8);
        val = (val << 4) | (size_t) v;
    }
    return val;
}

#ifdef  __cplusplus
}
#endif