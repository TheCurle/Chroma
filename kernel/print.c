/************************
 *** Team Kitty, 2019 ***
 ***       Sync       ***
 ***********************/

/* This file contains all of the xprintx functions.
 * The most commonly used will be printf */

#include <kernel.h>

static inline int Max(int a, int b) { return (a > b ? a : b); }

static void printchar(int Char, void* Args);
static char* ksprintn(char* Buffer, size_t Number, int Base, int* Size, int Upper);
static void snprintfFunc(int Char, void* Args);
static void Newline(void* AP);

#define BPP 8

const char HexAsciiData[] = "0123456789abcdefhijklmnopqrstuvwxyz";

#define HexToASCII(hex) (HexAsciiData[hex])
#define ToUpper(c)  ((c) - 0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define MaxConvertBufferLength (sizeof(size_t) * BPP + 1)

typedef struct {
    char*   String;
    size_t  Remain;
} snprintf_args;


static char* ksprintn(char* Buffer, size_t Number, int Base, int* Size, int Upper) {
    char* PrintBuffer;
    char CharBuffer;

    PrintBuffer = Buffer;
    *PrintBuffer = '\0'; // Initialize just in case.

    do {
        CharBuffer = HexToASCII( Number % Base );
        *++PrintBuffer = Upper ? ToUpper(CharBuffer) : CharBuffer;
    } while (Number /= Base);

    if(Size) {
        *Size = PrintBuffer - Buffer;
    }

    return (PrintBuffer);
}

// This function is taken from the Linux kernel.
// TODO: make readable.

