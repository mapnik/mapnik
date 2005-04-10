/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef EXPRESSION_HH
#define EXPRESSION_HH

namespace mapnik
{
    //arithmetic expressions
    template <typename T>
    struct add
    {
	T operator()(T operand1,T operand2) const
	{
	    return operand1 + operand2;
	}
    };

    template <typename T>
    struct divide
    {
	T operator()(T operand1,T operand2) const
	{
	    assert(operand2);
	    return operand1 / operand2;
	}
    };
    
    template <typename T>
    struct mult
    {
	T operator() (T operand1,T operand2) const
	{
	    return operand1 * operand2;
	}
    };

    template <typename T>
    struct sub
    {
	T operator() (T operand1,T operand2) const
	{
	    return operand1 - operand2;
	}
    }; 
}

#endif //EXPRESSION_HH
