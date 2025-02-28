#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>

// dependence
#include "RecActsTracking.h"

#include "GearSvc/IGearSvc.h"

// csv parser
// #include "csv2/writer.hpp"

// MC
#include "CLHEP/Units/SystemOfUnits.h"


using namespace Acts::UnitLiterals;

DECLARE_COMPONENT(RecActsTracking)

RecActsTracking::RecActsTracking(const std::string& name, ISvcLocator* svcLoc)
    : GaudiAlgorithm(name, svcLoc)
{
}

StatusCode RecActsTracking::initialize()
{
    chronoStatSvc = service<IChronoStatSvc>("ChronoStatSvc");
    _nEvt = 0;

    if (!std::filesystem::exists(TGeo_path.value())) {
        error() << "CEPC TGeo file: " << TGeo_path.value() << " does not exist!" << endmsg;
        return StatusCode::FAILURE;
    }

    if (!std::filesystem::exists(TGeo_config_path.value())) {
        error() << "CEPC TGeo config file: " << TGeo_config_path.value() << " does not exist!" << endmsg;
        return StatusCode::FAILURE;
    }

    if (!std::filesystem::exists(MaterialMap_path.value())) {
        error() << "CEPC Material map file: " << MaterialMap_path.value() << " does not exist!" << endmsg;
        return StatusCode::FAILURE;
    }

    if(!m_particle.value().empty()){
        info() << "Assume Particle: " << m_particle.value() << endmsg;
        if(m_particle.value() == "muon"){
            particleHypothesis = Acts::ParticleHypothesis::muon();
        }
        else if(m_particle.value() == "pion"){
            particleHypothesis = Acts::ParticleHypothesis::pion();
        }
        else if(m_particle.value() == "electron"){
            particleHypothesis = Acts::ParticleHypothesis::electron();
        }
        else if(m_particle.value() == "kaon"){
            particleHypothesis = Acts::ParticleHypothesis::kaon();
        }
        else if(m_particle.value() == "proton"){
            particleHypothesis = Acts::ParticleHypothesis::proton();
        }
        else if(m_particle.value() == "photon"){
            particleHypothesis = Acts::ParticleHypothesis::photon();
        }
        else if(m_particle.value() == "geantino"){
            particleHypothesis = Acts::ParticleHypothesis::geantino();
        }
        else if(m_particle.value() == "chargedgeantino"){
            particleHypothesis = Acts::ParticleHypothesis::chargedGeantino();
        }
        else{
            info() << "Supported Assumed Particle: " << particleNames[0] << ", " << particleNames[1] << ", " << particleNames[2] << ", "
                                             << particleNames[3] << ", " << particleNames[4] << ", " << particleNames[5] << ", "
                                             << particleNames[6] << ", " << particleNames[7] << endmsg;
            error() << "Unsupported particle name " << m_particle.value() << endmsg;
            return StatusCode::FAILURE;
        }
    }

    TGeo_ROOTFilePath = TGeo_path.value();
    TGeoConfig_jFilePath = TGeo_config_path.value();
    MaterialMap_jFilePath = MaterialMap_path.value();

    chronoStatSvc->chronoStart("read geometry");
    m_geosvc = service<IGeomSvc>("GeomSvc");
    if (!m_geosvc) {
        error() << "Failed to find GeomSvc." << endmsg;
        return StatusCode::FAILURE;
    }

    if(m_geosvc){
        const dd4hep::Direction& field = m_geosvc->lcdd()->field().magneticField(dd4hep::Position(0,0,0));
        m_field = field.z()/dd4hep::tesla;
    }

    m_vtx_surfaces = m_geosvc->getSurfaceMap("VXD");
    debug() << "Surface map size: " << m_vtx_surfaces->size() << endmsg;

    m_sit_surfaces = m_geosvc->getSurfaceMap("SIT");
    debug() << "Surface map size: " << m_sit_surfaces->size() << endmsg;

    m_ftd_surfaces = m_geosvc->getSurfaceMap("FTD");
    debug() << "Surface map size: " << m_ftd_surfaces->size() << endmsg;

    vxd_decoder = m_geosvc->getDecoder("VXDCollection");
    if(!vxd_decoder){
        return StatusCode::FAILURE;
    }

    sit_decoder = m_geosvc->getDecoder("SITCollection");
    if(!sit_decoder){
        return StatusCode::FAILURE;
    }

    ftd_decoder = m_geosvc->getDecoder("FTDCollection");
    if(!ftd_decoder){
        return StatusCode::FAILURE;
    }

    info() << "ACTS Tracking Geometry initialized successfully!" << endmsg;
    // initialize tgeo detector
    auto logger = Acts::getDefaultLogger("TGeoDetector", Acts::Logging::INFO);
    trackingGeometry = buildTGeoDetector(geoContext, detElementStore, TGeo_ROOTFilePath, TGeoConfig_jFilePath, MaterialMap_jFilePath, *logger);

    info() << "Seeding tools initialized successfully!" << endmsg;

    // configure the acts tools
    seed_cfg.seedFinderOptions.bFieldInZ = m_field.value();
    seed_cfg.seedFinderConfig.deltaRMin = SeedDeltaRMin.value();
    seed_cfg.seedFinderConfig.deltaRMax = SeedDeltaRMax.value();
    seed_cfg.seedFinderConfig.rMax = SeedRMax.value();
    seed_cfg.seedFinderConfig.rMin = SeedRMin.value();
    seed_cfg.seedFinderConfig.impactMax = SeedImpactMax.value();
    seed_cfg.seedFinderConfig.useVariableMiddleSPRange = false;
    seed_cfg.seedFinderConfig.rMinMiddle = SeedRMinMiddle.value();
    seed_cfg.seedFinderConfig.rMaxMiddle = SeedRMaxMiddle.value();

    // initialize the acts tools
    seed_cfg.seedFilterConfig = seed_cfg.seedFilterConfig.toInternalUnits();
    seed_cfg.seedFinderConfig.seedFilter =
        std::make_unique<Acts::SeedFilter<SimSpacePoint>>(seed_cfg.seedFilterConfig);
    seed_cfg.seedFinderConfig =
        seed_cfg.seedFinderConfig.toInternalUnits().calculateDerivedQuantities();
    seed_cfg.seedFinderOptions =
        seed_cfg.seedFinderOptions.toInternalUnits().calculateDerivedQuantities(seed_cfg.seedFinderConfig);
    seed_cfg.gridConfig = seed_cfg.gridConfig.toInternalUnits();
    seed_cfg.gridOptions = seed_cfg.gridOptions.toInternalUnits();

    if (std::isnan(seed_cfg.seedFinderConfig.deltaRMaxTopSP)) {
        seed_cfg.seedFinderConfig.deltaRMaxTopSP = seed_cfg.seedFinderConfig.deltaRMax;}
    if (std::isnan(seed_cfg.seedFinderConfig.deltaRMinTopSP)) {
        seed_cfg.seedFinderConfig.deltaRMinTopSP = seed_cfg.seedFinderConfig.deltaRMin;}
    if (std::isnan(seed_cfg.seedFinderConfig.deltaRMaxBottomSP)) {
        seed_cfg.seedFinderConfig.deltaRMaxBottomSP = seed_cfg.seedFinderConfig.deltaRMax;}
    if (std::isnan(seed_cfg.seedFinderConfig.deltaRMinBottomSP)) {
        seed_cfg.seedFinderConfig.deltaRMinBottomSP = seed_cfg.seedFinderConfig.deltaRMin;}
        
    m_bottomBinFinder = std::make_unique<const Acts::GridBinFinder<2ul>>(
        seed_cfg.numPhiNeighbors, seed_cfg.zBinNeighborsBottom);
    m_topBinFinder = std::make_unique<const Acts::GridBinFinder<2ul>>(
        seed_cfg.numPhiNeighbors, seed_cfg.zBinNeighborsTop);

    seed_cfg.seedFinderConfig.seedFilter =
        std::make_unique<Acts::SeedFilter<SimSpacePoint>>(seed_cfg.seedFilterConfig);
    m_seedFinder =
        Acts::SeedFinder<SimSpacePoint, Acts::CylindricalSpacePointGrid<SimSpacePoint>>(seed_cfg.seedFinderConfig);

    // initialize the ckf
    findTracks = makeTrackFinderFunction(trackingGeometry, magneticField);

    info() << "CKF Track Finder initialized successfully!" << endmsg;
    chronoStatSvc->chronoStop("read geometry");

    return GaudiAlgorithm::initialize();
}

