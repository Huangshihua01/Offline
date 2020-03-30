#define _USE_MATH_DEFINES
#include <iostream>
#include <string>
#include <cmath>

// Cosmic Tracks:
#include "CosmicReco/inc/CosmicTrackFit.hh"
#include "RecoDataProducts/inc/CosmicTrack.hh"
#include "RecoDataProducts/inc/CosmicTrackSeed.hh"
#include "CosmicReco/inc/CosmicTrackMCInfo.hh"
#include "DataProducts/inc/EventWindowMarker.hh"

//Mu2e Data Prods:
#include "RecoDataProducts/inc/StrawHitFlagCollection.hh"
#include "RecoDataProducts/inc/StrawHit.hh"
#include "RecoDataProducts/inc/StrawHitFlag.hh"
#include "MCDataProducts/inc/StrawDigiMC.hh"
#include "MCDataProducts/inc/MCRelationship.hh"
#include "MCDataProducts/inc/SimParticleCollection.hh"
#include "MCDataProducts/inc/StepPointMCCollection.hh"
#include "RecoDataProducts/inc/ComboHit.hh"
#include "RecoDataProducts/inc/LineSeed.hh"
#include "DataProducts/inc/XYZVec.hh"
#include "ProditionsService/inc/ProditionsHandle.hh"

//Utilities
#include "Mu2eUtilities/inc/SimParticleTimeOffset.hh"
#include "TrkDiag/inc/TrkMCTools.hh"
#include "CosmicReco/inc/DriftFitUtils.hh"
#include "Mu2eUtilities/inc/ParametricFit.hh"
#include "Mu2eUtilities/inc/TwoLinePCA.hh"

// Mu2e diagnostics
#include "TrkDiag/inc/ComboHitInfo.hh"
#include "GeneralUtilities/inc/ParameterSetHelpers.hh"
#include "GeometryService/inc/GeomHandle.hh"
#include "GeometryService/inc/DetectorSystem.hh"
// Framework includes.
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art_root_io/TFileService.h"
//ROOT
#include "TStyle.h"
#include "Rtypes.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TTree.h"
#include "TLatex.h"
#include "TGraph.h"
#include "TProfile.h"
using namespace std; 

namespace mu2e 
{
  class CosmicTrackDiag : public art::EDAnalyzer {
    public:
      struct Config{
        using Name=fhicl::Name;
        using Comment=fhicl::Comment;
        fhicl::Atom<int> diag{Name("diagLevel"), Comment("set to 1 for info"),2};
        fhicl::Atom<int> printlevel{Name("printLevel"), Comment("print level"), 0};
        fhicl::Atom<bool> mcdiag{Name("mcdiag"), Comment("set on for MC info"),true};
        fhicl::Atom<art::InputTag> chtag{Name("ComboHitCollection"),Comment("tag for combo hit collection")};
        fhicl::Atom<art::InputTag> tctag{Name("TimeClusterCollection"),Comment("tag for time cluster collection")};
        fhicl::Atom<art::InputTag> lstag{Name("LineSeedTag"),Comment("StrawDigi collection tag"),"LineFinder"};
        fhicl::Atom<art::InputTag> mcdigistag{Name("StrawDigiMCCollection"),Comment("StrawDigi collection tag"),"makeSD"};
        fhicl::Atom<art::InputTag> ewMarkerTag{Name("EventWindowMarkerLabel"),Comment("Event window marker tag"),"EWMProducer"};
        fhicl::Table<SimParticleTimeOffset::Config> toff{Name("TimeOffsets"), Comment("Sim particle time offset ")};
      };
      typedef art::EDAnalyzer::Table<Config> Parameters;

      explicit CosmicTrackDiag(const Parameters& conf);
      virtual ~CosmicTrackDiag();
      virtual void beginJob() override;
      virtual void beginRun(const art::Run& r) override;
      virtual void analyze(const art::Event& e) override;
    private: 

      Config _conf;

