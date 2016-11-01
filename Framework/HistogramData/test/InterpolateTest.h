#ifndef MANTID_HISTOGRAMDATA_INTERPOLATETEST_H_
#define MANTID_HISTOGRAMDATA_INTERPOLATETEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidHistogramData/Histogram.h"
#include "MantidHistogramData/Interpolate.h"
#include "MantidHistogramData/LinearGenerator.h"
#include "MantidTestHelpers/HistogramDataTestHelper.h"

using namespace Mantid::HistogramData;

class InterpolateTest : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static InterpolateTest *createSuite() { return new InterpolateTest(); }
  static void destroySuite(InterpolateTest *suite) { delete suite; }

  // ---------------------------------------------------------------------------
  // Success cases - point X data
  // ---------------------------------------------------------------------------
  void test_interpolateLinearPointDataSet_Stepsize_One_Less_Point_Size() {
    Histogram input(Points(5, LinearGenerator(0, 0.5)), {-2, 0, 0, 0, 2});
    auto output = interpolateLinear(input, 4);

    checkSizesUnchanged(input, output);
    std::vector<double> expectedY = {-2., -1., 0., 1., 2.};
    checkData(input, output, expectedY);

    // Inplace
    Histogram inOut(input);
    TS_ASSERT_THROWS_NOTHING(interpolateLinearInplace(inOut, 4));

    checkSizesUnchanged(input, inOut);
    checkData(input, inOut, expectedY);
  }

  void test_interpolateLinearPointDataSet_Even_StepSize() {
    Histogram input(Points(5, LinearGenerator(0, 0.5)), {-2, 0, 0.5, 0, 2});
    auto output = interpolateLinear(input, 2);

    checkSizesUnchanged(input, output);
    std::vector<double> expectedY = {-2., -0.75, 0.5, 1.25, 2.};
    checkData(input, output, expectedY);

    // Inplace
    Histogram inOut(input);
    TS_ASSERT_THROWS_NOTHING(interpolateLinearInplace(inOut, 2));

    checkSizesUnchanged(input, inOut);
    checkData(input, inOut, expectedY);
  }

  void test_interpolateLinearPointDataSet_Odd_StepSize() {
    Histogram input(Points(5, LinearGenerator(0, 0.5)), {-2, 0, 0.0, 0.5, 2});
    auto output = interpolateLinear(input, 3);

    checkSizesUnchanged(input, output);
    std::vector<double> expectedY = {-2., -2 + (2.5 / 1.5) * 0.5, -1. / 3., 0.5,
                                     2.};
    checkData(input, output, expectedY);

    // Inplace
    Histogram inOut(input);
    TS_ASSERT_THROWS_NOTHING(interpolateLinearInplace(inOut, 3));

    checkSizesUnchanged(input, inOut);
    checkData(input, inOut, expectedY);
  }

  // ---------------------------------------------------------------------------
  // Success cases - edge X data
  // ---------------------------------------------------------------------------
  void test_interpolateLinearEdgeDataSet_Stepsize_One_Less_Point_Size() {
    Histogram input(BinEdges(6, LinearGenerator(-0.25, 0.5)),
                    Counts({-2, 0, 0, 0, 2}));
    auto output = interpolateLinear(input, 4);

    checkSizesUnchanged(input, output);
    std::vector<double> expectedY = {-2., -1., 0., 1., 2.};
    checkData(input, output, expectedY);

    // Inplace
    Histogram inOut(input);
    TS_ASSERT_THROWS_NOTHING(interpolateLinearInplace(inOut, 4));

    checkSizesUnchanged(input, inOut);
    checkData(input, inOut, expectedY);
  }

  void test_interpolateLinearEdgeDataSet_Even_StepSize() {
    Histogram input(BinEdges(6, LinearGenerator(-0.25, 0.5)),
                    {-2, 0, 0.5, 0, 2});
    auto output = interpolateLinear(input, 2);

    checkSizesUnchanged(input, output);
    std::vector<double> expectedY = {-2., -0.75, 0.5, 1.25, 2.};
    checkData(input, output, expectedY);

    // Inplace
    Histogram inOut(input);
    TS_ASSERT_THROWS_NOTHING(interpolateLinearInplace(inOut, 2));

    checkSizesUnchanged(input, inOut);
    checkData(input, inOut, expectedY);
  }

  void test_interpolateLinearEdgeDataSet_Odd_StepSize() {
    Histogram input(BinEdges(6, LinearGenerator(-0.25, 0.5)),
                    {-2, 0, 0.0, 0.5, 2});
    Histogram output = interpolateLinear(input, 3);

    checkSizesUnchanged(input, output);
    std::vector<double> expectedY = {-2., -2 + (2.5 / 1.5) * 0.5, -1. / 3., 0.5,
                                     2.};
    checkData(input, output, expectedY);

    // Inplace
    Histogram inOut(input);
    TS_ASSERT_THROWS_NOTHING(interpolateLinearInplace(inOut, 3));

    checkSizesUnchanged(input, inOut);
    checkData(input, inOut, expectedY);
  }

  // ---------------------------------------------------------------------------
  // Success cases - Point data with frequencies
  //                 single test case as whitebox testing tells us the algorithm
  //                 is the same
  // ---------------------------------------------------------------------------

  void test_interpolateLinearPointFrequencyData_Stepsize_One_Less_Point_Size() {
    Histogram input(Points(5, LinearGenerator(0., 0.5)),
                    Frequencies({-2, 0, 0, 0, 2}));
    auto output = interpolateLinear(input, 4);

    checkSizesUnchanged(input, output);
    std::vector<double> expectedY = {-2., -1., 0., 1., 2.};
    checkData(input, output, expectedY);

    // Inplace
    Histogram inOut(input);
    TS_ASSERT_THROWS_NOTHING(interpolateLinearInplace(inOut, 4));

    checkSizesUnchanged(input, inOut);
    checkData(input, inOut, expectedY);
  }

  // ---------------------------------------------------------------------------
  // Common checking code
  // ---------------------------------------------------------------------------
  void checkSizesUnchanged(const Histogram &input, const Histogram &output) {
    TS_ASSERT_EQUALS(input.y().size(), output.y().size());
    TS_ASSERT_EQUALS(input.x().size(), output.x().size());
  }

  void checkData(const Histogram &input, const Histogram &output,
                 const std::vector<double> &expectedY) {
    TS_ASSERT_EQUALS(input.x(), output.x());
    TS_ASSERT_EQUALS(input.xMode(), output.xMode());
    TS_ASSERT_EQUALS(input.yMode(), output.yMode());
    const auto &outY = output.y();
    for (size_t i = 0; i < expectedY.size(); ++i) {
      TS_ASSERT_DELTA(expectedY[i], outY[i], 1e-14);
    }
  }

  // ---------------------------------------------------------------------------
  // Failure cases
  // ---------------------------------------------------------------------------
  void test_interpolatelinear_throws_for_undefined_ymode_type() {
    TS_ASSERT_THROWS(
        interpolateLinear(Histogram(Points(10, LinearGenerator(0, 0.5))), 10),
        std::runtime_error);
  }

  void test_interpolatelinear_throws_if_number_points_less_than_3() {
    TS_ASSERT_THROWS(
        interpolateLinear(Histogram(Points(2, LinearGenerator(0, 0.5))), 1),
        std::runtime_error);
    TS_ASSERT_THROWS(
        interpolateLinear(Histogram(Points(2, LinearGenerator(0, 0.5))), 1),
        std::runtime_error);
  }
  void
  test_interpolatelinear_throws_if_stepsize_greater_or_equal_number_points() {
    TS_ASSERT_THROWS(
        interpolateLinear(Histogram(Points(6, LinearGenerator(0, 0.5))), 6),
        std::runtime_error);
    TS_ASSERT_THROWS(
        interpolateLinear(Histogram(Points(6, LinearGenerator(0, 0.5))), 7),
        std::runtime_error);
  }
};

#endif /* MANTID_HISTOGRAMDATA_INTERPOLATETEST_H_ */