StatusCode RecActsTracking::execute()
{
    auto trkCol = _outColHdl.createAndPut();

    SpacePointPtrs.clear();
    sourceLinks.clear();
    measurements.clear();
    initialParameters.clear();
    Selected_Seeds.clear();

    chronoStatSvc->chronoStart("read input hits");

    int successVTX = InitialiseVTX();
    if (successVTX == 0)
    {
        _nEvt++;
        return StatusCode::SUCCESS;
    }

    int successSIT = InitialiseSIT();
    if (successSIT == 0)
    {
        _nEvt++;
        return StatusCode::SUCCESS;
    }

    int successFTD = InitialiseFTD();
    if(successFTD == 0){
        _nEvt++;
        return StatusCode::SUCCESS;
    }

    chronoStatSvc->chronoStop("read input hits");
    // info() << "Generated " << SpacePointPtrs.size() << " spacepoints for event " << _nEvt << "!" << endmsg;
    // info() << "Generated " << measurements.size() << " measurements for event " << _nEvt << "!" << endmsg;

    // --------------------------------------------
    //                 Seeding
    // --------------------------------------------
    chronoStatSvc->chronoStart("seeding");

    // construct the seeding tools
    // covariance tool, extracts covariances per spacepoint as required
    auto extractGlobalQuantities = [=](const SimSpacePoint& sp, float, float, float)
    {
        Acts::Vector3 position{sp.x(), sp.y(), sp.z()};
        Acts::Vector2 covariance{sp.varianceR(), sp.varianceZ()};
        return std::make_tuple(position, covariance, sp.t());
    };

    // extent used to store r range for middle spacepoint
    Acts::Extent rRangeSPExtent;

    // construct the seeding tool
    Acts::CylindricalSpacePointGrid<SimSpacePoint> grid =
        Acts::CylindricalSpacePointGridCreator::createGrid<SimSpacePoint>(seed_cfg.gridConfig, seed_cfg.gridOptions);
    Acts::CylindricalSpacePointGridCreator::fillGrid(
        seed_cfg.seedFinderConfig, seed_cfg.seedFinderOptions, grid,
        SpacePointPtrs.begin(), SpacePointPtrs.end(), extractGlobalQuantities, rRangeSPExtent);

    std::array<std::vector<std::size_t>, 2ul> navigation;
    navigation[1ul] = seed_cfg.seedFinderConfig.zBinsCustomLooping;

    auto spacePointsGrouping = Acts::CylindricalBinnedGroup<SimSpacePoint>(
        std::move(grid), *m_bottomBinFinder, *m_topBinFinder, std::move(navigation));

    // safely clamp double to float
    float up = Acts::clampValue<float>(
        std::floor(rRangeSPExtent.max(Acts::binR) / 2) * 2);

    /// variable middle SP radial region of interest
    const Acts::Range1D<float> rMiddleSPRange(
        std::floor(rRangeSPExtent.min(Acts::binR) / 2) * 2 +
        seed_cfg.seedFinderConfig.deltaRMiddleMinSPRange,
        up - seed_cfg.seedFinderConfig.deltaRMiddleMaxSPRange);

    // run the seeding
    static thread_local SimSeedContainer seeds;
    seeds.clear();
    static thread_local decltype(m_seedFinder)::SeedingState state;
    state.spacePointData.resize(
        SpacePointPtrs.size(),
        seed_cfg.seedFinderConfig.useDetailedDoubleMeasurementInfo);

    // use double stripe measurement
    if (seed_cfg.seedFinderConfig.useDetailedDoubleMeasurementInfo)
    {
        for (std::size_t grid_glob_bin(0); grid_glob_bin < spacePointsGrouping.grid().size(); ++grid_glob_bin)
        {
            const auto& collection = spacePointsGrouping.grid().at(grid_glob_bin);
            for (const auto& sp : collection)
            {
                std::size_t index = sp->index();
                const float topHalfStripLength =
                    seed_cfg.seedFinderConfig.getTopHalfStripLength(sp->sp());
                const float bottomHalfStripLength =
                    seed_cfg.seedFinderConfig.getBottomHalfStripLength(sp->sp());
                const Acts::Vector3 topStripDirection =
                    seed_cfg.seedFinderConfig.getTopStripDirection(sp->sp());
                const Acts::Vector3 bottomStripDirection =
                    seed_cfg.seedFinderConfig.getBottomStripDirection(sp->sp());

                state.spacePointData.setTopStripVector(
                    index, topHalfStripLength * topStripDirection);
                state.spacePointData.setBottomStripVector(
                    index, bottomHalfStripLength * bottomStripDirection);
                state.spacePointData.setStripCenterDistance(
                    index, seed_cfg.seedFinderConfig.getStripCenterDistance(sp->sp()));
                state.spacePointData.setTopStripCenterPosition(
                    index, seed_cfg.seedFinderConfig.getTopStripCenterPosition(sp->sp()));
            }
        }
    }

    for (const auto [bottom, middle, top] : spacePointsGrouping)
    {
        m_seedFinder.createSeedsForGroup(
            seed_cfg.seedFinderOptions, state, spacePointsGrouping.grid(),
            std::back_inserter(seeds), bottom, middle, top, rMiddleSPRange);
    }

    // int seed_counter = 0;
    // for (const auto& seed : seeds)
    // {
    //     for (const auto& sp : seed.sp())
    //     {
    //         info() << "found seed #" << seed_counter << ": x:" << sp->x() << " y: " << sp->y() << " z: " << sp->z() << endmsg;
    //     }
    //     seed_counter++;
    // }

    chronoStatSvc->chronoStop("seeding");
    debug() << "Found " << seeds.size() << " seeds for event " << _nEvt << "!" << endmsg;

    // --------------------------------------------
    //            track estimation
    // --------------------------------------------
    chronoStatSvc->chronoStart("track_param");

    IndexSourceLink::SurfaceAccessor surfaceAccessor{*trackingGeometry};

    for (std::size_t iseed = 0; iseed < seeds.size(); ++iseed)
    {
        const auto& seed = seeds[iseed];
        // Get the bottom space point and its reference surface
        const auto bottomSP = seed.sp().front();
        const auto& sourceLink = bottomSP->sourceLinks()[0];
        const Acts::Surface* surface = surfaceAccessor(sourceLink);
        if (surface == nullptr) {
            debug() << "Surface from source link is not found in the tracking geometry: iseed " << iseed << endmsg;
            continue;
        }

        auto optParams = Acts::estimateTrackParamsFromSeed(
            geoContext, seed.sp().begin(), seed.sp().end(), *surface, acts_field_value, bFieldMin);
        
        if (!optParams.has_value()) {
            debug() << "Estimation of track parameters for seed " << iseed << " failed." << endmsg;
            continue;
        }

        const auto& params = optParams.value();
        Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Zero();
        for (std::size_t i = Acts::eBoundLoc0; i < Acts::eBoundSize; ++i) {
            double sigma = initialSigmas[i];
            sigma += abs(initialSimgaQoverPCoefficients[i] * params[Acts::eBoundQOverP]);
            double var = sigma * sigma;
            if (i == Acts::eBoundTime && !bottomSP->t().has_value()) { var *= noTimeVarInflation; }
            var *= initialVarInflation.value()[i];
            
            cov(i, i) = var;
        }
        initialParameters.emplace_back(surface->getSharedPtr(), params, cov, particleHypothesis);
        Selected_Seeds.push_back(seed);
    }

    chronoStatSvc->chronoStop("track_param");
    debug() << "Found " << initialParameters.size() << " tracks for event " << _nEvt << "!" << endmsg;

    // --------------------------------------------
    //            CKF track finding
    // --------------------------------------------
    chronoStatSvc->chronoStart("ckf_findTracks");

    // Construct a perigee surface as the target surface
    auto pSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3{0., 0., 0.});
    PassThroughCalibrator pcalibrator;
    MeasurementCalibratorAdapter calibrator(pcalibrator, measurements);
    Acts::GainMatrixUpdater kfUpdater;
    Acts::MeasurementSelector::Config measurementSelectorCfg =
    {
        {Acts::GeometryIdentifier(), {{}, {CKFchi2Cut.value()}, {numMeasurementsCutOff.value()}}},
    };

    MeasurementSelector measSel{ Acts::MeasurementSelector(measurementSelectorCfg) };
    using Extensions = Acts::CombinatorialKalmanFilterExtensions<Acts::VectorMultiTrajectory>;
    BranchStopper branchStopper(trackSelectorCfg);

    // Construct the CKF
    Extensions extensions;
    extensions.calibrator.connect<&MeasurementCalibratorAdapter::calibrate>(&calibrator);
    extensions.updater.connect<&Acts::GainMatrixUpdater::operator()<Acts::VectorMultiTrajectory>>(&kfUpdater);
    extensions.measurementSelector.connect<&MeasurementSelector::select>(&measSel);
    extensions.branchStopper.connect<&BranchStopper::operator()>(&branchStopper);

    IndexSourceLinkAccessor slAccessor;
    slAccessor.container = &sourceLinks;
    Acts::SourceLinkAccessorDelegate<IndexSourceLinkAccessor::Iterator> slAccessorDelegate;
    slAccessorDelegate.connect<&IndexSourceLinkAccessor::range>(&slAccessor);

    Acts::PropagatorPlainOptions firstPropOptions;
    firstPropOptions.maxSteps = maxSteps;
    firstPropOptions.direction = Acts::Direction::Forward;

    Acts::PropagatorPlainOptions secondPropOptions;
    secondPropOptions.maxSteps = maxSteps;
    secondPropOptions.direction = firstPropOptions.direction.invert();

    // Set the CombinatorialKalmanFilter options
    TrackFinderOptions firstOptions(
        geoContext, magFieldContext, calibContext,
        slAccessorDelegate, extensions, firstPropOptions);
    
    TrackFinderOptions secondOptions(
        geoContext, magFieldContext, calibContext,
        slAccessorDelegate, extensions, secondPropOptions);
    secondOptions.targetSurface = pSurface.get();

    Acts::Propagator<Acts::EigenStepper<>, Acts::Navigator> extrapolator(
        Acts::EigenStepper<>(magneticField), Acts::Navigator({trackingGeometry}));

    Acts::PropagatorOptions<Acts::ActionList<Acts::MaterialInteractor>, Acts::AbortList<Acts::EndOfWorldReached>>
        extrapolationOptions(geoContext, magFieldContext);

    auto trackContainer = std::make_shared<Acts::VectorTrackContainer>();
    auto trackStateContainer = std::make_shared<Acts::VectorMultiTrajectory>();
    auto trackContainerTemp = std::make_shared<Acts::VectorTrackContainer>();
    auto trackStateContainerTemp = std::make_shared<Acts::VectorMultiTrajectory>();
    TrackContainer tracks(trackContainer, trackStateContainer);
    TrackContainer tracksTemp(trackContainerTemp, trackStateContainerTemp);

    tracks.addColumn<unsigned int>("trackGroup");
    tracksTemp.addColumn<unsigned int>("trackGroup");
    Acts::ProxyAccessor<unsigned int> seedNumber("trackGroup");

    unsigned int nSeed = 0;
    // A map indicating whether a seed has been discovered already
    std::unordered_map<SeedIdentifier, bool> discoveredSeeds;
    auto addTrack = [&](const TrackProxy& track)
    {
        ++m_nFoundTracks;
        // flag seeds which are covered by the track
        visitSeedIdentifiers(track, [&](const SeedIdentifier& seedIdentifier)
        {
            if (auto it = discoveredSeeds.find(seedIdentifier); it != discoveredSeeds.end())
            {
                it->second = true;
            }
        });

        if (m_trackSelector.has_value() && !m_trackSelector->isValidTrack(track)) { return; }

        ++m_nSelectedTracks;
        auto destProxy = tracks.makeTrack();
        // make sure we copy track states!
        destProxy.copyFrom(track, true);
    };

    for (const auto& seed : Selected_Seeds) {
        SeedIdentifier seedIdentifier = makeSeedIdentifier(seed);
        discoveredSeeds.emplace(seedIdentifier, false);
    }

    for (std::size_t iSeed = 0; iSeed < initialParameters.size(); ++iSeed)
    {           
        m_nTotalSeeds++;
        const auto& seed = Selected_Seeds[iSeed];

        SeedIdentifier seedIdentifier = makeSeedIdentifier(seed);
        // check if the seed has been discovered already
        if (auto it = discoveredSeeds.find(seedIdentifier); it != discoveredSeeds.end() && it->second)
        {
            m_nDeduplicatedSeeds++;
            continue;
        }

        /// Whether to stick on the seed measurements during track finding.
        // measSel.setSeed(seed);

        // Clear trackContainerTemp and trackStateContainerTemp
        tracksTemp.clear();

        const Acts::BoundTrackParameters& firstInitialParameters = initialParameters.at(iSeed);
        auto firstResult = (*findTracks)(firstInitialParameters, firstOptions, tracksTemp);

        nSeed++;
        if (!firstResult.ok())
        {
            m_nFailedSeeds++;
            continue;
        }

        auto& firstTracksForSeed = firstResult.value();
        for (auto& firstTrack : firstTracksForSeed)
        {
            auto trackCandidate = tracksTemp.makeTrack();
            trackCandidate.copyFrom(firstTrack, true);

            auto firstSmoothingResult = Acts::smoothTrack(geoContext, trackCandidate);

            if (!firstSmoothingResult.ok())
            {
                m_nFailedSmoothing++;
                debug() << "First smoothing for seed " 
                       << iSeed << " and track " << firstTrack.index()
                       << " failed with error " << firstSmoothingResult.error() << endmsg;
                continue;
            }

            seedNumber(trackCandidate) = nSeed - 1;
      
            // second way track finding
            std::size_t nSecond = 0;
            if (CKFtwoWay)
            {
                std::optional<Acts::VectorMultiTrajectory::TrackStateProxy> firstMeasurement;
                for (auto trackState : trackCandidate.trackStatesReversed())
                {
                    bool isMeasurement = trackState.typeFlags().test(Acts::TrackStateFlag::MeasurementFlag);
                    bool isOutlier = trackState.typeFlags().test(Acts::TrackStateFlag::OutlierFlag);
                    if (isMeasurement && !isOutlier) { firstMeasurement = trackState; }
                }
                
                if (firstMeasurement.has_value())
                {
                    Acts::BoundTrackParameters secondInitialParameters = trackCandidate.createParametersFromState(*firstMeasurement);
                    auto secondResult = (*findTracks)(secondInitialParameters, secondOptions, tracksTemp);
                    if (!secondResult.ok())
                    {
                        debug() << "Second track finding failed for seed "
                                << iSeed << " with error" << secondResult.error() << endmsg;
                    } else {
                        auto firstState = *std::next(trackCandidate.trackStatesReversed().begin(), trackCandidate.nTrackStates() - 1);
                        auto& secondTracksForSeed = secondResult.value();
                        for (auto& secondTrack : secondTracksForSeed)
                        { 
                            if (secondTrack.nTrackStates() < 2) { continue; }
                            auto secondTrackCopy = tracksTemp.makeTrack();
                            secondTrackCopy.copyFrom(secondTrack, true);
                            secondTrackCopy.reverseTrackStates(true);

                            firstState.previous() = (*std::next(secondTrackCopy.trackStatesReversed().begin())).index();
                            Acts::calculateTrackQuantities(trackCandidate);
                            auto secondExtrapolationResult = Acts::extrapolateTrackToReferenceSurface(
                                trackCandidate, *pSurface, extrapolator,
                                extrapolationOptions, extrapolationStrategy);
                            if (!secondExtrapolationResult.ok())
                            {
                                m_nFailedExtrapolation++;
                                debug() << "Second extrapolation for seed " 
                                        << iSeed << " and track " << secondTrack.index()
                                        << " failed with error "
                                        << secondExtrapolationResult.error() << endmsg;
                                continue;
                            }

                            addTrack(trackCandidate);

                            ++nSecond;
                        }

                        firstState.previous() = Acts::kTrackIndexInvalid;
                        Acts::calculateTrackQuantities(trackCandidate);
                    }
                }
            }

            // if no second track was found, we will use only the first track
            if (nSecond == 0) {
                auto firstExtrapolationResult = Acts::extrapolateTrackToReferenceSurface(
                    trackCandidate, *pSurface, extrapolator, extrapolationOptions, extrapolationStrategy);

                if (!firstExtrapolationResult.ok())
                {
                    m_nFailedExtrapolation++;
                    continue;
                }

                addTrack(trackCandidate);
            }
        }
    }

    chronoStatSvc->chronoStop("ckf_findTracks");
    debug() << "CKF found " << tracks.size() << " tracks for event " << _nEvt << "!" << endmsg;

    m_memoryStatistics.local().hist += tracks.trackStateContainer().statistics().hist;
    auto constTrackStateContainer = std::make_shared<Acts::ConstVectorMultiTrajectory>(std::move(*trackStateContainer));
    auto constTrackContainer = std::make_shared<Acts::ConstVectorTrackContainer>(std::move(*trackContainer));
    ConstTrackContainer constTracks{constTrackContainer, constTrackStateContainer};

    chronoStatSvc->chronoStart("writeout tracks");

    if (constTracks.size() == 0) {
        chronoStatSvc->chronoStop("writeout tracks");
        _nEvt++;
        return StatusCode::SUCCESS;
    }
    for (const auto& cur_track : constTracks)
    {
        auto writeout_track = trkCol->create();
        int nVTXHit = 0, nFTDHit = 0, nSITHit = 0, nlayerHits = 9;
        int getFirstHit = 0;
        
        writeout_track.setChi2(cur_track.chi2());
        writeout_track.setNdf(cur_track.nDoF());
        writeout_track.setDEdx(cur_track.absoluteMomentum() / Acts::UnitConstants::GeV);
        // writeout_track.setDEdxError(cur_track.qOverP());
        std::array<int, 6> nlayer_VTX{0, 0, 0, 0, 0, 0};
        std::array<int, 3> nlayer_SIT{0, 0, 0};
        std::array<int, 3> nlayer_FTD{0, 0, 0};

        for (auto trackState : cur_track.trackStates())
        {
            if (trackState.hasUncalibratedSourceLink())
            {
                auto cur_measurement_sl = trackState.getUncalibratedSourceLink();
                const auto& MeasSourceLink = cur_measurement_sl.get<IndexSourceLink>();
                auto cur_measurement = measurements[MeasSourceLink.index()];
                auto cur_measurement_gid = MeasSourceLink.geometryId();
                
                for (int i = 0; i < 5; i++)
                {
                    if (cur_measurement_gid.volume() == VXD_volume_ids[i])
                    {
                        if (i == 4){
                            if (cur_measurement_gid.sensitive() & 1 == 1)
                            {
                                nlayer_VTX[5]++;
                            } else{
                                nlayer_VTX[4]++;
                            }
                        } else {
                            nlayer_VTX[i]++;
                        }
                        nVTXHit++;
                        break;
                    }
                }

                for (int i = 0; i < 3; i++){
                    if (cur_measurement_gid.volume() == SIT_volume_ids[i]){
                        nlayer_SIT[i]++;
                        nSITHit++;
                        break;
                    }
                }

                for (int i = 0; i < 3; i++){
                    if (cur_measurement_gid.volume() == FTD_positive_volume_ids[i]){
                        nlayer_FTD[i]++;
                        nFTDHit++;
                        break;
                    }

                    if (cur_measurement_gid.volume() == FTD_negative_volume_ids[i]){
                        nlayer_FTD[i]++;
                        nFTDHit++;
                        break;
                    }
                }

                writeout_track.addToTrackerHits(MeasSourceLink.getTrackerHit());

                if (!getFirstHit){
                    const auto& par = std::get<1>(cur_measurement).parameters();
                    const Acts::Surface* surface = surfaceAccessor(cur_measurement_sl);
                    auto acts_global_postion = surface->localToGlobal(geoContext, par, globalFakeMom);
                    writeout_track.setRadiusOfInnermostHit(
                    std::sqrt(acts_global_postion[0] * acts_global_postion[0] + 
                              acts_global_postion[1] * acts_global_postion[1] + 
                              acts_global_postion[2] * acts_global_postion[2])
                    );
                    getFirstHit = 1;
                }
            }
        }
        for (int i = 0; i < 6; i++){
            m_nRec_VTX[i] += nlayer_VTX[i];
            if (nlayer_VTX[i] == 0) {
                m_n0EventHits[i]++;
                nlayerHits--;
            } else if (nlayer_VTX[i] == 1) {
                m_n1EventHits[i]++;
            } else if (nlayer_VTX[i] == 2) {
                m_n2EventHits[i]++;
            } else if (nlayer_VTX[i] >= 3) {
                m_n3EventHits[i]++;
            }
        }
        for (int i = 0; i < 3; i++){
            m_nRec_SIT[i] += nlayer_SIT[i];
            if (nlayer_SIT[i] == 0) {
                m_n0EventHits[i+6]++;
                nlayerHits--;
            } else if (nlayer_SIT[i] == 1) {
                m_n1EventHits[i+6]++;
            } else if (nlayer_SIT[i] == 2) {
                m_n2EventHits[i+6]++;
            } else if (nlayer_SIT[i] >= 3) {
                m_n3EventHits[i+6]++;
            }
        }

        for (int i = 0; i < 3; i++){
            m_nRec_FTD[i] += nlayer_FTD[i];
        }

        // SubdetectorHitNumbers: VXD = 0, FTD = 1, SIT = 2
        writeout_track.setDEdxError(nlayerHits);
        writeout_track.addToSubdetectorHitNumbers(nVTXHit);
        writeout_track.addToSubdetectorHitNumbers(nFTDHit);
        writeout_track.addToSubdetectorHitNumbers(nSITHit);
        
        // TODO: covmatrix need to be converted
        std::array<float, 21> writeout_covMatrix;
        auto cur_track_covariance = cur_track.covariance();
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6-i; j++) {
                writeout_covMatrix[int((13-i)*i/2 + j)] = cur_track_covariance(writeout_indices[i], writeout_indices[j]);
            }
        }
        // location: At{Other|IP|FirstHit|LastHit|Calorimeter|Vertex}|LastLocation
        // TrackState: location, d0, phi, omega, z0, tanLambda, time, referencePoint, covMatrix
        edm4hep::TrackState writeout_trackState{
            1, // location: AtOther
            cur_track.loc0() / Acts::UnitConstants::mm, // d0
            cur_track.phi(), // phi
            cur_track.qOverP() * sin(cur_track.theta()) * _FCT * m_field, // omega = qop * sin(theta) * _FCT * bf
            cur_track.loc1() / Acts::UnitConstants::mm, // z0
            1 / tan(cur_track.theta()), // tanLambda = 1 / tan(theta)
            cur_track.time(), // time
            ::edm4hep::Vector3f(0, 0, 0), // referencePoint
            writeout_covMatrix
        };

        debug() << "Found track with momentum " << cur_track.absoluteMomentum() / Acts::UnitConstants::GeV << " !" << endmsg;
        writeout_track.addToTrackStates(writeout_trackState);
    }

    chronoStatSvc->chronoStop("writeout tracks");

    _nEvt++;
    return StatusCode::SUCCESS;
}

