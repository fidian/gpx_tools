/* This file is part of the package gpx_tools
 * - Read and reference INI settings
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini_settings.h"
#include "mem_str.h"

#define SETTING_LINE_LEN 1024


// Gets a setting from the SettingsStruct linked list
char *GetSetting(SettingsStruct *node, char *keyname)
{
   char *keyname_lower = NULL;
   
   AppendString(&keyname_lower, keyname);
   LowercaseString(keyname_lower);
   
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


void WriteSetting(SettingsStruct **head, const char *key, const char *value)
{
   SettingsStruct *ss;
   
   ss = getMemory(sizeof(SettingsStruct));
   AppendString(&(ss->key), key);
   LowercaseString(ss->key);
   AppendString(&(ss->value), value);
   ss->next = *head;
   *head = ss;
}


void ReadSettings(SettingsStruct **head, const char *filename)
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
		  WriteSetting(head, key_start, value_start);
	       }
	  }
     }

   freeMemory((void **) &buffer);
   fclose(fp);
}


void FreeSettings(SettingsStruct **head) 
{
   SettingsStruct *cur;
   while (*head != NULL)
     {
	cur = *head;
	*head = cur->next;
	freeMemory((void **) &(cur->key));
	freeMemory((void **) &(cur->value));
	freeMemory((void **) &cur);
     }
}