      int  _diag;
      int _printlevel;
      bool _mcdiag;
      std::ofstream outputfile;
      art::InputTag   _chtag;//combo
      art::InputTag   _tctag;//timeclusters
      art::InputTag   _lstag;//Striaght tracks
      art::InputTag   _mcdigistag; //MC digis
      art::InputTag _ewMarkerTag;
      SimParticleTimeOffset _toff;
      const ComboHitCollection* _chcol;
      const TimeClusterCollection* _tccol;
      const LineSeedCollection* _lfcol;
      const StrawDigiMCCollection* _mcdigis;
      CLHEP::Hep3Vector _mcpos, _mcdir;

      Float_t _ewMarkerOffset;
      const Tracker* tracker;
      ProditionsHandle<StrawResponse> _strawResponse_h; 


      //TTree Info:
      TTree* _trackT;
      TTree* _hitT;

      // track tree 
      Int_t _evt; 
      Int_t _ntrack;
      Int_t _nsh, _nch; // # associated straw hits / event
      Int_t _ntc; // # clusters/event
      Int_t _n_panels; // # panels
      Int_t _n_stations; // # stations
      Int_t _n_planes; // # stations
      Float_t _mct0;
      Float_t _fitt0;
      Int_t _mcnsh;
      Float_t _lfdoca, _lfangle;
      Int_t _tcnhits, _lfsize;
      Int_t _converged;
      Float_t _maxllike, _maxtresid, _lfllike;

      // hit tree 
      Float_t _hitlfdoca, _hitlflong, _hitlfdpocat;
      Float_t _hittruedoca, _hitmcdoca, _hitrecodoca;
      Float_t _hitmcdpocat;
      Float_t _hittruelong, _hitmclong, _hitrecolong;
      Float_t _hitmctresid;
      Int_t _hitbackground;
      Int_t _hitused;
      Int_t _hitambig, _hitmcambig;
      Float_t _hitrecolongrms;
      Float_t _hittresid, _hittresidrms, _hitllike;
      Float_t _hitedep;


      void GetMCTrack(const art::Event& event, const StrawDigiMCCollection& mccol);
      void hitDiag(const art::Event& event, StrawResponse const& srep);
      bool findData(const art::Event& evt);
  };

  CosmicTrackDiag::CosmicTrackDiag(const Parameters& conf) :
    art::EDAnalyzer(conf),
    _diag (conf().diag()),
    _printlevel (conf().printlevel()),
    _mcdiag (conf().mcdiag()),
    _chtag (conf().chtag()),
    _tctag (conf().tctag()),
    _lstag (conf().lstag()),
    _mcdigistag (conf().mcdigistag()),
    _ewMarkerTag (conf().ewMarkerTag()),
    _toff (conf().toff())
  {
    if(_mcdiag){
      for (auto const& tag : conf().toff().inputs()) {
        consumes<SimParticleTimeMap>(tag);
      }
    }
  }

  CosmicTrackDiag::~CosmicTrackDiag(){}


