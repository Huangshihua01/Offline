#ifndef RecoDataProducts_StrawCluster_hh
#define RecoDataProducts_StrawCluster_hh
//
// First version of a Cluster.
//
// $Id: StrawCluster.hh,v 1.6 2011/06/07 21:32:22 kutschke Exp $
// $Author: kutschke $
// $Date: 2011/06/07 21:32:22 $
//
// Original author Hans Wenzel
//

// Mu2e includes:
#include "canvas/Persistency/Common/Ptr.h"

// C++ includes
#include <vector>

namespace mu2e {

  typedef art::Ptr<StrawHit>       StrawHitPtr;
  typedef std::vector<StrawHitPtr> StrawHitPtrVector;

  class StrawCluster{

  public:

    StrawCluster() {}

    StrawCluster( StrawHitPtrVector const & astrawHits):
      strawHits_(astrawHits){}

    // Accessors
    StrawHitPtrVector const & strawHits() const { return strawHits_; }
    StrawHitPtrVector & strawHits() { return strawHits_; }

  private:

    //   const Tracker& tracker;
    StrawHitPtrVector strawHits_;
  };

} // namespace mu2e

#endif /* RecoDataProducts_StrawCluster_hh */
