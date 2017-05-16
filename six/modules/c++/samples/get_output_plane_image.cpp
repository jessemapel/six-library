/* =========================================================================
 * This file is part of six-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2017, MDA Information Systems LLC
 *
 * six-c++ is free software; you can redistribute it and/or modify
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

#include <cli/ArgumentParser.h>
#include <cli/Results.h>
#include <math/Round.h>
#include <sio/lite/FileWriter.h>
#include <scene/ProjectionPolynomialFitter.h>
#include <six/XMLControl.h>
#include <six/XMLControlFactory.h>
#include <six/sicd/AreaPlaneUtility.h>
#include <six/sicd/ComplexXMLControl.h>
#include <six/sicd/Utilities.h>

namespace
{
void findOutputToSlantPolynomials(const six::sicd::ComplexData& complexData,
        size_t polyOrderX, size_t polyOrderY,
        six::Poly2D& toSlantRow, six::Poly2D& toSlantCol)
{
    std::auto_ptr<scene::ProjectionPolynomialFitter> polynomialFitter(
            six::sicd::Utilities::getPolynomialFitter(complexData));
    const six::RowColDouble sampleSpacing(
            complexData.grid->row->sampleSpacing,
            complexData.grid->col->sampleSpacing);
    const six::RowColDouble offset(
            complexData.imageData->firstRow,
            complexData.imageData->firstCol);
    polynomialFitter->fitOutputToSlantPolynomials(
            offset,
            complexData.imageData->scpPixel,
            complexData.imageData->scpPixel,
            sampleSpacing,
            polyOrderX,
            polyOrderY,
            toSlantRow,
            toSlantCol);
}

}

int main(int argc, char** argv)
{
    /*
     * This program gets a rough output plane from the SICD, and writes it as
     * an SIO. No filtering is done. As a result, the output plane contains
     * some massive values that skew the image. Therefore, when viewing the
     * image, you will have to mitigate this. When viewing in MATLAB,
     * for example, this may be done by
     * imagesc(read_sio('outputPlane.sio', [0, 50]));
     */
    try
    {
        cli::ArgumentParser parser;
        parser.setDescription("Read a SICD, project the image data to the "
                "output plane, and write it as an SIO");
        parser.addArgument("-x --polyOrderX", "Order for x-direction polynomials",
                           cli::STORE, "polyOrderX", "POLY_ORDER_X", 1, 1)->
                           setDefault(3);
        parser.addArgument("-y --polyOrderY", "Order for y-direction polynomials",
                           cli::STORE, "polyOrderY", "POLY_ORDER_Y", 1, 1)->
                           setDefault(3);
        parser.addArgument("input", "Input SICD", cli::STORE, "input", "INPUT",
                            1, 1);
        parser.addArgument("output", "Output SIO Pathname", cli::STORE,
                           "output", "OUTPUT", 1, 1);

        const std::auto_ptr<cli::Results> options(parser.parse(argc, argv));

        const std::string sicdPathname(options->get<std::string>("input"));
        const std::string outputPathname(options->get<std::string>("output"));
        const size_t polyOrderX(options->get<size_t>("polyOrderX"));
        const size_t polyOrderY(options->get<size_t>("polyOrderY"));
        const std::vector<std::string> schemaPaths;

        six::XMLControlRegistry registry;
        registry.addCreator(six::DataType::COMPLEX,
                new six::XMLControlCreatorT<six::sicd::ComplexXMLControl>());

        std::auto_ptr<six::sicd::ComplexData> complexData;
        std::vector<std::complex<float> > buffer;
        six::sicd::Utilities::readSicd(sicdPathname, schemaPaths, complexData,
                buffer);

        // Derive AreaPlane if not defined
        if (!six::sicd::AreaPlaneUtility::hasAreaPlane(*complexData))
        {
            six::sicd::AreaPlaneUtility::setAreaPlane(*complexData);
        }
        const six::sicd::AreaPlane& plane =
                *complexData->radarCollection->area->plane;

        // Grab outputToSlant plane polynomials
        six::Poly2D toSlantRow;
        six::Poly2D toSlantCol;
        findOutputToSlantPolynomials(*complexData, polyOrderX, polyOrderY,
                toSlantRow, toSlantCol);

        mem::ScopedArray<float> outputArray(new float[
                plane.xDirection->elements * plane.yDirection->elements]);
        // Iterate over output plane, grabbing the appropriate input point
        for (size_t yy = 0; yy < plane.yDirection->elements; ++yy)
        {
            const six::Poly1D rowPoly = toSlantRow.atY(yy);
            const six::Poly1D colPoly = toSlantCol.atY(yy);
            for (size_t xx = 0; xx < plane.xDirection->elements; ++xx)
            {
                const size_t line = math::round(rowPoly(xx));
                const size_t sample = math::round(colPoly(xx));
                const size_t inputPoint =
                        line * complexData->getNumCols() + sample;
                const size_t outputPoint =
                        xx + (yy * plane.xDirection->elements);
                const float outputValue = inputPoint >= buffer.size() ? 0 :
                        std::abs(buffer[inputPoint]);
                outputArray[outputPoint] = outputValue;
            }
        }

        sio::lite::FileWriter writer(outputPathname);
        writer.write(plane.yDirection->elements, plane.xDirection->elements,
                sizeof(float), sio::lite::FileHeader::FLOAT, outputArray.get());

        return 0;
    }
    catch (const except::Exception& ex)
    {
        std::cerr << ex.toString() << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "An unknown error occured\n";
    }
    return 1;
}