int kvprintf(char const *fmt, void (*func)(int, void*), void *arg, int radix, va_list ap) {
#define PCHAR(c) {int cc=(c); if (func) (*func)(cc,arg); else *d++ = cc; retval++; }
	char nbuf[MaxConvertBufferLength];
	char *d;
	const char *p, *percent, *q;
	unsigned char *up;
	int ch, n;
	uintmax_t num;
	int base, lflag, qflag, tmp, width, ladjust, sharpflag, neg, sign, dot;
	int cflag, hflag, jflag, tflag, zflag;
	int bconv, dwidth, upper;
	char padc;
	int stop = 0, retval = 0;

	num = 0;
	q = NULL;
	if (!func)
		d = (char *) arg;
	else
		d = NULL;

	if (fmt == NULL)
		fmt = "(fmt null)\n";

	if (radix < 2 || radix > 36)
		radix = 10;

	for (;;) {
		padc = ' ';
		width = 0;
		while ((ch = (unsigned char)*fmt++) != '%' || stop) {
			if (ch == '\0')
				return (retval);
			PCHAR(ch);
		}
		percent = fmt - 1;
		qflag = 0; lflag = 0; ladjust = 0; sharpflag = 0; neg = 0;
		sign = 0; dot = 0; bconv = 0; dwidth = 0; upper = 0;
		cflag = 0; hflag = 0; jflag = 0; tflag = 0; zflag = 0;
reswitch:	switch (ch = (unsigned char)*fmt++) {
		case '.':
			dot = 1;
			goto reswitch;
		case '#':
			sharpflag = 1;
			goto reswitch;
		case '+':
			sign = 1;
			goto reswitch;
		case '-':
			ladjust = 1;
			goto reswitch;
		case '%':
			PCHAR(ch);
			break;
		case '*':
			if (!dot) {
				width = va_arg(ap, int);
				if (width < 0) {
					ladjust = !ladjust;
					width = -width;
				}
			} else {
				dwidth = va_arg(ap, int);
			}
			goto reswitch;
		case '0':
			if (!dot) {
				padc = '0';
				goto reswitch;
			}
			/* FALLTHROUGH */
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
				for (n = 0;; ++fmt) {
					n = n * 10 + ch - '0';
					ch = *fmt;
					if (ch < '0' || ch > '9')
						break;
				}
			if (dot)
				dwidth = n;
			else
				width = n;
			goto reswitch;
		case 'b':
			ladjust = 1;
			bconv = 1;
			goto handle_nosign;
		case 'c':
			width -= 1;

			if (!ladjust && width > 0)
				while (width--)
					PCHAR(padc);
			PCHAR(va_arg(ap, int));
			if (ladjust && width > 0)
				while (width--)
					PCHAR(padc);
			break;
		case 'D':
			up = va_arg(ap, unsigned char *);
			p = va_arg(ap, char *);
			if (!width)
				width = 16;
			while(width--) {
				PCHAR(HexToASCII(*up >> 4));
				PCHAR(HexToASCII(*up & 0x0f));
				up++;
				if (width)
					for (q=p;*q;q++)
						PCHAR(*q);
			}
			break;
		case 'd':
		case 'i':
			base = 10;
			sign = 1;
			goto handle_sign;
		case 'h':
			if (hflag) {
				hflag = 0;
				cflag = 1;
			} else
				hflag = 1;
			goto reswitch;
		case 'j':
			jflag = 1;
			goto reswitch;
		case 'l':
			if (lflag) {
				lflag = 0;
				qflag = 1;
			} else
				lflag = 1;
			goto reswitch;
		case 'n':
			if (jflag)
				*(va_arg(ap, intmax_t *)) = retval;
			else if (qflag)
				*(va_arg(ap, long long *)) = retval;
			else if (lflag)
				*(va_arg(ap, long *)) = retval;
			else if (zflag)
				*(va_arg(ap, size_t *)) = retval;
			else if (hflag)
				*(va_arg(ap, short *)) = retval;
			else if (cflag)
				*(va_arg(ap, char *)) = retval;
			else
				*(va_arg(ap, int *)) = retval;
			break;
		case 'o':
			base = 8;
			goto handle_nosign;
		case 'p':
			base = 16;
			sharpflag = (width == 0);
			sign = 0;
			num = (uintptr_t)va_arg(ap, void *);
			goto number;
		case 'q':
			qflag = 1;
			goto reswitch;
		case 'r':
			base = radix;
			if (sign)
				goto handle_sign;
			goto handle_nosign;
		case 's':
			p = va_arg(ap, char *);
			if (p == NULL)
				p = "(null)";
			if (!dot)
				n = strlen (p);
			else
				for (n = 0; n < dwidth && p[n]; n++)
					continue;

			width -= n;

			if (!ladjust && width > 0)
				while (width--)
					PCHAR(padc);
			while (n--)
				PCHAR(*p++);
			if (ladjust && width > 0)
				while (width--)
					PCHAR(padc);
			break;
		case 't':
			tflag = 1;
			goto reswitch;
		case 'u':
			base = 10;
			goto handle_nosign;
		case 'X':
			upper = 1;
			__attribute__ ((fallthrough)); // For GCC to stop warning about a fallthrough here
		case 'x':
			base = 16;
			goto handle_nosign;
		case 'y':
			base = 16;
			sign = 1;
			goto handle_sign;
		case 'z':
			zflag = 1;
			goto reswitch;
handle_nosign:
			sign = 0;
			if (jflag)
				num = va_arg(ap, uintmax_t);
			else if (qflag)
				num = va_arg(ap, unsigned long long);
			else if (tflag)
				num = va_arg(ap, ptrdiff_t);
			else if (lflag)
				num = va_arg(ap, unsigned long);
			else if (zflag)
				num = va_arg(ap, size_t);
			else if (hflag)
				num = (unsigned short)va_arg(ap, int);
			else if (cflag)
				num = (unsigned char)va_arg(ap, int);
			else
				num = va_arg(ap, unsigned int);
			if (bconv) {
				q = va_arg(ap, char *);
				base = *q++;
			}
			goto number;
handle_sign:
			if (jflag)
				num = va_arg(ap, intmax_t);
			else if (qflag)
				num = va_arg(ap, long long);
			else if (tflag)
				num = va_arg(ap, ptrdiff_t);
			else if (lflag)
				num = va_arg(ap, long);
			else if (zflag)
				num = va_arg(ap, size_t);
			else if (hflag)
				num = (short)va_arg(ap, int);
			else if (cflag)
				num = (char)va_arg(ap, int);
			else
				num = va_arg(ap, int);
number:
			if (sign && (intmax_t)num < 0) {
				neg = 1;
				num = -(intmax_t)num;
			}
			p = ksprintn(nbuf, num, base, &n, upper);
			tmp = 0;

			// There's weird behavior here with #. Don't use # to get 0x with zero-padding
			// (e.g. use 0x%016qx instead, not %#016qx or %#018qx, the latter of which will pad
			// 16 characters for nonzero numbers but zeros will have 18 characters).
			// Same with octal: use a leading zero and don't rely on # if you want zero-padding.
			// # works if you don't need zero padding, though.

			if (sharpflag && num != 0) {
				if (base == 8)
					tmp++;
				else if (base == 16)
					tmp += 2;
			}
			if (neg)
				tmp++;

			if (!ladjust && padc == '0')
				dwidth = width - tmp;
			width -= tmp + Max(dwidth, n);
			dwidth -= n;
			if (!ladjust)
				while (width-- > 0)
					PCHAR(' ');
			if (neg)
				PCHAR('-');
			if (sharpflag && num != 0) {
				if (base == 8) {
					PCHAR('0');
				} else if (base == 16) {
					PCHAR('0');
					PCHAR('x');
				}
			}
			while (dwidth-- > 0)
				PCHAR('0');

			while (*p)
				PCHAR(*p--);

			if (bconv && num != 0) {
				/* %b conversion flag format. */
				tmp = retval;
				while (*q) {
					n = *q++;
					if (num & (1 << (n - 1))) {
						PCHAR(retval != tmp ?
						    ',' : '<');
						for (; (n = *q) > ' '; ++q)
							PCHAR(n);
					} else
						for (; *q > ' '; ++q)
							continue;
				}
				if (retval != tmp) {
					PCHAR('>');
					width -= retval - tmp;
				}
			}

			if (ladjust)
				while (width-- > 0)
					PCHAR(' ');

			break;
		default:
			while (percent < fmt)
				PCHAR(*percent++);
			/*
			 * Since we ignore a formatting argument it is no
			 * longer safe to obey the remaining formatting
			 * arguments as the arguments will no longer match
			 * the format specs.
			 */
			stop = 1;
			break;
		}
	}
#undef PCHAR
}

