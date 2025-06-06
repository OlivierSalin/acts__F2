// This file is part of the Acts project.
//
// Copyright (C) 2019-2021 CERN for the benefit of the Acts project
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/test/unit_test.hpp>

#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Definitions/Direction.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/EventData/GenericBoundTrackParameters.hpp"
#include "Acts/EventData/GenericCurvilinearTrackParameters.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackContainer.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/TrackProxy.hpp"
#include "Acts/EventData/TrackStatePropMask.hpp"
#include "Acts/EventData/VectorMultiTrajectory.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/MagneticField/ConstantBField.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Navigator.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/StraightLineStepper.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Tests/CommonHelpers/CubicTrackingGeometry.hpp"
#include "Acts/Tests/CommonHelpers/LineSurfaceStub.hpp"
#include "Acts/Tests/CommonHelpers/MeasurementsCreator.hpp"
#include "Acts/Tests/CommonHelpers/TestSourceLink.hpp"
#include "Acts/TrackFinding/CombinatorialKalmanFilter.hpp"
#include "Acts/TrackFinding/MeasurementSelector.hpp"
#include "Acts/TrackFitting/GainMatrixSmoother.hpp"
#include "Acts/TrackFitting/GainMatrixUpdater.hpp"
#include "Acts/TrackFitting/KalmanFitter.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Utilities/Delegate.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "Acts/Utilities/Holders.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Utilities/Result.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <ostream>
#include <random>
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Acts {
class TrackingGeometry;
}  // namespace Acts

namespace {

using namespace Acts::Test;
using namespace Acts::UnitLiterals;

static const auto pion = Acts::ParticleHypothesis::pion();

struct Detector {
  // expected number of measurements for the given detector
  std::size_t numMeasurements = 6u;

  // geometry
  CubicTrackingGeometry store;
  std::shared_ptr<const Acts::TrackingGeometry> geometry;

  // resolutions
  MeasurementResolution resPixel = {MeasurementType::eLoc01, {25_um, 50_um}};
  MeasurementResolution resStrip0 = {MeasurementType::eLoc0, {100_um}};
  MeasurementResolution resStrip1 = {MeasurementType::eLoc1, {150_um}};
  MeasurementResolutionMap resolutions = {
      {Acts::GeometryIdentifier().setVolume(2), resPixel},
      {Acts::GeometryIdentifier().setVolume(3).setLayer(2), resStrip0},
      {Acts::GeometryIdentifier().setVolume(3).setLayer(4), resStrip1},
      {Acts::GeometryIdentifier().setVolume(3).setLayer(6), resStrip0},
      {Acts::GeometryIdentifier().setVolume(3).setLayer(8), resStrip1},
  };

  Detector(const Acts::GeometryContext& geoCtx)
      : store(geoCtx), geometry(store()) {}
};

/// The map(-like) container accessor
template <typename container_t>
struct TestContainerAccessor {
  using Container = container_t;
  using Key = typename container_t::key_type;
  using Value = typename container_t::mapped_type;

  /// This iterator adapter is needed to have the deref operator return a single
  /// source link instead of the map pair <GeometryIdentifier,SourceLink>
  struct Iterator {
    using BaseIterator = typename container_t::const_iterator;

    using iterator_category = typename BaseIterator::iterator_category;
    using value_type = typename BaseIterator::value_type;
    using difference_type = typename BaseIterator::difference_type;
    using pointer = typename BaseIterator::pointer;
    using reference = typename BaseIterator::reference;

    Iterator& operator++() {
      ++m_iterator;
      return *this;
    }

    bool operator==(const Iterator& other) const {
      return m_iterator == other.m_iterator;
    }

    bool operator!=(const Iterator& other) const { return !(*this == other); }

    Acts::SourceLink operator*() const {
      const auto& sl = m_iterator->second;
      return Acts::SourceLink{sl};
    }

    BaseIterator m_iterator;
  };

  // pointer to the container
  const Container* container = nullptr;

