
#include "Offline/Print/inc/ProtonBunchTimePrinter.hh"
#include "art/Framework/Principal/Provenance.h"
#include <string>
#include <iomanip>

void
mu2e::ProtonBunchTimePrinter::Print(art::Event const& event,
                                std::ostream& os) {
  if(verbose()<1) return;
  if(tags().empty()) {
    // if a list of instances not specified, print all instances
    std::vector< art::Handle<ProtonBunchTime> > vah = event.getMany<ProtonBunchTime>();
    for (auto const & ah : vah) Print(ah);
  } else {
    // print requested instances
    for(const auto& tag : tags() ) {
      auto ih = event.getValidHandle<ProtonBunchTime>(tag);
      Print(ih);
    }
  }
}

void
mu2e::ProtonBunchTimePrinter::Print(const art::Handle<ProtonBunchTime>& handle,
                                std::ostream& os) {
  if(verbose()<1) return;
  // the product tags with all four fields, with underscores
  std::string tag = handle.provenance()->productDescription().branchName();
  tag.pop_back(); // remove trailing dot
  PrintHeader(tag,os);
  Print(*handle);
}

void
mu2e::ProtonBunchTimePrinter::Print(const art::ValidHandle<ProtonBunchTime>& handle,
                                std::ostream& os) {
  if(verbose()<1) return;
  // the product tags with all four fields, with underscores
  std::string tag = handle.provenance()->productDescription().branchName();
  tag.pop_back(); // remove trailing dot
  PrintHeader(tag,os);
  Print(*handle);
}


void
mu2e::ProtonBunchTimePrinter::Print(const mu2e::ProtonBunchTime& obj, int ind, std::ostream& os) {
  if(verbose()<1) return;

  os << std::setiosflags(std::ios::fixed | std::ios::right);

  os
    << " time: " << std::setw(8) << std::setprecision(2) << obj.pbtime_
    << "   error: " << std::setw(6) << std::setprecision(2) <<  obj.pbterr_;
  os << std::endl;

}

void
mu2e::ProtonBunchTimePrinter::PrintHeader(const std::string& tag, std::ostream& os) {
  if(verbose()<1) return;
  os << "\nProductPrint " << tag << "\n";
}