  void CosmicTrackDiag::beginJob() {

    // create diagnostics if requested...
    if(_diag > 0){
      art::ServiceHandle<art::TFileService> tfs;
      //Tree for detailed diagnostics
      _trackT=tfs->make<TTree>("trackT","Track level diagnostics");

      //Create branches:
      _trackT->Branch("evt",&_evt,"evt/I");  // add event id
      _trackT->Branch("ntrack",&_ntrack,"ntrack/I");
      _trackT->Branch("StrawHitsInEvent", &_nsh, "StrawHitsInEvent/I");
      _trackT->Branch("ComboHitsInEvent", &_nch, "ComboHitsInEvent/I");
      _trackT->Branch("PanelsCrossedInEvent", &_n_panels, "PanelsCrossedInEvent/I");
      _trackT->Branch("PlanesCrossedInEvent", &_n_planes, "PlanesCrossedInEvent/I");
      _trackT->Branch("StatonsCrossedInEvent", &_n_stations, "StationsCrossedInEvent/I");
      _trackT->Branch("TimeClustersInEvent", &_ntc, "TimeClusterInEvent/I"); 
      _trackT->Branch("t0",&_fitt0,"t0/F");
      _trackT->Branch("tcnhits",&_tcnhits,"tcnhits/I");
      _trackT->Branch("lfsize",&_lfsize,"lfsize/I");
      _trackT->Branch("ewmoffset",&_ewMarkerOffset,"ewmoffset/F");
      _trackT->Branch("converged",&_converged,"converged/I");
      _trackT->Branch("lfllike",&_lfllike,"lfllike/F");
      _trackT->Branch("maxtresid",&_maxtresid,"maxtresid/F");
      _trackT->Branch("maxllike",&_maxllike,"maxllike/F");
      if (_mcdiag){
        _trackT->Branch("mcnsh",&_mcnsh,"mcnsh/I");
        _trackT->Branch("mct0",&_mct0,"mct0/F");
        _trackT->Branch("lfdoca",&_lfdoca,"lfdoca/F");
        _trackT->Branch("lfangle",&_lfangle,"lfangle/F");
      }

      _hitT=tfs->make<TTree>("hitT","Hit tree");
      _hitT->Branch("evt",&_evt,"evt/I");  // add event id
      _hitT->Branch("doca",&_hitlfdoca,"doca/F");
      _hitT->Branch("hitused",&_hitused,"hitused/I");
      _hitT->Branch("long",&_hitlflong,"long/F");
      _hitT->Branch("recolong",&_hitrecolong,"recolong/F");
      _hitT->Branch("recodoca",&_hitrecodoca,"recodoca/F");
      _hitT->Branch("recolongrms",&_hitrecolongrms,"recolongrms/F");
      _hitT->Branch("tresid",&_hittresid,"tresid/F");
      _hitT->Branch("tresidrms",&_hittresid,"tresidrms/F");
      _hitT->Branch("llike",&_hitllike,"llike/F");
      _hitT->Branch("tcnhits",&_tcnhits,"tcnhits/I");
      _hitT->Branch("lfsize",&_lfsize,"lfsize/I");
      _hitT->Branch("t0",&_fitt0,"t0/F");
      _hitT->Branch("ewmoffset",&_ewMarkerOffset,"ewmoffset/F");
      _hitT->Branch("converged",&_converged,"converged/I");
      _hitT->Branch("lfllike",&_lfllike,"lfllike/F");
      _hitT->Branch("maxtresid",&_maxtresid,"maxtresid/F");
      _hitT->Branch("maxllike",&_maxllike,"maxllike/F");

      if (_mcdiag){
        _hitT->Branch("background",&_hitbackground,"background/I");
        _hitT->Branch("mct0",&_mct0,"mct0/F");
        _hitT->Branch("truedoca",&_hittruedoca,"truedoca/F");
        _hitT->Branch("mcdoca",&_hitmcdoca,"mcdoca/F");
        _hitT->Branch("mctresid",&_hitmctresid,"mctresid/F");
        _hitT->Branch("mcdpocat",&_hitmcdpocat,"mcdpocat/F");
        _hitT->Branch("lfdpocat",&_hitlfdpocat,"lfdpocat/F");
        _hitT->Branch("truelong",&_hittruelong,"truelong/F");
        _hitT->Branch("mclong",&_hitmclong,"mclong/F");
        _hitT->Branch("mcnsh",&_mcnsh,"mcnsh/I");
        _hitT->Branch("mct0",&_mct0,"mct0/F");
        _hitT->Branch("lfdoca",&_lfdoca,"lfdoca/F");
        _hitT->Branch("lfangle",&_lfangle,"lfangle/F");
        _hitT->Branch("ambig",&_hitambig,"ambig/I");
        _hitT->Branch("mcambig",&_hitmcambig,"mcambig/I");
        _hitT->Branch("edep",&_hitedep,"edep/F");
      }
    }
  } 

  void CosmicTrackDiag::beginRun(const art::Run& run){
    mu2e::GeomHandle<mu2e::Tracker> th;
    tracker = th.get();
  }