  // get the range of elements with requested key
  std::pair<Iterator, Iterator> range(const Acts::Surface& surface) const {
    assert(container != nullptr);
    auto [begin, end] = container->equal_range(surface.geometryId());
    return {Iterator{begin}, Iterator{end}};
  }
};

struct Fixture {
  using StraightPropagator =
      Acts::Propagator<Acts::StraightLineStepper, Acts::Navigator>;
  using ConstantFieldStepper = Acts::EigenStepper<>;
  using ConstantFieldPropagator =
      Acts::Propagator<ConstantFieldStepper, Acts::Navigator>;

  using Trajectory = Acts::VectorMultiTrajectory;

  using KalmanUpdater = Acts::GainMatrixUpdater;
  using KalmanSmoother = Acts::GainMatrixSmoother;
  using CombinatorialKalmanFilter =
      Acts::CombinatorialKalmanFilter<ConstantFieldPropagator, Trajectory>;
  using TestSourceLinkContainer =
      std::unordered_multimap<Acts::GeometryIdentifier, TestSourceLink>;
  using TestSourceLinkAccessor = TestContainerAccessor<TestSourceLinkContainer>;
  using CombinatorialKalmanFilterOptions =
      Acts::CombinatorialKalmanFilterOptions<TestSourceLinkAccessor::Iterator,
                                             Trajectory>;

  KalmanUpdater kfUpdater;
  KalmanSmoother kfSmoother;

  Acts::GeometryContext geoCtx;
  Acts::MagneticFieldContext magCtx;
  Acts::CalibrationContext calCtx;

  Detector detector;

  // track parameters before and after the detector
  std::vector<Acts::CurvilinearTrackParameters> startParameters;
  std::vector<Acts::CurvilinearTrackParameters> endParameters;

  // generated measurements
  TestSourceLinkContainer sourceLinks;

  // CKF implementation to be tested
  CombinatorialKalmanFilter ckf;
  // configuration for the measurement selector
  Acts::MeasurementSelector::Config measurementSelectorCfg = {
      // global default: no chi2 cut, only one measurement per surface
      {Acts::GeometryIdentifier(),
       {{}, {std::numeric_limits<double>::max()}, {1u}}},
  };

  Acts::MeasurementSelector measSel{measurementSelectorCfg};

  Acts::CombinatorialKalmanFilterExtensions<Trajectory> getExtensions() const {
    Acts::CombinatorialKalmanFilterExtensions<Trajectory> extensions;
    extensions.calibrator
        .template connect<&testSourceLinkCalibrator<Trajectory>>();
    extensions.updater.template connect<&KalmanUpdater::operator()<Trajectory>>(
        &kfUpdater);
    extensions.smoother
        .template connect<&KalmanSmoother::operator()<Trajectory>>(&kfSmoother);
    extensions.measurementSelector
        .template connect<&Acts::MeasurementSelector::select<Trajectory>>(
            &measSel);
    return extensions;
  }

  std::unique_ptr<const Acts::Logger> logger;

