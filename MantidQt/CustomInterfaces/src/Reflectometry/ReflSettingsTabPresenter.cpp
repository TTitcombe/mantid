#include "MantidQtCustomInterfaces/Reflectometry/ReflSettingsTabPresenter.h"
#include "MantidQtCustomInterfaces/Reflectometry/IReflMainWindowPresenter.h"
#include "MantidQtCustomInterfaces/Reflectometry/IReflSettingsTabView.h"
#include "MantidQtMantidWidgets/AlgorithmHintStrategy.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidGeometry/Instrument.h"

#include <boost/algorithm/string.hpp>

namespace MantidQt {
namespace CustomInterfaces {

using namespace Mantid::API;
using namespace MantidQt::MantidWidgets;

/** Constructor
* @param view :: The view we are handling
*/
ReflSettingsTabPresenter::ReflSettingsTabPresenter(IReflSettingsTabView *view)
    : m_view(view), m_mainPresenter() {

  // Create the 'HintingLineEdits'
  createStitchHints();
}

/** Destructor
*/
ReflSettingsTabPresenter::~ReflSettingsTabPresenter() {}

/** Accept a main presenter
* @param mainPresenter :: [input] The main presenter
*/
void ReflSettingsTabPresenter::acceptMainPresenter(
    IReflMainWindowPresenter *mainPresenter) {
  m_mainPresenter = mainPresenter;
}

/** Used by the view to tell the presenter something has changed
*/
void ReflSettingsTabPresenter::notify(IReflSettingsTabPresenter::Flag flag) {
  switch (flag) {
  case IReflSettingsTabPresenter::ExpDefaultsFlag:
    getExpDefaults();
    break;
  case IReflSettingsTabPresenter::InstDefaultsFlag:
    getInstDefaults();
    break;
  }
  // Not having a 'default' case is deliberate. gcc issues a warning if there's
  // a flag we aren't handling.
}

/** Returns global options for 'CreateTransmissionWorkspaceAuto'
* @return :: Global options for 'CreateTransmissionWorkspaceAuto'
*/
std::string ReflSettingsTabPresenter::getTransmissionOptions() const {

  std::vector<std::string> options;

  // Add analysis mode
  auto analysisMode = m_view->getAnalysisMode();
  if (!analysisMode.empty())
    options.push_back("AnalysisMode=" + analysisMode);

  // Add monitor integral min
  auto monIntMin = m_view->getMonitorIntegralMin();
  if (!monIntMin.empty())
    options.push_back("MonitorIntegrationWavelengthMin=" + monIntMin);

  // Add monitor integral max
  auto monIntMax = m_view->getMonitorIntegralMax();
  if (!monIntMax.empty())
    options.push_back("MonitorIntegrationWavelengthMax=" + monIntMax);

  // Add monitor background min
  auto monBgMin = m_view->getMonitorBackgroundMin();
  if (!monBgMin.empty())
    options.push_back("MonitorBackgroundWavelengthMin=" + monBgMin);

  // Add monitor background max
  auto monBgMax = m_view->getMonitorBackgroundMax();
  if (!monBgMax.empty())
    options.push_back("MonitorBackgroundWavelengthMax=" + monBgMax);

  // Add lambda min
  auto lamMin = m_view->getLambdaMin();
  if (!lamMin.empty())
    options.push_back("WavelengthMin=" + lamMin);

  // Add lambda max
  auto lamMax = m_view->getLambdaMax();
  if (!lamMax.empty())
    options.push_back("WavelengthMax=" + lamMax);

  // Add I0MonitorIndex
  auto I0MonitorIndex = m_view->getI0MonitorIndex();
  if (!I0MonitorIndex.empty())
    options.push_back("I0MonitorIndex=" + I0MonitorIndex);

  // Add detector limits
  auto procInst = m_view->getProcessingInstructions();
  if (!procInst.empty())
    options.push_back("ProcessingInstructions=" + procInst);

  return boost::algorithm::join(options, ",");
}

/** Returns global options for 'ReflectometryReductionOneAuto'
* @return :: Global options for 'ReflectometryReductionOneAuto'
*/
std::string ReflSettingsTabPresenter::getReductionOptions() const {

  std::vector<std::string> options;

  // Add analysis mode
  auto analysisMode = m_view->getAnalysisMode();
  if (!analysisMode.empty())
    options.push_back("AnalysisMode=" + analysisMode);

  // Add CRho
  auto crho = m_view->getCRho();
  if (!crho.empty())
    options.push_back("CRho=" + crho);

  // Add CAlpha
  auto calpha = m_view->getCAlpha();
  if (!calpha.empty())
    options.push_back("CAlpha=" + calpha);

  // Add CAp
  auto cap = m_view->getCAp();
  if (!cap.empty())
    options.push_back("CAp=" + cap);

  // Add CPp
  auto cpp = m_view->getCPp();
  if (!cpp.empty())
    options.push_back("CPp=" + cpp);

  // Add direct beam
  auto dbnr = m_view->getDirectBeam();
  if (!dbnr.empty())
    options.push_back("RegionOfDirectBeam=" + dbnr);

  // Add polarisation corrections
  auto polCorr = m_view->getPolarisationCorrections();
  if (!polCorr.empty())
    options.push_back("PolarizationAnalysis=" + polCorr);

  // Add integrated monitors option
  auto intMonCheck = m_view->getIntMonCheck();
  if (!intMonCheck.empty())
    options.push_back("NormalizeByIntegratedMonitors=" + intMonCheck);

  // Add monitor integral min
  auto monIntMin = m_view->getMonitorIntegralMin();
  if (!monIntMin.empty())
    options.push_back("MonitorIntegrationWavelengthMin=" + monIntMin);

  // Add monitor integral max
  auto monIntMax = m_view->getMonitorIntegralMax();
  if (!monIntMax.empty())
    options.push_back("MonitorIntegrationWavelengthMax=" + monIntMax);

  // Add monitor background min
  auto monBgMin = m_view->getMonitorBackgroundMin();
  if (!monBgMin.empty())
    options.push_back("MonitorBackgroundWavelengthMin=" + monBgMin);

  // Add monitor background max
  auto monBgMax = m_view->getMonitorBackgroundMax();
  if (!monBgMax.empty())
    options.push_back("MonitorBackgroundWavelengthMax=" + monBgMax);

  // Add lambda min
  auto lamMin = m_view->getLambdaMin();
  if (!lamMin.empty())
    options.push_back("WavelengthMin=" + lamMin);

  // Add lambda max
  auto lamMax = m_view->getLambdaMax();
  if (!lamMax.empty())
    options.push_back("WavelengthMax=" + lamMax);

  // Add I0MonitorIndex
  auto I0MonitorIndex = m_view->getI0MonitorIndex();
  if (!I0MonitorIndex.empty())
    options.push_back("I0MonitorIndex=" + I0MonitorIndex);

  // Add scale factor
  auto scaleFactor = m_view->getScaleFactor();
  if (!scaleFactor.empty())
    options.push_back("ScaleFactor=" + scaleFactor);

  // Add momentum transfer limits
  auto qTransStep = m_view->getMomentumTransferStep();
  if (!qTransStep.empty()) {
    options.push_back("MomentumTransferStep=" + qTransStep);
  }

  // Add detector limits
  auto procInst = m_view->getProcessingInstructions();
  if (!procInst.empty())
    options.push_back("ProcessingInstructions=" + procInst);

  return boost::algorithm::join(options, ",");
}

/** Returns global options for 'Stitch1DMany'
* @return :: Global options for 'Stitch1DMany'
*/
std::string ReflSettingsTabPresenter::getStitchOptions() const {

  return m_view->getStitchOptions();
}

/** Creates hints for 'Stitch1DMany'
*/
void ReflSettingsTabPresenter::createStitchHints() {

  // The algorithm
  IAlgorithm_sptr alg = AlgorithmManager::Instance().create("Stitch1DMany");
  // The blacklist
  std::set<std::string> blacklist = {"InputWorkspaces", "OutputWorkspace",
                                     "OutputWorkspace"};
  AlgorithmHintStrategy strategy(alg, blacklist);

  m_view->createStitchHints(strategy.createHints());
}

/** Fills experiment settings with default values
*/
void ReflSettingsTabPresenter::getExpDefaults() {
  // The algorithm
  IAlgorithm_sptr alg =
      AlgorithmManager::Instance().create("ReflectometryReductionOneAuto");

  // Collect all default values and set them in view
  std::vector<std::string> defaults;
  defaults.push_back(alg->getPropertyValue("AnalysisMode"));
  defaults.push_back(alg->getPropertyValue("PolarizationAnalysis"));
  defaults.push_back(alg->getPropertyValue("ScaleFactor"));
  m_view->setExpDefaults(defaults);
}

/** Fills instrument settings with default values
*/
void ReflSettingsTabPresenter::getInstDefaults() {

  // The instrument
  IAlgorithm_sptr loadInst =
      AlgorithmManager::Instance().create("LoadEmptyInstrument");
  loadInst->setChild(true);
  loadInst->setProperty("OutputWorkspace", "outWs");
  loadInst->setProperty("InstrumentName", m_mainPresenter->getInstrumentName());
  loadInst->execute();
  MatrixWorkspace_const_sptr ws = loadInst->getProperty("OutputWorkspace");
  auto inst = ws->getInstrument();

  // Collect all default values
  std::vector<double> defaults;
  defaults.push_back(inst->getNumberParameter("MonitorIntegralMin")[0]);
  defaults.push_back(inst->getNumberParameter("MonitorIntegralMax")[0]);
  defaults.push_back(inst->getNumberParameter("MonitorBackgroundMin")[0]);
  defaults.push_back(inst->getNumberParameter("MonitorBackgroundMax")[0]);
  defaults.push_back(inst->getNumberParameter("LambdaMin")[0]);
  defaults.push_back(inst->getNumberParameter("LambdaMax")[0]);
  defaults.push_back(inst->getNumberParameter("I0MonitorIndex")[0]);

  m_view->setInstDefaults(defaults);
}
}
}