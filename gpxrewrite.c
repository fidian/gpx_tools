/* This file is part of the package gpx_tools
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef EXPAT_XMLPARSE
#include <xmlparse.h>
#else
#include <expat.h>
#endif

#define CHUNK_SIZE 1024
#define SETTING_LINE_LEN 1024
#define STRING_CHUNK_SIZE 256

/* Cache Types, literally from the GPX   Default Prefix
 * ----------------------------------------------------
 * Benchmark  -- Not used by GC.com      X
 * Cache In Trash Out Event              C
 * Earthcache                            G
 * Event Cache                           E
 * Letterbox Hybrid                      B
 * Locationless (Reverse) Cache          L
 * Mega-Event Cache                      E
 * Multi-cache                           M
 * Project APE Cache                     A
 * Traditional Cache                     T
 * Unknown Cache                         U
 * Virtual Cache                         V
 * Webcam Cache                          W
 * 
 * Sizes, literally from the GPX
 * -----------------------------
 * Large
 * Micro
 * Other
 * Regular
 * Small
 * Unknown
 * Virtual
 */
   
typedef struct settings
{
   char *key, *value;
   struct settings *next;
} SettingsStruct;

typedef struct app_data 
{
   FILE *fpout;
   XML_Parser parser;
   int depth;
   SettingsStruct *settings;
   int in_wpt_tag;
   int in_log_tag;
   int in_log_type_tag;
   int wrote_log_type;
   int copy_to_data;
   
   char *wpt_tag;  // The wpt open tag
   char *wpt_data;  // The XML data, excluding the name, desc, sym
   char **currentTarget;  // Where we are sticking text
   
   char *name;  // GC1234
   char *desc;  // Super Fake Cache by Someone Cool, Traditional Cache (2/2.5)
   char *urlname;  // Super Fake Cache
   char *sym;  // Geocache // Geocache Found
   char *type;  // Geocache|Traditional Cache
   int available;  // 1 // 0
   int archived;  // 1 // 0
   char *placedBy;  // Someone Cool
   char *owner;  // Someone Cool
   char *type2;  // Traditional Cache
   char *container;  // Small
   char *difficulty;  // 2
   char *terrain;  // 2.5
   char *hints;  // Look under the log
   char *logSummary;  //  First letters of the log types (FFDFWO)
   int bugs;  // Number of bugs in the cache
} AppData;


void *getMemory(size)
{
   void *memptr;
   
   memptr = malloc(size);
   if (! memptr) 
     {
	fprintf(stderr, "Unable to allocate memory!\n");
	exit(20);
     }
   memset(memptr, 0, size);
   
   return memptr;
}


void freeMemory(void **memory) 
{
   if (*memory == NULL)
     {
	return;
     }
   
   free(*memory);
   *memory = NULL;
}