  Fixture(double bz)
      : detector(geoCtx),
        ckf(makeConstantFieldPropagator(detector.geometry, bz)),
        logger(Acts::getDefaultLogger("CkfTest", Acts::Logging::INFO)) {
    // construct initial parameters
    // create common covariance matrix from reasonable standard deviations
    Acts::BoundVector stddev;
    stddev[Acts::eBoundLoc0] = 100_um;
    stddev[Acts::eBoundLoc1] = 100_um;
    stddev[Acts::eBoundTime] = 25_ns;
    stddev[Acts::eBoundPhi] = 2_degree;
    stddev[Acts::eBoundTheta] = 2_degree;
    stddev[Acts::eBoundQOverP] = 1 / 100_GeV;
    Acts::BoundSquareMatrix cov = stddev.cwiseProduct(stddev).asDiagonal();
    // all tracks close to the transverse plane along the x axis w/ small
    // variations in position, direction.
    Acts::Vector4 mStartPos0(-3_m, 0.0, 0.0, 1_ns);
    Acts::Vector4 mStartPos1(-3_m, -15_mm, -15_mm, 2_ns);
    Acts::Vector4 mStartPos2(-3_m, 15_mm, 15_mm, -1_ns);
    startParameters = {
        {mStartPos0, 0_degree, 90_degree, 1_e / 1_GeV, cov, pion},
        {mStartPos1, -1_degree, 91_degree, 1_e / 1_GeV, cov, pion},
        {mStartPos2, 1_degree, 89_degree, -1_e / 1_GeV, cov, pion},
    };
    Acts::Vector4 mEndPos0(3_m, 0.0, 0.0, 1_ns);
    Acts::Vector4 mEndPos1(3_m, -100_mm, -100_mm, 2_ns);
    Acts::Vector4 mEndPos2(3_m, 100_mm, 100_mm, -1_ns);
    endParameters = {
        {mEndPos0, 0_degree, 90_degree, 1_e / 1_GeV, cov * 100, pion},
        {mEndPos1, -1_degree, 91_degree, 1_e / 1_GeV, cov * 100, pion},
        {mEndPos2, 1_degree, 89_degree, -1_e / 1_GeV, cov * 100, pion},
    };

    // create some measurements
    auto measPropagator = makeStraightPropagator(detector.geometry);
    std::default_random_engine rng(421235);
    for (std::size_t trackId = 0u; trackId < startParameters.size();
         ++trackId) {
      auto measurements = createMeasurements(
          measPropagator, geoCtx, magCtx, startParameters[trackId],
          detector.resolutions, rng, trackId);
      for (auto& sl : measurements.sourceLinks) {
        sourceLinks.emplace(sl.m_geometryId, std::move(sl));
      }
    }
  }

  // Construct a straight-line propagator.
  static StraightPropagator makeStraightPropagator(
      std::shared_ptr<const Acts::TrackingGeometry> geo) {
    Acts::Navigator::Config cfg{std::move(geo)};
    cfg.resolvePassive = false;
    cfg.resolveMaterial = true;
    cfg.resolveSensitive = true;
    Acts::Navigator navigator{cfg};
    Acts::StraightLineStepper stepper;
    return StraightPropagator(stepper, std::move(navigator));
  }

  // Construct a propagator using a constant magnetic field along z.
  static ConstantFieldPropagator makeConstantFieldPropagator(
      std::shared_ptr<const Acts::TrackingGeometry> geo, double bz) {
    Acts::Navigator::Config cfg{std::move(geo)};
    cfg.resolvePassive = false;
    cfg.resolveMaterial = true;
    cfg.resolveSensitive = true;
    Acts::Navigator navigator{cfg};
    auto field =
        std::make_shared<Acts::ConstantBField>(Acts::Vector3(0.0, 0.0, bz));
    ConstantFieldStepper stepper(std::move(field));
    return ConstantFieldPropagator(std::move(stepper), std::move(navigator));
  }

  CombinatorialKalmanFilterOptions makeCkfOptions() const {
    return CombinatorialKalmanFilterOptions(
        geoCtx, magCtx, calCtx,
        Acts::SourceLinkAccessorDelegate<
            TestSourceLinkAccessor::Iterator>{},  // leave the accessor empty,
                                                  // this will have to be set
                                                  // before running the CKF
        getExtensions(), Acts::PropagatorPlainOptions());
  }
};

}  // namespace

BOOST_AUTO_TEST_SUITE(TrackFindingCombinatorialKalmanFilter)