  void CosmicTrackDiag::analyze(const art::Event& event) {

    _evt = event.id().event();  // add event id
    if(!findData(event)) // find data
      throw cet::exception("RECO")<<"No Time Clusters in event"<< endl; 
    
    StrawResponse const& srep = _strawResponse_h.get(event.id());

    //find time clusters:
    _ntc = _tccol->size();
    _nch = _chcol->size();


    if (_mcdiag){
      GetMCTrack(event, *_mcdigis);
    }

    std::vector<uint16_t> panels, planes, stations;
    _nsh = 0;
    for(size_t ich = 0;ich < _chcol->size(); ++ich){
      ComboHit const& chit =(*_chcol)[ich];
      _nsh += chit.nStrawHits(); 
      uint16_t panelid = chit.strawId().uniquePanel();
      if (std::find(panels.begin(),panels.end(),panelid) == panels.end())
        panels.push_back(panelid);
      uint16_t planeid = chit.strawId().plane();
      if (std::find(planes.begin(),planes.end(),planeid) == planes.end())
        planes.push_back(planeid);
      uint16_t stationid = chit.strawId().station();
      if (std::find(stations.begin(),stations.end(),stationid) == stations.end())
        stations.push_back(stationid);
    }
    _n_panels = panels.size();
    _n_planes = planes.size();
    _n_stations = stations.size();
    _ntrack = -1;

    if (_lfcol->size() > 0){
      auto lseed = _lfcol->at(0);

      auto tclust = lseed._timeCluster;
      _tcnhits = 0;
      const std::vector<StrawHitIndex>& shIndices = tclust->hits();
      for (size_t i=0; i<shIndices.size(); ++i) {
        int loc = shIndices[i];
        const ComboHit& ch  = _chcol->at(loc);
        _tcnhits += ch.nStrawHits();
      }

      _lfsize = lseed._seedSize;
      _fitt0 = lseed._t0;
      _converged = lseed._converged;
      _ntrack = 0;
      auto lfpos = lseed._seedInt; 
      auto lfdir = lseed._seedDir.unit();

      const std::vector<StrawHitIndex>& lseedShIndices = lseed._strawHitIdxs;
      _maxtresid = 0;
      _maxllike = 0;
      _lfllike = 0;
      for (size_t i=0; i<lseedShIndices.size(); ++i) {
        int loc = lseedShIndices[i];
        //FIXME this for loop only makes sense if chcol is strawhits
        if (loc >= (int)_chcol->size())
          continue;
        const ComboHit& sh  = _chcol->at(loc);
        
        double llike = 0;
        Straw const& straw = tracker->getStraw(sh.strawId());
        TwoLinePCA pca(straw.getMidPoint(), straw.getDirection(), lfpos, lfdir);
        double longdist = (pca.point1()-straw.getMidPoint()).dot(straw.getDirection());
        double longres = srep.wpRes(sh.energyDep()*1000., fabs(longdist));

        llike += pow(longdist-sh.wireDist(),2)/pow(longres,2);

        double drift_time = srep.driftDistanceToTime(sh.strawId(), pca.dca(), 0);
        drift_time += srep.driftDistanceOffset(sh.strawId(), 0, 0, pca.dca());

        double drift_res = srep.driftDistanceError(sh.strawId(), 0, 0, pca.dca());

        double traj_time = ((pca.point2() - lfpos).dot(lfdir))/299.9;
        double hit_t0 = sh.time() - sh.propTime() - traj_time - drift_time;
        double tresid = hit_t0-_fitt0;
        llike += pow(tresid,2)/pow(drift_res,2);

        if (fabs(tresid) > _maxtresid)
          _maxtresid = fabs(tresid);
        if (llike > _maxllike)
          _maxllike = llike;
        _lfllike += llike;
      }


      TwoLinePCA lfpca( _mcpos, _mcdir, lfpos, lfdir);
      _lfdoca = lfpca.dca(); 
      _lfangle = _mcdir.dot(lfdir);
    }
    _trackT->Fill();

    if (_diag > 1)
      hitDiag(event, srep);

  }

