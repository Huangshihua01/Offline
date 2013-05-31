#ifndef TargetGeom_zBinningForFoils_hh
#define TargetGeom_zBinningForFoils_hh

//
// Free function to compute a convenient binning for a histogram that will show
// the z positions of the target foils.  If the spacing of foils is uniform, then all
// foil centers are guaranteed to be on bin centers.
//
// The argument controls the bin size: it specifies how many bins to make between
// the centers of adjacent foils.  The binning definition returned includes extra
// space at both ends, equal to the distance between foils; so for a detector with N
// foils, the histogram covers N+1 foil spacings.
//
// The case of a target with one foil is handed differently.
// This uses the DetectorSystem coordinates, not the Mu2e system.
//
// $Id: zBinningForFoils.hh,v 1.4 2013/05/31 18:07:29 gandr Exp $
// $Author: gandr $
// $Date: 2013/05/31 18:07:29 $
//
// Original author Rob Kutschke

#include "GeneralUtilities/inc/Binning.hh"

namespace mu2e {

  class Target;

  Binning zBinningForFoils( Target const& target, int nBinsDZ );

} // namespace mu2e

#endif /* TargetGeom_zBinningForFoils_hh */
