/* This file is part of the package gpx_tools
 * - Main source file for gpxinfo tool
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

#include "../config.h"
#include "mem_str.h"
#include "waypoint.h"


typedef struct type_tally_struct
{
   char *type;  // Dynamically allocated
   char *typeorig;
   int typelen;
   int count;
   struct type_tally_struct *next;
} TypeTally;


typedef struct app_data 
{
   int waypoints;
   int available;
   int archived;
   int id_set, id_min, id_max;
   int lat_set;
   float lat_min, lat_max;
   int lon_set;
   float lon_min, lon_max;
   TypeTally *difficulty;
   TypeTally *terrain;
   TypeTally *type;
   TypeTally *size;
} AppData;


void IncrementType(TypeTally **head, char *key, int key_len)
{
   TypeTally *node;
   char *keystr = NULL;
   
   AppendStringN(&keystr, key, key_len);
   LowercaseString(keystr);
   
   if (*head == NULL)
     {
	node = (TypeTally *) getMemory(sizeof(TypeTally));
	AppendStringN(&(node->typeorig), key, key_len);
	node->type = keystr;
	node->typelen = key_len;
	node->count = 1;
	*head = node;
	return;
     }
   
   node = *head;
   while (1)
     {
	if (node->typelen == key_len)
	  {
	     if (strcmp(keystr, node->type) == 0)
	       {
		  node->count ++;
		  freeMemory((void **) &keystr);
		  return;
	       }
	  }
	
	if (node->next == NULL)
	  {
	     node->next = (TypeTally *) getMemory(sizeof(TypeTally));
	     node = node->next;
	     AppendStringN(&(node->typeorig), key, key_len);
	     node->type = keystr;
	     node->typelen = key_len;
	     node->count = 1;
	     return;
	  }
	
	node = node->next;
     }
}


void WaypointHandler(Waypoint_Info *wpi, void *extra_data)
{
   AppData *ad = (AppData *) extra_data;
   int i;
   
   ad->waypoints ++;
   if (wpi->available) 
     {
	ad->available ++;
     }
   if (wpi->archived)
     {
	ad->archived ++;
     }
   if (wpi->id != 0)
     {
	if (ad->id_set) 
	  {
	     if (ad->id_min > wpi->id)
	       {
		  ad->id_min = wpi->id;
	       }
	     else if (ad->id_max < wpi->id)
	       {
		  ad->id_max = wpi->id;
	       }
	  }
	else
	  {
	     ad->id_set = 1;
	     ad->id_min = wpi->id;
	     ad->id_max = wpi->id;
	  }
     }
   if (ad->lat_set) 
     {
	if (ad->lat_min > wpi->lat)
	  {
	     ad->lat_min = wpi->lat;
	  }
	else if (ad->lat_max < wpi->lat)
	  {
	     ad->lat_max = wpi->lat;
	  }
     }
   else
     {
	ad->lat_set = 1;
	ad->lat_min = wpi->lat;
	ad->lat_max = wpi->lat;
     }
   if (ad->lon_set) 
     {
	if (ad->lon_min > wpi->lon)
	  {
	     ad->lon_min = wpi->lon;
	  }
	else if (ad->lon_max < wpi->lon)
	  {
	     ad->lon_max = wpi->lon;
	  }
     }
   else
     {
	ad->lon_set = 1;
	ad->lon_min = wpi->lon;
	ad->lon_max = wpi->lon;
     }
   IncrementType(&(ad->difficulty),
		 &(wpi->WaypointXML[wpi->difficulty_off]),
		 wpi->difficulty_len);
   IncrementType(&(ad->terrain),
		 &(wpi->WaypointXML[wpi->terrain_off]),
		 wpi->terrain_len);
   IncrementType(&(ad->type),
		 &(wpi->WaypointXML[wpi->type2_off]),
		 wpi->type2_len);
   IncrementType(&(ad->size),
		 &(wpi->WaypointXML[wpi->container_off]),
		 wpi->container_len);
}


// Also frees the memory
// sort_order = 0 for type, 1 for count
void ShowTypeCount(char *label, TypeTally **head, int sort_order)
{
   TypeTally *node, *prev, *bestnode, *bestprev;
   printf("%s Counts:\n", label);
   while (*head != NULL)
     {
	prev = NULL;
	bestnode = NULL;
	node = *head;
	while (node != NULL)
	  {
	     if (sort_order == 0)
	       {
		  if (bestnode != NULL && 
		      strcmp(bestnode->type, node->type) > 0)
		    {
		       bestnode = NULL;
		    }
	       }
	     else
	       {
		  if (bestnode != NULL &&
		      bestnode->count < node->count)
		    {
		       bestnode = NULL;
		    }
	       }
	     
	     if (bestnode == NULL)
	       {
		  bestnode = node;
		  bestprev = prev;
	       }
	     
	     prev = node;
	     node = node->next;
	  }
	
	printf("\t%s:  %d\n", bestnode->typeorig, bestnode->count);
	
	if (bestprev != NULL)
	  {
	     bestprev->next = bestnode->next;
	  }
	else
	  {
	     *head = bestnode->next;
	  }
	
	freeMemory((void **) &(bestnode->type));
	freeMemory((void **) &(bestnode->typeorig));
	freeMemory((void **) &(bestnode));
     }
}


int main(int argc, char **argv)
{
   FILE *fpin;
   AppData *ad;
   int i;
   
   if (argc != 2)
     {
	fprintf(stderr, "gpxinfo - Part of %s\n", PACKAGE_STRING);
	fprintf(stderr, "Error:  Incorrect arguments\n");
	fprintf(stderr, "    gpxinfo infile.gpx\n");
	fprintf(stderr, "Writes statistics to screen about the GPX file.\n");
	fprintf(stderr, "If infile.gpx is -, uses stdin.\n");
	exit(1);
     }

   ad = getMemory(sizeof(AppData));
   
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

   ParseXML(fpin, &WaypointHandler, NULL, (void *) ad);
   
   if (fpin != stdin)
     {
	fclose(fpin);
     }
   
   printf("Waypoints:  %d\n", ad->waypoints);
   printf("Available:  %d\n", ad->available);
   printf("Archived:  %d\n", ad->archived);
   printf("ID Range:  %d - %d\n", ad->id_min, ad->id_max);
   printf("Latitude Range:  %f - %f\n", ad->lat_min, ad->lat_max);
   printf("Longitude Range:  %f - %f\n", ad->lon_min, ad->lon_max);
   ShowTypeCount("Difficulty", &(ad->difficulty), 0);
   ShowTypeCount("Terrain", &(ad->terrain), 0);
   ShowTypeCount("Type", &(ad->type), 1);
   ShowTypeCount("Size", &(ad->size), 1);

   freeMemory((void **) &ad);

   return 0;
}
