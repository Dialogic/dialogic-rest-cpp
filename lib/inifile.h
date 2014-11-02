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
 * \file    inifile.h
 * \brief   Simple loader for 'ini' style formatted files.
 * \author  John Tarlton  <john.tarlton@dialogic.com>
 * \version 0.1
 */

#ifndef _INIFILE_H
#define _INIFILE_H

/*------------------------------ Dependencies --------------------------------*/

#include <string>
#include <map>
#include <vector>

/*----------------------------------------------------------------------------*/

/*!
 * \class IniFile
 */
class IniFile
{
public:

	/*!
	 * Container for a section's parameters. A vector is used to preserve the
	 * order of the params and to allow multiple definitions of the same key.
	 */
	typedef std::vector<std::pair<std::string, std::string> > section_data_t;

	/*!
	 * ctor
	 */
	IniFile() {}

	/*!
	 * dtor
	 */
	~IniFile() {}

	/*!
	 * Load and parse an ini file.
	 * \param filename - file to load.
	 */
	bool load( const std::string& filename );

	/*!
	 * Load multiple ini files.
	 * \param dir_name - directory containing the files.
	 * \param file_ext - filename extension for the files to load.
	 */
	bool loadMultiple( const std::string& dir_name,
	                   const std::string& file_ext );

	/*!
	 * Read a string value of a key. return a default value if not found.
	 * \param section - name of the section
	 * \param key - name of the key.
	 * \param default_value - value returned if key in not found.
	 */
	const std::string& get( const std::string& section,
	                        const std::string& key,
	                        const std::string& default_value ) const;

	/*!
	 * Read an integer value of a key. return a default value if not found.
	 * \param section - name of the section
	 * \param key - name of the key.
	 * \param default_value - value returned if key in not found.
	 */
	int get( const std::string& section,
	         const std::string& key,
	         int default_value ) const;

	/*!
	 * Read all key / values from a section.
	 * \param section - name of the section
	 * \param data - container to receive the information.
	 */
	bool getSection( const std::string& section,
	                 section_data_t& data ) const;

	/*!
	 * Get all section names.
	 * \param names - container to receive the information.
	 */
	void getSectionNames( std::vector<std::string>& names ) const;

	/*!
	 * debug aid
	 */
	void print( std::ostream &os ) const;

private:

	void trim( std::string& s );

private:

	/* section_name => data
	 */
	std::map<std::string, section_data_t > data_;
};

/*!
 * Overload << for printing IniFile objects.
 */
inline std::ostream& operator << ( std::ostream &os, const IniFile& ini )
{
	ini.print(os);
	return os;
}

#endif // _INIFILE_H


/* vim:ts=4:set nu:
 * EOF
 */