void AppendStringN(char **dest, const char *src, const int slen)
{
   char *newdest;
   int dlen, newlen;
   int old_blocks, new_blocks;

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


void AppendString(char **dest, const char *src)
{
   int slen;

   slen = strlen(src);
   AppendStringN(dest, src, slen);
}


void LowercaseString(char *s)
{
   while (*s != '\0')
     {
	*s = tolower((int) *s);
	s ++;
     }
}


char *GetSetting(AppData *ad, char *keyname)
{
   SettingsStruct *node;
   char *keyname_lower = NULL;
   
   AppendString(&keyname_lower, keyname);
   LowercaseString(keyname_lower);
   
   node = ad->settings;
   
   while (node != NULL)
     {
	if (strcmp(node->key, keyname_lower) == 0)
	  {
	     freeMemory((void **) &keyname_lower);
	     return node->value;
	  }
	
	node = node->next;
     }
   
   freeMemory((void **) &keyname_lower);
   
   return (char *) NULL;
}


char *GetFormattedCacheType(AppData *ad)
{
   char *formattedType = NULL;
   int i;
   
   AppendString(&formattedType, ad->type2);
   i = 0;
   while (formattedType[i] != ' ' && formattedType[i] != '-' &&
	  formattedType[i] != '\0')
     {
	i ++;
     }
   formattedType[i] = '\0';
   
   if (strcmp(formattedType, "Cache") == 0)
     {
	freeMemory((void **) &formattedType);
	AppendString(&formattedType, "CITO_Event");
     }
   else if (strcmp(formattedType, "Letterbox") == 0 ||
	    strcmp(formattedType, "Project") == 0)
     {
	formattedType[i] = '_';
     }
   
   return formattedType;
}


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
   
   // If there are no special characters, abort.
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


char *AssembleFormat(AppData *ad, char *Format, char *NameType)
{
   // Format is "Percent Code [Length]" where Length is optional
   // and can be 0 (auto-fit), or a number (up to that number)
   // 
   // %C = code as specified with "CACHETYPE_Prefix"
   // %Y = Cache type, spelled out
   // %I = ID (the xxxx part of GCxxxx)
   // %D = Difficulty as 1 or 3 character string
   // %d = Difficulty as a single digit, 1-9
   // %T = Terrain as 1 or 3 character string
   // %t = Terrain as a single digit, 1-9
   // %S = Cache size
   // %s = First letter of cache size
   // %O = Owner
   // %P = Placed by
   // %h = Hint
   // %N = Name of cache (smart truncated -- todo?)
   // %L = First letter of logs
   // 
   // %H = Hint (just for consistency)
   // %y = Same as %C
   // 
   // %b = Is there a bug?
   // %f = Did you find it?
   // %a = Is it active?
   // *** These use the preferences Bug_Yes, Bug_No, Found_Yes, Found_No,
   // Active_Yes, Active_No and defaults to the single Y or N character.
   // 
   // %% = Literal percent symbol
   // %0 - %9 = Literal number
   // 
   // Anything else is copied verbatim
   
   int length, i, j, k, max_length;
   char *out = NULL, *lastEnd, *current, *tmp = NULL, *tmp2 = NULL;
   char c, *allowed;

   AppendString(&tmp, NameType);
   AppendString(&tmp, "_Allowed_Chars");
   allowed = GetSetting(ad, tmp);
   freeMemory((void **) &tmp);
   
   AppendString(&tmp, NameType);
   AppendString(&tmp, "_Max_Length");
   tmp2 = GetSetting(ad, tmp);
   freeMemory((void **) &tmp);
   max_length = 0;
   if (tmp2 != NULL && tmp2[0] >= '0' && tmp2[0] <= '9')
     {
	i = 0;
	while (tmp2[i] >= '0' && tmp2[i] <= '9')
	  {
	     max_length *= 10;
	     max_length += tmp2[i] - '0';
	     i ++;
	  }
     }
   tmp2 = NULL;
   
   lastEnd = Format;
   current = Format;
   while (*current != '\0')
     {
	if (*current != '%')
	  {
	     current ++;
	  }
	else
	  {
	     // Copy the last chunk to out
	     if (lastEnd != current)
	       {
		  AppendStringN(&out, lastEnd, current - lastEnd);
	       }
	     
	     // Handle the % code
	     length = -1;
	     current ++;
	     switch (*current)
	       {
		case 'a': // Active
		  if (ad->available) 
		    {
		       AppendString(&tmp, GetSetting(ad, "Active_Yes"));
		    }
		  else
		    {
		       AppendString(&tmp, GetSetting(ad, "Active_No"));
		    }
		  break;
		  
		case 'b': // Bugs
		  if (ad->bugs)
		    {
		       AppendString(&tmp, GetSetting(ad, "Bug_Yes"));
		    }
		  else
		    {
		       AppendString(&tmp, GetSetting(ad, "Bug_No"));
		    }
		  break;
		  
		case 'C': // Code, as specified with "CACHETYPE_Prefix"
		case 'y': // Added to be consistent
		  tmp2 = GetFormattedCacheType(ad);
		  AppendString(&tmp2, "_Prefix");
		  AppendString(&tmp, GetSetting(ad, tmp2));
		  freeMemory((void **) &tmp2);
		  break;
		  
		case 'D': // Difficulty, full string
		  AppendString(&tmp, ad->difficulty);
		  break;
		  
		case 'd': // Difficulty, as single digit
		  c = ad->difficulty[0] - '0';
		  c *= 2;
		  c --;
		  if (ad->difficulty[1] == '.' && ad->difficulty[2] == '5')
		    {
		       c ++;
		    }
		  c += '0';
		  AppendStringN(&tmp, &c, 1);
		  break;
		  
		case 'f': // Found it
		  if (ad->available) 
		    {
		       AppendString(&tmp, GetSetting(ad, "Found_Yes"));
		    }
		  else
		    {
		       AppendString(&tmp, GetSetting(ad, "Found_No"));
		    }
		  break;
		  
		case 'h': // Hint, full text
		case 'H': // Added to be consistent
		  AppendString(&tmp, ad->hints);
		  break;
		  
		case 'I': // ID (the xxxx part of GCxxxx)
		  AppendString(&tmp, ad->name);
		  tmp[0] = ' ';
		  tmp[1] = ' ';
		  break;
		  
		case 'L': // First letter of the logs
		  AppendString(&tmp, ad->logSummary);
		  break;
		  
	        case 'N': // Cache name (TODO: smart truncated)
		  AppendString(&tmp, ad->urlname);
		  break;
		  
		case 'O': // Owner, full name
		  AppendString(&tmp, ad->owner);
		  break;
		  
		case 'P': // Placed by, full name
		  AppendString(&tmp, ad->placedBy);
		  break;
		  
		case 'S': // Size, full string
		  AppendString(&tmp, ad->container);
		  break;
		  
		case 's': // Size, first letter
		  AppendStringN(&tmp, ad->container, 1);
		  break;

		case 'T': // Terrain, full string
		  AppendString(&tmp, ad->terrain);
		  break;
		  
		case 't': // Terrain, as single digit
		  c = ad->terrain[0] - '0';
		  c *= 2;
		  c --;
		  if (ad->terrain[1] == '.' && ad->terrain[2] == '5')
		    {
		       c ++;
		    }
		  c += '0';
		  AppendStringN(&tmp, &c, 1);
		  break;
		  
		case 'Y': // Cache type, full name
		  AppendString(&tmp, ad->type2);
		  break;
		  
		case '%': // Literals
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		  AppendStringN(&tmp, current, 1);
		  length = -2;
		  break;
		  
		default:
		  AppendStringN(&tmp, current - 1, 2);
		  length = -2;
	       }
	     
	     current ++;
	     
	     // Remove beginning whitespace and strip invalid characters
	     i = 0;
	     j = 0;
	     while (tmp[i] == '\t' || tmp[i] == '\r' || 
		    tmp[i] == '\n' || tmp[i] == ' ')
	       {
		  i ++;
	       }
	     while (tmp[i] != '\0')
	       {
		  c = tmp[i];
		  if ((c >= '0' && c <= '9') ||
		      (c >= 'A' && c <= 'Z') ||
		      (c >= 'a' && c <= 'z') ||
		      c == ' ' || c == '.')
		    {
		       // A "known good" letter
		       tmp[j] = c;
		       j ++;
		    }
		  else if (allowed == NULL)
		    {
		       // No allowed list, so let's say all are allowed
		       tmp[j] = c;
		       j ++;
		    }
		  else if (allowed[0] != '\0')
		    {
		       for (k = 0; k > 0 && allowed[k] != '\0'; k ++)
			 {
			    if (allowed[k] == c)
			      {
				 // A letter specified in the ini file
				 tmp[j] = c;
				 j ++;
			      }
			 }
		    }
		  i ++;
	       }
	     tmp[j] = '\0';
	     j --;
	     
	     // Remove trailing whitespace
	     while (tmp[j] == '\t' || tmp[j] == '\r' || 
		    tmp[j] == '\n' || tmp[j] == ' ')
	       {
		  tmp[j] = '\0';
		  j --;
	       }

	     // Handle length specifiers (numbers after the code) if
	     // length == -1
	     if (length == -1 && *current >= '0' && *current <= '0')
	       {
		  // Find and parse a number after the format code
		  length = 0;
		  while (*current >= '0' && *current <= '9')
		    {
		       length *= 10;
		       length += *current - '0';
		       current ++;
		    }
	       }
	     if (length > 0)
	       {
		  // Static length - Easy
		  if (strlen(tmp) > length)
		    tmp[length] = '\0';
	       }
	     
	     //fprintf(ad->fpout, "Processed:  %s\n", tmp);
	     
	     if (length == 0)
	       {
		  // AutoFit - we handle this a bit later.
		  c = 0xFF;
		  AppendStringN(&out, &c, 1);
		  AppendString(&out, tmp);
		  AppendStringN(&out, &c, 1);
	       }
	     else
	       {
		  AppendString(&out, tmp);
	       }
	     
	     // Finish up
	     freeMemory((void **) &tmp);
	     lastEnd = current;
	  }
     }
   
   // Copy any remaining characters
   if (lastEnd != current)
     {
	AppendStringN(&out, lastEnd, current - lastEnd);
     }
   
   // Reformat the auto-size fields
   if (max_length > 0)
     {
	AutoSizeString(out, max_length);
	
	if (strlen(out) > max_length) 
	  {
	     out[max_length] = '\0';
	  }
     }
   else
     {
	// Remove the special markers
	AutoSizeString(out, strlen(out));
     }
   

   return out;
}


void WriteSymTag(AppData *ad)
{
   char *formattedType = NULL;
   char *keyName = NULL, *value = NULL, *foundStatus = NULL;
   
   formattedType = GetFormattedCacheType(ad);
   
   if (strcmp(ad->sym, "Geocache Found") == 0)
     {
	AppendString(&foundStatus, "Found");
     }
   else
     {
	AppendString(&foundStatus, "Not_Found");
     }
   
   // TYPE_SIZE_FOUND
   AppendString(&keyName, formattedType);
   AppendString(&keyName, "_");
   AppendString(&keyName, ad->container);
   AppendString(&keyName, "_");
   AppendString(&keyName, foundStatus);
   value = GetSetting(ad, keyName);
   freeMemory((void **) &keyName);
   
   // TYPE_FOUND
   if (value == NULL) 
     {
	AppendString(&keyName, formattedType);
	AppendString(&keyName, "_");
	AppendString(&keyName, foundStatus);
	value = GetSetting(ad, keyName);
	freeMemory((void **) &keyName);
     }
   
   // FOUND
   if (value == NULL) 
     {
	AppendString(&keyName, foundStatus);
	value = GetSetting(ad, keyName);
	freeMemory((void **) &keyName);
     }
   
   if (value == NULL)
     {
	value = ad->sym;
     }
   
   fprintf(ad->fpout, "\n    <sym>%s</sym>", value);
   
   freeMemory((void **) &formattedType);
   freeMemory((void **) &foundStatus);
}


void WriteFormattedTags(AppData *ad)
{
   char *format, *output = NULL;
   
//   fprintf(ad->fpout, "Name: %s\nDesc: %s\nUrlname: %s\nSym: %s\n",
//	   ad->name, ad->desc, ad->urlname, ad->sym);
//   fprintf(ad->fpout, "Type: %s\nAvailable: %d\nArchived: %d\n",
//	   ad->type, ad->available, ad->archived);
//   fprintf(ad->fpout, "Placed By: %s\nOwner: %s\nType 2: %s\n",
//	   ad->placedBy, ad->owner, ad->type2);
//   fprintf(ad->fpout, "Container: %s\nDifficulty: %s\nTerrain: %s\n",
//	   ad->container, ad->difficulty, ad->terrain);
//   fprintf(ad->fpout, "Hints: %s\nLogs: %s\nBugs: %d\n",
//	   ad->hints, ad->logSummary, ad->bugs);

   format = GetSetting(ad, "Waypoint_Format");
   if (format != NULL)
     {
	output = AssembleFormat(ad, format, "Waypoint");
     }
   else
     {
	AppendString(&output, ad->name);
     }
   
   fprintf(ad->fpout, "\n    <name>%s</name>", output);
   freeMemory((void **) &output);
   
   format = GetSetting(ad, "Desc_Format");
   if (format != NULL)
     {
	output = AssembleFormat(ad, format, "Desc");
     }
   else
     {
	AppendString(&output, ad->desc);
     }
   
   fprintf(ad->fpout, "\n    <desc>%s</desc>", output);
   freeMemory((void **) &output);
   
   WriteSymTag(ad);
}


void ClearAppData(AppData *ad, int full)
{
   SettingsStruct *ptr, *nxt;

   freeMemory((void **) &(ad->wpt_tag));
   freeMemory((void **) &(ad->wpt_data));
   freeMemory((void **) &(ad->name));
   freeMemory((void **) &(ad->desc));
   freeMemory((void **) &(ad->urlname));
   freeMemory((void **) &(ad->sym));
   freeMemory((void **) &(ad->type));
   ad->available = 0;
   ad->archived = 0;
   freeMemory((void **) &(ad->placedBy));
   freeMemory((void **) &(ad->owner));
   freeMemory((void **) &(ad->type2));
   freeMemory((void **) &(ad->container));
   freeMemory((void **) &(ad->difficulty));
   freeMemory((void **) &(ad->terrain));
   freeMemory((void **) &(ad->hints));
   freeMemory((void **) &(ad->logSummary));
   ad->bugs = 0;
   
   if (full)
     {
	ptr = ad->settings;
	while (ptr != NULL) 
	  {
	     nxt = ptr->next;
	     freeMemory((void **) &(ptr->key));
	     freeMemory((void **) &(ptr->value));
	     freeMemory((void **) &ptr);
	     ptr = nxt;
	  }
	ad->settings = NULL;
     }
}


void WriteSetting(AppData *ad, const char *key, const char *value)
{
   SettingsStruct *ss;
   
   ss = getMemory(sizeof(SettingsStruct));
   AppendString(&(ss->key), key);
   LowercaseString(ss->key);
   AppendString(&(ss->value), value);
   ss->next = ad->settings;
   ad->settings = ss;
}


void WriteDefaultSettings(AppData *ad)
{
   WriteSetting(ad, "Benchmark_Prefix", "X");
   WriteSetting(ad, "CITO_Event_Prefix", "C");
   WriteSetting(ad, "Earthcache_Prefix", "G");
   WriteSetting(ad, "Event_Prefix", "E");
   WriteSetting(ad, "Letterbox_Hybrid_Prefix", "B");
   WriteSetting(ad, "Locationless_Prefix", "L");
   WriteSetting(ad, "Mega_Prefix", "E");
   WriteSetting(ad, "Multi_Prefix", "M");
   WriteSetting(ad, "Project_APE_Prefix", "A");
   WriteSetting(ad, "Traditional_Prefix", "T");
   WriteSetting(ad, "Unknown_Prefix", "U");
   WriteSetting(ad, "Virtual_Prefix", "V");
   WriteSetting(ad, "Webcam_Prefix", "W");
   
   WriteSetting(ad, "Active_Yes", "Y");
   WriteSetting(ad, "Active_No", "N");
   WriteSetting(ad, "Bug_Yes", "Y");
   WriteSetting(ad, "Bug_No", "N");
   WriteSetting(ad, "Found_Yes", "Y");
   WriteSetting(ad, "Found_No", "N");
}


void ReadSettings(AppData *ad, const char *filename)
{
   FILE *fp;
   char *buffer, *key_start, *value_start, *tmp_ptr;
   
   fp = fopen(filename, "r");
   if (! fp)
     {
	fprintf(stderr, "Error opening settings file: %s\n", filename);
	exit(10);
     }
   
   buffer = getMemory(SETTING_LINE_LEN);
   while (! feof(fp))
     {
	fgets(buffer, SETTING_LINE_LEN, fp);
	//printf("buff: %s", buffer);

	// Trim the end of the string
	tmp_ptr = buffer + strlen(buffer) - 1;
	while (*tmp_ptr == '\n' || *tmp_ptr == '\r' ||
	       *tmp_ptr == '\t' || *tmp_ptr == ' ')
	  {
	     *tmp_ptr = '\0';
	     tmp_ptr --;
	  }
	
	// Trim the beginning
	key_start = buffer;
	while (*key_start == ' ' || *key_start == '\t')
	  {
	     key_start ++;
	  }
	
	// Find the value beginning and trim end of the key and
	// the beginning of the value
	value_start = key_start + 1;
	while (*value_start != '=' && *value_start != '\0')
	  {
	     value_start ++;
	  }
	if (*value_start != '\0')
	  {
	     *value_start = ' ';
	     value_start --;
	     while (*value_start == '\t' || *value_start == ' ')
	       {
		  value_start --;
	       }
	     value_start ++;
	     while (*value_start == '\t' || *value_start == ' ')
	       {
		  *value_start = '\0';
		  value_start ++;
	       }
	     //printf("key: %s\n", key_start);
	     
	     // If we have something, save it.
	     if (*value_start != '\0')
	       {
		  //printf("[%s] = [%s]\n", key_start, value_start);
		  WriteSetting(ad, key_start, value_start);
	       }
	  }
     }
   
   freeMemory((void **) &buffer);
   fclose(fp);
}


void start_hndl(void *data, const char *el, const char **attr)
{
   AppData *ad = (AppData *) data;
   int i;
   char *out;
   
//   for (i = 0; i < ad->depth; i ++) 
//     {
//	printf("  ");
//     }
//
//   printf("%s ", el);
//   for (i = 0; attr[i]; i += 2) 
//     {
//	printf("(%s = %s)", attr[i], attr[i + 1]);
//     }
//   
//   printf("\n");

   if (strcmp(el, "wpt") == 0) 
     {
	ad->in_wpt_tag ++;
     }
   if (strcmp(el, "groundspeak:log") == 0)
     {
	ad->in_log_tag ++;
     }
   if (ad->in_log_tag && strcmp(el, "groundspeak:type") == 0)
     {
	ad->in_log_type_tag ++;
     }
   
   ad->currentTarget = NULL;
   ad->copy_to_data = 1;
   
   out = NULL;
   AppendString(&out, "<");
   AppendString(&out, (const char *) el);
   for (i = 0; attr[i]; i += 2)
     {
	AppendString(&out, " ");
	AppendString(&out, (const char *) attr[i]);
	AppendString(&out, "=\"");
	AppendString(&out, (const char *) attr[i + 1]);
	AppendString(&out, "\"");
     }
   AppendString(&out, ">");
   if (ad->in_wpt_tag == 0)
     {
	fprintf(ad->fpout, out);
     }
   else if (ad->in_wpt_tag == 1)
     {
	if (strcmp(el, "wpt") == 0) 
	  {
	     AppendString(&(ad->wpt_tag), out);
	  }
	else if (strcmp(el, "name") == 0 && ad->name == NULL)
	  {
	     ad->currentTarget = &(ad->name);
	     ad->copy_to_data = 0;
	  }
	else if (strcmp(el, "desc") == 0 && ad->desc == NULL)
	  {
	     ad->currentTarget = &(ad->desc);
	     ad->copy_to_data = 0;
	  }
	else if (strcmp(el, "urlname") == 0 && ad->urlname == NULL)
	  {
	     ad->currentTarget = &(ad->urlname);
	  }
	else if (strcmp(el, "sym") == 0 && ad->sym == NULL)
	  {
	     ad->currentTarget = &(ad->sym);
	     ad->copy_to_data = 0;
	  }
	else if (strcmp(el, "type") == 0 && ad->type == NULL)
	  {
	     ad->currentTarget = &(ad->type);
	  }
	else if (strcmp(el, "groundspeak:cache") == 0)
	  {
	     for (i = 0; attr[i]; i += 2)
	       {
		  if (strcmp(attr[i], "available") == 0)
		    {
		       if (attr[i + 1][0] == 'T' || attr[i + 1][0] == 't')
			 {
			    ad->available = 1;
			 }
		    }
		  else if (strcmp(attr[i], "archived") == 0)
		    {
		       if (attr[i + 1][0] == 'T' || attr[i + 1][0] == 't')
			 {
			    ad->archived = 1;
			 }
		    }
	       }
	  }
	else if (strcmp(el, "groundspeak:placed_by") == 0 && 
		 ad->placedBy == NULL)
	  {
	     ad->currentTarget = &(ad->placedBy);
	  }
	else if (strcmp(el, "groundspeak:owner") == 0 && ad->owner == NULL)
	  {
	     ad->currentTarget = &(ad->owner);
	  }
	else if (strcmp(el, "groundspeak:type") == 0 && ad->type2 == NULL)
	  {
	     ad->currentTarget = &(ad->type2);
	  }
	else if (strcmp(el, "groundspeak:container") == 0 &&
		 ad->container == NULL)
	  {
	     ad->currentTarget = &(ad->container);
	  }
	else if (strcmp(el, "groundspeak:difficulty") == 0 &&
		 ad->difficulty == NULL)
	  {
	     ad->currentTarget = &(ad->difficulty);
	  }
	else if (strcmp(el, "groundspeak:terrain") == 0 &&
		 ad->terrain == NULL)
	  {
	     ad->currentTarget = &(ad->terrain);
	  }
	else if (strcmp(el, "groundspeak:encoded_hints") == 0 &&
		 ad->hints == NULL)
	  {
	     ad->currentTarget = &(ad->hints);
	  }
	else if (strcmp(el, "groundspeak:type") == 0 &&
		 ad->in_log_type_tag == 1)
	  {
	     ad->currentTarget = &(ad->logSummary);
	     ad->copy_to_data = 0;
	  }
	else if (strcmp(el, "groundspeak:travelbug") == 0)
	  {
	     ad->bugs ++;
	  }
	
	if (ad->copy_to_data && strcmp(el, "wpt") != 0)
	  {
	     AppendString(&(ad->wpt_data), out);
	  }
     }

   freeMemory((void **) &out);
   
   ad->depth ++;
}


void end_hndl(void *data, const char *el) 
{
   AppData *ad = (AppData *) data;
   
   if (ad->in_wpt_tag == 0)
     {
	fprintf(ad->fpout, "</%s>", el);
     }
   else if (ad->copy_to_data)
     {
	AppendString(&(ad->wpt_data), "</");
	AppendString(&(ad->wpt_data), el);
	AppendString(&(ad->wpt_data), ">");
     }
   
   if (strcmp(el, "wpt") == 0)
     {
	ad->in_wpt_tag --;
	if (ad->in_wpt_tag == 0)
	  {
	     fwrite(ad->wpt_tag, 1, strlen(ad->wpt_tag), ad->fpout);
	     WriteFormattedTags(ad);
	     fwrite(ad->wpt_data, 1, strlen(ad->wpt_data), ad->fpout);
	  }
	ClearAppData(ad, 0);
     }
   if (strcmp(el, "groundspeak:log") == 0)
     {
	ad->in_log_tag --;
     }
   if (ad->in_log_tag && strcmp(el, "groundspeak:type") == 0)
     {
	ad->in_log_type_tag --;
	if (ad->in_log_type_tag == 0)
	  {
	     ad->wrote_log_type = 0;
	  }
     }
   
   ad->currentTarget = NULL;
   ad->depth --;
}


void char_hndl(void *data, const char *txt, int txtlen)
{
   AppData *ad = (AppData *) data;
   int i, handled = 0;
   
   if (ad->in_wpt_tag == 0)
     {
	fwrite(txt, 1, txtlen, ad->fpout);
	return;
     }
   
   if (ad->in_log_type_tag && ad->wrote_log_type == 0)
     {
	for (i = 0; i < txtlen && ad->wrote_log_type == 0; i ++)
	  {
	     if (txt[i] != '\0' && txt[i] != ' ' && txt[i] != '\t' &&
		 txt[i] != '\r' && txt[i] != '\n')
	       {
		  AppendStringN(&(ad->logSummary), &(txt[i]), 1);
		  ad->wrote_log_type = 1;
	       }
	  }
	handled = 1;
     }
   
   if (ad->copy_to_data)
     {
	// Send to waypoint buffer
	AppendStringN(&(ad->wpt_data), txt, txtlen);
     }
   
   if (ad->currentTarget != NULL && ! handled)
     {
	// Also maybe send to the AppData struct
	// currentTarget is already a char **
	AppendStringN(ad->currentTarget, txt, txtlen);
     }
}


int main(int argc, char **argv)
{
   int len;
   FILE *fpin;
   AppData *ad;
   char *buffer;
   
   if (argc < 2 || argc > 4)
     {
	fprintf(stderr, "Error:  Incorrect arguments\n");
	fprintf(stderr, "    gpxrewrite settings.ini   (reads file from stdin, writes to stdout)\n");
	fprintf(stderr, "    gpxrewrite settings.ini gpxfile.gpx   (writes to stdout)\n");
	fprintf(stderr, "    gpxrewrite settings.ini gpxfile.gpx output.gpx\n");
	exit(1);
     }
   
   ad = getMemory(sizeof(AppData));
   WriteDefaultSettings(ad);
   ReadSettings(ad, argv[1]);
   
   ad->parser = XML_ParserCreate(NULL);
   if (! ad->parser)
     {
	fprintf(stderr, "Could not allocate memory for parser\n");
	exit(2);
     }

   XML_SetUserData(ad->parser, (void *) ad);
   XML_SetElementHandler(ad->parser, start_hndl, end_hndl);
   XML_SetCharacterDataHandler(ad->parser, char_hndl);
   
   if (argc >= 3)
     {
	fpin = fopen(argv[2], "r");
	if (! fpin)
	  {
	     fprintf(stderr, "Error opening input file: %s\n", argv[2]);
	     exit(3);
	  }
     }
   else
     {
	fpin = stdin;
     }
   
   if (argc >= 4)
     {
	ad->fpout = fopen(argv[3], "w");
	if (! ad->fpout)
	  {
	     fprintf(stderr, "Error opening output file: %s\n", argv[3]);
	     exit(4);
	  }
     }
   else
     {
	ad->fpout = stdout;
     }
   
   fprintf(ad->fpout, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
   
   while (! feof(fpin)) 
     {
	buffer = XML_GetBuffer(ad->parser, CHUNK_SIZE);
	
	if (! buffer) 
	  {
	     fprintf(stderr, "Ran out of memory for parse buffer!\n");
	     exit(4);
	  }
	
	len = fread(buffer, 1, CHUNK_SIZE, fpin);
	if (ferror(fpin)) 
	  {
	     fprintf(stderr, "Read error\n");
	     exit(5);
	  }
	if (! XML_ParseBuffer(ad->parser, len, feof(fpin)))
	  {
	     fprintf(stderr, "Parse error at line %d:\n%s\n",
		     XML_GetCurrentLineNumber(ad->parser),
		     XML_ErrorString(XML_GetErrorCode(ad->parser)));
	     exit(6);
	  }
     }
   
   fprintf(ad->fpout, "\n");
   
   if (fpin != stdin)
     {
	fclose(fpin);
     }
   
   if (ad->fpout != stdout)
     {
	fclose(ad->fpout);
     }
   
   ClearAppData(ad, 1);
   freeMemory((void **) &ad);

   return 0;
}
