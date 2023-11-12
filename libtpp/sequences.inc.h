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

#ifndef OSC1
#define OSC1(...)
#endif

#ifndef OSC2
#define OSC2(...)
#endif

#ifndef TPP1
#define TPP1(...)
#endif

#ifndef TPP2
#define TPP2(...)
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
CSI2(CUP, CursorPosition, 'H', x, 1, y, 1)
// Same as CUP above, sets cursor to x;y
CSI2(HVP, HorizontalVerticalPosition, 'f', x, 1, y, 1)
// Saves the current cursor position on stack
CSI0(ANSISYSSC, SaveCursor, 's')
// Restores the current cursor position from stack
CSI0(ANSISYSRC, RestoreCursor, 'u')

DEC(DCT25, ShowCursor, 25)
DEC(DCT1004, EnableFocusReporting, 1004)
DEC(DCT1049, EnableAlternativeBuffer, 1049)
DEC(DCT2004, EnableBracketedPaste, 2004)


/** Changes window icon and title. 
 
    Changes the title of the window, pretty much identical to OSC 2 as far as I can tell. 

    TODO is it really, what are the differences? 
*/
OSC1(WINFO, ChangeWindowIconAndTitle, 0, payload)

/** Changes the window icon. 
 
    TODO how is the icon specified? 
*/
OSC1(WICON, ChangeWindowIcon, 1, payload)

/** Changes the window title. 
 */
OSC1(WTITLE, ChangeWindowTitle, 2, payload)

/** Hyperlink. 
 
    Hyperlink starts with the HLINK command, where params consists of k-v pairs separated by comma and the uri is the link target. All displayable cells *after* the HLINK seuences should link to the provided uri until a HLINK close control seuquence is found, which is HLINK with both params and uri being empty strings. 

    For more details, see https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda
 */
OSC2(HLINK, Hyperlink, 8, params, uri)

/** Sets clipboard contents. 
 
    The buffer name specifies which selection buffer to modify. 'c' stands for the standard clipboard, but other values are possible as well. 
*/
OSC2(CLIPCOPY, SetClipboard, 52, bufferName, data)

// TODO OSC112 cursor color reset







/** Sent when terminal window size changes.
 */
TPP2(TERMRES, TerminalResize, 0, cols, int, rows, int)

#undef CSI0
#undef CSI1
#undef CSI2
#undef CSIn
#undef DEC
#undef OSC1
#undef OSC2
#undef TPP1
#undef TPP2