StatusCode RecActsTracking::finalize()
{
    debug() << "finalize RecActsTracking" << endmsg;
    info() << "Total number of events processed: " << _nEvt << endmsg;
    info() << "Total number of **TotalSeeds** processed: " << m_nTotalSeeds << endmsg;
    info() << "Total number of **FoundTracks** processed: " << m_nFoundTracks << endmsg;
    info() << "Total number of **SelectedTracks** processed: " << m_nSelectedTracks << endmsg;
    info() << "Total number of **LayerHits** processed: " << m_nLayerHits << endmsg;
    info() << "Total number of **Rec_VTX** processed: " << m_nRec_VTX << endmsg;
    info() << "Total number of **Rec_SIT** processed: " << m_nRec_SIT << endmsg;
    info() << "Total number of **Rec_FTD** processed: " << m_nRec_FTD << endmsg;
    info() << "Total number of **EventHits0** processed: " << m_n0EventHits << endmsg;
    info() << "Total number of **EventHits1** processed: " << m_n1EventHits << endmsg;
    info() << "Total number of **EventHits2** processed: " << m_n2EventHits << endmsg;
    info() << "Total number of **EventHitsmore** processed: " << m_n3EventHits << endmsg;

    return GaudiAlgorithm::finalize();
}

int RecActsTracking::InitialiseVTX()
{   
    int success = 1;

    const edm4hep::TrackerHitCollection* hitVTXCol = nullptr;
    const edm4hep::SimTrackerHitCollection* SimhitVTXCol = nullptr;

    try {
        hitVTXCol = _inVTXTrackHdl.get();
    } catch (GaudiException& e) {
        debug() << "Collection " << _inVTXTrackHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
        success = 0;
    }

    try {
        SimhitVTXCol = _inVTXColHdl.get();
    } catch (GaudiException& e) {
        debug() << "Sim Collection " << _inVTXColHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
        success = 0;
    }

    if(hitVTXCol && SimhitVTXCol)
    {
        int nelem = hitVTXCol->size();
        debug() << "Number of VTX hits = " << nelem << endmsg;
        if ((nelem < 3) or (nelem > 10)) { success = 0; }

        // std::string truth_file = "obj/vtx/truth/event" + std::to_string(_nEvt) + ".csv";
        // std::ofstream truth_stream(truth_file);
        // csv2::Writer<csv2::delimiter<','>> truth_writer(truth_stream);
        // std::vector<std::string> truth_header = {"layer", "x", "y", "z"};
        // truth_writer.write_row(truth_header);

        // std::string converted_file = "obj/vtx/converted/event" + std::to_string(_nEvt) + ".csv";
        // std::ofstream converted_stream(converted_file);
        // csv2::Writer<csv2::delimiter<','>> converted_writer(converted_stream);
        // std::vector<std::string> converted_header = {"layer", "x", "y", "z"};
        // converted_writer.write_row(converted_header);

        for (int ielem = 0; ielem < nelem; ++ielem)
        {
            auto hit = hitVTXCol->at(ielem);
            auto simhit = SimhitVTXCol->at(ielem);

            auto simcellid = simhit.getCellID();
            // system:5,side:-2,layer:9,module:8,sensor:32:8
            uint64_t m_layer   = vxd_decoder->get(simcellid, "layer");
            uint64_t m_module  = vxd_decoder->get(simcellid, "module");
            uint64_t m_sensor  = vxd_decoder->get(simcellid, "sensor");
            double acts_x = simhit.getPosition()[0];
            double acts_y = simhit.getPosition()[1];
            double acts_z = simhit.getPosition()[2];

            double momentum_x = simhit.getMomentum()[0];
            double momentum_y = simhit.getMomentum()[1];
            double momentum_z = simhit.getMomentum()[2];

            const Acts::Vector3 globalmom{momentum_x, momentum_y, momentum_z};
            std::array<float, 6> m_covMatrix = hit.getCovMatrix();

            dd4hep::rec::ISurface* surface = nullptr;
            auto it = m_vtx_surfaces->find(simcellid);
            if (it != m_vtx_surfaces->end()) {
                surface = it->second;
                if (!surface) {
                    fatal() << "found surface for VTX cell id " << simcellid << ", but NULL" << endmsg;
                    return 0;
                }
            }
            else {
                fatal() << "not found surface for VTX cell id " << simcellid << endmsg;
                return 0;
            }

            // dd4hep::rec::Vector3D oldPos(simhit.getPosition()[0]*dd4hep::mm/CLHEP::mm, simhit.getPosition()[1]*dd4hep::mm/CLHEP::mm, simhit.getPosition()[2]*dd4hep::mm/CLHEP::mm);
            // dd4hep::rec::Vector2D localPoint = surface->globalToLocal(oldPos);

            // if (m_layer < current_layer){
            //     info() << "ring hits happend in layer " << m_layer << " before layer " << current_layer << ", at event " << _nEvt << endmsg;
            //     success = 0;
            //     break;
            // }
            // current_layer = m_layer;

            if (m_layer <= 3){
                // set acts geometry identifier
                uint64_t acts_volume = VXD_volume_ids[m_layer];
                uint64_t acts_boundary = 0;
                uint64_t acts_layer = 2;
                uint64_t acts_approach = 0;
                // uint64_t acts_sensitive = m_module + 1;
                uint64_t acts_sensitive = 1;

                Acts::GeometryIdentifier moduleGeoId;
                moduleGeoId.setVolume(acts_volume);
                moduleGeoId.setBoundary(acts_boundary);
                moduleGeoId.setLayer(acts_layer);
                moduleGeoId.setApproach(acts_approach);
                moduleGeoId.setSensitive(acts_sensitive);
        
                // create and store the source link
                uint32_t measurementIdx = measurements.size();
                IndexSourceLink sourceLink{moduleGeoId, measurementIdx, hit};
                sourceLinks.insert(sourceLinks.end(), sourceLink);
                Acts::SourceLink sl{sourceLink};
                boost::container::static_vector<Acts::SourceLink, 2> slinks;
                slinks.emplace_back(sl);

                // get the surface of the hit
                IndexSourceLink::SurfaceAccessor surfaceAccessor{*trackingGeometry};
                const Acts::Surface* acts_surface = surfaceAccessor(sl);

                // get the local position of the hit
                const Acts::Vector3 globalPos{acts_x, acts_y, acts_z};
                auto acts_local_postion = acts_surface->globalToLocal(geoContext, globalPos, globalmom, onSurfaceTolerance);
                if (!acts_local_postion.ok()){
                    info() << "Error: failed to get local position for VTX hit " << simcellid << endmsg;
                    acts_local_postion = acts_surface->globalToLocal(geoContext, globalPos, globalmom, 100*onSurfaceTolerance);
                }
                const std::array<Acts::BoundIndices, 2> indices{Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundLoc1};
                const Acts::Vector2 par{acts_local_postion.value()[0], acts_local_postion.value()[1]};

                // *** debug ***
                debug() << "VXD measurements global position(x,y,z): " << simhit.getPosition()[0] << ", " << simhit.getPosition()[1] << ", " << simhit.getPosition()[2]
                       << "; local position(loc0, loc1): "<< acts_local_postion.value()[0] << ", " << acts_local_postion.value()[1] << endmsg;
                auto acts_global_postion = acts_surface->localToGlobal(geoContext, par, globalFakeMom);
                debug() << "debug surface at: x:" << acts_global_postion[0] << ", y:" << acts_global_postion[1] << ", z:" << acts_global_postion[2] << endmsg;

                // SimSpacePoint *hitExt = new SimSpacePoint(hit, simhit, slinks);
                SimSpacePoint *hitExt = new SimSpacePoint(hit, acts_global_postion[0], acts_global_postion[1], acts_global_postion[2], 0.002, slinks);
                // debug() << "debug hitExt at: x:" << hitExt->x() << ", y:" << hitExt->y() << ", z:" << hitExt->z() << endmsg;
                SpacePointPtrs.push_back(hitExt);

                // create and store the measurement
                // Cylinder covMatrix[6] = {resU*resU/2, 0, resU*resU/2, 0, 0, resV*resV}
                Acts::ActsSquareMatrix<2> cov = Acts::ActsSquareMatrix<2>::Identity();
                cov(0, 0) = std::max<double>(double(std::sqrt(m_covMatrix[2]*2)), eps.value());
                cov(1, 1) = std::max<double>(double(std::sqrt(m_covMatrix[5])), eps.value());
                measurements.emplace_back(Acts::Measurement<Acts::BoundIndices, 2>(sl, indices, par, cov));

                // std::vector<std::string> truth_col = {std::to_string(m_layer*2 + m_module), std::to_string(simhit.getPosition()[0]), std::to_string(simhit.getPosition()[1]), std::to_string(simhit.getPosition()[2])};
                // truth_writer.write_row(truth_col);
                // std::vector<std::string> converted_col = {std::to_string(m_layer*2 + m_module), std::to_string(acts_global_postion[0]), std::to_string(acts_global_postion[1]), std::to_string(acts_global_postion[2])};
                // converted_writer.write_row(converted_col);

            } else {
                // set acts geometry identifier
                uint64_t acts_volume = VXD_volume_ids[4];
                uint64_t acts_boundary = 0;
                uint64_t acts_layer = 2;
                uint64_t acts_approach = 0;
                uint64_t acts_sensitive = (m_layer == 5) ? m_module*2 + 1 : m_module*2 + 2;

                Acts::GeometryIdentifier moduleGeoId;
                moduleGeoId.setVolume(acts_volume);
                moduleGeoId.setBoundary(acts_boundary);
                moduleGeoId.setLayer(acts_layer);
                moduleGeoId.setApproach(acts_approach);
                moduleGeoId.setSensitive(acts_sensitive);
        
                // create and store the source link
                uint32_t measurementIdx = measurements.size();
                IndexSourceLink sourceLink{moduleGeoId, measurementIdx, hit};
                sourceLinks.insert(sourceLinks.end(), sourceLink);
                Acts::SourceLink sl{sourceLink};
                boost::container::static_vector<Acts::SourceLink, 2> slinks;
                slinks.emplace_back(sl);

                // get the local position of the hit
                IndexSourceLink::SurfaceAccessor surfaceAccessor{*trackingGeometry};
                const Acts::Surface* acts_surface = surfaceAccessor(sl);
                const Acts::Vector3 globalPos{acts_x, acts_y, acts_z};
                auto acts_local_postion = acts_surface->globalToLocal(geoContext, globalPos, globalmom, onSurfaceTolerance);
                if (!acts_local_postion.ok()){
                    info() << "Error: failed to get local position for VTX hit " << simcellid << endmsg;
                    acts_local_postion = acts_surface->globalToLocal(geoContext, globalPos, globalmom, 100*onSurfaceTolerance);
                }
                const std::array<Acts::BoundIndices, 2> indices{Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundLoc1};
                const Acts::Vector2 par{acts_local_postion.value()[0], acts_local_postion.value()[1]};

                // *** debug ***
                debug() << "VXD measurements global position(x,y,z): " << simhit.getPosition()[0] << ", " << simhit.getPosition()[1] << ", " << simhit.getPosition()[2]
                       << "; local position(loc0, loc1): "<< acts_local_postion.value()[0] << ", " << acts_local_postion.value()[1] << endmsg;
                auto acts_global_postion = acts_surface->localToGlobal(geoContext, par, globalFakeMom);
                debug() << "debug surface at: x:" << acts_global_postion[0] << ", y:" << acts_global_postion[1] << ", z:" << acts_global_postion[2] << endmsg;

                if (ExtendSeedRange.value()) {
                    SimSpacePoint *hitExt = new SimSpacePoint(hit, acts_global_postion[0], acts_global_postion[1], acts_global_postion[2], 0.002, slinks);
                    SpacePointPtrs.push_back(hitExt);
                }
                
                // create and store the measurement
                // Plane covMatrix[6] = {u_direction[0], u_direction[1], resU, v_direction[0], v_direction[1], resV}
                Acts::ActsSquareMatrix<2> cov = Acts::ActsSquareMatrix<2>::Identity();
                cov(0, 0) = std::max<double>(double(m_covMatrix[2]), eps.value());
                cov(1, 1) = std::max<double>(double(m_covMatrix[5]), eps.value());
                measurements.emplace_back(Acts::Measurement<Acts::BoundIndices, 2>(sl, indices, par, cov));

                // std::vector<std::string> truth_col = {std::to_string(m_layer+4), std::to_string(simhit.getPosition()[0]), std::to_string(simhit.getPosition()[1]), std::to_string(simhit.getPosition()[2])};
                // truth_writer.write_row(truth_col);
                // std::vector<std::string> converted_col = {std::to_string(m_layer+4), std::to_string(acts_global_postion[0]), std::to_string(acts_global_postion[1]), std::to_string(acts_global_postion[2])};
                // converted_writer.write_row(converted_col);
            }
        }
    } else { success = 0; }

    return success;
}

