/*
 * IndexPeaksWithSatellitesTest.h
 *
 *  Created on: July, 2018
 *      Author: Vickie Lynch
 */

#ifndef INDEXPEAKSWITHSATELLITESTEST_H_
#define INDEXPEAKSWITHSATELLITESTEST_H_

#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/Run.h"
#include "MantidCrystal/CalculateUMatrix.h"
#include "MantidCrystal/IndexPeaksWithSatellites.h"
#include "MantidCrystal/LoadIsawUB.h"
#include "MantidCrystal/PredictSatellitePeaks.h"
#include "MantidDataObjects/PeaksWorkspace.h"
#include "MantidKernel/System.h"
#include "MantidKernel/Timer.h"
#include "MantidTestHelpers/ComponentCreationHelper.h"
#include <cxxtest/TestSuite.h>

using namespace Mantid::Crystal;
using namespace Mantid::API;
using namespace Mantid::DataObjects;
using namespace Mantid::Kernel;

class IndexPeaksWithSatellitesTest : public CxxTest::TestSuite {

public:
  void test_Init() {
    IndexPeaksWithSatellites alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT(alg.isInitialized());
  }

  void test_exec() {
    Mantid::Geometry::Instrument_sptr inst =
        ComponentCreationHelper::createTestInstrumentRectangular2(1, 100, 10.);
    inst->setName("SillyInstrument");
    PeaksWorkspace_sptr pw(new PeaksWorkspace);
    pw->setInstrument(inst);
    std::string val = "value";
    pw->mutableRun().addProperty("TestProp", val);
    Peak p(inst, 1, 3.0, V3D(1, 0, 0)); // HKL=1,0,0
    pw->addPeak(p);
    Peak p1(inst, 10, 3.0, V3D(2, 0, 0)); // HKL=2,0,0
    Peak p2(inst, 20, 3.0, V3D(2, 2, 0)); // HKL=2,2,0
    std::string WSName("peaks-fail");
    AnalysisDataService::Instance().addOrReplace(WSName, pw);
    pw->addPeak(p1);
    pw->addPeak(p2);
    AnalysisDataService::Instance().addOrReplace(WSName, pw);
    CalculateUMatrix alg2;
    TS_ASSERT_THROWS_NOTHING(alg2.initialize())
    TS_ASSERT(alg2.isInitialized())
    TS_ASSERT_THROWS_NOTHING(alg2.setPropertyValue("a", "14.1526"));
    TS_ASSERT_THROWS_NOTHING(alg2.setPropertyValue("b", "19.2903"));
    TS_ASSERT_THROWS_NOTHING(alg2.setPropertyValue("c", "8.5813"));
    TS_ASSERT_THROWS_NOTHING(alg2.setPropertyValue("alpha", "90"));
    TS_ASSERT_THROWS_NOTHING(alg2.setPropertyValue("beta", "105.0738"));
    TS_ASSERT_THROWS_NOTHING(alg2.setPropertyValue("gamma", "90"));
    TS_ASSERT_THROWS_NOTHING(alg2.setPropertyValue("PeaksWorkspace", WSName));
    TS_ASSERT_THROWS_NOTHING(alg2.execute(););
    TS_ASSERT(alg2.isExecuted());

    PredictSatellitePeaks alg3;
    TS_ASSERT_THROWS_NOTHING(alg3.initialize());
    TS_ASSERT(alg3.isInitialized());

    alg3.setProperty("Peaks", WSName);

    alg3.setProperty("SatellitePeaks", std::string("SatellitePeaks"));
    alg3.setProperty("ModVector1", "0.5,0,.2");
    alg3.setProperty("MaxOrder", "1");
    TS_ASSERT(alg3.execute());
    TS_ASSERT(alg3.isExecuted());
    alg3.setPropertyValue("SatellitePeaks", "SatellitePeaks");
    PeaksWorkspace_sptr SatellitePeaks = alg3.getProperty("SatellitePeaks");

    IndexPeaksWithSatellites alg;
    TS_ASSERT_THROWS_NOTHING(alg.initialize());
    TS_ASSERT(alg.isInitialized());

    alg.setProperty("PeaksWorkspace", std::string("SatellitePeaks"));
    alg.setProperty("ModVector1", "0.5,0,.2");
    alg.setProperty("MaxOrder", "1");
    TS_ASSERT(alg.execute());
    TS_ASSERT(alg.isExecuted());

    TS_ASSERT_EQUALS(SatellitePeaks->getNumberPeaks(), 5);

    auto &peak0 = SatellitePeaks->getPeak(0);
    TS_ASSERT_DELTA(peak0.getH(), 0.5, .0001);
    TS_ASSERT_DELTA(peak0.getK(), 0., .0001);
    TS_ASSERT_DELTA(peak0.getL(), -0.2, .0001);

    auto &peak3 = SatellitePeaks->getPeak(3);
    TS_ASSERT_DELTA(peak3.getH(), 2.5, .0001);
    TS_ASSERT_DELTA(peak3.getK(), 0., .0001);
    TS_ASSERT_DELTA(peak3.getL(), 0.2, .0001);

    AnalysisDataService::Instance().remove(WSName);
  }
};

#endif /* INDEXPEAKSWITHSATELLITESTEST__H_ */