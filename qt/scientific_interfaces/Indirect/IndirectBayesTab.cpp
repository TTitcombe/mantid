// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "IndirectBayesTab.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidQtWidgets/Common/UserSubWindow.h"
#include "MantidQtWidgets/LegacyQwt/QwtWorkspaceSpectrumData.h"

namespace MantidQt {
namespace CustomInterfaces {

IndirectBayesTab::IndirectBayesTab(QWidget *parent)
    : IndirectTab(parent), m_propTree(new QtTreePropertyBrowser()) {
  m_propTree->setFactoryForManager(m_dblManager, m_dblEdFac);

  connect(m_dblManager, SIGNAL(valueChanged(QtProperty *, double)), this,
          SLOT(updateProperties(QtProperty *, double)));
}

IndirectBayesTab::~IndirectBayesTab() {}

/**
 * Prevents the loading of data with incorrect naming if passed true
 *
 * @param filter :: true if you want to allow filtering
 */
void IndirectBayesTab::filterInputData(bool filter) {
  setFileExtensionsByName(filter);
}

/**
 * Allows the user to turn the plotting of error bars off and on
 *
 * @param errorBars :: true if you want output plots to have error bars
 */
void IndirectBayesTab::setPlotErrorBars(bool errorBars) {
  IndirectTab::setPlotErrorBars(errorBars);
}

/**
 * Emits a signal to run a python script using the method in the parent
 * UserSubWindow
 *
 * @param pyInput :: A string of python code to execute
 */
void IndirectBayesTab::runPythonScript(const QString &pyInput) {
  emit runAsPythonScript(pyInput, true);
}

} // namespace CustomInterfaces
} // namespace MantidQt