  bool CosmicTrackDiag::findData(const art::Event& evt){
    _chcol = 0; 
    _tccol = 0;
    _lfcol = 0; 
    _ewMarkerOffset = 0;
    auto chH = evt.getValidHandle<ComboHitCollection>(_chtag);
    _chcol = chH.product();
    auto tcH = evt.getValidHandle<TimeClusterCollection>(_tctag);
    _tccol =tcH.product();
    auto lsH = evt.getValidHandle<LineSeedCollection>(_lstag);
    _lfcol =lsH.product();
    auto ewMarkerHandle = evt.getValidHandle<EventWindowMarker>(_ewMarkerTag);
    auto ewMarker = ewMarkerHandle.product();
    _ewMarkerOffset = ewMarker->timeOffset();
    if(_mcdiag){
      _mcdigis=0;
      auto mcdH = evt.getValidHandle<StrawDigiMCCollection>(_mcdigistag);
      _mcdigis = mcdH.product();
      _toff.updateMap(evt);
    }
    return _chcol != 0 && _tccol!=0 && _lfcol !=0 && (_mcdigis != 0 || !_mcdiag);
  }


  void CosmicTrackDiag::GetMCTrack(const art::Event& event, const StrawDigiMCCollection& mccol) {
    // get all possible directions
    std::vector<CLHEP::Hep3Vector> pppos;
    std::vector<CLHEP::Hep3Vector> ppdir;
    for (size_t i=0;i<mccol.size();i++){
      StrawDigiMC mcdigi = mccol[i]; 
      auto const& sgsptr = mcdigi.earlyStrawGasStep();
      auto const& sgs = *sgsptr;
      auto const& sp = *sgs.simParticle();
      auto posi = Geom::Hep3Vec(sgs.startPosition());
      if ((sp.pdgId() == 13 || sp.pdgId() == -13) && sp.creationCode() == 56){
        for (size_t j=i+1;j<mccol.size();j++){
          StrawDigiMC jmcdigi = mccol[j]; 
          auto const& jsgsptr = jmcdigi.earlyStrawGasStep();
          auto const& jsgs = *jsgsptr;
          auto const& jsp = *jsgs.simParticle();
          auto posj = Geom::Hep3Vec(jsgs.startPosition());
          if ((jsp.pdgId() == 13 || jsp.pdgId() == -13) && jsp.creationCode() == 56){
            pppos.push_back(posi);
            ppdir.push_back((posi-posj).unit());
          } 
        }
      }
    }

    // get the one that has the most hits within 500 microns
    int max = 0;
    for (size_t j=0;j<pppos.size();j++){
      int count = 0;
      double avg_t0 = 0;
      CLHEP::Hep3Vector ppintercept(0,0,0);
      CLHEP::Hep3Vector ppdirection(0,1,0);
      for (size_t i=0;i<mccol.size();i++){
        StrawDigiMC mcdigi = mccol[i]; 

        const Straw& straw = tracker->getStraw( mcdigi.strawId() );
        auto const& sgsptr = mcdigi.earlyStrawGasStep();
        auto const& sgs = *sgsptr;
        auto const& sp = *sgs.simParticle();

        if ((sp.pdgId() == 13 || sp.pdgId() == -13) && sp.creationCode() == 56){
          TwoLinePCA pca( straw.getMidPoint(), straw.getDirection(),
              Geom::Hep3Vec(sgs.startPosition()), Geom::Hep3Vec(sgs.endPosition()-sgs.startPosition()) );
          double true_doca = pca.dca(); 

          TwoLinePCA pca2( straw.getMidPoint(), straw.getDirection(),
              pppos[j], ppdir[j]);

          double mctrack_doca = pca2.dca(); 
          if (fabs(true_doca - mctrack_doca) < 0.5){
            count++;
            ppintercept = pppos[j] - ppdir[j]*pppos[j].y()/ppdir[j].y();
            ppdirection = ppdir[j];
            if (ppdirection.y() > 0)
              ppdirection *= -1;
            double mctime = sgs.time() + _toff.totalTimeOffset(sgs.simParticle());// - _ewMarkerOffset;
            double trajtime = (pca2.point2()-ppintercept).dot(ppdirection.unit())/299.9;
            mctime -= trajtime;
            avg_t0 += mctime;
          }
        }
      }
      if (count > max){
        max = count;
        _mcpos = ppintercept;
        _mcdir = ppdirection;
        _mct0 = avg_t0/count;
      }
    }
    if (_mcdir.y() > 0)
      _mcdir *= -1;

    _mcnsh = 0;
    // now lets count the number of straw hits in this mc event
    // loop over combohits, for each combo hit get all the sub strawhits
    for (size_t ich=0;ich<_chcol->size();ich++){
        const ComboHit &sh = _chcol->at(ich);
        const Straw& straw = tracker->getStraw( sh.strawId() );

        // get the StrawDigi indices for this combohit
        std::vector<StrawDigiIndex> shids;
        _chcol->fillStrawDigiIndices(event,ich,shids);
        // shids.size() should be 1 if this is really all single StrawHits
        if (shids.size() != 1){
          std::cout << "INCORRECT NUMBER OF StrawDigis " << shids.size() << std::endl;
          continue;
        }
        const StrawDigiMC &mcdigi = _mcdigis->at(shids[0]);

        auto const& sgsptr = mcdigi.earlyStrawGasStep();
        auto const& sgs = *sgsptr;
        auto const& sp = *sgs.simParticle();
        if ((sp.pdgId() == 13 || sp.pdgId() == -13) && sp.creationCode() == 56){
          TwoLinePCA pca( straw.getMidPoint(), straw.getDirection(),
              Geom::Hep3Vec(sgs.startPosition()), Geom::Hep3Vec(sgs.endPosition()-sgs.startPosition()) );
          double true_doca = pca.dca(); 

          TwoLinePCA pca2( straw.getMidPoint(), straw.getDirection(),
              _mcpos, _mcdir);

          double mctrack_doca = pca2.dca(); 
          if (fabs(true_doca - mctrack_doca) < 0.5){
            _mcnsh += 1;
          }
        }
    }
  }

