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

typedef struct settings
{
	char *key, *key_orig, *value;
	struct settings *next;
} SettingsStruct;


// Gets a setting from the SettingsStruct linked list
char *GetSetting(SettingsStruct *node, char *keyname);

// Saves a setting to the linked list
void WriteSetting(SettingsStruct **head, const char *key, const char *value);

// Reads settings from an INI file
void ReadSettings(SettingsStruct **head, const char *filename);

// Frees memory
void FreeSettings(SettingsStruct **head);

// Reads a line, newline independent
void ReadLine(char *buffer, unsigned int len, FILE *fp);
