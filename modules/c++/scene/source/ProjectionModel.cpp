#include "scene/ProjectionModel.h"

using namespace scene;

ProjectionModel::
ProjectionModel(const Vector3& slantPlaneNormal,
                const Vector3& imagePlaneRowVector,
                const Vector3& imagePlaneColVector,
                const Vector3& scp,
                const math::poly::OneD<Vector3>& arpPoly,
                const math::poly::TwoD<double>& timeCOAPoly,
                int lookDir) :
    mSlantPlaneNormal(slantPlaneNormal),
    mImagePlaneRowVector(imagePlaneRowVector),
    mImagePlaneColVector(imagePlaneColVector),
    mSCP(scp),
    mARPPoly(arpPoly),
    mTimeCOAPoly(timeCOAPoly),
    mLookDir(lookDir)
    
{
    mImagePlaneNormal =
        math::linear::cross(imagePlaneRowVector, imagePlaneColVector);
    
    mSlantPlaneNormal.normalize();
    mImagePlaneNormal.normalize();
    
    mScaleFactor = mSlantPlaneNormal.dot(mImagePlaneNormal);
    
    mARPVelPoly = mARPPoly.derivative();
    
}

/*!
 *  Calculations for section 5.2 in SICD Image Projections:
 *  R/Rdot Contour Ground Plane Intersection
 *
 */
Vector3
ProjectionModel::contourToGroundPlane(double rCOA, double rDotCOA,
                                      const Vector3& arpCOA,
                                      const Vector3& velCOA,
                                      double timeCOA,
                                      const Vector3& groundPlaneNormal,
                                      const Vector3& groundRefPoint) const
{
    // Compute the ARP distance from the plane (ARP Z)
    Vector3 tmp(arpCOA - groundRefPoint);
    const double arpZ = tmp.dot(groundPlaneNormal);
    
    // Compute the ARP ground plane nadir
    const Vector3 arpGround(arpCOA - groundPlaneNormal * arpZ);
    
    // Compute the ground plane distance from the ARP nadir to
    // the circle of constant range.
    if (std::abs(arpZ) > std::abs(rCOA))
    {
        throw except::Exception(Ctxt(
                  "No solution: arpZ = " + str::toString(arpZ) + ", rCOA = " +
                  str::toString(rCOA)));
    }
    const double groundRange = sqrt(rCOA * rCOA - arpZ * arpZ);
    
    // Compute cos and sin of the grazing angle
    const double cosGraz( groundRange / rCOA );
    const double sinGraz( arpZ / rCOA );
    
    // Compute the velocity components normal to the ground
    // plane and parallel to the ground plane
    const double vz = velCOA.dot(groundPlaneNormal);
    
    const double vmag = velCOA.norm();
    
    if (std::abs(vz) >= std::abs(vmag))
    {
        throw except::Exception(Ctxt(
                  "No solution: vz = " + str::toString(vz) + ", vmag = " +
                  str::toString(vmag)));
    }
    const double vx = sqrt(vmag * vmag - vz * vz);
    
    // Orient the x direction in the ground plane such that
    // vx > 0.  Compute unit vectors unitX and unitY
    const Vector3 unitX = (velCOA - groundPlaneNormal * vz) / vx;
    const Vector3 unitY = cross(groundPlaneNormal, unitX);
    
    // Compute the cosine of the azimuth angle to the ground
    // plane point
    const double cosAzimuth =
        (-rDotCOA + vz * sinGraz) / (vx * cosGraz);
    if (cosAzimuth < -1.0 || cosAzimuth > 1.0)
    {
        throw except::Exception(Ctxt(
                  "No solution: cosAzimuth = " + str::toString(cosAzimuth)));
    }
    
    const double sinAzimuth =
        mLookDir * sqrt(1.0 - cosAzimuth * cosAzimuth);
    
    return Vector3(arpGround + unitX * groundRange * cosAzimuth +
                          unitY * groundRange * sinAzimuth);
}


