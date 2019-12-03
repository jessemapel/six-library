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

#include <complex>

#include <cphd/PVP.h>
#include <cphd/PVPBlock.h>
#include <cphd/TestDataGenerator.h>

#include "TestCase.h"

static const size_t NUM_CHANNELS = 3;
static const size_t NUM_VECTORS = 2;

namespace
{
void setVectorParameters(size_t channel,
                         size_t vector,
                         cphd::PVPBlock& pvpBlock,
                         cphd::PVPBlock& pvpBlock2)
{
    const double txTime = cphd::getRandom();
    pvpBlock.setTxTime(txTime, channel, vector);
    pvpBlock2.setTxTime(txTime, channel, vector);

    const cphd::Vector3 txPos = cphd::getRandomVector3();
    pvpBlock.setTxPos(txPos, channel, vector);
    pvpBlock2.setTxPos(txPos, channel, vector);

    const cphd::Vector3 txVel = cphd::getRandomVector3();
    pvpBlock.setTxVel(txVel, channel, vector);
    pvpBlock2.setTxVel(txVel, channel, vector);

    const double rcvTime = cphd::getRandom();
    pvpBlock.setRcvTime(rcvTime, channel, vector);
    pvpBlock2.setRcvTime(rcvTime, channel, vector);

    const cphd::Vector3 rcvPos = cphd::getRandomVector3();
    pvpBlock.setRcvPos(rcvPos, channel, vector);
    pvpBlock2.setRcvPos(rcvPos, channel, vector);

    const cphd::Vector3 rcvVel = cphd::getRandomVector3();
    pvpBlock.setRcvVel(rcvVel, channel, vector);
    pvpBlock2.setRcvVel(rcvVel, channel, vector);

    const cphd::Vector3 srpPos = cphd::getRandomVector3();
    pvpBlock.setSRPPos(srpPos, channel, vector);
    pvpBlock2.setSRPPos(srpPos, channel, vector);

    const double aFDOP = cphd::getRandom();
    pvpBlock.setaFDOP(aFDOP, channel, vector);
    pvpBlock2.setaFDOP(aFDOP, channel, vector);

    const double aFRR1 = cphd::getRandom();
    pvpBlock.setaFRR1(aFRR1, channel, vector);
    pvpBlock2.setaFRR1(aFRR1, channel, vector);

    const double aFRR2 = cphd::getRandom();
    pvpBlock.setaFRR2(aFRR2, channel, vector);
    pvpBlock2.setaFRR2(aFRR2, channel, vector);

    const double fx1 = cphd::getRandom();
    pvpBlock.setFx1(fx1, channel, vector);
    pvpBlock2.setFx1(fx1, channel, vector);

    const double fx2 = cphd::getRandom();
    pvpBlock.setFx2(fx2, channel, vector);
    pvpBlock2.setFx2(fx2, channel, vector);

    const double toa1 = cphd::getRandom();
    pvpBlock.setTOA1(toa1, channel, vector);
    pvpBlock2.setTOA1(toa1, channel, vector);

    const double toa2 = cphd::getRandom();
    pvpBlock.setTOA2(toa2, channel, vector);
    pvpBlock2.setTOA2(toa2, channel, vector);

    const double tdTropoSRP = cphd::getRandom();
    pvpBlock.setTdTropoSRP(tdTropoSRP, channel, vector);
    pvpBlock2.setTdTropoSRP(tdTropoSRP, channel, vector);

    const double sc0 = cphd::getRandom();
    pvpBlock.setSC0(sc0, channel, vector);
    pvpBlock2.setSC0(sc0, channel, vector);

    const double scss = cphd::getRandom();
    pvpBlock.setSCSS(scss, channel, vector);
    pvpBlock2.setSCSS(scss, channel, vector);
}

TEST_CASE(testPvpRequired)
{
    cphd::Pvp pvp;
    cphd::setPVPXML(pvp);
    cphd::PVPBlock pvpBlock(NUM_CHANNELS,
                            std::vector<size_t>(NUM_CHANNELS, NUM_VECTORS),
                            pvp);

    for (size_t channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        for (size_t vector = 0; vector < NUM_VECTORS; ++vector)
        {
            cphd::setVectorParameters(channel, vector, pvpBlock);
        }
    }
}

TEST_CASE(testPvpOptional)
{
    cphd::Pvp pvp;
    cphd::setPVPXML(pvp);
    pvp.ampSF.reset(new cphd::PVPType());
    pvp.setData(1, 28, "F8", *pvp.ampSF);
    pvp.fxN1.reset(new cphd::PVPType());
    pvp.setData(1, 27, "F8", *pvp.fxN1);
    pvp.fxN2.reset(new cphd::PVPType());
    pvp.setData(1, 29, "F8", *pvp.fxN2);
    pvp.setData(1, 30, "F8", "Param1");
    pvp.setData(1, 31, "S10", "Param2");
    pvp.setData(1, 32, "CI16", "Param3");
    cphd::PVPBlock pvpBlock(NUM_CHANNELS,
                            std::vector<size_t>(NUM_CHANNELS, NUM_VECTORS),
                            pvp);

    for (size_t channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        for (size_t vector = 0; vector < NUM_VECTORS; ++vector)
        {
            cphd::setVectorParameters(channel, vector, pvpBlock);

            const double ampSF = cphd::getRandom();
            pvpBlock.setAmpSF(ampSF, channel, vector);
            const double fxN1 = cphd::getRandom();
            pvpBlock.setFxN1(fxN1, channel, vector);
            const double fxN2 = cphd::getRandom();
            pvpBlock.setFxN2(fxN2, channel, vector);
            const double addedParam1 = cphd::getRandom();
            pvpBlock.setAddedPVP(addedParam1, channel, vector, "Param1");
            const std::string addedParam2 = "Parameter2";
            pvpBlock.setAddedPVP(addedParam2, channel, vector, "Param2");
            const std::complex<int> addedParam3(3,4);
            pvpBlock.setAddedPVP(addedParam3, channel, vector, "Param3");

            TEST_ASSERT_EQ(ampSF, pvpBlock.getAmpSF(channel, vector));
            TEST_ASSERT_EQ(fxN1, pvpBlock.getFxN1(channel, vector));
            TEST_ASSERT_EQ(fxN2, pvpBlock.getFxN2(channel, vector));
            TEST_ASSERT_EQ(addedParam1, pvpBlock.getAddedPVP<double>(channel, vector, "Param1"));
            TEST_ASSERT_EQ(addedParam2, pvpBlock.getAddedPVP<std::string>(channel, vector, "Param2"));
            TEST_ASSERT_EQ(addedParam3, pvpBlock.getAddedPVP<std::complex<int> >(channel, vector, "Param3"));
        }
    }
}

TEST_CASE(testPvpThrow)
{
    cphd::Pvp pvp;
    cphd::setPVPXML(pvp);
    pvp.ampSF.reset(new cphd::PVPType());
    pvp.setData(1, 29, "F8", *pvp.ampSF);
    pvp.fxN1.reset(new cphd::PVPType());
    pvp.setData(1, 28, "F8", *pvp.fxN1);
    pvp.toaE1.reset(new cphd::PVPType());
    TEST_EXCEPTION(pvp.setData(1, 15, "F8", *pvp.toaE1)); // Overwrite block
    TEST_EXCEPTION(pvp.setData(3, 27, "F8", *pvp.toaE1)); // Overwrite block
    pvp.setData(1, 27, "F8", *pvp.toaE1);

    pvp.setData(1, 30, "F8", "Param1");
    pvp.setData(1, 31, "F8", "Param2");
    TEST_EXCEPTION(pvp.setData(1, 30, "F8", "Param1")); //Rewriting to existing memory block
    TEST_EXCEPTION(pvp.setData(1, 30, "X=F8;YF8;", "Param1")); //
    TEST_EXCEPTION(pvp.setData(1, 30, "X=F8;Y=F8;Z=", "Param1"));

    cphd::PVPBlock pvpBlock(NUM_CHANNELS,
                            std::vector<size_t>(NUM_CHANNELS, NUM_VECTORS),
                            pvp);

    for (size_t channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        for (size_t vector = 0; vector < NUM_VECTORS; ++vector)
        {
            cphd::setVectorParameters(channel, vector, pvpBlock);

            const double ampSF = cphd::getRandom();
            pvpBlock.setAmpSF(ampSF, channel, vector);

            const double fxN1 = cphd::getRandom();
            pvpBlock.setFxN1(fxN1, channel, vector);

            const double toaE1 = cphd::getRandom();
            pvpBlock.setTOAE1(toaE1, channel, vector);

            TEST_ASSERT_EQ(toaE1, pvpBlock.getTOAE1(channel, vector));

            const double addedParam1 = cphd::getRandom();
            pvpBlock.setAddedPVP(addedParam1, channel, vector, "Param1");

            const double addedParam2 = cphd::getRandom();
            TEST_EXCEPTION(pvpBlock.setAddedPVP(addedParam2, channel, vector, "Param3"));
            TEST_EXCEPTION(pvpBlock.getAddedPVP<double>(channel, vector, "Param3"));

            const double fxN2 = cphd::getRandom();
            TEST_EXCEPTION(pvpBlock.setFxN2(fxN2, channel, vector));
            TEST_EXCEPTION(pvpBlock.getFxN2(channel, vector));
        }
    }

    cphd::Pvp pvp2;
    cphd::setPVPXML(pvp2);
    pvp2.setData(1, 27, "F8", "Param1");
    cphd::PVPBlock pvpBlock2(NUM_CHANNELS,
                             std::vector<size_t>(NUM_CHANNELS,NUM_VECTORS),
                             pvp2);

        for (size_t channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        for (size_t vector = 0; vector < NUM_VECTORS; ++vector)
        {
            cphd::setVectorParameters(channel, vector, pvpBlock2);

            const double addedParam1 = cphd::getRandom();
            pvpBlock2.setAddedPVP(addedParam1, channel, vector, "Param1");
            pvpBlock2.getAddedPVP<double>(channel, vector, "Param1");
        }
    }

    cphd::Pvp pvp3;
    cphd::setPVPXML(pvp3);
    pvp3.setData(1, 27, "F8", "Param1");
    cphd::PVPBlock pvpBlock3(NUM_CHANNELS,
                             std::vector<size_t>(NUM_CHANNELS,NUM_VECTORS),
                             pvp3);

        for (size_t channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        for (size_t vector = 0; vector < NUM_VECTORS; ++vector)
        {
            cphd::setVectorParameters(channel, vector, pvpBlock3);

            const double addedParam1 = cphd::getRandom();
            pvpBlock3.setAddedPVP(addedParam1, channel, vector, "Param1");
            pvpBlock3.getAddedPVP<double>(channel, vector, "Param1");
        }
    }
}

TEST_CASE(testPvpEquality)
{
    cphd::Pvp pvp1;
    cphd::setPVPXML(pvp1);
    pvp1.ampSF.reset(new cphd::PVPType());
    pvp1.setData(1, 28, "F8", *pvp1.ampSF);
    pvp1.fxN1.reset(new cphd::PVPType());
    pvp1.setData(1, 27, "F8", *pvp1.fxN1);
    pvp1.fxN2.reset(new cphd::PVPType());
    pvp1.setData(1, 29, "F8", *pvp1.fxN2);
    pvp1.setData(1, 30, "F8", "Param1");
    pvp1.setData(1, 31, "CI8", "Param2");
    cphd::PVPBlock pvpBlock1(NUM_CHANNELS,
                             std::vector<size_t>(NUM_CHANNELS, NUM_VECTORS),
                             pvp1);

    cphd::Pvp pvp2;
    cphd::setPVPXML(pvp2);
    pvp2.ampSF.reset(new cphd::PVPType());
    pvp2.setData(1, 28, "F8", *pvp2.ampSF);
    pvp2.fxN1.reset(new cphd::PVPType());
    pvp2.setData(1, 27, "F8", *pvp2.fxN1);
    pvp2.fxN2.reset(new cphd::PVPType());
    pvp2.setData(1, 29, "F8", *pvp2.fxN2);
    pvp2.setData(1, 30, "F8", "Param1");
    pvp2.setData(1, 31, "CI8", "Param2");
    cphd::PVPBlock pvpBlock2(NUM_CHANNELS,
                             std::vector<size_t>(NUM_CHANNELS, NUM_VECTORS),
                             pvp2);

    TEST_ASSERT_EQ(pvp1, pvp2);
    TEST_ASSERT_TRUE(pvpBlock1 == pvpBlock2);

    for (size_t channel = 0; channel < NUM_CHANNELS; ++channel)
    {
        for (size_t vector = 0; vector < NUM_VECTORS; ++vector)
        {
            setVectorParameters(channel, vector, pvpBlock1, pvpBlock2);
            const double ampSF = cphd::getRandom();
            pvpBlock1.setAmpSF(ampSF, channel, vector);
            pvpBlock2.setAmpSF(ampSF, channel, vector);

            const double fxN1 = cphd::getRandom();
            pvpBlock1.setFxN1(fxN1, channel, vector);
            pvpBlock2.setFxN1(fxN1, channel, vector);

            const double fxN2 = cphd::getRandom();
            pvpBlock1.setFxN2(fxN2, channel, vector);
            pvpBlock2.setFxN2(fxN2, channel, vector);

            const double addedParam1 = cphd::getRandom();
            pvpBlock1.setAddedPVP(addedParam1, channel, vector, "Param1");
            pvpBlock2.setAddedPVP(addedParam1, channel, vector, "Param1");

            const std::complex<int> addedParam2(3,4);
            pvpBlock1.setAddedPVP(addedParam2, channel, vector, "Param2");
            pvpBlock2.setAddedPVP(addedParam2, channel, vector, "Param2");
        }
    }
    TEST_ASSERT_EQ(pvpBlock1, pvpBlock2);
}
}

int main(int , char** )
{
    ::srand(174);
    TEST_CHECK(testPvpRequired);
    TEST_CHECK(testPvpOptional);
    TEST_CHECK(testPvpThrow);
    TEST_CHECK(testPvpEquality);
    return 0;
}

