#include "MantidAlgorithms/RebinToWorkspace.h"
#include "MantidAPI/HistogramValidator.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidDataObjects/EventWorkspace.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidDataObjects/WorkspaceCreation.h"
#include "MantidHistogramData/Rebin.h"

namespace Mantid {
namespace Algorithms {

using namespace API;
using DataObjects::EventWorkspace;
using DataObjects::EventWorkspace_const_sptr;

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(RebinToWorkspace)

//---------------------------
// Private Methods
//---------------------------
/**
 * Initialise the algorithm
 */
void RebinToWorkspace::init() {
  //  using namespace Mantid::DataObjects;
  declareProperty(Kernel::make_unique<WorkspaceProperty<>>(
                      "WorkspaceToRebin", "", Kernel::Direction::Input,
                      Kernel::make_unique<API::HistogramValidator>()),
                  "The workspace on which to perform the algorithm "
                  "This must be a Histogram workspace, not Point data. "
                  "If this is a problem try ConvertToHistogram.");
  declareProperty(Kernel::make_unique<WorkspaceProperty<>>(
                      "WorkspaceToMatch", "", Kernel::Direction::Input,
                      Kernel::make_unique<API::HistogramValidator>()),
                  "The workspace to match the bin boundaries against. "
                  "This must be a Histogram workspace, not Point data. "
                  "If this is a problem try ConvertToHistogram.");
  declareProperty(
      Kernel::make_unique<WorkspaceProperty<>>("OutputWorkspace", "",
                                               Kernel::Direction::Output),
      "The name of the workspace to be created as the output of the algorithm");
  declareProperty("PreserveEvents", true,
                  "Keep the output workspace as an EventWorkspace, if the "
                  "input has events (default). "
                  "If the input and output EventWorkspace names are the same, "
                  "only the X bins are set, which is very quick. "
                  "If false, then the workspace gets converted to a "
                  "Workspace2D histogram.");
}

namespace {
bool needToRebin(const MatrixWorkspace_sptr &left,
                 const MatrixWorkspace_sptr &rght) {
  // if pointers match they are the same object
  if (left == rght)
    return false;

  // see if there is the same number of histograms
  const size_t numHist = left->getNumberHistograms();
  if (numHist != rght->getNumberHistograms())
    return true;

  // look for first non-equal x-axis between the workspaces
  for (size_t i = 0; i < numHist; ++i) {
    if (left->getSpectrum(i).x() != rght->getSpectrum(i).x()) {
      return true;
    }
  }
  // everything must be the same
  return false;
}
} // namespace

/**
 * Execute the algorithm
 */
void RebinToWorkspace::exec() {
  // The input workspaces ...
  MatrixWorkspace_sptr toRebin = getProperty("WorkspaceToRebin");
  MatrixWorkspace_sptr toMatch = getProperty("WorkspaceToMatch");
  m_preserveEvents = getProperty("PreserveEvents");
  m_isEvents = bool(boost::dynamic_pointer_cast<const EventWorkspace>(toRebin));

  if (needToRebin(toRebin, toMatch)) {
    g_log.information("Rebinning");
    if (m_isEvents && (!m_preserveEvents)) {
      this->histogram(toRebin, toMatch);
    } else {
      this->rebin(toRebin, toMatch);
    }
  } else { // don't need to rebin
    g_log.information(
        "WorkspaceToRebin and WorkspaceToMatch already have matched binning");
    auto outputWS = this->createOutputWorkspace(toRebin, 1.);
    this->setProperty("OutputWorkspace", outputWS);
  }
}

MatrixWorkspace_sptr
RebinToWorkspace::createOutputWorkspace(MatrixWorkspace_sptr &inputWS,
                                        const double endProgress) {
  MatrixWorkspace_sptr outputWS = getProperty("OutputWorkspace");

  bool inPlace = (inputWS == outputWS);

  if (inPlace) {
    if (m_isEvents && (!m_preserveEvents)) {
      // convert to a MatrixWorkspace
      IAlgorithm_sptr convert =
          createChildAlgorithm("ConvertToMatrixWorkspace");
      convert->setProperty<MatrixWorkspace_sptr>("InputWorkspace", inputWS);
      convert->setProperty<MatrixWorkspace_sptr>("OutputWorkspace", outputWS);
      convert->setChildStartProgress(0.);
      convert->setChildStartProgress(endProgress);
      convert->executeAsChildAlg();
      outputWS = convert->getProperty("OutputWorkspace");
    }
    // all other cases are already handled by not doing anything
  } else {
    if (m_isEvents && (!m_preserveEvents)) {
      // convert to a MatrixWorkspace
      IAlgorithm_sptr convert =
          createChildAlgorithm("ConvertToMatrixWorkspace");
      convert->setProperty<MatrixWorkspace_sptr>("InputWorkspace", inputWS);
      convert->setProperty<MatrixWorkspace_sptr>("OutputWorkspace", outputWS);
      convert->setChildStartProgress(0.);
      convert->setChildStartProgress(endProgress);
      convert->executeAsChildAlg();
      outputWS = convert->getProperty("OutputWorkspace");
    } else {
      outputWS = inputWS->clone();
    }
  }

  return outputWS;
}

// this follows closely what Rebin does except each x-axis is different
void RebinToWorkspace::rebin(MatrixWorkspace_sptr &toRebin,
                             MatrixWorkspace_sptr &toMatch) {
  // create the output workspace
  auto outputWS = this->createOutputWorkspace(toRebin, .5);
  auto outputWSEvents = boost::dynamic_pointer_cast<EventWorkspace>(outputWS);

  const int numHist = static_cast<int>(toRebin->getNumberHistograms());
  Progress prog(this, 0.5, 1.0, numHist);

  // everything gets the same bin boundaries
  const bool matchingX = (toMatch->getNumberHistograms() == 1);

  // rebin
  PARALLEL_FOR_IF(Kernel::threadSafe(*toMatch, *outputWS))
  for (int i = 0; i < numHist; ++i) {
    PARALLEL_START_INTERUPT_REGION
    const auto &edges = matchingX ? toMatch->histogram(0).binEdges()
                                  : toMatch->histogram(i).binEdges();
    if (m_isEvents)
      outputWSEvents->getSpectrum(i).mutableX() = edges.rawData();
    else
      outputWS->setHistogram(
          i, HistogramData::rebin(toRebin->histogram(i), edges));
    prog.report();
    PARALLEL_END_INTERUPT_REGION
  }
  PARALLEL_CHECK_INTERUPT_REGION

  setProperty("OutputWorkspace", outputWS);
}

// handles the special case when binning from EventWorkspace to Workspace2D most
// efficiently by histogramming the events to the correct binning directly
void RebinToWorkspace::histogram(API::MatrixWorkspace_sptr &toRebin,
                                 API::MatrixWorkspace_sptr &toMatch) {
  const auto &inputWS =
      boost::dynamic_pointer_cast<const EventWorkspace>(toRebin);
  const int numHist = static_cast<int>(toRebin->getNumberHistograms());
  // everything gets the same bin boundaries
  const bool matchingX = (toMatch->getNumberHistograms() == 1);

  auto outputWS = DataObjects::create<API::HistoWorkspace>(*toRebin);
  Progress prog(this, 0.25, 1.0, numHist);

  // histogram
  PARALLEL_FOR_IF(Kernel::threadSafe(*toMatch, *outputWS))
  for (int i = 0; i < numHist; ++i) {
    PARALLEL_START_INTERUPT_REGION
    const auto &edges = matchingX ? toMatch->histogram(0).binEdges()
                                  : toMatch->histogram(i).binEdges();

    // TODO this should be in HistogramData/Rebin
    const auto &eventlist = inputWS->getSpectrum(i);
    MantidVec y_data(edges.size() - 1), e_data(edges.size() - 1);
    eventlist.generateHistogram(edges.rawData(), y_data, e_data);

    auto histo = HistogramData::Histogram(edges);
    histo.setCounts(y_data);
    histo.setCountStandardDeviations(e_data);

    outputWS->setHistogram(i, histo);
    prog.report();
    PARALLEL_END_INTERUPT_REGION
  }
  PARALLEL_CHECK_INTERUPT_REGION

  setProperty("OutputWorkspace", std::move(outputWS));
}

Parallel::ExecutionMode RebinToWorkspace::getParallelExecutionMode(
    const std::map<std::string, Parallel::StorageMode> &storageModes) const {
  // Probably we can relax these restrictions based on particular combination
  // with storage mode of WorkspaceToRebin, but this is simple and sufficient
  // for now.
  if (storageModes.at("WorkspaceToMatch") != Parallel::StorageMode::Cloned)
    throw std::runtime_error("WorkspaceToMatch must have " +
                             Parallel::toString(Parallel::StorageMode::Cloned));
  return Parallel::getCorrespondingExecutionMode(
      storageModes.at("WorkspaceToRebin"));
}

} // namespace Algorithms
} // namespace Mantid