RowCol<double>
ProjectionModel::sceneToImage(const Vector3& scenePoint,
                              double* oTimeCOA) const
{
    // For scenePoint, we will compute the spherical earth
    // unit ground plane normal (uGPN)
    Vector3 groundRefPoint(scenePoint);
    Vector3 groundPlaneNormal(groundRefPoint);
    groundPlaneNormal.normalize();
    
    // Set initial ground plane position to the scenePoint
    Vector3 groundPlanePoint(scenePoint);
        
    for (size_t i = 0; i < MAX_ITER; ++i)
    {
        
        // We are projecting the ground plane point to the image
        // plane point. 
        Vector3 diff(mSCP - groundPlanePoint);
        
        // Dist contains the projection difference
        double dist = diff.dot(mImagePlaneNormal) * mScaleFactor;
        
        Vector3 imagePlanePoint =
            groundPlanePoint + mSlantPlaneNormal * dist;
        
        // Compute the imageCoordinates for the plane point
        RowCol<double> imageGridPoint =
            computeImageCoordinates(imagePlanePoint);
        
        // Find out if scene point is the same as the guessed output
        // of imageToScenee
        diff = scenePoint - imageToScene(imageGridPoint,
                                         groundRefPoint,
                                         groundPlaneNormal,
                                         oTimeCOA);
        
        dist = diff.norm();
        
        if (dist < DELTA_GP_MAX)
            return imageGridPoint;
        
        // Otherwise we are not so lucky, add to our point
        // the difference
        groundPlanePoint += diff;
        
    }
    
    throw except::Exception(Ctxt("Point failed to converge"));
}

Vector3
ProjectionModel::imageToScene(const RowCol<double>& imageGridPoint,
                              const Vector3& groundRefPoint,
                              const Vector3& groundPlaneNormal,
                              double *oTimeCOA) const
{
    
    // Compute the timeCOA
    double timeCOA = mTimeCOAPoly(imageGridPoint.row,
                                  imageGridPoint.col);
    
    if (oTimeCOA != NULL)
        *oTimeCOA = timeCOA;
    
    // Compute ARP position
    Vector3 arpCOA = mARPPoly(timeCOA);
    
    // Compute ARP velocity
    Vector3 velCOA = mARPVelPoly(timeCOA);
    
    double r;
    double rDot;
    
    computeContour(arpCOA, velCOA, timeCOA,
                   imageGridPoint,
                   &r,
                   &rDot);
    
    return contourToGroundPlane(r, rDot,
                                arpCOA, velCOA, timeCOA,
                                groundPlaneNormal,
                                groundRefPoint);
}

