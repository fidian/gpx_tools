/* This file is part of the package gpx_tools
 * - General memory and string functions
 * 
 * gpx_tools is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 * 
 * gpx_tools is distributed in the hope that it will be useful but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

// Allocates memory, may exit if memory can not be allocated.
// Also clears out the memory so it is always filled with NULLs.
void *getMemory(int size);

// Deallocates memory and sets the memory pointer to NULL.
void freeMemory(void **memory);

// Dynamically appends slen bytes of src to dest.
// dest must be a dynamically allocated string or NULL to allocate memory.
// If src is NULL, no change is made.  If slen is 0, no change is made.
// The dest pointer may be changed to a new memory location if it needs
// to be resized.
// Allocates memory in chunks for better efficiency.
void AppendStringN(char **dest, const char *src, const int slen);

// Appends src to dest - calls AppendStringN
// Returns if src is NULL.
void AppendString(char **dest, const char *src);

// Changes a string into lowercase.  Don't pass NULL.
void LowercaseString(char *s);

// Changes a string into uppercase.  Don't pass NULL.
void UppercaseString(char *s);

// String replace
// Returns 0 on success, 1 if there was a problem (string not found)
int StringReplace(char **where, const char *from, const char *to);

// Takes the dynamic string s and converts the following characters:
// & - &amp;   > - &gt;      ' - &apos;
// < - &lt;    " - &quote;
// s will be changed to the new string pointer if a change needed to
// be made.
void HTMLEscapeString(char **s);

// And the opposite
void HTMLUnescapeString(char **s);

// Attempts to resize a string so it fits into a specified size.
// Takes in a string that has 0xFF as a delimiter surrounding areas that
// can be shrunk.  Working from the end of the string, the sections are
// shrunk down to as little as one character.
// If * is the delimiter, here are some examples with a max length of 10:
// abcde*FGHIJKLMN*    -> abcdeFGHIJ
// abc*DEFG*hijk*LMNO* -> abcDEhijkL
// abc*DEFG*hijkl      -> abcDEFhijkl
void AutoSizeString(char *s, int max);

// Quick number parsing routine
float ParseNumber(char *s);