int snprintf(char* String, size_t Length, const char* Format, ...) {
    int ReturnVal;
    va_list AP;

    va_start(AP, Format);
    ReturnVal = vsnprintf(String, Length, Format, AP);
    va_end(AP);
    return (ReturnVal);
}

int vsnprintf(char* String, size_t Length, const char* Format, va_list AP) {
    snprintf_args Info;
    int ReturnVal;

    Info.String = Format;
    Info.Remain = Length;
    ReturnVal = kvprintf(Format, snprintfFunc, &Info, 10, AP);

    if (Info.Remain >= 1) {
        *Info.String++ = '\0';
    }

    return ReturnVal;
}

static void snprintfFunc(int Char, void* Args) {
    snprintf_args* Info = Args;

    if(Info->Remain >= 2) {
        *Info->String++ = Char;
        Info->Remain--;
    }
}

int vsnrprintf(char* String, size_t Length, int Radix, const char* Format, va_list AP) {
    snprintf_args Info;
    int ReturnVal;

    Info.String = Format;
    Info.Remain = Length;
    ReturnVal = kvprintf(Format, snprintfFunc, &Info, Radix, AP);
    if(Info.Remain >= 1) {
        *Info.String++ = '\0';
    }

    return ReturnVal;
}

static void printchar(int Char, void* Args) {
    PRINT_INFO* Arg = (PRINT_INFO*)Args;

    switch(Char) {
        case '\033':
            // TODO: Escape codes!
            break;
        case '\x7F':
            // TODO: Decide on DEL functionality?
            break;

        case '\x85': // Next Line (not Newline, this does some hijinks.)
            Arg->cursorPos = 0;
            Newline(Arg);
            break;
        case '\014': // Form Feed (Clear Screen)
            ResetScreen();
            break;
        
        case '\a': // Alert
            // TODO: Audio alert.
            break;
        case '\b': // Back a space - NOT backspace, does not remove previous character.
            if(Arg->cursorPos != 0) {
                Arg->cursorPos--;
            }
            break;
        case '\r': // Carriage Return
            Arg->cursorPos = 0;
            break;
        case '\v': // Vertical Tab - 6 line breaks.
            for(int Line = 0; Line < 6; Line++) {
                Newline(Arg);
            }
            break;
        case '\n': // Newline.
            Newline(Arg);
            break;
        case '\t': // Tab
            for(int Spaces = 0; Spaces < 8; Spaces++) { // That's right. There are 8 spaces in a tab.
                RenderText(Arg->defaultGPU, ' ', Arg->charHeight, Arg->charWidth, Arg->charFGColor, Arg->charHLColor, Arg->screenMinX, Arg->screenMinY, Arg->charScale, Arg->cursorPos);
                Arg->cursorPos++; // Move our cursor one space along.
                if(Arg->cursorPos * Arg->charWidth * Arg->charScale > (Arg->defaultGPU.Info->HorizontalResolution - Arg->charWidth * Arg->charScale)) { // Check for wraparound
                    Arg->cursorPos = 0;
                    Newline(Arg);
                }
            }
            break;
        // TODO: More codes!
        default: 
            RenderText(Arg->defaultGPU, Char, Arg->charHeight, Arg->charWidth, Arg->charFGColor, Arg->charHLColor, Arg->screenMinX, Arg->screenMinY, Arg->charScale, Arg->cursorPos);
            if(Arg->cursorPos * Arg->charWidth * Arg->charScale > (Arg->defaultGPU.Info->HorizontalResolution - Arg->charWidth * Arg->charScale)) { // Check for wraparound
                Arg->cursorPos = 0;
                Newline(Arg);
            }
            break;
    }
}

