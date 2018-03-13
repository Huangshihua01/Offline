#ifndef Mu2eG4_TrackingAction_hh
#define Mu2eG4_TrackingAction_hh
//
// Steering routine for user tracking actions.
// If Mu2e needs many different user tracking actions, they
// should be called from this class.
//
// $Id: TrackingAction.hh,v 1.30 2014/08/25 20:01:30 genser Exp $
// $Author: genser $
// $Date: 2014/08/25 20:01:30 $
//
// Original author Rob Kutschke
//

#include "CLHEP/Vector/ThreeVector.h"

#include "G4UserTrackingAction.hh"

#include "MCDataProducts/inc/MCTrajectoryCollection.hh"
#include "MCDataProducts/inc/GenParticleCollection.hh"
#include "MCDataProducts/inc/SimParticleCollection.hh"
#include "MCDataProducts/inc/SimParticleRemapping.hh"

#include "Mu2eG4/inc/EventNumberList.hh"
#include "Mu2eG4/inc/PhysicalVolumeHelper.hh"
#include "Mu2eG4/inc/PhysicsProcessInfo.hh"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib/cpu_timer.h"

#include <map>
#include <string>

namespace fhicl { class ParameterSet; }

namespace mu2e {

  // Forward declarations in mu2e namespace
  class SimpleConfig;
  class Mu2eG4SteppingAction;
  class SimParticleHelper;
  class SimParticlePrimaryHelper;
  class Mu2eG4ResourceLimits;
  class Mu2eG4TrajectoryControl;

  class TrackingAction: public G4UserTrackingAction{

  public:

    TrackingAction( const SimpleConfig& config, Mu2eG4SteppingAction *);

    TrackingAction(const fhicl::ParameterSet& pset,
                   Mu2eG4SteppingAction *,
                   const Mu2eG4TrajectoryControl& trajectoryControl,
                   const Mu2eG4ResourceLimits &lim);

    virtual ~TrackingAction();

    // These methods are required by G4
    virtual void PreUserTrackingAction (const G4Track* trk);
    virtual void PostUserTrackingAction(const G4Track* trk);

    // All methods after here are Mu2e specific.

    // Do all things that need to be done at the beginning/end of an event.
    void beginEvent( const art::Handle<SimParticleCollection>& inputSims,
                     const art::Handle<MCTrajectoryCollection>& inputMCTraj,
                     const SimParticleHelper& spHelper,
                     const SimParticlePrimaryHelper& primaryHelper,
                     MCTrajectoryCollection& mcTrajectories,
                     SimParticleRemapping& simsRemap
                     );

    void endEvent( SimParticleCollection& simParticles );

    // Record start and end points of each track created by G4.
    // Copy to the event data.
    void saveSimParticleStart(const G4Track* trk);
    void saveSimParticleEnd  (const G4Track* trk);

    // Receive persistent volume information and save it for the duration of the run.
    void beginRun( const PhysicalVolumeHelper* physVolHelper,
                   PhysicsProcessInfo* processInfo,
                   CLHEP::Hep3Vector const& mu2eOrigin );

    // Clean up at end of run.
    void endRun();

    // Accessors for status information.
    unsigned        nG4Tracks() const { return _currentSize;}
    bool overflowSimParticles() const { return _overflowSimParticles; }

  private:

    typedef SimParticleCollection::key_type    key_type;
    typedef SimParticleCollection::mapped_type mapped_type;
    typedef std::map<key_type,mapped_type>     map_type;

    // Lists of events and tracks for which to enable debug printout.
    EventNumberList _debugList;

    // Non-owing pointer to the utility that translates between transient and persistent
    // representations of info about volumes.
    const PhysicalVolumeHelper* _physVolHelper;

    // Origin of Mu2e Coordinate system in the G4 world system.
    CLHEP::Hep3Vector _mu2eOrigin;

    // Event timer.
    cet::cpu_timer _timer;

    // Information about SimParticles is collected in this map
    // during the operation of G4.  This is not persistent.
    map_type _transientMap;

    // Non-owning pointer to the collection of trajectories generated by G4.
    MCTrajectoryCollection* _trajectories;

    // Limit maximum size of the steps collection
    unsigned _sizeLimit;
    unsigned _currentSize;
    bool _overflowSimParticles;
    double _mcTrajectoryMomentumCut;
    double _saveTrajectoryMomentumCut;
    int    _mcTrajectoryMinSteps;

    // Non-owning pointer to stepping action; lifetime of pointee is one run.
    Mu2eG4SteppingAction * _steppingAction;

    // Non-owning pointer to the information about physical processes;
    // lifetime of pointee is one run.
    PhysicsProcessInfo *  _processInfo;
    bool _printTrackTiming;

    // Helper to obtain SimParticle Ptr (non-owning)
    const SimParticleHelper *_spHelper;
    const SimParticlePrimaryHelper *_primaryHelper;

    // Some helper functions.
    void insertOrThrow(std::pair<int,SimParticle> const& value);

    // If the track passes, the min hits cut and the momentum cut, add the
    // trajectory information to the output data product.
    void swapTrajectory( const G4Track* trk );
      
  };

} // end namespace mu2e

#endif /* Mu2eG4_TrackingAction_hh */