BOOST_AUTO_TEST_CASE(ZeroFieldForward) {
  Fixture f(0_T);

  auto options = f.makeCkfOptions();
  // this is the default option. set anyway for consistency
  options.propagatorPlainOptions.direction = Acts::Direction::Forward;
  // Construct a plane surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PlaneSurface>(
      Acts::Vector3{-3_m, 0., 0.}, Acts::Vector3{1., 0., 0});
  // Set the target surface
  options.smoothingTargetSurface = pSurface.get();

  Fixture::TestSourceLinkAccessor slAccessor;
  slAccessor.container = &f.sourceLinks;
  options.sourcelinkAccessor.connect<&Fixture::TestSourceLinkAccessor::range>(
      &slAccessor);

  Acts::TrackContainer tc{Acts::VectorTrackContainer{},
                          Acts::VectorMultiTrajectory{}};

  // run the CKF for all initial track states
  for (std::size_t trackId = 0u; trackId < f.startParameters.size();
       ++trackId) {
    auto res = f.ckf.findTracks(f.startParameters.at(trackId), options, tc);
    if (!res.ok()) {
      BOOST_TEST_INFO(res.error() << " " << res.error().message());
    }
    BOOST_REQUIRE(res.ok());
  }

  // There should be three track finding results with three initial track states
  BOOST_CHECK_EQUAL(tc.size(), 3u);

  // check the found tracks
  for (std::size_t trackId = 0u; trackId < f.startParameters.size();
       ++trackId) {
    const auto track = tc.getTrack(trackId);
    const auto& params = f.startParameters[trackId];
    BOOST_TEST_INFO("initial parameters before detector:\n" << params);

    BOOST_CHECK_EQUAL(track.nTrackStates(), f.detector.numMeasurements);

    // check purity of first found track
    // find the number of hits not originating from the right track
    std::size_t numHits = 0u;
    std::size_t nummismatchedHits = 0u;
    for (const auto trackState : track.trackStatesReversed()) {
      numHits += 1u;
      auto sl =
          trackState.getUncalibratedSourceLink().template get<TestSourceLink>();
      if (trackId != sl.sourceId) {
        nummismatchedHits++;
      }
    }

    BOOST_CHECK_EQUAL(numHits, f.detector.numMeasurements);
    BOOST_CHECK_EQUAL(nummismatchedHits, 0u);
  }
}

BOOST_AUTO_TEST_CASE(ZeroFieldBackward) {
  Fixture f(0_T);

  auto options = f.makeCkfOptions();
  options.propagatorPlainOptions.direction = Acts::Direction::Backward;
  // Construct a plane surface as the target surface
  auto pSurface = Acts::Surface::makeShared<Acts::PlaneSurface>(
      Acts::Vector3{3_m, 0., 0.}, Acts::Vector3{1., 0., 0});
  // Set the target surface
  options.smoothingTargetSurface = pSurface.get();

  Fixture::TestSourceLinkAccessor slAccessor;
  slAccessor.container = &f.sourceLinks;
  options.sourcelinkAccessor.connect<&Fixture::TestSourceLinkAccessor::range>(
      &slAccessor);

  Acts::TrackContainer tc{Acts::VectorTrackContainer{},
                          Acts::VectorMultiTrajectory{}};

  // run the CKF for all initial track states
  for (std::size_t trackId = 0u; trackId < f.startParameters.size();
       ++trackId) {
    auto res = f.ckf.findTracks(f.endParameters.at(trackId), options, tc);
    if (!res.ok()) {
      BOOST_TEST_INFO(res.error() << " " << res.error().message());
    }
    BOOST_REQUIRE(res.ok());
  }
  // There should be three found tracks with three initial track states
  BOOST_CHECK_EQUAL(tc.size(), 3u);

  // check the found tracks
  for (std::size_t trackId = 0u; trackId < f.endParameters.size(); ++trackId) {
    const auto track = tc.getTrack(trackId);
    const auto& params = f.endParameters[trackId];
    BOOST_TEST_INFO("initial parameters after detector:\n" << params);

    BOOST_CHECK_EQUAL(track.nTrackStates(), f.detector.numMeasurements);

    // check purity of first found track
    // find the number of hits not originating from the right track
    std::size_t numHits = 0u;
    std::size_t nummismatchedHits = 0u;
    for (const auto trackState : track.trackStatesReversed()) {
      numHits += 1u;
      auto sl =
          trackState.getUncalibratedSourceLink().template get<TestSourceLink>();
      if (trackId != sl.sourceId) {
        nummismatchedHits++;
      }
    }

    BOOST_CHECK_EQUAL(numHits, f.detector.numMeasurements);
    BOOST_CHECK_EQUAL(nummismatchedHits, 0u);
  }
}

BOOST_AUTO_TEST_SUITE_END()
