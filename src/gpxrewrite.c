/* This file is part of the package gpx_tools
 * - Main source file for gpxrewrite tool
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
#include "../config.h"
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else
#error NO STDLIB_H
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#else
#error NO STRING_H
#endif

#include "config.h"
#include "ini_settings.h"
#include "mem_str.h"
#include "waypoint.h"


typedef struct app_data 
{
   FILE *fpout;
   SettingsStruct *settings;
} AppData;


char *GetFormattedCacheType(Waypoint_Info *wpi)
{
   char *formattedType = NULL;
   int i;
   
   AppendStringN(&formattedType, &(wpi->WaypointXML[wpi->type2_off]),
		 wpi->type2_len);
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


char *AssembleFormat(Waypoint_Info *wpi, AppData *ad, 
		     char *Format, char *NameType)
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
   
   int length, i, j, k, max_length, should_escape;
   char *out = NULL, *lastEnd, *current, *tmp = NULL, *tmp2 = NULL;
   char c, *allowed;

   AppendString(&tmp, NameType);
   AppendString(&tmp, "_Allowed_Chars");
   allowed = GetSetting(ad->settings, tmp);
   freeMemory((void **) &tmp);
   
   AppendString(&tmp, NameType);
   AppendString(&tmp, "_Max_Length");
   tmp2 = GetSetting(ad->settings, tmp);
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
   should_escape = 1;
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
		  if (wpi->available) 
		    {
		       AppendString(&tmp, GetSetting(ad->settings, "Active_Yes"));
		    }
		  else
		    {
		       AppendString(&tmp, GetSetting(ad->settings, "Active_No"));
		    }
		  break;
		  
		case 'b': // Bugs
		  if (wpi->bugs)
		    {
		       AppendString(&tmp, GetSetting(ad->settings, "Bug_Yes"));
		    }
		  else
		    {
		       AppendString(&tmp, GetSetting(ad->settings, "Bug_No"));
		    }
		  break;
		  
		case 'C': // Code, as specified with "CACHETYPE_Prefix"
		case 'y': // Added to be consistent
		  tmp2 = GetFormattedCacheType(wpi);
		  AppendString(&tmp2, "_Prefix");
		  AppendString(&tmp, GetSetting(ad->settings, tmp2));
		  freeMemory((void **) &tmp2);
		  break;
		  
		case 'D': // Difficulty, full string
		  AppendStringN(&tmp, 
				&(wpi->WaypointXML[wpi->difficulty_off]),
				wpi->difficulty_len);
		  break;
		  
		case 'd': // Difficulty, as single digit
		  i = ChangeToSingleNumber(&(wpi->WaypointXML[wpi->difficulty_off]),
					   wpi->difficulty_len);
		  c = i + '0';
		  AppendStringN(&tmp, &c, 1);
		  break;
		  
		case 'f': // Found it
		  if (wpi->available) 
		    {
		       AppendString(&tmp, GetSetting(ad->settings, "Found_Yes"));
		    }
		  else
		    {
		       AppendString(&tmp, GetSetting(ad->settings, "Found_No"));
		    }
		  break;
		  
		case 'h': // Hint, full text
		case 'H': // Added to be consistent
		  AppendStringN(&tmp2, &(wpi->WaypointXML[wpi->hints_off]),
				wpi->hints_len);
		  HTMLUnescapeString(&tmp2);
		  AppendString(&tmp, tmp2);
		  freeMemory((void **) &tmp2);
		  break;
		  
		case 'I': // ID (the xxxx part of GCxxxx)
		  AppendStringN(&tmp, &(wpi->WaypointXML[wpi->name_off]),
				wpi->name_len);
		  if (wpi->name_len > 2 && strncmp(tmp, "GC", 2) == 0)
		    {
		       tmp[0] = ' ';
		       tmp[1] = ' ';
		    }
		  break;
		  
		case 'L': // First letter of the logs
		  AppendString(&tmp, wpi->logSummary);
		  break;
		  
	        case 'N': // Cache name
		  // TODO: smart truncated - see doc/smart_truncation
		  AppendStringN(&tmp2, &(wpi->WaypointXML[wpi->urlname_off]),
			       wpi->urlname_len);
		  HTMLUnescapeString(&tmp2);
		  AppendString(&tmp, tmp2);
		  freeMemory((void **) &tmp2);
		  break;
		  
		case 'O': // Owner, full name
		  AppendStringN(&tmp2, &(wpi->WaypointXML[wpi->owner_off]),
			       wpi->owner_len);
		  HTMLUnescapeString(&tmp2);
		  AppendString(&tmp, tmp2);
		  freeMemory((void **) &tmp2);
		  break;
		  
		case 'P': // Placed by, full name
		  AppendStringN(&tmp2, &(wpi->WaypointXML[wpi->placedBy_off]),
			       wpi->placedBy_len);
		  HTMLUnescapeString(&tmp2);
		  AppendString(&tmp, tmp2);
		  freeMemory((void **) &tmp2);
		  break;
		  
		case 'S': // Size, full string
		  AppendStringN(&tmp, &(wpi->WaypointXML[wpi->container_off]),
			       wpi->container_len);
		  break;
		  
		case 's': // Size, first letter
		  if (wpi->container_len)
		    {
		       AppendStringN(&tmp, &(wpi->WaypointXML[wpi->container_off]),
				     1);
		    }
		  break;

		case 'T': // Terrain, full string
		  AppendStringN(&tmp, &(wpi->WaypointXML[wpi->terrain_off]),
				wpi->terrain_len);
		  break;
		  
		case 't': // Terrain, as single digit
		  i = ChangeToSingleNumber(&(wpi->WaypointXML[wpi->terrain_off]),
					   wpi->terrain_len);
		  c = i + '0';
		  AppendStringN(&tmp, &c, 1);
		  break;
		  
		case 'Y': // Cache type, full name
		  AppendStringN(&tmp, &(wpi->WaypointXML[wpi->type2_off]),
				wpi->type2_len);
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
		       for (k = 0; k >= 0 && allowed[k] != '\0'; k ++)
			 {
			    if (allowed[k] == c)
			      {
				 // A letter specified in the ini file
				 tmp[j] = c;
				 j ++;
				 // k gets incremented at the end of the
				 // loop, so -1 won't work here
				 k = -2;
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
	     if (length == -1 && *current >= '0' && *current <= '9')
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

   return out;
}


// Static char or NULL if no change.
// Needs to be HTML escaped.
char *BuildSymTag(Waypoint_Info *wpi, AppData *ad)
{
   char *formattedType = NULL;
   char *keyName = NULL, *value = NULL, *foundStatus = NULL;
   
   if (wpi->sym_len == 0)
     {
	return (char *) NULL;
     }
   
   formattedType = GetFormattedCacheType(wpi);
   
   if (wpi->sym_len == 14 &&
       strncmp(&(wpi->WaypointXML[wpi->sym_off]), "Geocache Found",
	       wpi->sym_len) == 0)
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
   AppendStringN(&keyName, &(wpi->WaypointXML[wpi->container_off]),
		 wpi->container_len);
   AppendString(&keyName, "_");
   AppendString(&keyName, foundStatus);
   value = GetSetting(ad->settings, keyName);
   freeMemory((void **) &keyName);
   
   // TYPE_FOUND
   if (value == NULL) 
     {
	AppendString(&keyName, formattedType);
	AppendString(&keyName, "_");
	AppendString(&keyName, foundStatus);
	value = GetSetting(ad->settings, keyName);
	freeMemory((void **) &keyName);
     }
   
   // FOUND
   if (value == NULL) 
     {
	AppendString(&keyName, foundStatus);
	value = GetSetting(ad->settings, keyName);
	freeMemory((void **) &keyName);
     }
   
   freeMemory((void **) &formattedType);
   freeMemory((void **) &foundStatus);

   return value;
}


void WriteFormattedTags(Waypoint_Info *wpi, AppData *ad)
{
   char *format, *output = NULL;
   
   format = GetSetting(ad->settings, "Waypoint_Format");
   if (format != NULL)
     {
	output = AssembleFormat(wpi, ad, format, "Waypoint");
     }
   else
     {
	AppendStringN(&output, &(wpi->WaypointXML[wpi->name_off]),
		      wpi->name_len);
     }
   
   HTMLEscapeString(&output);
   SwapWaypointString(wpi, wpi->name_off, wpi->name_len, output);
   freeMemory((void **) &output);
   
   format = GetSetting(ad->settings, "Desc_Format");
   if (format != NULL)
     {
	output = AssembleFormat(wpi, ad, format, "Desc");
     }
   else
     {
	AppendStringN(&output, &(wpi->WaypointXML[wpi->gcname_off]),
		      wpi->gcname_len);
     }
   
   HTMLEscapeString(&output);
   SwapWaypointString(wpi, wpi->gcname_off, wpi->gcname_len, output);
   freeMemory((void **) &output);
   
   output = BuildSymTag(wpi, ad);
   SwapWaypointString(wpi, wpi->sym_off, wpi->sym_len, output);
   // Don't free this one
}


void WriteDefaultSettings(SettingsStruct **head)
{
   WriteSetting(head, "Benchmark_Prefix", "X");
   WriteSetting(head, "CITO_Event_Prefix", "C");
   WriteSetting(head, "Earthcache_Prefix", "G");
   WriteSetting(head, "Event_Prefix", "E");
   WriteSetting(head, "Letterbox_Hybrid_Prefix", "B");
   WriteSetting(head, "Locationless_Prefix", "L");
   WriteSetting(head, "Mega_Prefix", "E");
   WriteSetting(head, "Multi_Prefix", "M");
   WriteSetting(head, "Project_APE_Prefix", "A");
   WriteSetting(head, "Traditional_Prefix", "T");
   WriteSetting(head, "Unknown_Prefix", "U");
   WriteSetting(head, "Virtual_Prefix", "V");
   WriteSetting(head, "Webcam_Prefix", "W");
   
   WriteSetting(head, "Active_Yes", "Y");
   WriteSetting(head, "Active_No", "N");
   WriteSetting(head, "Bug_Yes", "Y");
   WriteSetting(head, "Bug_No", "N");
   WriteSetting(head, "Found_Yes", "Y");
   WriteSetting(head, "Found_No", "N");
}


void WaypointHandler(Waypoint_Info *wpi, void *extra_data)
{
   AppData *ad = (AppData *) extra_data;
   
   WriteFormattedTags(wpi, ad);

   fputs(wpi->WaypointXML, ad->fpout);
}


void NonWaypointHandler(const char *txt, int len, void *extra_data)
{
   char *out = NULL;
   AppData *ad = (AppData *) extra_data;
   
   AppendStringN(&out, txt, len);
   fputs(out, ad->fpout);
   freeMemory((void **) &out);
}


int main(int argc, char **argv)
{
   FILE *fpin;
   AppData *ad;
   
   if (argc < 2 || argc > 4)
     {
	fprintf(stderr, "gpxrewrite - Part of %s\n", PACKAGE_STRING);
	fprintf(stderr, "Error:  Incorrect arguments\n");
	fprintf(stderr, "    gpxrewrite settings.ini   (reads file from stdin, writes to stdout)\n");
	fprintf(stderr, "    gpxrewrite settings.ini gpxfile.gpx   (writes to stdout)\n");
	fprintf(stderr, "    gpxrewrite settings.ini gpxfile.gpx output.gpx\n");
	exit(1);
     }
   
   ad = getMemory(sizeof(AppData));
   WriteDefaultSettings(&(ad->settings));
   ReadSettings(&(ad->settings), argv[1]);
   
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
   
   fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", ad->fpout);
   ParseXML(fpin, &WaypointHandler, &NonWaypointHandler, (void *) ad);
   fprintf(ad->fpout, "\n");
   
   if (fpin != stdin)
     {
	fclose(fpin);
     }
   
   if (ad->fpout != stdout)
     {
	fclose(ad->fpout);
     }

   FreeSettings(&(ad->settings));
   freeMemory((void **) &ad);

   return 0;
}