static void Newline(void* Args) {
    PRINT_INFO* Arg = (PRINT_INFO* )Args;
    if ((Arg->screenMinY + Arg->charHeight * Arg->charScale) > (Arg->defaultGPU.Info->VerticalResolution - Arg->charHeight * Arg->charScale)) {
        if(!Arg->scrollMode) {
            // Scrollmode isn't set, so just overwrite the top.
            Arg->screenMinY = 0;
        } else if (Arg->scrollMode == Arg->charHeight * Arg->charScale) {
            // Scroll mode is set to "smooth". This leaves no empty space at the bottom of the screen.
            size_t ScrollLine = Arg->screenMinY + 2 * Arg->charHeight * Arg->charScale - Arg->defaultGPU.Info->VerticalResolution;
            Arg->screenMinY = Arg->defaultGPU.Info->VerticalResolution - Arg->charHeight * Arg->charScale;
            memmoveAVX( (EFI_PHYSICAL_ADDRESS*) Arg->defaultGPU.FrameBufferBase, (EFI_PHYSICAL_ADDRESS*)(Arg->defaultGPU.FrameBufferBase + Arg->defaultGPU.Info->PixelsPerScanline * 4 * ScrollLine), (Arg->defaultGPU.Info->VerticalResolution - ScrollLine) * Arg->defaultGPU.Info->PixelsPerScanline * 4);
            if(Arg->charBGColor != 0xFF000000) {
                memsetAVX_By4Bytes( (EFI_PHYSICAL_ADDRESS*)(Arg->defaultGPU.FrameBufferBase + (Arg->defaultGPU.Info->VerticalResolution - ScrollLine) * Arg->defaultGPU.Info->PixelsPerScanline * 4), Arg->charBGColor, (Arg->defaultGPU.Info->VerticalResolution - ScrollLine) * Arg->defaultGPU.Info->PixelsPerScanline);
            }
        } // TODO: Implement "quick" scroll.
        else if (Arg->scrollMode == Arg->defaultGPU.Info->VerticalResolution) {
            // Wipe screen.
            if(Arg->charBGColor != 0xFF000000) {
                memsetAVX_By4Bytes( (EFI_PHYSICAL_ADDRESS*)Arg->defaultGPU.FrameBufferBase, Arg->charBGColor, Arg->defaultGPU.Info->VerticalResolution * Arg->defaultGPU.Info->PixelsPerScanline);
            }
        } else {
            size_t ScrollLine = Arg->screenMinY + 2 * Arg->charHeight * Arg->charScale - Arg->defaultGPU.Info->VerticalResolution;
			Arg->screenMinY = Arg->defaultGPU.Info->VerticalResolution - Arg->charHeight * Arg->charScale;
			// This offset correction is needed in case a font size/scale combination is not an integer multiple of the vertical resolution.
			// Even if it is, changing scales or arg->y could cause a variable offset and that needs to be accounted for.
			for(size_t SmoothScroll = 0; SmoothScroll < ScrollLine; SmoothScroll += Arg->scrollMode) {
				memmoveAVX((EFI_PHYSICAL_ADDRESS* )Arg->defaultGPU.FrameBufferBase, (EFI_PHYSICAL_ADDRESS* )(Arg->defaultGPU.FrameBufferBase + Arg->defaultGPU.Info->PixelsPerScanline * 4 * Arg->scrollMode), (Arg->defaultGPU.Info->VerticalResolution - Arg->scrollMode - SmoothScroll) * Arg->defaultGPU.Info->PixelsPerScanline * 4);
				if(Arg->charBGColor != 0xFF000000) {
					memsetAVX_By4Bytes((EFI_PHYSICAL_ADDRESS* )(Arg->defaultGPU.FrameBufferBase + (Arg->defaultGPU.Info->VerticalResolution - Arg->scrollMode - SmoothScroll) * Arg->defaultGPU.Info->PixelsPerScanline * 4), Arg->charBGColor, Arg->scrollMode * Arg->defaultGPU.Info->PixelsPerScanline);
				}
			}
        }
    
    } else {
        // No wraparound needed. Just move along.
        Arg->screenMinY += Arg->charHeight * Arg->charScale;
    }
}

int printf(const char* Format, ...) {
    va_list AP;
    int ReturnVal;

    va_start(AP, Format);
    ReturnVal = kvprintf(Format, printchar, &Print_Info, 10, AP);
    va_end(AP);

    return ReturnVal;
}

int vprintf(const char* Format, va_list AP) {
    return kvprintf(Format, printchar, &Print_Info, 10, AP);
}

int sprintf(char* Buffer, const char* Format, ...) {
    va_list AP;

    va_start(AP, Format);
    int ReturnVal = kvprintf(Format, NULL, (void*) Buffer, 10, AP);
    Buffer[ReturnVal] = '\0';
    va_end(AP);

    return ReturnVal;
}

int vsprintf(char* Buffer, const char* Format, va_list AP) {
    int ReturnVal = kvprintf(Format, NULL, (void*) Buffer, 10, AP);
    Buffer[ReturnVal] = '\0';
    return ReturnVal;
}

void printUnicode(CHAR16* String, size_t Size) {
    for(size_t Char = 0; Char < Size; Char++) {
        if( ((char*)String)[Char] != 0) {
            printf("%c", ((char*)String)[Char]);
        }
    }
}