int RecActsTracking::InitialiseSIT()
{    
    int success = 1;

    const edm4hep::TrackerHitCollection* hitSITCol = nullptr;
    const edm4hep::SimTrackerHitCollection* SimhitSITCol = nullptr;

    double min_z = 0;
    try {
        hitSITCol = _inSITTrackHdl.get();
    } catch (GaudiException& e) {
        debug() << "Collection " << _inSITTrackHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
        success = 0;
    }

    try {
        SimhitSITCol = _inSITColHdl.get();
    } catch (GaudiException& e) {
        debug() << "Sim Collection " << _inSITColHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
        success = 0;
    }

    if(hitSITCol && SimhitSITCol)
    {
        int nelem = hitSITCol->size();
        debug() << "Number of SIT hits = " << nelem << endmsg;
        // SpacePointPtrs.resize(nelem);

        // std::string truth_file = "obj/sit/truth/event" + std::to_string(_nEvt) + ".csv";
        // std::ofstream truth_stream(truth_file);
        // csv2::Writer<csv2::delimiter<','>> truth_writer(truth_stream);
        // std::vector<std::string> truth_header = {"layer", "x", "y", "z"};
        // truth_writer.write_row(truth_header);

        // std::string converted_file = "obj/sit/converted/event" + std::to_string(_nEvt) + ".csv";
        // std::ofstream converted_stream(converted_file);
        // csv2::Writer<csv2::delimiter<','>> converted_writer(converted_stream);
        // std::vector<std::string> converted_header = {"layer", "x", "y", "z"};
        // converted_writer.write_row(converted_header);

        for (int ielem = 0; ielem < nelem; ++ielem)
        {
            auto hit = hitSITCol->at(ielem);
            auto simhit = SimhitSITCol->at(ielem);

            auto simcellid = simhit.getCellID();
            // <id>system:5,side:-2,layer:9,stave:8,module:8,sensor:5,y:-11,z:-11</id>
            uint64_t m_layer   = sit_decoder->get(simcellid, "layer");
            uint64_t m_stave   = sit_decoder->get(simcellid, "stave");
            uint64_t m_module  = sit_decoder->get(simcellid, "module");
            uint64_t m_sensor  = sit_decoder->get(simcellid, "sensor");
            double acts_x = simhit.getPosition()[0];
            double acts_y = simhit.getPosition()[1];
            double acts_z = simhit.getPosition()[2];
            double momentum_x = simhit.getMomentum()[0];
            double momentum_y = simhit.getMomentum()[1];
            double momentum_z = simhit.getMomentum()[2];
            const Acts::Vector3 globalmom{momentum_x, momentum_y, momentum_z};
            std::array<float, 6> m_covMatrix = hit.getCovMatrix();

            dd4hep::rec::ISurface* surface = nullptr;
            auto it = m_sit_surfaces->find(simcellid);
            if (it != m_sit_surfaces->end()) {
                surface = it->second;
                if (!surface) {
                    fatal() << "found surface for SIT cell id " << simcellid << ", but NULL" << endmsg;
                    return 0;
                }
            }
            else {
                fatal() << "not found surface for SIT cell id " << simcellid << endmsg;
                return 0;
            }

            // set acts geometry identifier
            uint64_t acts_volume = SIT_volume_ids[m_layer];
            uint64_t acts_boundary = 0;
            uint64_t acts_layer = 2;
            uint64_t acts_approach = 0;
            uint64_t acts_sensitive = m_stave*SIT_module_nums[m_layer] + m_module + 1;

            Acts::GeometryIdentifier moduleGeoId;
            moduleGeoId.setVolume(acts_volume);
            moduleGeoId.setBoundary(acts_boundary);
            moduleGeoId.setLayer(acts_layer);
            moduleGeoId.setApproach(acts_approach);
            moduleGeoId.setSensitive(acts_sensitive);
    
            // create and store the source link
            uint32_t measurementIdx = measurements.size();
            IndexSourceLink sourceLink{moduleGeoId, measurementIdx, hit};
            sourceLinks.insert(sourceLinks.end(), sourceLink);
            Acts::SourceLink sl{sourceLink};
            boost::container::static_vector<Acts::SourceLink, 2> slinks;
            slinks.emplace_back(sl);

            // get the local position of the hit
            IndexSourceLink::SurfaceAccessor surfaceAccessor{*trackingGeometry};
            const Acts::Surface* acts_surface = surfaceAccessor(sl);
            const Acts::Vector3 globalPos{acts_x, acts_y, acts_z};
            auto acts_local_postion = acts_surface->globalToLocal(geoContext, globalPos, globalmom, onSurfaceTolerance);
            if (!acts_local_postion.ok()){
                info() << "Error: failed to get local position for SIT hit " << simcellid << endmsg;
                acts_local_postion = acts_surface->globalToLocal(geoContext, globalPos, globalmom, 100*onSurfaceTolerance);
            }
            const std::array<Acts::BoundIndices, 2> indices{Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundLoc1};
            const Acts::Vector2 par{acts_local_postion.value()[0], acts_local_postion.value()[1]};

            // *** debug ***
            debug() << "SIT measurements global position(x,y,z): " << simhit.getPosition()[0] << ", " << simhit.getPosition()[1] << ", " << simhit.getPosition()[2]
                << "; local position(loc0, loc1): "<< acts_local_postion.value()[0] << ", " << acts_local_postion.value()[1] << endmsg;
            auto acts_global_postion = acts_surface->localToGlobal(geoContext, par, globalFakeMom);
            debug() << "debug surface at: x:" << acts_global_postion[0] << ", y:" << acts_global_postion[1] << ", z:" << acts_global_postion[2] << endmsg;

            if (ExtendSeedRange.value()) {
                SimSpacePoint *hitExt = new SimSpacePoint(hit, acts_global_postion[0], acts_global_postion[1], acts_global_postion[2], 0.002, slinks);
                SpacePointPtrs.push_back(hitExt);
            }

            // create and store the measurement
            Acts::ActsSquareMatrix<2> cov = Acts::ActsSquareMatrix<2>::Identity();
            cov(0, 0) = std::max<double>(double(m_covMatrix[2]), eps.value());
            cov(1, 1) = std::max<double>(double(m_covMatrix[5]), eps.value());
            measurements.emplace_back(Acts::Measurement<Acts::BoundIndices, 2>(sl, indices, par, cov));

            // std::vector<std::string> truth_col = {std::to_string(m_layer), std::to_string(simhit.getPosition()[0]), std::to_string(simhit.getPosition()[1]), std::to_string(simhit.getPosition()[2])};
            // truth_writer.write_row(truth_col);
            // std::vector<std::string> converted_col = {std::to_string(m_layer), std::to_string(acts_global_postion[0]), std::to_string(acts_global_postion[1]), std::to_string(acts_global_postion[2])};
            // converted_writer.write_row(converted_col);
        }
    } else { success = 0; }

    return success;
}

