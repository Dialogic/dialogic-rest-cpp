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
 * \file    getoption.h
 * \brief   Get command line options, C++ wrapper for getopt_long().
 * \author  John Tarlton
 * \version 0.1
 */

#ifndef _GETOPTION_H
#define _GETOPTION_H

/*----------------------------- Dependencies -------------------------------*/

#include <getopt.h>

#include <vector>
#include <map>
#include <ostream>
#include <iomanip>

/*--------------------------------------------------------------------------*/


/*!
 * \class GetOptions
 * Get program options from the command line argv array.
 */
class GetOptions
{
private:

	/*!
	 * \struct Option
	 * Stores the definition of a single option flag.
	 */
	struct Option
	{
		Option( char sname,
		        const std::string& lname,
		        const std::string& description,
		        int hasarg)
			: short_name (sname),
			  long_name (lname),
			  desc (description),
			  has_arg (hasarg),
			  found (false)
		{}

		char         short_name; /*!< e.g. -v */
		std::string  long_name;  /*!< e.g. --foo */
		std::string  desc;       /*!< description, useful for help messages */
		int          has_arg;    /*!< no_argument | required_argument | optional_argument */
		bool         found;      /*!< true, if this option was present */
		std::string  value;      /*!< receives the argument's value */
	};

public:

	/*!
	 * ctor.
	 */
	GetOptions() {}

	/*!
	 * dtor, remove the Option objects.
	 */
	~GetOptions()
	{
		std::vector<Option*>::iterator i;
		for ( i = options_.begin();  i != options_.end(); ++i )
		{
			delete *i;
		}
	}

	/*!
	 * Add an option that does not have an explicit value. This is used for
	 * boolean options. E.g. --foo
	 *
	 * \param sname - single character name, use '\0' if not required.
	 * \param lname - long name
	 * \param description a short description of the option
	 */
	void addOptionNoArg( char sname,
	                     const std::string& lname,
	                     const std::string& description )
	{
		Option* op = new Option(sname, lname, description, no_argument);
		options_.push_back(op);
	}

	/*!
	 * Add an option that requires an explicit argument.
	 * e.g. --foo=bar or --foo bar
	 *
	 * \param sname - single character name, use '\0' if not required.
	 * \param lname - long name
	 * \param description - a short description of the option
	 */
	void addOptionRequiredArg( char sname,
	                           const std::string& lname,
	                           const std::string& description )
	{
		Option* op = new Option(sname, lname, description, required_argument);
		options_.push_back(op);
	}

	/*!
	 * Add an option that has an optional argument.
	 * e.g. --foo=bar or --foo
	 *
	 * \param sname - single character name, use '\0' if not required.
	 * \param lname - long name
	 * \param description - a short description of the option
	 */
	void addOptionOptionalArg( char sname,
	                           const std::string& lname,
	                           const std::string& description )
	{
		Option* op = new Option(sname, lname, description, optional_argument);
		options_.push_back(op);
	}

	/*!
	 * Lookup an option using is short name and return its arg value.
	 * \param sname - single character name,
	 * \return the value or an empty string if the option is not found.
	 */
	const std::string getValue( char sname ) const
	{
		Option*	op = find(sname);
		return op ? op->value : std::string();
	}

	/*!
	 * Lookup an option using its long name and return its arg value.
	 * \param lname - long name
	 * \return the value or an empty string if the option is not found.
	 */
	const std::string getValue( const std::string& lname ) const
	{
		Option*	op = find(lname);
		return op ? op->value : std::string();
	}

	/*!
	 * Lookup an option using its short name and test if it was found.
	 * \param sname - single character name.
	 * \return true if the option was found; otherwise, false.
	 */
	bool isFound( char sname ) const
	{
		Option* op = find(sname);
		return op ? op->found : false;
	}

	/*!
	 * Lookup an option using its long name and test if it was found.
	 * \param lname - long name.
	 * \return true if the option was found; otherwise, false.
	 */
	bool isFound( const std::string& lname ) const
	{
		Option* op = find(lname);
		return op ? op->found : false;
	}

