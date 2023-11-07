// CSI sequence with no argument
#ifndef CSI0
#define CSI0(...)
#endif

// CSI sequence with one arguments 
#ifndef CSI1
#define CSI1(...)
#endif

// CSI Sequence with two arguments
#ifndef CSI2
#define CSI2(...)
#endif

// CSI sequence with arbitrary number of arguments
#ifndef CSIn
#define CSIn(...)
#endif

// DEC sequences (ESC [ ? value [h | l])
#ifndef DEC
#define DEC(...)
#endif

#ifndef OSC
#define OSC(...)
#endif

CSI1(CUU, CursorUp, 'A', value, 1)
CSI1(CUD, CursorDown, 'B', value, 1)
CSI1(CUF, CursorRight, 'C', value, 1)
CSI1(CUB, CursorLeft, 'D', value, 1)
CSI1(CNL, CursorNextLine, 'E', value, 1)
CSI1(CPL, CursorPrevLine, 'F', value, 1)
// Moves cursor to the absolute horizontal position (column from left) on the current line
CSI1(CHA, CursorHorizontalAbsolute, 'G', value, 1)
// MOves cursor to the absolute vertical posirion (row from top), keeping the current column
CSI1(VPA, CursorVerticalAbsolute, 'v', value, 1)
// Sets the cursor position to give row(y) and column (x)
CSI2(CUP, CursorPosition, 'H', x, y, 1, 1)
// Same as CUP above, sets cursor to x;y
CSI2(HVP, HorizontalVerticalPosition, 'f', x, y, 1, 1)
// Saves the current cursor position on stack
CSI0(ANSISYSSC, SaveCursor, 's')
// Restores the current cursor position from stack
CSI0(ANSISYSRC, RestoreCursor, 'u')

DEC(DCT25, ShowCursor, 25)
DEC(DCT1004, EnableFocusReporting, 1004)
DEC(DCT1049, EnableAlternativeBuffer, 1049)
DEC(DCT2004, EnableBracketedPaste, 2004)

#undef CSI0
#undef CSI1
#undef CSI2
#undef CSIn
#undef DEC
#undef OSC