  void CosmicTrackDiag::hitDiag(const art::Event& event, StrawResponse const& srep) {
    // loop over combohits
    for (size_t ich=0;ich<_chcol->size();ich++){
      const ComboHit &sh = _chcol->at(ich);
      const Straw& straw = tracker->getStraw( sh.strawId() );


      _hitlfdoca = -999;
      _hitbackground = 1;
      _hittruedoca = -999;
      _hitmcdoca = -999;
      _hitmcdpocat = -999;
      _hitlfdpocat = -999;
      _hitused = 0;
      _hittruelong = -999;
      _hitmclong = -999;
      _hitlflong = -999;
      _hitrecolong = -999;
      _hitrecolongrms = -999;
      _hittresid = -999;
      _hittresidrms = -999;
      _hitllike = -999;
      _hitrecodoca = -999;
      _hitambig = -999;
      _hitmcambig = -999;
      _hitedep = -999;



      _hitedep = sh.energyDep()*1000.;
      _hitrecolong = sh.wireDist();
      _hitrecolongrms = sh.wireRes();
      _hitrecodoca = sh.driftTime();

      CLHEP::Hep3Vector pcapoint2(0,0,0);
      if (_mcdiag){
        // get the StrawDigi indices for this combohit
        std::vector<StrawDigiIndex> shids;
        _chcol->fillStrawDigiIndices(event,ich,shids);
        // shids.size() should be 1 if this is really all single StrawHits
        if (shids.size() != 1){
          std::cout << "INCORRECT NUMBER OF StrawDigis " << shids.size() << std::endl;
          continue;
        }

        const StrawDigiMC &mcdigi = _mcdigis->at(shids[0]);

        auto const& sgsptr = mcdigi.earlyStrawGasStep();
        auto const& sgs = *sgsptr;
        auto const& sp = *sgs.simParticle();

        // get true ambiguity
        CLHEP::Hep3Vector mcsep = sgs.position()-straw.getMidPoint();
        CLHEP::Hep3Vector sgsdir = Geom::Hep3Vec(sgs.momentum()).unit();
        CLHEP::Hep3Vector mcperp = (sgsdir.cross(straw.getDirection())).unit();
        double mcdperp = mcperp.dot(mcsep);
        _hitmcambig = mcdperp > 0 ? -1 : 1;

        if (sp.creationCode() == 56){
          _hitbackground = 0;
        }

        // Get the actual DOCA of the MC step
        TwoLinePCA pca( straw.getMidPoint(), straw.getDirection(),
            Geom::Hep3Vec(sgs.startPosition()), Geom::Hep3Vec(sgs.endPosition()-sgs.startPosition()) );
        _hittruedoca = pca.dca(); 
        _hittruelong = (Geom::Hep3Vec(sgs.startPosition())-straw.getMidPoint()).dot(straw.getDirection());

        // Get the DOCA from the MC straight line track
        TwoLinePCA pca1( straw.getMidPoint(), straw.getDirection(),
            _mcpos, _mcdir);
        pcapoint2 = pca.point2();
        _hitmcdoca = pca1.dca(); 
        // get the delta transverse distance between POCA of step and POCA of track
        _hitmcdpocat = sqrt((pca.point2()-pca1.point2()).mag2()-pow((pca.point2()-pca1.point2()).dot(straw.getDirection()),2));
        _hitmclong = (pca1.point1()-straw.getMidPoint()).dot(straw.getDirection());

        TwoLinePCA mcpca( _mcpos, _mcdir,
            Geom::Hep3Vec(sgs.startPosition()), Geom::Hep3Vec(sgs.endPosition()-sgs.startPosition()) );
        double trajtime = (mcpca.point1()-_mcpos).dot(_mcdir.unit())/299.9;
        if (mcpca.closeToParallel()){
          trajtime = (Geom::Hep3Vec(sgs.startPosition())-_mcpos).dot(_mcdir.unit())/299.9;
        }
        _hitmctresid = sh.time() - (_mct0 + trajtime);

      }


      if (_lfcol->size() > 0){
        auto lfpos = _lfcol->at(0)._seedInt; 
        auto lfdir = _lfcol->at(0)._seedDir.unit();

        if (std::find(_lfcol->at(0)._strawHitIdxs.begin(),_lfcol->at(0)._strawHitIdxs.end(),ich) != _lfcol->at(0)._strawHitIdxs.end())
          _hitused = 1;

        auto lfdowndir = lfdir;
        if (lfdowndir.y() > 0)
          lfdowndir *= -1;

        TwoLinePCA pca3( straw.getMidPoint(), straw.getDirection(),
            lfpos, lfdir);
        _hitlfdoca = pca3.dca();
        _hitlflong = (pca3.point1()-straw.getMidPoint()).dot(straw.getDirection());

        CLHEP::Hep3Vector sep = (pca3.point2()-straw.getMidPoint()); // vector from PCA to straw
        CLHEP::Hep3Vector perp = (lfdowndir.cross(straw.getDirection())).unit();
        double dperp = perp.dot(sep);
        _hitambig = dperp > 0 ? -1 : 1;



        _hitllike = pow(_hitlflong-_hitrecolong,2)/pow(_hitrecolongrms,2);

        double traj_time = ((pca3.point2() - lfpos).dot(lfdir))/299.9;

        double drift_time = srep.driftDistanceToTime(sh.strawId(), pca3.dca(), 0);
        drift_time += srep.driftDistanceOffset(sh.strawId(), 0, 0, pca3.dca());

        _hittresidrms = srep.driftDistanceError(sh.strawId(), 0, 0, pca3.dca());
        double hit_t0 = sh.time() - sh.propTime() - traj_time - drift_time;
        _hittresid = hit_t0-_lfcol->at(0)._t0;
        _hitllike += pow(_hittresid,2)/pow(_hittresidrms,2);


        if (_printlevel > 0){
          if (_hitused){
            std::cout << "   " << _hitlfdoca << " " << _hitlflong << " " << _hitmcdoca << " " << _hitmclong << " " << sh.strawId() << std::endl;
          }else{
            std::cout << " X " << _hitlfdoca << " " << _hitlflong << " " << _hitmcdoca << " " << _hitmclong << " " << sh.strawId() << std::endl;
          }
        }


        if (_mcdiag){
          _hitlfdpocat = sqrt((pcapoint2-pca3.point2()).mag2()-pow((pcapoint2-pca3.point2()).dot(straw.getDirection()),2));
        }
      }
      _hitT->Fill();
    }
  }

}

using mu2e::CosmicTrackDiag;
DEFINE_ART_MODULE(CosmicTrackDiag);
