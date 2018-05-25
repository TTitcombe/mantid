//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/MuonAsymmetryHelper.h"
#include "MantidAPI/IFunction.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/Progress.h"
#include "MantidAPI/Run.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/Workspace_fwd.h"


#include "MantidAPI/TableRow.h"
#include "MantidAPI/AnalysisDataService.h"


#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/PhysicalConstants.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include <vector>

namespace {
/// Number of microseconds in one second (10^6)
constexpr double MICROSECONDS_PER_SECOND{1000000.0};
/// Muon lifetime in microseconds
constexpr double MUON_LIFETIME_MICROSECONDS{
    Mantid::PhysicalConstants::MuonLifetime * MICROSECONDS_PER_SECOND};
}

namespace Mantid {
using namespace Kernel;
using std::size_t;

/**
 * Corrects the data and errors for one spectrum.
 * The muon lifetime is in microseconds, not seconds, because the data is in
 * microseconds.
 * @param histogram :: [input] Input histogram
 * @param numGoodFrames :: [input] the number of good frames
 * @returns :: Histogram of the normalised counts
 */
HistogramData::Histogram
normaliseCounts(const HistogramData::Histogram &histogram,
                const double numGoodFrames) {
  HistogramData::Histogram result(histogram);
  auto &yData = result.mutableY();
  auto &eData = result.mutableE();
  for (size_t i = 0; i < yData.size(); ++i) {
    const double factor = exp(result.x()[i] / MUON_LIFETIME_MICROSECONDS);
    // Correct the Y data
    if (yData[i] != 0.0) {
      yData[i] *= factor / numGoodFrames;
    } else {
      yData[i] = 0.1 * factor / numGoodFrames;
    }

    // Correct the E data
    if (eData[i] != 0.0) {
      eData[i] *= factor / numGoodFrames;
    } else {
      eData[i] = factor / numGoodFrames;
    }
  }

  return result;
}
/**
* Estimates normalisation constant via
* N_0 = (Delta/f)*(sum_i W_i)/(int_a^b exp(-t/tau)dt )
* where W is the raw data, tau is the muon
* lifetime, t is time, f is the
* number of good frames Delta is the time step,
* a is the start of the range and b is the end of the range.
* @param histogram :: [input] Input histogram
* @param numGoodFrames :: [input] the number of good frames
* @param  startX :: [input] the start time
* @param  endX :: [input] the end time
* @returns :: The normalization constant N_0
*/
double estimateNormalisationConst(const HistogramData::Histogram &histogram,
                                  const double numGoodFrames,
                                  const double startX, const double endX) {

  auto &&xData = histogram.binEdges();
  auto &&yData = histogram.y();

  size_t i0 = startIndexFromTime(xData, startX);
  size_t iN = endIndexFromTime(xData, endX);
  // remove an extra index as XData is bin boundaries and not point data
  auto iy0 = std::next(yData.rawData().begin(), i0);
  auto iyN = std::next(yData.rawData().begin(), iN);
  double summation = std::accumulate(iy0, iyN, 0.0);
  double denominator = 0.0;
  /* this replaces (from doc):
        delta_t/tau(exp(-t_N/tau) - exp(-t_0/tau))
     with trapezium rule (convert it to integral first):
        1/(sum_{j=0}{N} exp(-t_j/tau)) - 0.5*(exp(-t_0/tau)+exp(-t_N/tau))
  */
  for (size_t k = i0; k < iN; k++) {
    denominator += exp(-xData[k] / MUON_LIFETIME_MICROSECONDS);
  }
  denominator -= 0.5 * (exp(-xData[i0] / MUON_LIFETIME_MICROSECONDS));
  denominator -= 0.5 * (exp(-xData[iN] / MUON_LIFETIME_MICROSECONDS));
  return summation / (denominator * numGoodFrames);
}
/**
* Finds the first index in bin edges that is after
* the start time.
* @param xData :: [input] Input HistogramData as bin edges
* @param startX :: [input] the start time
* @returns :: The index to start calculations from
*/
size_t startIndexFromTime(const HistogramData::BinEdges &xData,
                          const double startX) {
  auto upper =
      std::upper_bound(xData.rawData().begin(), xData.rawData().end(), startX);
  return std::distance(xData.rawData().begin(), upper);
}
/**
* find the first index in bin edges that is after
* the endtime.
* @param xData :: [input] HistogramData as bin edges
* @param endX :: [input] the end time
* @returns :: The last index to  include in calculations
*/
size_t endIndexFromTime(const HistogramData::BinEdges &xData,
                        const double endX) {
  auto lower =
      std::upper_bound(xData.rawData().begin(), xData.rawData().end(), endX);
  return std::distance(xData.rawData().begin(), lower - 1);
}

/*****
 The following functions are for manipulating the normalisation table
******/
/**
* Updates the normalization in the table WS
* assumes that the change is due to a calculation
* @param norms :: map of updated normalization values
*/
/*void updateNormalizationTable(std::map<std::string, double> &norms) {
	auto oldNorm = readMultipleNormalization();
	Mantid::API::ITableWorkspace_sptr table = Mantid::API::WorkspaceFactory::Instance().createTable();
	Mantid::API::AnalysisDataService::Instance().addOrReplace("MuonAnalysisTFNormalizations",
		table);
	table->addColumn("double", "norm");
	table->addColumn("str", "name");
	table->addColumn("str", "method");

	for (auto norm : oldNorm) {
		Mantid::API::TableRow row = table->appendRow();
		auto it = norms.find(std::get<0>(norm));
		if (it != norms.end() && it->second != std::get<1>(norm)) {
			// write new norm
			row << it->second << std::get<0>(norm) << "Calculated";
		}
		else {
			// write old norm
			row << std::get<1>(norm) << std::get<0>(norm) << "Estimated";
		}
	}
}*/

Mantid::API::ITableWorkspace_sptr updateNormalizationTable(const std::string &tableName, const std::vector<std::string> &wsNames, const std::vector<double> &norms, const std::vector<std::string> &methods){
//check if table exists, if not make it
	Mantid::API::ITableWorkspace_sptr table =boost::dynamic_pointer_cast<Mantid::API::ITableWorkspace>(
			Mantid::API::AnalysisDataService::Instance().retrieve(
				tableName));
	
	for (size_t j = 0; j < wsNames.size(); j++) {
		bool updated = false;
		for (size_t row = 0; row < table->rowCount(); row++) {

			if (table->String(row, 1) == wsNames[j]) {
				table->removeRow(row);
				table->insertRow(row);
				Mantid::API::TableRow tableRow = table->getRow(row);
				tableRow << static_cast<double>(norms[j]) << wsNames[j] << methods[j];
				updated = true;
			}
		}
		if (!updated) {
			Mantid::API::TableRow tableRow = table->appendRow();
			tableRow << static_cast<double>(norms[j]) << wsNames[j] << methods[j];
		}
	}
	return table;
}



/** Reads the normalization constants and which WS
* they belong to
* @returns :: A map of normalization constants and WS names
*/
std::map<std::string, double> readMultipleNormalization() {
	std::map<std::string, double> norm;
	if (Mantid::API::AnalysisDataService::Instance().doesExist(
		"MuonAnalysisTFNormalizations")) {
		Mantid::API::ITableWorkspace_sptr table =
			boost::dynamic_pointer_cast<Mantid::API::ITableWorkspace>(
				Mantid::API::AnalysisDataService::Instance().retrieve(
					"MuonAnalysisTFNormalizations"));
		auto colNorm = table->getColumn("norm");
		auto colName = table->getColumn("name");
		for (size_t j = 0; j < table->rowCount(); j++) {
			norm[colName->cell<std::string>(j)] = ((*colNorm)[j]); // read norm
		}
	}
	return norm;
}

} // namespace Mantid
