//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidDataHandling/GroupDetectors.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidAPI/SpectraDetectorMap.h"
#include "MantidKernel/ArrayProperty.h"
#include <set>
#include <numeric>

namespace Mantid
{
namespace DataHandling
{
// Register the algorithm into the algorithm factory
DECLARE_ALGORITHM(GroupDetectors)

/// Sets documentation strings for this algorithm
void GroupDetectors::initDocs()
{
  this->setWikiSummary("Sums spectra bin-by-bin, equivalent to grouping the data from a set of detectors.  Individual groups can be specified by passing the algorithm a list of spectrum numbers, detector IDs or workspace indices. Many spectra groups can be created in one execution via an input file. ");
  this->setOptionalMessage("Sums spectra bin-by-bin, equivalent to grouping the data from a set of detectors.  Individual groups can be specified by passing the algorithm a list of spectrum numbers, detector IDs or workspace indices. Many spectra groups can be created in one execution via an input file.");
}


using namespace Kernel;
using namespace API;

/// (Empty) Constructor
GroupDetectors::GroupDetectors() {}

/// Destructor
GroupDetectors::~GroupDetectors() {}

void GroupDetectors::init()
{
  declareProperty(new WorkspaceProperty<>("Workspace","",Direction::InOut,
    new CommonBinsValidator<>),
    "The name of the workspace2D on which to perform the algorithm");

  declareProperty(new ArrayProperty<specid_t>("SpectraList"),
    "An array containing a list of the indexes of the spectra to combine\n"
    "(DetectorList and WorkspaceIndexList are ignored if this is set)" );

  declareProperty(new ArrayProperty<detid_t>("DetectorList"),
    "An array of detector ID's (WorkspaceIndexList is ignored if this is\n"
    "set)" );

  declareProperty(new ArrayProperty<size_t>("WorkspaceIndexList"),
    "An array of workspace indices to combine" );

  declareProperty("ResultIndex", -1,
    "The workspace index of the summed spectrum (or -1 on error)",
    Direction::Output);
}

void GroupDetectors::exec()
{
  // Get the input workspace
  const MatrixWorkspace_sptr WS = getProperty("Workspace");

  std::vector<size_t> indexList = getProperty("WorkspaceIndexList");
  std::vector<specid_t> spectraList = getProperty("SpectraList");
  const std::vector<detid_t> detectorList = getProperty("DetectorList");

  // Could create a Validator to replace the below
  if ( indexList.empty() && spectraList.empty() && detectorList.empty() )
  {
    g_log.information(name() +
      ": WorkspaceIndexList, SpectraList, and DetectorList properties are all empty, no grouping done");
    return;
  }

  // Bin boundaries need to be the same, so check if they actually are
  if (!API::WorkspaceHelpers::commonBoundaries(WS))
  {
    g_log.error("Can only group if the histograms have common bin boundaries");
    throw std::runtime_error("Can only group if the histograms have common bin boundaries");
  }

  // If the spectraList property has been set, need to loop over the workspace looking for the
  // appropriate spectra number and adding the indices they are linked to the list to be processed
  if ( ! spectraList.empty() )
  {
    WS->getIndicesFromSpectra(spectraList,indexList);
  }// End dealing with spectraList
  else if ( ! detectorList.empty() )
  {// Dealing with DetectorList
    //convert from detectors to spectra numbers
    std::vector<specid_t> mySpectraList = WS->spectraMap().getSpectra(detectorList);
    //then from spectra numbers to indices
    WS->getIndicesFromSpectra(mySpectraList,indexList);
  }

  if ( indexList.size() == 0 )
  {
      g_log.warning("Nothing to group");
      return;
  }

  const size_t vectorSize = WS->blocksize();

  const specid_t firstIndex = static_cast<specid_t>(indexList[0]);
  ISpectrum * firstSpectrum = WS->getSpectrum(firstIndex);

  setProperty("ResultIndex",firstIndex);
  const Geometry::ISpectraDetectorMap & inputSpectra = WS->spectraMap();
  API::SpectraDetectorMap *groupedMap = dynamic_cast<API::SpectraDetectorMap*>(inputSpectra.clone());
  if( !groupedMap )
  {
    throw std::invalid_argument("Input workspace with a 1:1 spectra-detector map is not supported "
				"by this algorithm.");
  }

  // loop over the spectra to group
  Progress progress(this, 0.0, 1.0, static_cast<int>(indexList.size()-1));
  for (size_t i = 0; i < indexList.size()-1; ++i)
  {
    // The current spectrum
    const size_t currentIndex = indexList[i+1];
    ISpectrum * spec = WS->getSpectrum(currentIndex);

    // Add the current detector to belong to the first spectrum
    firstSpectrum->addDetectorIDs(spec->getDetectorIDs());
    //groupedMap->remap(spectraAxis->spectraNo(currentIndex), firstSpectrum);

    // Add up all the Y spectra and store the result in the first one
    // Need to keep the next 3 lines inside loop for now until ManagedWorkspace mru-list works properly
    MantidVec &firstY = WS->dataY(firstIndex);
    MantidVec::iterator fYit;
    MantidVec::iterator fEit = firstSpectrum->dataE().begin();
    MantidVec::iterator Yit = spec->dataY().begin();
    MantidVec::iterator Eit = spec->dataE().begin();
    for (fYit = firstY.begin(); fYit != firstY.end(); ++fYit, ++fEit, ++Yit, ++Eit)
    {
      *fYit += *Yit;
      // Assume 'normal' (i.e. Gaussian) combination of errors
      *fEit = sqrt( (*fEit)*(*fEit) + (*Eit)*(*Eit) );
    }

    // Now zero the now redundant spectrum and set its spectraNo to indicate this (using -1)
    // N.B. Deleting spectra would cause issues for ManagedWorkspace2D, hence the the approach taken here
    spec->dataY().assign(vectorSize,0.0);
    spec->dataE().assign(vectorSize,0.0);
    spec->setSpectrumNo(-1);
    spec->clearDetectorIDs();
    progress.report();
  }
  g_log.information() << "Testing " << groupedMap->nElements() << "\n";

  // Replace the old map
  WS->generateSpectraMap();
//  WS->replaceSpectraMap(groupedMap);
}

} // namespace DataHandling
} // namespace Mantid

