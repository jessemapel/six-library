/* =========================================================================
 * This file is part of six-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2020, MDA Information Systems LLC
 *
 * six.sicd-c++ is free software; you can redistribute it and/or modify
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
#ifndef __SIX_SICD_VERSION_UPDATER_H__
#define __SIX_SICD_VERSION_UPDATER_H__

#include <string>
#include <logging/Logger.h>
#include <six/VersionUpdater.h>
#include <six/sicd/ComplexData.h>

namespace six
{
namespace sicd
{
/*!
 * \class VersionUpdater
 * Responsible for updating SICD metadata to a valid state
 * for a new version.
 * This is intended to be used quick prototyping work for standards
 * updates. Some inserted fields will be valid XML but
 * mathematically garbage.
 */
class SicdVersionUpdater : public VersionUpdater
{
public:
    /*!
     * Constructor
     * \param[in,out] complexData SICD metadata to update
     * \param targetVersion Version to update `data` to.
     *                      Must be greater than current version of `data`.
     * \param[out] log Warnings will be emitting for fields that were populated
     *                 with either bad data or guesses
     * \throws If `targetVersion` is invalid
     */
    SicdVersionUpdater(six::sicd::ComplexData& complexData,
                       const std::string& targetVersion,
                       logging::Logger& log);

private:
    void updateSingleIncrement() override;

    six::sicd::ComplexData& mData;
};

}
}

#endif