void ProjectionModel::computeProjectionPolynomials(
    const scene::GridGeometry& gridGeom,
    const RowCol<size_t>& inPixelStart,
    const RowCol<double>& inSceneCenter,
    const RowCol<double>& interimSceneCenter,
    const RowCol<double>& interimSampleSpacing,
    const RowCol<double>& outSceneCenter,
    const RowCol<double>& outSampleSpacing,
    const RowCol<size_t>& outExtent,
    size_t polyOrder,
    math::poly::TwoD<double>& outputToSlantRow,
    math::poly::TwoD<double>& outputToSlantCol,
    math::poly::TwoD<double>& timeCOAPoly,
    double* meanResidualErrorRow,
    double* meanResidualErrorCol,
    double* meanResidualErrorTCOA) const
{
    // Points to use in each direction
    static const size_t POINTS_1D = 10;

    // Want to sample [0, outExtent) in the loop below
    const RowCol<double> skip(
        static_cast<double>(outExtent.row - 1) / (POINTS_1D - 1),
        static_cast<double>(outExtent.col - 1) / (POINTS_1D - 1));

    const RowCol<double> ratio(interimSceneCenter.row / inSceneCenter.row,
                               interimSceneCenter.col / inSceneCenter.col);

    const RowCol<double> outOffset(inPixelStart.row * ratio.row,
                                   inPixelStart.col * ratio.col);

    math::linear::Matrix2D<double> rowMapping(POINTS_1D, POINTS_1D);
    math::linear::Matrix2D<double> colMapping(POINTS_1D, POINTS_1D);
    math::linear::Matrix2D<double> tcoaMapping(POINTS_1D, POINTS_1D);
    math::linear::Matrix2D<double> lines(POINTS_1D, POINTS_1D);
    math::linear::Matrix2D<double> samples(POINTS_1D, POINTS_1D);
    math::linear::Matrix2D<double> tcoaLines(POINTS_1D, POINTS_1D);
    math::linear::Matrix2D<double> tcoaSamples(POINTS_1D, POINTS_1D);

    RowCol<double> currentOffset(0., 0.);
    
    for (size_t ii = 0; ii < POINTS_1D; ++ii, currentOffset.row += skip.row)
    {
        currentOffset.col = 0;
        for (size_t jj = 0; jj < POINTS_1D; ++jj, currentOffset.col += skip.col)
        {
            // Rows, these are essentially meshgrid-like (replicated)
            lines(ii, jj) = currentOffset.row;
            // This represents columns
            samples(ii, jj) = currentOffset.col;

            // For the time COA poly, we need to map lines/samples to meters
            // from SCP in the output grid
            tcoaLines(ii, jj) =
                (lines(ii,jj) - outSceneCenter.row) * outSampleSpacing.row;

            tcoaSamples(ii, jj) =
                (samples(ii,jj) - outSceneCenter.col) * outSampleSpacing.col;

            // Find initial position in the scene
            const scene::Vector3 sPos =
                gridGeom.rowColToECEF(currentOffset.row, currentOffset.col);

            // This HAS to be a scene coordinate
            double timeCOA(0.0);
            const scene::RowCol<double> rgAz = sceneToImage(sPos, &timeCOA);

            // Adjust here for the start offset
            rowMapping(ii, jj) = rgAz.row / interimSampleSpacing.row +
                    interimSceneCenter.row - outOffset.row;
            colMapping(ii, jj) = rgAz.col / interimSampleSpacing.col +
                    interimSceneCenter.col - outOffset.col;

            tcoaMapping(ii, jj) = timeCOA;
        }
    }

    // Now fit the outputToSlantRows, outputToSlantCols, and timeCOAPoly
    outputToSlantRow = math::poly::fit(lines, samples, rowMapping,
                                       polyOrder, polyOrder);

    outputToSlantCol = math::poly::fit(lines, samples, colMapping,
                                       polyOrder, polyOrder);

    timeCOAPoly = math::poly::fit(tcoaLines, tcoaSamples, tcoaMapping,
                                  polyOrder, polyOrder);

    if (meanResidualErrorRow || meanResidualErrorCol || meanResidualErrorTCOA)
    {
        double errorSumRow(0.0);
        double errorSumCol(0.0);
        double errorSumTCOA(0.0);

        for (size_t ii = 0; ii < POINTS_1D; ++ii)
        {
            for (size_t jj = 0; jj < POINTS_1D; ++jj)
            {
                const double row(lines(ii, jj));
                const double col(samples(ii, jj));

                double diff = rowMapping(ii, jj) - outputToSlantRow(row, col);
                errorSumRow += diff * diff;

                diff = colMapping(ii, jj) - outputToSlantCol(row, col);
                errorSumCol += diff * diff;

                diff = tcoaMapping(ii, jj) - timeCOAPoly(row, col);
                errorSumTCOA += diff * diff;
            }
        }

        static const size_t NUM_POINTS = POINTS_1D * POINTS_1D;
        if (meanResidualErrorRow)
        {
            *meanResidualErrorRow = errorSumRow / NUM_POINTS;
        }
        if (meanResidualErrorCol)
        {
            *meanResidualErrorCol = errorSumCol / NUM_POINTS;
        }
        if (meanResidualErrorTCOA)
        {
            *meanResidualErrorTCOA = errorSumTCOA / NUM_POINTS;
        }
    }
}

RangeAzimProjectionModel::
RangeAzimProjectionModel(const math::poly::OneD<double>& polarAnglePoly,
                         const math::poly::OneD<double>& ksfPoly,
                         const Vector3& slantPlaneNormal,
                         const Vector3& imagePlaneRowVector,
                         const Vector3& imagePlaneColVector,
                         const Vector3& scp,
                         const math::poly::OneD<Vector3>& arpPoly,
                         const math::poly::TwoD<double>& timeCOAPoly,
                         int lookDir) :
    ProjectionModel(slantPlaneNormal,
                    imagePlaneRowVector,
                    imagePlaneColVector,
                    scp,
                    arpPoly,
                    timeCOAPoly,
                    lookDir),
    mPolarAnglePoly(polarAnglePoly), mKSFPoly(ksfPoly)
{
    mPolarAnglePolyPrime = mPolarAnglePoly.derivative();
    mKSFPolyPrime = mKSFPoly.derivative();
}

