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
#define NEED_CTYPE_H
#define NEED_STDIO_H
#define NEED_STDLIB_H
#define NEED_STRING_H
#include "include.h"

#define STRING_CHUNK_SIZE 256


// Allocates memory, may exit if memory can not be allocated.
// Also clears out the memory so it is always filled with NULLs.
void *getMemory(int size)
{
	void *memptr;

	memptr = malloc(size);
	if (! memptr) 
	{
		fprintf(stderr, "Unable to allocate memory!\n");
		exit(20);
	}
#ifdef HAVE_MEMSET
	memset(memptr, 0, size);
#else
#error NO MEMSET
#endif

	return memptr;
}


// Deallocates memory and sets the memory pointer to NULL.
void freeMemory(void **memory) 
{
	if (*memory == NULL)
	{
		return;
	}

	free(*memory);
	*memory = NULL;
}


// Dynamically appends slen bytes of src to dest.
// dest must be a dynamically allocated string or NULL to allocate memory.
// If src is NULL, no change is made.  If slen is 0, no change is made.
// The dest pointer may be changed to a new memory location if it needs
// to be resized.
// Allocates memory in chunks for better efficiency.
void AppendStringN(char **dest, const char *src, const int slen)
{
	char *newdest;
	int dlen, newlen;
	int old_blocks, new_blocks;

	if (src == NULL)
	{
		DEBUG("src is null");
		return;
	}

	if (*dest == NULL)
	{
		dlen = 0;
		old_blocks = 0;
	}
	else
	{
		dlen = strlen(*dest);
		old_blocks = 1 + (dlen / STRING_CHUNK_SIZE);
	}

	newlen = dlen + slen + 1;
	new_blocks = 1 + (newlen / STRING_CHUNK_SIZE);

	if (new_blocks != old_blocks) 
	{
		// realloc doesn't seem work for me.  Inept programmer?
		newdest = getMemory(new_blocks * STRING_CHUNK_SIZE);
		newdest[0] = '\0';
		if (*dest != NULL) 
		{
			strcpy(newdest, *dest);
		}
		freeMemory((void **) dest);

		*dest = newdest;
	}

	strncat(*dest, src, slen);
}


// Appends src to dest - calls AppendStringN
// Returns if src is NULL.
void AppendString(char **dest, const char *src)
{
	int slen;

	if (src == NULL)
		return;

	slen = strlen(src);
	AppendStringN(dest, src, slen);
}


// Changes a string into lowercase.  Don't pass NULL.
void LowercaseString(char *s)
{
	while (*s != '\0')
	{
		*s = tolower((int) *s);
		s ++;
	}
}


// Changes a string into uppercase.  Don't pass NULL.
void UppercaseString(char *s)
{
	while (*s != '\0')
	{
		*s = toupper((int) *s);
		s ++;
	}
}


// Replaces all occurances of "from" with "to" in "where"
// Returns 0 on success, 1 on failure
int StringReplace(char **where, const char *from, const char *to)
{
	char *out = NULL, *start;
	int i, fl, replaced;

	if (*where == NULL)
	{
		return 0;
	}

	fl = strlen(from);
	start = *where;
	replaced = 0;
	i = 0;

	while (start[i] != '\0')
	{
		if (start[i] == *from && strncmp(&(start[i]), from, fl) == 0)
		{
			// Found the string.
			AppendStringN(&out, start, i);
			AppendString(&out, to);
			start += i + fl;
			i = 0;
			replaced ++;
		}
		else
		{
			i ++;
		}
	}

	if (replaced == 0)
	{
		return 0;
	}

	AppendString(&out, start);
	freeMemory((void **) where);
	*where = out;

	return replaced;
}


// Strings to encode/decode in HTML
// "&" must be first for HTMLEscapeString()
static char *HTMLEscaped[] = { "&amp;", "&lt;", "&gt;", 
	"&quot;", "&apos;", "" };
static char *HTMLUnescaped[] = { "&", "<", ">", 
	"\"", "'", "" };

// Takes the dynamic string s and escapes HTML for XML
// s will be changed to the new string pointer if a change needed to
// be made.
void HTMLEscapeString(char **s)
{
	int i;

	for (i = 0; HTMLUnescaped[i][0] != '\0'; i ++)
	{
		StringReplace(s, HTMLUnescaped[i], HTMLEscaped[i]);
	}   
}


// Takes the dynamic string s and converts the following characters:
// & - &amp;   > - &gt;      ' - &apos;
// < - &lt;    " - &quote;
// s will be changed to the new string pointer if a change needed to
// be made.
void HTMLUnescapeString(char **s)
{
	int i;

	for (i = 0; HTMLEscaped[i][0] != '\0'; i ++)
	{
		StringReplace(s, HTMLEscaped[i], HTMLUnescaped[i]);
	}
}


// Attempts to resize a string so it fits into a specified size.
// Takes in a string that has 0xFF as a delimiter surrounding areas that
// can be shrunk.  Working from the end of the string, the sections are
// shrunk down to as little as one character.
// If * is the delimiter, here are some examples with a max length of 10:
// abcde*FGHIJKLMN*    -> abcdeFGHIJ
// abc*DEFG*hijk*LMNO* -> abcDEhijkL
// abc*DEFG*hijkl      -> abcDEFhijkl
void AutoSizeString(char *s, int max)
{
	char *s2 = s;
	int len = 0, i = 0, in_auto = 0;

	// Count the length of the string excluding the special markers
	while (*s2 != '\0')
	{
		if ((unsigned char) *s2 != 0xFF) 
		{
			len ++;
		}
		s2 ++;
		i ++;
	}

	// If there are no special characters, we are done.
	if (i == len) 
	{
		return;
	}

	// Adjust lengths, keeping a minimum of one character
	while (i && len > max)
	{
		i --;
		if (in_auto)
		{
			if ((unsigned char) s[i] == 0xFF)
			{
				in_auto = 0;
			}
			else if ((unsigned char) s[i - 1] != 0xFF)
			{
				s[i] = 0xFF;
				len --;
			}
		}
		else
		{
			if ((unsigned char) s[i] == 0xFF)
			{
				in_auto = 1;
			}
		}
	}

	// Remove special markers
	s2 = s;
	while (*s2 != '\0')
	{
		if ((unsigned char ) *s2 != 0xFF)
		{
			*s = *s2;
			s ++;
		}
		s2 ++;
	}
	*s = *s2;
}


// Quick number parsing routine
float ParseNumber(char *s)
{
	float sign = 1;
	float value = 0;
	float factor = 0.1;

	if (*s == '-')
	{
		sign = -1;
		s ++;
	}
	while (*s >= '0' && *s <= '9')
	{
		value *= 10;
		value += *s - '0';
		s ++;
	}
	if (*s == '.')
	{
		s ++;
		while (*s >= '0' && *s <= '9')
		{
			value += factor * (*s - '0');
			factor /= 10;
			s ++;
		}
	}

	return sign * value;
}
