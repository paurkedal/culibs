/* Part of the culibs project, <http://www.eideticdew.org/culibs/>.
 * Copyright (C) 2003--2007  Petter Urkedal <urkedal@nbi.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define CUCON_BITS_ALGO_COLOUR_C
#include <cucon/algo_colour.h>

static char *algo_colour_name_arr[] = {
    "white", "gray", "black", "red", "yellow", "green", "blue"
};

char const *
cucon_algo_colour_name(cucon_algo_colour_t colour)
{
    return algo_colour_name_arr[colour];
}

cucon_algo_colour_t
cucon_algo_colour_from_name(char const *s)
{
    switch (s[0]) {
    case 'b':
	switch (s[1]) {
	case 'l':
	    switch (s[2]) {
	    case 'a':
		switch (s[3]) {
		case 'c':
		    switch (s[4]) {
		    case 'k':
			switch (s[5]) {
			case 0:
			    return cucon_algo_colour_black;
			}
			break;
		    }
		    break;
		}
		break;
	    case 'u':
		switch (s[3]) {
		case 'e':
		    switch (s[4]) {
		    case 0:
			return cucon_algo_colour_blue;
		    }
		    break;
		}
		break;
	    }
	    break;
	}
	break;
    case 'g':
	switch (s[1]) {
	case 'r':
	    switch (s[2]) {
	    case 'a':
		switch (s[3]) {
		case 'y':
		    switch (s[4]) {
		    case 0:
			return cucon_algo_colour_gray;
		    }
		    break;
		}
		break;
	    case 'e':
		switch (s[3]) {
		case 'e':
		    switch (s[4]) {
		    case 'n':
			switch (s[5]) {
			case 0:
			    return cucon_algo_colour_green;
			}
			break;
		    }
		    break;
		}
		break;
	    }
	    break;
	}
	break;
    case 'r':
	switch (s[1]) {
	case 'e':
	    switch (s[2]) {
	    case 'd':
		switch (s[3]) {
		case 0:
		    return cucon_algo_colour_red;
		}
		break;
	    }
	    break;
	}
	break;
    case 'w':
	switch (s[1]) {
	case 'h':
	    switch (s[2]) {
	    case 'i':
		switch (s[3]) {
		case 't':
		    switch (s[4]) {
		    case 'e':
			switch (s[5]) {
			case 0:
			    return cucon_algo_colour_white;
			}
			break;
		    }
		    break;
		}
		break;
	    }
	    break;
	}
	break;
    case 'y':
	switch (s[1]) {
	case 'e':
	    switch (s[2]) {
	    case 'l':
		switch (s[3]) {
		case 'l':
		    switch (s[4]) {
		    case 'o':
			switch (s[5]) {
			case 'w':
			    switch (s[6]) {
			    case 0:
				return cucon_algo_colour_yellow;
			    }
			    break;
			}
			break;
		    }
		    break;
		}
		break;
	    }
	    break;
	}
	break;
    }
    cu_errf("%s is not a algorithm colour.", s);
    return cucon_algo_colour_white;
}