	/*!
	 * Get all options that were found.
	 * \param opts - receives name/value pairs for each option
	 */
	void getAllValues( std::map<std::string, std::string>& opts ) const
	{
		opts.clear();
		std::vector<Option*>::const_iterator i;
		for ( i = options_.begin();  i != options_.end(); ++i )
		{
			if ( (*i)->found )
			{
				opts.insert(std::pair<std::string, std::string>((*i)->long_name, (*i)->value));
			}
		}
	}

	/*!
	 * Parse command-line options.
	 * \param argc - as passed into main()
	 * \param argv - as passed into main()
	 * \return the index of the first non-option in argv
	 */
	int parseOptions( int argc, char** argv )
	{
		/* build an array of option structS using data from Options and
		 * terminate it with a zeroed-out element.
		 */
		::option* opt_array = new ::option[options_.size() + 1];
		::option* opt = opt_array;

		std::string optstring;
		std::vector<Option*>::const_iterator i;
		for ( i = options_.begin(); i != options_.end(); ++i )
		{
			opt->name = (*i)->long_name.c_str();
			opt->has_arg = (*i)->has_arg;
			opt->flag = 0;
			opt->val = (*i)->short_name;

			if ( (*i)->short_name )
			{
				optstring += (*i)->short_name;
				if ( opt->has_arg == required_argument )
				{
					optstring += ':';
				}
				if ( opt->has_arg == optional_argument )
				{
					optstring += "::";
				}
			}
			++opt;
		}
		/* mark the end of the opt_array */
		opt->name = 0;
		opt->has_arg = 0;
		opt->flag = 0;
		opt->val = 0;

		/* stops getopt_long() from writing to stderr. */
		::opterr = 0;

		for (;;)
		{
			int opt_index = -1;

			int val = ::getopt_long(argc,
			                        argv,
			                        optstring.c_str(),
			                        opt_array,
			                        &opt_index);
			if ( val == -1 )
			{
				/* completed */
				break;
			}

			Option*	op = find(val);
			if ( !op && (opt_index != -1) )
			{
				op = find(opt_array[opt_index].name);
			}
			if ( op )
			{
				op->found = true;
				if ( op->has_arg != no_argument )
				{
					op->value = optarg ? optarg : "";
				}
			}
		}
		delete[] opt_array;
		return ::optind;
	}

    /*!
	 * Build a text string describing all options.
     */
	std::ostream& print( std::ostream& s ) const
	{
		std::vector<Option*>::const_iterator i;
		for ( i = options_.begin();  i != options_.end(); ++i )
		{
			if ( (*i)->short_name )
			{
				s << "-" << (*i)->short_name << ", ";
			}
			s << "--" << std::setw(8) << std::left << (*i)->long_name;
			s << "\t" << (*i)->desc;
			s << std::endl;
		}
		return s;
	}

   friend std::ostream& operator << (std::ostream& s, const GetOptions& go);

private:

	/*!
	 * Lookup an Option object by its short name.
	 * \param sname - single character name.
	 * \return a pointer to an Option object or 0 if not found.
	 */
	Option* find( char sname ) const
	{
		if ( sname != '\0' )
		{
			std::vector<Option*>::const_iterator i;
			for ( i = options_.begin(); i != options_.end(); ++i )
			{
				if ( (*i)->short_name == sname )
				{
					return *i;
				}
			}
		}
		return 0;
	}

	/*!
	 * Lookup an Option object by its long name.
	 * \param lname - long name.
	 * \return a pointer to an Option object or 0 if not found.
	 */
	Option* find( const std::string& lname ) const
	{
		std::vector<Option*>::const_iterator i;
		for ( i = options_.begin();  i != options_.end(); ++i )
		{
			if ( (*i)->long_name == lname )
			{
				return *i;
			}
		}
		return 0;
	}

private:

	std::vector<Option*> options_;
};

/*!
 * Overload << for printing options.
 */
inline std::ostream& operator << (std::ostream &s, const GetOptions& go)
{
	return go.print(s);
}


#endif // _GETOPTION_H

/*
 * vim:ts=4:set nu:
 */
