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

#ifdef EXPAT_XMLPARSE
#include <xmlparse.h>
#else
#include <expat.h>
#endif

typedef struct waypointinfo
{
   char *WaypointXML;  // This is the one that actually gets data
   char *logSummary;  // Dynamically allocated
   char *TagStack;  // Dynamically allocated
   
   int available, archived, bugs, in_log_type_tag;
   int *len_ptr;
   
   float lat, lon;
   int id;
   
   int name_off, name_len;
   int desc_off, desc_len;
   int gcname_off, gcname_len;
   int urlname_off, urlname_len;
   int sym_off, sym_len;
   int type_off, type_len;
   int placedBy_off, placedBy_len;
   int owner_off, owner_len;
   int type2_off, type2_len;
   int container_off, container_len;
   int difficulty_off, difficulty_len;
   int terrain_off, terrain_len;
   int hints_off, hints_len;
} Waypoint_Info;

typedef void (*WaypointFunc)(Waypoint_Info *, void *);
typedef void (*NonWaypointFunc)(const char *, int, void *);

typedef struct wp_parser
{
   NonWaypointFunc NonWaypoint;
   WaypointFunc Waypoint;
   XML_Parser parser;
   void *extra_data;
   Waypoint_Info *wpi;
} Waypoint_Parser;

void ParseXML(FILE *fp, WaypointFunc wp_func, NonWaypointFunc nonwp_func,
	      void *extra_data);

void SwapWaypointString(Waypoint_Info *wpi, int offset, int len, char *str);

int ChangeToSingleNumber(char *source, int slen);

char *GetXMLNewline(void);
