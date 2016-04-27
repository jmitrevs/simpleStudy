################################################################################
# Package: simpleStudy
################################################################################

# Declare the package name:
atlas_subdir( simpleStudy )

# Declare the package's dependencies:
atlas_depends_on_subdirs(
   PUBLIC
   Control/AthenaBaseComps
   Control/StoreGate
   GaudiKernel
   Generators/GeneratorObjects
   PhysicsAnalysis/MCTruthClassifier
   PhysicsAnalysis/TruthParticleID/McParticleEvent
   PhysicsAnalysis/TruthParticleID/McParticleUtils
   Reconstruction/Particle
   Reconstruction/ParticleTruth
   Reconstruction/egamma/egammaEvent
   Tracking/TrkEvent/VxVertex
   PRIVATE
   Control/AthenaKernel
   Event/EventInfo
   Event/FourMomUtils
   Event/xAOD/xAODEventInfo
   Event/xAOD/xAODEgamma
   Event/xAOD/xAODTruth
   PhysicsAnalysis/AnalysisCommon/AnalysisUtils
   PhysicsAnalysis/AnalysisCommon/PATCore
   PhysicsAnalysis/ElectronPhotonID/ElectronPhotonSelectorTools
   Tracking/TrkEvent/TrkParticleBase
   Tracking/TrkEvent/TrkTrack
   Tracking/TrkEvent/TrkTrackLink
   Tracking/TrkEvent/TrkTrackSummary )

# External dependencies:
find_package( GSL )
find_package( HepMC )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_component( simpleStudy
   simpleStudy/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS} ${HEPMC_INCLUDE_DIRS}
   LINK_LIBRARIES ${ROOT_LIBRARIES} ${GSL_LIBRARIES} ${HEPMC_LIBRARIES}
   AthenaBaseComps StoreGateLib GaudiKernel GeneratorObjects MCTruthClassifier
   McParticleEvent McParticleUtils Particle ParticleTruth egammaEvent
   VxVertex AthenaKernel EventInfo FourMomUtils xAODEventInfo xAODEgamma xAODTruth
   AnalysisUtils PATCoreLib ElectronPhotonSelectorToolsLib TrkParticleBase
   TrkTrack TrkTrackSummary )
atlas_install_joboptions( run/*.py )
