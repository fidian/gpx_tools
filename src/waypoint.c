/* This file is part of the package gpx_tools
 * - Common code to read the waypoints from the GPX file
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
#define NEED_STDLIB_H
#define NEED_STRING_H
#define NEED_EXPAT_H
#include "include.h"
#include "mem_str.h"
#include "waypoint.h"

#define XML_CHUNK_SIZE 4096

int XML_Newline_Type = 0;


void Waypoint_XML_Start(void *data, const char *el, const char **attr)
{
   Waypoint_Parser *wpp = (Waypoint_Parser *) data;
   Waypoint_Info *wpi;
   int i;
   char *output = NULL;

   // Reconstruct the tag
   DEBUG("Waypoint XML Start (start)");
   AppendString(&output, "<");
   AppendString(&output, (const char *) el);
   for (i = 0; attr[i]; i += 2)
     {
	AppendString(&output, " ");
	AppendString(&output, (const char *) attr[i]);
	AppendString(&output, "=\"");
	AppendString(&output, (const char *) attr[i + 1]);
	AppendString(&output, "\"");
     }
   AppendString(&output, ">");
   
   if (wpp->wpi == (Waypoint_Info *) NULL)
     {
	// We are not in a wpt tag
	if (strcmp(el, "wpt") == 0)
	  {
	     // Opening wpt tag
	     wpp->wpi = (Waypoint_Info *) getMemory(sizeof(Waypoint_Info));
	     AppendString(&(wpp->wpi->WaypointXML), output);
	     AppendString(&(wpp->wpi->TagStack), el);
	     for (i = 0; attr[i]; i += 2)
	       {
		  if (strcmp(attr[i], "lat") == 0) 
		    {
		       wpp->wpi->lat = ParseNumber((char *) attr[i + 1]);
		    }
		  else if (strcmp(attr[i], "lon") == 0)
		    {
		       wpp->wpi->lon = ParseNumber((char *) attr[i + 1]);
		    }
	       }
	     DEBUG("Waypoint XML Start (end)");
	     return;
	  }
	else
	  {
	     if (wpp->NonWaypoint != NULL) 
	       {
		  (*(wpp->NonWaypoint))(output, strlen(output), wpp->extra_data);
	       }
	     
	     freeMemory((void **) &output);
	     DEBUG("Waypoint XML Start (end)");
	     return;
	  }
     }
   
   wpi = wpp->wpi;
   
   AppendString(&(wpi->WaypointXML), output);
   freeMemory((void **) &output);
   
   AppendString(&(wpi->TagStack), " ");
   AppendString(&(wpi->TagStack), el);

   DEBUG(wpi->TagStack);
   if (strcmp(wpi->TagStack, "wpt name") == 0)
     {
	wpi->name_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->name_len);
     }
   else if (strcmp(wpi->TagStack, "wpt desc") == 0)
     {
	wpi->desc_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->desc_len);
     }
   else if (strcmp(wpi->TagStack, "wpt urlname") == 0)
     {
	wpi->urlname_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->urlname_len);
     }
   else if (strcmp(wpi->TagStack, "wpt sym") == 0)
     {
	wpi->sym_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->sym_len);
     }
   else if (strcmp(wpi->TagStack, "wpt type") == 0)
     {
	wpi->type_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->type_len);
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache") == 0)
     {
	for (i = 0; attr[i]; i += 2)
	  {
	     if (strcmp(attr[i], "available") == 0)
	       {
		  if (attr[i + 1][0] == 'T' || attr[i + 1][0] == 't')
		    {
		       wpi->available = 1;
		    }
	       }
	     else if (strcmp(attr[i], "archived") == 0)
	       {
		  if (attr[i + 1][0] == 'T' || attr[i + 1][0] == 't')
		    {
		       wpi->archived = 1;
		    }
	       }
	     else if (strcmp(attr[i], "id") == 0)
	       {
		  wpi->id = (int) ParseNumber((char *) attr[i + 1]);
	       }
	  }
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:name") == 0)
     {
	wpi->gcname_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->gcname_len);
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:placed_by") == 0)
     {
	wpi->placedBy_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->placedBy_len);
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:owner") == 0)
     {
	wpi->owner_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->owner_len);
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:type") == 0)
     {
	wpi->type2_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->type2_len);
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:container") == 0)
     {
	wpi->container_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->container_len);
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:difficulty") == 0)
     {
	wpi->difficulty_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->difficulty_len);
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:terrain") == 0)
     {
	wpi->terrain_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->terrain_len);
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:encoded_hints") == 0)
     {
	wpi->hints_off = strlen(wpi->WaypointXML);
	wpi->len_ptr = &(wpi->hints_len);
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:logs groundspeak:log groundspeak:type") == 0)
     {
	wpi->in_log_type_tag = 1;
     }
   else if (strcmp(wpi->TagStack, "wpt groundspeak:cache groundspeak:travelbugs groundspeak:travelbug") == 0)
     {
	wpi->bugs ++;
     }
   
   DEBUG("Waypoint XML Start (end)");
}


void Waypoint_XML_End(void *data, const char *el) 
{
   Waypoint_Parser *wpp = (Waypoint_Parser *) data;
   char *output = NULL;
   int i;

   DEBUG("Waypoint XML End (start)");
   AppendString(&output, "</");
   AppendString(&output, el);
   AppendString(&output, ">");
   
   if (wpp->wpi == NULL)
     {
	if (wpp->NonWaypoint != NULL)
	  {
	     (*(wpp->NonWaypoint))(output, strlen(output), wpp->extra_data);
	  }
	
	freeMemory((void **) &output);
	DEBUG("Waypoint XML End (end)");
	return;
     }
   
   AppendString(&(wpp->wpi->WaypointXML), output);
   freeMemory((void **) &output);
   wpp->wpi->len_ptr = NULL;
   
   i = strlen(wpp->wpi->TagStack) - 1;
   while (wpp->wpi->TagStack[i] != ' ' && i > 0)
     {
	i --;
     }
   wpp->wpi->TagStack[i] = '\0';
   if (i == 0)
     {
	// Finished with wpt tag
	if (wpp->Waypoint != NULL) 
	  {
	     (*(wpp->Waypoint))(wpp->wpi, wpp->extra_data);
	  }
	
	freeMemory((void **) &(wpp->wpi->WaypointXML));
	freeMemory((void **) &(wpp->wpi->logSummary));
	freeMemory((void **) &(wpp->wpi->TagStack));
	freeMemory((void **) &(wpp->wpi));
     }
   DEBUG("Waypoint XML End (end)");
}


void Waypoint_XML_Char(void *data, const char *txt, int txtlen)
{
   Waypoint_Parser *wpp = (Waypoint_Parser *) data;
   int i;
   char *output = NULL;
   
   if (! wpp->wpi) 
     {
	if (wpp->NonWaypoint != NULL) 
	  {
	     (*(wpp->NonWaypoint))(txt, txtlen, wpp->extra_data);
	  }
	
	return;
     }

   if (wpp->wpi->in_log_type_tag)
     {
	for (i = 0; i < txtlen && wpp->wpi->in_log_type_tag; i ++)
	  {
	     if (txt[i] != '\0' && txt[i] != ' ' && txt[i] != '\t' &&
		 txt[i] != '\r' && txt[i] != '\n')
	       {
		  AppendStringN(&(wpp->wpi->logSummary), &(txt[i]), 1);
		  wpp->wpi->in_log_type_tag = 0;
	       }
	  }
     }
   
   // Send to waypoint buffer
   AppendStringN(&output, txt, txtlen);
   HTMLEscapeString(&output);
   AppendString(&(wpp->wpi->WaypointXML), output);
   if (wpp->wpi->len_ptr) 
     {
	*(wpp->wpi->len_ptr) += strlen(output);
     }
   freeMemory((void **) &output);
}


void ParseXML(FILE *fp, WaypointFunc wp_func, NonWaypointFunc nonwp_func,
	      void *extra_data)
{
   Waypoint_Parser *wpp;
   char *buffer;
   int len;
   
   wpp = getMemory(sizeof(Waypoint_Parser));
   
   wpp->parser = XML_ParserCreate(NULL);
   if (! wpp->parser)
     {
	fprintf(stderr, "Could not allocate memory for parser\n");
	exit(2);
     }
   wpp->Waypoint = wp_func;
   wpp->NonWaypoint = nonwp_func;
   wpp->extra_data = extra_data;

   XML_SetUserData(wpp->parser, (void *) wpp);
   XML_SetElementHandler(wpp->parser, Waypoint_XML_Start, Waypoint_XML_End);
   XML_SetCharacterDataHandler(wpp->parser, Waypoint_XML_Char);
   
   while (! feof(fp))
     {
	buffer = XML_GetBuffer(wpp->parser, XML_CHUNK_SIZE);
	
	if (! buffer) 
	  {
	     fprintf(stderr, "Ran out of memory for parse buffer!\n");
	     exit(4);
	  }
	
	len = fread(buffer, 1, XML_CHUNK_SIZE, fp);
	if (ferror(fp))
	  {
	     fprintf(stderr, "Read error\n");
	     exit(5);
	  }
	
	if (XML_Newline_Type == 0) 
	  {
	     // Try to auto-detect the newline type
	     if (strstr(buffer, "\r\n")) 
	       {
		  XML_Newline_Type = 3;
	       }
	     else if (strstr(buffer, "\n"))
	       {
		  XML_Newline_Type = 2;
	       }
	     else if (strstr(buffer, "\r"))
	       {
		  XML_Newline_Type = 1;
	       }
	  }
	
	if (! XML_ParseBuffer(wpp->parser, len, feof(fp)))
	  {
	     fprintf(stderr, "Parse error at line %ld:\n%s\n",
		     (long) XML_GetCurrentLineNumber(wpp->parser),
		     XML_ErrorString(XML_GetErrorCode(wpp->parser)));
	     exit(6);
	  }
     }
   
   freeMemory((void **) &wpp);
}


// offset and len descripe what is going away
// Does nothing if str == NULL
void SwapWaypointString(Waypoint_Info *wpi, int offset, int len, char *str)
{
   char *newXML = NULL;
   int lendiff;
   
   if (str == NULL)
     {
	return;
     }
   
   lendiff = strlen(str) - len;
   
   AppendStringN(&newXML, wpi->WaypointXML, offset);
   AppendString(&newXML, str);
   AppendString(&newXML, &(wpi->WaypointXML[offset + len]));
   
   freeMemory((void **) &(wpi->WaypointXML));
   wpi->WaypointXML = newXML;
   newXML = NULL;
   
   if (wpi->name_off > offset)
     {
	wpi->name_off += lendiff;
     }
   if (wpi->desc_off > offset)
     {
	wpi->desc_off += lendiff;
     }
   if (wpi->gcname_off > offset)
     {
	wpi->gcname_off += lendiff;
     }
   if (wpi->urlname_off > offset)
     {
	wpi->urlname_off += lendiff;
     }
   if (wpi->sym_off > offset)
     {
	wpi->sym_off += lendiff;
     }
   if (wpi->type_off > offset)
     {
	wpi->type_off += lendiff;
     }
   if (wpi->placedBy_off > offset)
     {
	wpi->placedBy_off += lendiff;
     }
   if (wpi->owner_off > offset)
     {
	wpi->owner_off += lendiff;
     }
   if (wpi->type2_off > offset)
     {
	wpi->type2_off += lendiff;
     }
   if (wpi->container_off > offset)
     {
	wpi->container_off += lendiff;
     }
   if (wpi->difficulty_off > offset)
     {
	wpi->difficulty_off += lendiff;
     }
   if (wpi->terrain_off > offset)
     {
	wpi->terrain_off += lendiff;
     }
   if (wpi->hints_off > offset)
     {
	wpi->hints_off += lendiff;
     }
}


// dest = dynamic string
int ChangeToSingleNumber(char *source, int slen)
{
   int num = 0;
   
   if (slen >= 1 && source[0] >= '1' && source[0] <= '5')
     {
	num = *source - '0';
	num *= 2;
	num --;
	
	if (slen == 3 && source[5] != '5' && source[1] == '.' && 
	    source[2] == '5') 
	  {
	     num ++;
	  }
     }
   
   return num;
}


// Returns the newline character as detected from the first line in the
// XML file
char *GetXMLNewline(void)
{
   if (XML_Newline_Type == 2)
     {
	return "\n";
     }
   
   if (XML_Newline_Type == 1)
     {
	return "\r";
     }
   
   return "\r\n";
}
