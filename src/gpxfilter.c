/* This file is part of the package gpx_tools
 * - Main source file for gpxfilter tool
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
#define NEED_STDIO_H
#define NEED_STRING_H
#include "include.h"
#include "ini_settings.h"
#include "mem_str.h"
#include "waypoint.h"


typedef struct app_data 
{
   int diff_min, diff_max;
   int terr_min, terr_max;
   char *sizes;
   char *types;
   SettingsStruct *sizes_map;
   SettingsStruct *types_map;
   FILE *fpout;
} AppData;


void WriteDefaultSizes(SettingsStruct **head)
{
   WriteSetting(head, "Other", "O");
   WriteSetting(head, "Virtual", "V");
   WriteSetting(head, "Large", "L");
   WriteSetting(head, "Regular", "R");
   WriteSetting(head, "Small", "S");
   WriteSetting(head, "Micro", "M");
   WriteSetting(head, "Unknown", "U");
}


void WriteDefaultTypes(SettingsStruct **head)
{
   WriteSetting(head, "Webcam Cache", "W");
   WriteSetting(head, "Virtual Cache", "V");
   WriteSetting(head, "Unknown Cache", "U");
   WriteSetting(head, "Traditional Cache", "T");
   WriteSetting(head, "Project APE Cache", "A");
   WriteSetting(head, "Multi-cache", "M");
   WriteSetting(head, "Mega-Event Cache", "E");
   WriteSetting(head, "Locationless (Reverse) Cache", "L");
   WriteSetting(head, "Letterbox Hybrid", "B");
   WriteSetting(head, "Event Cache", "E");
   WriteSetting(head, "Earthcache", "G");
   WriteSetting(head, "Cache In Trash Out Event", "C");
   WriteSetting(head, "Benchmark", "X");
}


void WaypointHandler(Waypoint_Info *wpi, void *extra_data)
{
   int i;
   AppData *ad = (AppData *) extra_data;
   char *key = NULL, *value = NULL;
   
   if (ad->diff_min > 0 || ad->diff_max > 0) 
     {
	i = ChangeToSingleNumber(&(wpi->WaypointXML[wpi->difficulty_off]),
				 wpi->difficulty_len);
	if (ad->diff_min && i < ad->diff_min) 
	  {
	     return;
	  }
	if (ad->diff_max && i > ad->diff_max)
	  {
	     return;
	  }
     }
   if (ad->terr_min > 0 || ad->terr_max > 0) 
     {
	i = ChangeToSingleNumber(&(wpi->WaypointXML[wpi->terrain_off]),
				 wpi->terrain_len);
	if (ad->terr_min && i < ad->terr_min) 
	  {
	     return;
	  }
	if (ad->terr_max && i > ad->terr_max)
	  {
	     return;
	  }
     }
   if (ad->sizes)
     {
	AppendStringN(&key, &(wpi->WaypointXML[wpi->container_off]),
		      wpi->container_len);
	value = GetSetting(ad->sizes_map, key);
	freeMemory((void **) &key);
	if (! value) 
	  {
	     return;
	  }
	i = 0;
	while (ad->sizes[i] != value[0])
	  {
	     if (ad->sizes[i] == '\0')
	       {
		  return;
	       }
	     i ++;
	  }
     }
   if (ad->types)
     {
	AppendStringN(&key, &(wpi->WaypointXML[wpi->type2_off]),
		      wpi->type2_len);
	value = GetSetting(ad->types_map, key);
	freeMemory((void **) &key);
	if (! value) 
	  {
	     return;
	  }
	i = 0;
	while (ad->types[i] != value[0])
	  {
	     if (ad->types[i] == '\0')
	       {
		  return;
	       }
	     i ++;
	  }
     }
   
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


void FreeAppData(AppData **adp)
{
   AppData *ad = *adp;
   
   
   FreeSettings(&(ad->sizes_map));
   FreeSettings(&(ad->types_map));
   
   freeMemory((void **) &(ad->sizes));
   freeMemory((void **) &(ad->types));
   freeMemory((void **) adp);
}


char *ParseArguments(AppData *ad, int argc, char **argv)
{
   int i;
   char *errmsg = NULL;
   
   for (i = 3; i < argc; i ++)
     {
	if (strcmp(argv[i], "-mindiff") == 0)
	  {
	     if (argc <= i + 1) 
	       {
		  AppendString(&errmsg, "Argument ");
		  AppendString(&errmsg, argv[i]);
		  AppendString(&errmsg, " requires another argument after it.");
		  return errmsg;
	       }
	     i ++;
	     ad->diff_min = ChangeToSingleNumber(argv[i], strlen(argv[i]));
	  }
	else if (strcmp(argv[i], "-maxdiff") == 0)
	  {
	     if (argc <= i + 1) 
	       {
		  AppendString(&errmsg, "Argument ");
		  AppendString(&errmsg, argv[i]);
		  AppendString(&errmsg, " requires another argument after it.");
		  return errmsg;
	       }
	     i ++;
	     ad->diff_max = ChangeToSingleNumber(argv[i], strlen(argv[i]));
	  }
	else if (strcmp(argv[i], "-minterr") == 0)
	  {
	     if (argc <= i + 1) 
	       {
		  AppendString(&errmsg, "Argument ");
		  AppendString(&errmsg, argv[i]);
		  AppendString(&errmsg, " requires another argument after it.");
		  return errmsg;
	       }
	     i ++;
	     ad->terr_min = ChangeToSingleNumber(argv[i], strlen(argv[i]));
	  }
	else if (strcmp(argv[i], "-maxterr") == 0)
	  {
	     if (argc <= i + 1) 
	       {
		  AppendString(&errmsg, "Argument ");
		  AppendString(&errmsg, argv[i]);
		  AppendString(&errmsg, " requires another argument after it.");
		  return errmsg;
	       }
	     i ++;
	     ad->terr_max = ChangeToSingleNumber(argv[i], strlen(argv[i]));
	  }
	else if (strcmp(argv[i], "-size") == 0)
	  {
	     if (argc <= i + 1) 
	       {
		  AppendString(&errmsg, "Argument ");
		  AppendString(&errmsg, argv[i]);
		  AppendString(&errmsg, " requires another argument after it.");
		  return errmsg;
	       }
	     i ++;
	     freeMemory((void **) &(ad->sizes));
	     AppendString(&(ad->sizes), argv[i]);
	     UppercaseString(ad->sizes);
	  }
	else if (strcmp(argv[i], "-type") == 0)
	  {
	     if (argc <= i + 1) 
	       {
		  AppendString(&errmsg, "Argument ");
		  AppendString(&errmsg, argv[i]);
		  AppendString(&errmsg, " requires another argument after it.");
		  return errmsg;
	       }
	     i ++;
	     freeMemory((void **) &(ad->types));
	     AppendString(&(ad->types), argv[i]);
	     UppercaseString(ad->types);
	  }
	else
	  {
	     AppendString(&errmsg, "Unknown argument:  ");
	     AppendString(&errmsg, argv[i]);
	     return errmsg;
	  }
     }
   
   return errmsg;
}


int main(int argc, char **argv)
{
   FILE *fpin;
   AppData *ad;
   SettingsStruct *ss;
   char *errmsg;
   
   ad = getMemory(sizeof(AppData));
   WriteDefaultSizes(&(ad->sizes_map));
   WriteDefaultTypes(&(ad->types_map));
   
   errmsg = ParseArguments(ad, argc, argv);
   if (argc < 3 || errmsg)
     {
	fprintf(stderr, "gpxfilter - Part of %s\n", PACKAGE_STRING);
	if (errmsg != NULL) 
	  {
	     fprintf(stderr, "%s\n", errmsg);
	     freeMemory((void **) &errmsg);
	  }
	fprintf(stderr, "Error:  Incorrect arguments\n");
	fprintf(stderr, "    gpxfilter infile.gpx outfile.gpx [filter [filter [...]]]\n");
	fprintf(stderr, "Writes all caches that match all of the listed filters.\n");
	fprintf(stderr, "If either infile.gpx or outfile.gpx is -, stdin or stdout is used instead.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Filters:\n");
	fprintf(stderr, "  -mindiff X  Sets the lowest allowed difficulty rating\n");
	fprintf(stderr, "  -maxdiff X  Sets the highest allowed difficulty rating\n");
	fprintf(stderr, "  -minterr X  Sets the lowest allowed terrain rating\n");
	fprintf(stderr, "  -maxterr X  Sets the highest allowed terrain rating\n");
	fprintf(stderr, "  -size X     Sets the list of codes for allowed sizes\n");
	ss = ad->sizes_map;
	while (ss != NULL)
	  {
	     fprintf(stderr, "      %s = %s\n", ss->value, ss->key_orig);
	     ss = ss->next;
	  }
	fprintf(stderr, "  -type X     Sets the list of codes for allowed containers\n");
	ss = ad->types_map;
	while (ss != NULL)
	  {
	     fprintf(stderr, "      %s = %s\n", ss->value, ss->key_orig);
	     ss = ss->next;
	  }
	fprintf(stderr, "\n");
	fprintf(stderr, "Example:  Find all caches whose terrains are easy but are difficult hides.\n");
	fprintf(stderr, "Only show traditional and unknown cache types with a container size of\n");
	fprintf(stderr, "micro through large (no virtual, other, or unknown sizes).\n");
	fprintf(stderr, "  gpxfilter caches.gpx -maxterr 2 -mindiff 3.5 -size MSRL -type TU\n");

	FreeAppData(&ad);
	exit(1);
     }
      
   if (strcmp(argv[1], "-"))
     {
	// File name
	fpin = fopen(argv[1], "r");
	if (! fpin)
	  {
	     fprintf(stderr, "Error opening input file: %s\n", argv[1]);
	     exit(3);
	  }
     }
   else
     {
	fpin = stdin;
     }
   
   if (strcmp(argv[2], "-"))
     {
	ad->fpout = fopen(argv[2], "w");
	if (! ad->fpout)
	  {
	     fprintf(stderr, "Error opening output file: %s\n", argv[2]);
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

   FreeAppData(&ad);

   return 0;
}