void RangeAzimProjectionModel::
computeContour(const Vector3& arpCOA,
               const Vector3& velCOA,
               double timeCOA,
               const RowCol<double>& imageGridPoint,
               double* r,
               double* rDot) const
{
    double thetaCOA = mPolarAnglePoly(timeCOA);
    double dThetaDt = mPolarAnglePolyPrime(timeCOA);
    
    double ksf = mKSFPoly(thetaCOA);
    double dKSFDTheta = mKSFPolyPrime(thetaCOA);
    
    double cosTheta = cos(thetaCOA);
    double sinTheta = sin(thetaCOA);
    
    double slopeRadial =
        imageGridPoint.row * cosTheta +
        imageGridPoint.col * sinTheta;
    
    double slopeCrossRadial =
        -imageGridPoint.row * sinTheta +
        imageGridPoint.col * cosTheta;
    
    double dR = ksf * slopeRadial;
    
    double dDrDTheta = dKSFDTheta * slopeRadial + ksf * slopeCrossRadial;
    double dRDot = dDrDTheta * dThetaDt;
    
    Vector3 vec = arpCOA - mSCP;
    *r = vec.norm();
    
    *rDot = velCOA.dot(vec) / *r;
    
    *r += dR;
    *rDot += dRDot;
    
}


RangeZeroProjectionModel::
RangeZeroProjectionModel(const math::poly::OneD<double>& timeCAPoly,
                         const math::poly::TwoD<double>& dsrfPoly,
                         double rangeCA,
                         const Vector3& slantPlaneNormal,
                         const Vector3& imagePlaneRowVector,
                         const Vector3& imagePlaneColVector,
                         const Vector3& scp,
                         const math::poly::OneD<Vector3>& arpPoly,
                         const math::poly::TwoD<double>& timeCOAPoly,
                         int lookDir) :
    ProjectionModel(slantPlaneNormal,
                    imagePlaneRowVector,
                    imagePlaneColVector,
                    scp,
                    arpPoly,
                    timeCOAPoly,
                    lookDir),
    mTimeCAPoly(timeCAPoly), mDSRFPoly(dsrfPoly), mRangeCA(rangeCA) {}

void RangeZeroProjectionModel::
computeContour(const Vector3& arpCOA,
               const Vector3& velCOA,
               double timeCOA,
               const RowCol<double>& imageGridPoint,
               double* r,
               double* rDot) const
{
    
    // Time of closest approach
    double timeCA = mTimeCAPoly(imageGridPoint.col);
    
    // Time Difference
    double deltaTimeCOA = timeCOA - timeCA;
    
    // Velocity at closest approach
    double velocityMagCA = mARPVelPoly(timeCA).norm();
    
    double t = deltaTimeCOA * velocityMagCA;
    
    double dsrf = mDSRFPoly(imageGridPoint.row, imageGridPoint.col);
    
    double rangeCA = mRangeCA + imageGridPoint.row;

    *r = sqrt(rangeCA * rangeCA + dsrf * (t * t));
    *rDot = dsrf / (*r) * t * velocityMagCA;
}

PlaneProjectionModel::
PlaneProjectionModel(const Vector3& slantPlaneNormal,
                     const Vector3& imagePlaneRowVector,
                     const Vector3& imagePlaneColVector,
                     const Vector3& scp,
                     const math::poly::OneD<Vector3>& arpPoly,
                     const math::poly::TwoD<double>& timeCOAPoly,
                     int lookDir) :
    ProjectionModel(slantPlaneNormal,
                    imagePlaneRowVector,
                    imagePlaneColVector,
                    scp,
                    arpPoly,
                    timeCOAPoly,
                    lookDir) {}


void PlaneProjectionModel::
computeContour(const Vector3& arpCOA,
               const Vector3& velCOA,
               double timeCOA,
               const RowCol<double>& imageGridPoint,
               double* r,
               double* rDot) const
{
    Vector3 vec(
        mSCP +
        (mImagePlaneRowVector * imageGridPoint.row) +
        (mImagePlaneColVector * imageGridPoint.col)
        );
    vec = arpCOA - vec;
    *r = vec.norm();
    
    *rDot = velCOA.dot(vec) / *r;
    
}
