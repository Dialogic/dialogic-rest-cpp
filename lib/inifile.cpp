/*
 * Copyright (C) 2009 Dialogic Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 * Alternatively see <http://www.gnu.org/licenses/>.
 * Or see the LICENSE file included within the source tree.
 *
 */

/*!
 * \file    inifile.cpp
 * \brief   Simple loader for 'ini' style formatted files.
 * \author  John Tarlton  <john.tarlton@dialogic.com>
 * \version 1.0
 */

/*------------------------------ Dependencies --------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include "inifile.h"

/*----------------------------------------------------------------------------*/

/*
 * Load and parse a file.
 * Malformed lines are ignored.
 * Leading/trailing whitespace is removed from all section names, key name
 * and values.
 */
bool IniFile::load( const std::string& filename )
{
	std::ifstream fin(filename.c_str());
	if ( !fin.is_open() )
	{
		return false;
	}

	std::string section;

	std::string line;
	while ( std::getline(fin, line) )
	{
		std::string::size_type pos = line.find_first_not_of("\t ");
		if ( pos == std::string::npos ) /* Blank line. */
		{
			continue;
		}
		if ( (line[pos] == ';') || (line[pos] == '#') ) /* Comment line. */
		{
			continue;
		}
		if ( line[pos] == '[' ) /* [section] */
		{
			std::string::size_type endpos = line.find(']', pos + 1);
			if ( endpos != std::string::npos )
			{
				section = line.substr(pos + 1, endpos - 1);
				trim(section);
				section_data_t empty_data;
				data_[section] = empty_data;
			}
		}
		else /* key = value */
		{
			std::string::size_type seperator = line.find('=', pos);
			if ( seperator != std::string::npos )
			{
				std::string key = line.substr(pos, seperator);
				std::string value = line.substr(seperator + 1);
				trim(key);
				trim(value);
				data_[section].push_back(std::pair<std::string, std::string>(key, value));
			}
		}
	}

	fin.close();
	return true;
}


/*
 * Search the named directory for regular files that have the specified file
 * name extension. For each matching file invoke load() to load an parse its
 * contents. Abort on error.
 */
bool IniFile::loadMultiple( const std::string& dir_name,
                            const std::string& file_ext )
{
	DIR* dirp = opendir(dir_name.c_str());
	if ( !dirp )
	{
		return true;   /* no data != bad data */
	}

	bool res = true;

	struct dirent* direntp = readdir(dirp);
	while ( direntp )
	{
		/* Ignore self & parent directory entries. */
		if ( (strcmp(".", direntp->d_name)) &&
		     (strcmp("..", direntp->d_name)) )
		{
			std::string filename(dir_name + "/" + direntp->d_name);
			struct stat st;
			if ( (lstat(filename.c_str(), &st) == 0) &&
			     (S_ISREG(st.st_mode)) )
			{
				std::string::size_type pos = filename.rfind('.');
				if ( pos != std::string::npos )
				{
					if ( filename.substr(pos+1) == file_ext )
					{
						res = load(filename);
						if ( !res )
						{
							break;
						}
					}
				}
			}
		}
		direntp = readdir(dirp);
	}
	closedir(dirp);

	return res;
}


/*
 * Lookup the section + key and return the value, if not found return
 * default_value.
 */
const std::string& IniFile::get( const std::string& section,
                                 const std::string& key,
                                 const std::string& default_value ) const
{
	std::map<std::string, section_data_t >::const_iterator i;
	i = data_.find(section);
	if ( i != data_.end() )
	{
		section_data_t::const_iterator j;
		for ( j = (*i).second.begin(); j != (*i).second.end(); ++j )
		{
			if ( (*j).first == key )
			{
				return (*j).second;
			}
		}
	}
	return default_value;
}

/*
 * Lookup the section + key and return the value, if not found return
 * default_value.
 */
int IniFile::get( const std::string& section,
                  const std::string& key,
                  int default_value ) const
{
	std::map<std::string, section_data_t >::const_iterator i;
	i = data_.find(section);
	if ( i != data_.end() )
	{
		section_data_t::const_iterator j;
		for ( j = (*i).second.begin(); j != (*i).second.end(); ++j )
		{
			if ( (*j).first == key )
			{
				return atoi((*j).second.c_str());
			}
		}
	}
	return default_value;
}


/*
 * Return all section names.
 */
void IniFile::getSectionNames( std::vector<std::string>& names ) const
{
	std::map<std::string, section_data_t >::const_iterator i;
	for ( i = data_.begin(); i != data_.end(); ++i )
	{
		names.push_back((*i).first);
	}
}


/*
 * Lookup the section and return all its key / value pairs.
 */
bool IniFile::getSection( const std::string& section,
                          section_data_t& data ) const
{
	std::map<std::string, section_data_t >::const_iterator i;
	i = data_.find(section);
	if ( i != data_.end() )
	{
		data = (*i).second;
		return true;
	}
	return false;
}



/*
 * Remove leading and trailing spaces from a string.
 */
void IniFile::trim( std::string& s )
{
	std::string::size_type pos = s.find_last_not_of("\t ");
	if ( pos != std::string::npos)
	{
		s.erase(pos + 1);
		pos = s.find_first_not_of("\t ");
		if ( pos != std::string::npos )
		{
			s.erase(0, pos);
		}
	}
	else
	{
		s.clear();
	}
}


/*
 * Print each section name and each of its key / value pairs.
 */
void IniFile::print( std::ostream &os ) const
{
	std::map<std::string, section_data_t >::const_iterator i;
	for ( i = data_.begin(); i != data_.end(); ++i )
	{
		os << "[" << (*i).first << "]" << std::endl;
		section_data_t::const_iterator j;
		for ( j = (*i).second.begin(); j != (*i).second.end(); ++j )
		{
			os << (*j).first << "=" << (*j).second << std::endl;
		}
		os << std::endl;
	}
}


/* vim:ts=4:set nu:
 * EOF
 */