int RecActsTracking::InitialiseFTD()
{
    const edm4hep::TrackerHitCollection* hitFTDCol = nullptr;
    const edm4hep::SimTrackerHitCollection* SimhitFTDCol = nullptr;

    try {
        hitFTDCol = _inFTDTrackHdl.get();
    } catch (GaudiException& e) {
        debug() << "Collection " << _inFTDTrackHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
        return 0;
    }

    try {
        SimhitFTDCol = _inFTDColHdl.get();
    } catch (GaudiException& e) {
        debug() << "Collection " << _inFTDColHdl.fullKey() << " is unavailable in event " << _nEvt << endmsg;
        return 0;
    }

    if (hitFTDCol && SimhitFTDCol)
    {
        int nelem = hitFTDCol->size();
        debug() << "Number of FTD hits = " << nelem << endmsg;
        for (int ielem = 0; ielem < nelem; ++ielem)
        {
            auto hit = hitFTDCol->at(ielem);
            auto simhit = SimhitFTDCol->at(ielem);
            auto simcellid = simhit.getCellID();
            uint64_t m_system = ftd_decoder->get(simcellid, "system");
            uint64_t m_side = ftd_decoder->get(simcellid, "side");
            uint64_t m_layer = ftd_decoder->get(simcellid, "layer");
            uint64_t m_module = ftd_decoder->get(simcellid, "module");
            uint64_t m_sensor = ftd_decoder->get(simcellid, "sensor");
            double acts_x = simhit.getPosition()[0];
            double acts_y = simhit.getPosition()[1];
            double acts_z = simhit.getPosition()[2];
            double momentum_x = simhit.getMomentum()[0];
            double momentum_y = simhit.getMomentum()[1];
            double momentum_z = simhit.getMomentum()[2];
            const Acts::Vector3 globalmom{momentum_x, momentum_y, momentum_z};
            std::array<float, 6> m_covMatrix = hit.getCovMatrix();
            
            if (m_layer > 2) {
                continue;
            }

            dd4hep::rec::ISurface* surface = nullptr;
            auto it = m_ftd_surfaces->find(simcellid);
            if (it != m_ftd_surfaces->end()) {
                surface = it->second;
                if (!surface) {
                    fatal() << "found surface for FTD cell id " << simcellid << ", but NULL" << endmsg;
                    return 0;
                }
            }

            // set acts geometry identifier
            uint64_t acts_volume = (acts_z > 0) ? FTD_positive_volume_ids[m_layer] : FTD_negative_volume_ids[m_layer];
            uint64_t acts_boundary = 0;
            uint64_t acts_layer = 2;
            uint64_t acts_approach = 0;
            uint64_t acts_sensitive = m_module + 1;

            Acts::GeometryIdentifier moduleGeoId;
            moduleGeoId.setVolume(acts_volume);
            moduleGeoId.setBoundary(acts_boundary);
            moduleGeoId.setLayer(acts_layer);
            moduleGeoId.setApproach(acts_approach);
            moduleGeoId.setSensitive(acts_sensitive);   

            // create and store the source link
            uint32_t measurementIdx = measurements.size();
            IndexSourceLink sourceLink{moduleGeoId, measurementIdx, hit};
            sourceLinks.insert(sourceLinks.end(), sourceLink);
            Acts::SourceLink sl{sourceLink};    
            boost::container::static_vector<Acts::SourceLink, 2> slinks;
            slinks.emplace_back(sl);

            // get the local position of the hit
            IndexSourceLink::SurfaceAccessor surfaceAccessor{*trackingGeometry};
            const Acts::Surface* acts_surface = surfaceAccessor(sl);
            const Acts::Vector3 globalPos{acts_x, acts_y, acts_z};
            auto acts_local_postion = acts_surface->globalToLocal(geoContext, globalPos, globalmom, onSurfaceTolerance);
            if (!acts_local_postion.ok()){
                info() << "Error: failed to get local position for FTD layer " << m_layer << " module " << m_module << " sensor " << m_sensor << endmsg;
                acts_local_postion = acts_surface->globalToLocal(geoContext, globalPos, globalmom, 100*onSurfaceTolerance);
            }
            const std::array<Acts::BoundIndices, 2> indices{Acts::BoundIndices::eBoundLoc0, Acts::BoundIndices::eBoundLoc1};
            const Acts::Vector2 par{acts_local_postion.value()[0], acts_local_postion.value()[1]};

            // debug
            debug() << "FTD measurements global position(x,y,z): " << simhit.getPosition()[0] << ", " << simhit.getPosition()[1] << ", " << simhit.getPosition()[2]
                << "; local position(loc0, loc1): "<< acts_local_postion.value()[0] << ", " << acts_local_postion.value()[1] << endmsg;
            auto acts_global_postion = acts_surface->localToGlobal(geoContext, par, globalFakeMom);
            debug() << "debug surface at: x:" << acts_global_postion[0] << ", y:" << acts_global_postion[1] << ", z:" << acts_global_postion[2] << endmsg;

            if (ExtendSeedRange.value()) {
                SimSpacePoint *hitExt = new SimSpacePoint(hit, acts_global_postion[0], acts_global_postion[1], acts_global_postion[2], 0.002, slinks);
                SpacePointPtrs.push_back(hitExt);
            }

            // create and store the measurement
            Acts::ActsSquareMatrix<2> cov = Acts::ActsSquareMatrix<2>::Identity();
            cov(0, 0) = std::max<double>(double(m_covMatrix[2]), eps.value());
            cov(1, 1) = std::max<double>(double(m_covMatrix[5]), eps.value());
            measurements.emplace_back(Acts::Measurement<Acts::BoundIndices, 2>(sl, indices, par, cov));
        }
    }

    return 1;
}