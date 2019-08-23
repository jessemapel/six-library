/* =========================================================================
 * This file is part of cphd-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2019, MDA Information Systems LLC
 *
 * cphd-c++ is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#include <cphd/Dwell.h>
#include <six/Init.h>

namespace cphd
{

DwellTime::DwellTime() :
    identifier(six::Init::undefined<std::string>()),
    dwellTimePoly(six::Init::undefined<Poly2D>())
{}

COD::COD() :
    identifier(six::Init::undefined<std::string>()),
    codTimePoly(six::Init::undefined<Poly2D>())
{}

Dwell::Dwell() :
    numCODTimes(six::Init::undefined<size_t>()),
    numDwellTimes(six::Init::undefined<size_t>())
{}


std::ostream& operator<< (std::ostream& os, const DwellTime& d)
{
    os << "  DwellTime:: \n"
        << "    Identifier     : " << d.identifier << "\n"
        << "    DwellTimePoly  : " << d.dwellTimePoly << "\n";
    return os;
}

std::ostream& operator<< (std::ostream& os, const COD& c)
{
    os << "  COD:: \n"
        << "    Identifier     : " << c.identifier << "\n"
        << "    CODTimePoly    : " << c.codTimePoly << "\n";
    return os;
}


std::ostream& operator<< (std::ostream& os, const Dwell& d)
{
    os << "Dwell:: \n"
        << "  NumCODTimes      : " << d.numCODTimes << "\n"
        << "  NumDwellTimes    : " << d.numDwellTimes << "\n";
    for (size_t i = 0; i < d.cod.size(); ++i)
    {
        os << d.cod[i] << "\n";
    }
    for (size_t i = 0; i < d.dtime.size(); ++i)
    {
        os << d.dtime[i] << "\n";
    }
    return os;
}

}