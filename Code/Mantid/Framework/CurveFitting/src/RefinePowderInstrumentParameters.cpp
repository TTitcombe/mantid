#include "MantidCurveFitting/RefinePowderInstrumentParameters.h"

#include "MantidKernel/ListValidator.h"
#include "MantidAPI/TableRow.h"
#include "MantidAPI/FunctionDomain1D.h"
#include "MantidAPI/FunctionValues.h"
#include "MantidAPI/ParameterTie.h"
#include "MantidCurveFitting/Fit.h"
#include "MantidAPI/IFunction.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidCurveFitting/BackgroundFunction.h"
#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/TextAxis.h"
#include "MantidCurveFitting/ThermalNeutronDtoTOFFunction.h"
#include "MantidCurveFitting/Polynomial.h"
#include "MantidAPI/ConstraintFactory.h"
#include "MantidCurveFitting/BoundaryConstraint.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace Mantid;
using namespace Mantid::API;
using namespace Mantid::Kernel;

#define PEAKRANGEFACTOR 20.0

namespace Mantid
{
namespace CurveFitting
{

DECLARE_ALGORITHM(RefinePowderInstrumentParameters)

  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  RefinePowderInstrumentParameters::RefinePowderInstrumentParameters()
  {
  }

  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  RefinePowderInstrumentParameters::~RefinePowderInstrumentParameters()
  {
  }

  void RefinePowderInstrumentParameters::initDocs()
  {

  }


  /** Parameter declaration
   */
  void RefinePowderInstrumentParameters::init()
  {
      // Input data workspace
      declareProperty(new API::WorkspaceProperty<API::MatrixWorkspace>("InputWorkspace", "Anonymous", Direction::Input),
                      "Input workspace for data");

      // Output workspace
      declareProperty(new API::WorkspaceProperty<DataObjects::Workspace2D>("OutputWorkspace", "", Direction::Output),
                      "Output Workspace2D for the fitted d-TOF curves. ");

      // Input/output peaks table workspace
      declareProperty(new API::WorkspaceProperty<DataObjects::TableWorkspace>("PeaksParametersWorkspace", "", Direction::Input),
                      "TableWorkspace containg all peaks' parameters.");

      // Input and output instrument parameters table workspace
      declareProperty(new API::WorkspaceProperty<DataObjects::TableWorkspace>("InstrumentParametersWorkspace", "", Direction::InOut),
                      "TableWorkspace containg instrument's parameters.");

      // Parameters to fit
      declareProperty(new Kernel::ArrayProperty<std::string>("ParametersToFit"), "Names of the parameters to fit. ");

      // Workspace to plot pattern
      declareProperty(new API::WorkspaceProperty<DataObjects::Workspace2D>("OutputDataWorkspace", "", Direction::Output),
                      "Output Workspace2D for the fitted data (peaks). ");

      // Workspace index of the
      declareProperty("WorkspaceIndex", 0, "Worskpace index for the data to refine against.");

      return;
  }

  /** Main execution
   */
  void RefinePowderInstrumentParameters::exec()
  {
      // 1. Get input
      dataWS = this->getProperty("InputWorkspace");
      int workspaceindex = this->getProperty("WorkspaceIndex");

      DataObjects::TableWorkspace_sptr peakWS = this->getProperty("PeaksParametersWorkspace");

      DataObjects::TableWorkspace_sptr parameterWS = this->getProperty("InstrumentParametersWorkspace");

      // 2. Parse
      generatePeaksFromInput(peakWS, mPeaks);
      importParametersFromTable(parameterWS, mFuncParameters);
      mInputFuncParameters = mFuncParameters;

      // 3. Fit peaks & get peak centers
      std::vector<std::vector<int> > goodfitpeaks;
      std::vector<double> goodfitchi2;
      fitPeaks(workspaceindex, goodfitpeaks, goodfitchi2);

      if (goodfitpeaks.size() <= 2)
      {
          std::stringstream errmsg;
          errmsg << "There are too few peaks (" << goodfitpeaks.size() << " peaks) that have good fitting. "
                 << "Unable to construct a workspace to fit for instrument geometry parameters. ";
          g_log.error(errmsg.str());
          throw std::runtime_error(errmsg.str());
      }

      genPeakCentersWorkspace(mPeaks, goodfitpeaks, goodfitchi2);

      // 4. Fit instrument geometry function
      fitInstrumentParameters();

      // 5. Set output workspace
      this->setProperty("OutputWorkspace", outWS);

      // 6. Set peak data output workspace
      DataObjects::Workspace2D_sptr peakdataws = createPeakDataWorkspace(workspaceindex);

      this->setProperty("OutputDataWorkspace", peakdataws);

      // 7. Output new instrument parameters
      DataObjects::TableWorkspace rawtable;
      DataObjects::TableWorkspace_sptr newtablews = boost::make_shared<DataObjects::TableWorkspace>(rawtable);
      newtablews->addColumn("str", "Name");
      newtablews->addColumn("double", "Value");
      newtablews->addColumn("str", "FitOrTie");

      std::map<std::string, double>::iterator pariter;
      for (pariter = mFuncParameters.begin(); pariter != mFuncParameters.end(); ++pariter)
      {
          API::TableRow newrow = newtablews->appendRow();
          std::string parname = pariter->first;
          double parvalue = pariter->second;
          newrow << parname << parvalue;
      }

      this->setProperty("InstrumentParametersWorkspace", newtablews);

      /*
      for (size_t i = 0; i < peakdataws->readX(0).size(); ++i)
      {
          std::cout << "++ " << peakdataws->readX(0)[i] << "\t\t" << peakdataws->readY(0)[i]
                    << "\t\t" << peakdataws->readY(1)[i]  << "\t\t" << peakdataws->readY(2)[i] << std::endl;
      }
      */

      return;
  }

  /** Create a workspace2D containing the peak patterns, including
      (1) input data (2) fitted peak (single peak fitting) (3) difference
   */
  DataObjects::Workspace2D_sptr RefinePowderInstrumentParameters::createPeakDataWorkspace(size_t workspaceindex)
  {
      // 1. Get some information about the workspace to create
      std::map<int, std::pair<std::vector<size_t>, API::FunctionValues> >::iterator diter;
      std::vector<int> hkl2s;
      size_t wssize  = 0;
      for (diter = mPeakData.begin(); diter != mPeakData.end(); ++diter)
      {
          int hkl2 = diter->first;
          hkl2s.push_back(hkl2);

          wssize += diter->second.first.size();
      }
      g_log.debug() << "Workspace X-size = " << wssize << std::endl;

      // 2. Create workspace
      DataObjects::Workspace2D_sptr peakdataws =
              boost::dynamic_pointer_cast<DataObjects::Workspace2D>
              (API::WorkspaceFactory::Instance().create("Workspace2D", 3, wssize, wssize));

      // 3. Set data
      std::sort(hkl2s.begin(), hkl2s.end());

      size_t index = 0;
      for (int i = int(hkl2s.size())-1; i >= 0; --i)
      {
          std::vector<size_t> itofs = mPeakData[hkl2s[i]].first;
          API::FunctionValues values = mPeakData[hkl2s[i]].second;

          for (size_t it = 0; it < itofs.size(); ++it)
          {
              // For X-axis
              size_t idata = itofs[it];
              size_t curindex = index+it;

              for (size_t wsindex = 0; wsindex < 3; ++wsindex)
              {
                  peakdataws->dataX(wsindex)[curindex] = dataWS->readX(workspaceindex)[idata];
              }

              // For Y[0]
              peakdataws->dataY(0)[curindex] = dataWS->readY(workspaceindex)[idata];

              // For Y[1]
              peakdataws->dataY(1)[curindex] = values[it];

              // For Y[2]
              peakdataws->dataY(2)[curindex] = peakdataws->dataY(0)[curindex] - peakdataws->dataY(1)[curindex];
          }

          index += itofs.size();
      }

      return peakdataws;

  }

  /** Fit each individual Bk2Bk-Exp-Conv-PV peaks
    * This part is under heavy construction, and will be applied to "FindPeaks2"
   */
  void RefinePowderInstrumentParameters::fitPeaks(int workspaceindex, std::vector<std::vector<int> >& goodfitpeaks,
                                                  std::vector<double>& goodfitchi2)
  {
      g_log.debug() << "Fit Every Individual Peak. " << std::endl;

      // 1. Clear the output vector
      goodfitpeaks.clear();

      // 2. Fit all peaks
      std::map<std::vector<int>, CurveFitting::Bk2BkExpConvPV_sptr>::iterator peakiter;

      double prog = 0.0;
      double deltaprog = 0.8/double(mPeaks.size());

      for (peakiter = mPeaks.begin(); peakiter != mPeaks.end(); ++peakiter)
      {
          // a) Set up composite function including peak and background
          // Peak
          std::vector<int> hkl = peakiter->first;
          CurveFitting::Bk2BkExpConvPV_sptr peak = peakiter->second;

          std::cout << std::endl << "-----------   DBx315 Fit Peak @ " << peak->centre() <<
                       " ------------------" << std::endl;

          // Background (Polynomial)
          CurveFitting::Polynomial_sptr background = boost::make_shared<CurveFitting::Polynomial>
                  (CurveFitting::Polynomial());
          background->setAttributeValue("n", 2);
          background->initialize();

          // Composite function
          API::CompositeFunction tempfunc;
          API::CompositeFunction_sptr compfunction = boost::make_shared<API::CompositeFunction>
                  (tempfunc);
          compfunction->addFunction(peak);
          compfunction->addFunction(background);

          // b) Determine the range of fitting: assuming the input is not far off
          double fwhm = peak->fwhm();
          double leftdev = PEAKRANGEFACTOR * fwhm * 0.5;
          double rightdev = PEAKRANGEFACTOR * fwhm * 0.5;

          double chi2;
          std::string fitstatus;
          bool fitpeakgood = this->fitSinglePeak(compfunction, peak, background, leftdev, rightdev, workspaceindex, prog, deltaprog,
                                                 chi2, fitstatus);

          // c) Work with the result of fitting
          g_log.information() << "Fitting result: " << "  chi2 = " << chi2 << std::endl;
          std::vector<std::string> peakparamnames = peak->getParameterNames();
          for (size_t im = 0; im < peakparamnames.size(); ++im)
          {
              std::string parname = peakparamnames[im];
              g_log.information() << "Parameter " << parname << " = " << peak->getParameter(parname) << std::endl;
          }

          if (!fitpeakgood || fitstatus.compare("success") || chi2 > 200.0)
          {
              // If not a good fit, ignore
              g_log.warning() << "Peak @ " << peak->getParameter("TOF_h") << " is not selected due to one of the following reason. " << std::endl
                              << "(1) Fit status = " << fitstatus << std::endl
                              << "(2) Chi2 = " << chi2 << "  > 200.0" << std::endl
                              << "(3) Fit peak good = " << fitpeakgood << std::endl;

              continue;
          }

          goodfitpeaks.push_back(peakiter->first);
          goodfitchi2.push_back(chi2);

          // d) For output
          fwhm = peak->fwhm();
          double tof_h = peak->centre();
          double leftbound = tof_h - PEAKRANGEFACTOR * fwhm;
          double rightbound = tof_h + PEAKRANGEFACTOR * fwhm;

          std::vector<double> tofs;
          std::vector<size_t> itofs;
          std::vector<double>::const_iterator vit;
          vit = std::lower_bound(dataWS->readX(workspaceindex).begin(), dataWS->readX(workspaceindex).end(), leftbound);
          size_t istart = size_t(vit-dataWS->readX(workspaceindex).begin());
          vit = std::lower_bound(dataWS->readX(workspaceindex).begin(), dataWS->readX(workspaceindex).end(), rightbound);
          size_t iend = size_t(vit-dataWS->readX(workspaceindex).begin());
          for (size_t i = istart; i < iend; ++i)
          {
              itofs.push_back(i);
              tofs.push_back(dataWS->readX(workspaceindex)[i]);
          }

          API::FunctionDomain1DVector domain(tofs);
          API::FunctionValues values(domain);
          compfunction->function(domain, values);

          int hkl2 = hkl[0]*hkl[0] + hkl[1]*hkl[1] + hkl[2]*hkl[2];

          mPeakData.insert(std::make_pair(hkl2, std::make_pair(itofs, values)));

      } // FOR EACH PEAK

      // 3. Create a new matrix workspace


      return;
  }


  /** Generate a workspace2D for peak parameters such that
    * X(0)[i] = d_h[i], Y(0)[i] = Alpha[i]; X(1)[i] = d_h[i], Y(1) = Beta[i]; ...
    */
void RefinePowderInstrumentParameters::generateOutputPeakParameterWorkspace(std::vector<std::vector<int> > goodfitpeaks)
{
    // 1. Calculate d-values (with whatever parmeters)
    std::vector<std::pair<double, std::vector<int> > > dhkls;
    std::vector<std::vector<int> >::iterator peakiter;
    for (peakiter = goodfitpeaks.begin(); peakiter != goodfitpeaks.end(); ++peakiter)
    {
        std::vector<int> hkl = *peakiter;
        double d_h = calculateDspaceValue(hkl);
        dhkls.push_back(std::make_pair(d_h, hkl));
    }

    // 2. Sort
    std::sort(dhkls.begin(), dhkls.end());

    // 3. Create otuput workspace
    throw std::runtime_error("You should have peak parameters names. ");
    size_t nspec = mPeakParameterNames.size();
    size_t datalen = goodfitpeaks.size();

    DataObjects::Workspace2D_sptr peakparamws
            = boost::dynamic_pointer_cast<DataObjects::Workspace2D>
            (API::WorkspaceFactory::Instance().create("Workspace2D", nspec, datalen, datalen));

    throw std::runtime_error("Implement add labels to spectrum according to LeBailFit.");

    // 4. Insert data
    std::vector<std::string>::iterator paramiter;
    for (size_t i = 0; i < dhkls.size(); ++i)
    {
        CurveFitting::Bk2BkExpConvPV_sptr peak = mPeaks[dhkls[i].second];
        for (size_t j = 0; j < mPeakParameterNames.size(); ++j)
        {
            std::string param = mPeakParameterNames[j];
            double parvalue = peak->getParameter(param);
            peakparamws->dataX(j)[i] = dhkls[i].first;
            peakparamws->dataY(j)[i] = parvalue;
        }
    }

    return;
}


  /** Fit a single peak
    * [NEW CODE: Under Construction Still]
    * Return: chi2 ... all the other parameter should be just in peak
    */
  bool RefinePowderInstrumentParameters::fitSinglePeak(API::CompositeFunction_sptr compfunction, CurveFitting::Bk2BkExpConvPV_sptr peak,
                                                       CurveFitting::BackgroundFunction_sptr background, double leftdev, double rightdev,
                                                       size_t workspaceindex, double prog, double deltaprog,
                                                       double& chi2, std::string& fitstatus)
  {
      // 1. Find the observed peak center
      double tof_h_inp = peak->centre();

      std::cout << "Find peak in range (" << tof_h_inp-leftdev << ", " <<
                   tof_h_inp+rightdev << ")" << std::endl;

      double tof_h_obs, tof_left, tof_right;
      findMaxHeight(dataWS, workspaceindex, tof_h_inp-leftdev, tof_h_inp+rightdev,
                    tof_h_obs, tof_left, tof_right);

      std::cout << "Found peak @ " << tof_h_obs << "  +/- " << tof_left << ", " << tof_right << std::endl;

      // 2) Find local background
      /*
      CurveFitting::Polynomial_sptr background = boost::make_shared<CurveFitting::Polynomial>
              (CurveFitting::Polynomial());
      background->setAttributeValue("n", 2);
      background->initialize();
      */

      double fwhm_inp = peak->fwhm();
      double leftbound = tof_h_obs - PEAKRANGEFACTOR * fwhm_inp * 0.5;
      double rightbound = tof_h_obs + PEAKRANGEFACTOR * fwhm_inp * 0.5;
      bool goodbkgdfit = fitLinearBackground(peak, workspaceindex, background, leftbound, rightbound);

      // 3. Construct the composite function to fit the peak
      /*
      API::CompositeFunction tempfunc;
      API::CompositeFunction_sptr compfunction = boost::make_shared<API::CompositeFunction>
              (tempfunc);
      compfunction->addFunction(peak);
      compfunction->addFunction(background);
      */

      // 4. Set up parameter
      peak->setParameter("TOF_h", tof_h_obs);
      CurveFitting::BoundaryConstraint* tofbound =
              new CurveFitting::BoundaryConstraint
              (peak.get(),"TOF_h", tof_left, tof_right, false);
      peak->addConstraint(tofbound);

      // FIXME:  This is not right!  Release this later!
      peak->tie("Gamma", "0.0");

      if (goodbkgdfit)
      {
          std::vector<std::string> bkgdparnames;
          bkgdparnames = background->getParameterNames();
          for (size_t ipm = 0; ipm < bkgdparnames.size(); ++ipm)
          {
              std::string paramname = bkgdparnames[ipm];
              double parvalue = background->getParameter(paramname);
              std::stringstream ss;
              ss << parvalue;
              background->tie(paramname, ss.str());

              g_log.debug() << "Tie Background Parameter " << paramname << " = " << parvalue << std::endl;
          }
      }

      // 5. Redefine peak fit range
      leftbound = tof_h_obs - 4.0*fwhm_inp;
      rightbound = tof_h_obs + 4.0*fwhm_inp;

      g_log.information() << "Single peak fitting range: Left bound = " << leftbound << "  Right bound = "
                    << rightbound << std::endl;

      // 6) Fit peak
      g_log.information() << "FitSinglePeak + Background Function: " << compfunction->asString() << std::endl;

      API::IAlgorithm_sptr fitalg = createSubAlgorithm("Fit", prog, prog+deltaprog, true);
      prog += deltaprog;
      fitalg->initialize();

      fitalg->setProperty("Function", boost::dynamic_pointer_cast<API::IFunction>(compfunction));
      fitalg->setProperty("InputWorkspace", dataWS);
      fitalg->setProperty("WorkspaceIndex", int(workspaceindex));
      fitalg->setProperty("StartX", leftbound);
      fitalg->setProperty("EndX", rightbound);
      fitalg->setProperty("Minimizer", "Levenberg-MarquardtMD");
      fitalg->setProperty("CostFunction", "Least squares");
      fitalg->setProperty("MaxIterations", 1000);

      // 7.  Result
      bool successfulfit = fitalg->execute();

      bool goodfit = false;

      // a) good execution
      if (!fitalg->isExecuted() || ! successfulfit)
      {
          // Early return due to bad fit & thus early return
          g_log.warning() << "Fitting B2kBk-Exp-conv-PV + Background to Peak around "
                          << peak->getParameter("TOF_h") << " failed during execution. " << std::endl;
          goodfit = false;

          chi2 = 1.0E10;
      }
      else
      {
          // Fit is good
          goodfit = true;
      }

      if (goodfit)
      {
        chi2 = fitalg->getProperty("OutputChi2overDoF");
        std::string fs = fitalg->getProperty("OutputStatus");
        fitstatus = fs;

        g_log.information() << "[FitSinglePeak] @ " << peak->getParameter("TOF_h")
                            << " successful:  Chi^2 = " << chi2
                            << "; Fit Status = " << fitstatus
                            << "; Height = " << peak->getParameter("Height") << std::endl;

        if (fitstatus.compare("success"))
        {
              goodfit = false;
        }
        else
        {
            goodfit = true;
        }
      }

      if (!goodfit)
      {
          // Early return
          return goodfit;
      }

      // 8. Fit background again
      // FIXME : Continue to implement from here
      // throw std::runtime_error("Continue to implement from here!");
      // goodbkgdfit = fitBackground(peak, workspaceindex, background, leftbound, rightbound);


      // 9. Fit peak again


      // b) fit status is good
      if (goodfit)
      {
        chi2 = fitalg->getProperty("OutputChi2overDoF");
        std::string fs = fitalg->getProperty("OutputStatus");
        fitstatus = fs;

        g_log.information() << "[FitSinglePeak] @ " << peak->getParameter("TOF_h")
                              << " successful:  Chi^2 = " << chi2
                              << "; Fit Status = " << fitstatus
                              << "; Height = " << peak->getParameter("Height") << std::endl;

        if (fitstatus.compare("success"))
        {
              goodfit = false;
        }
        else
        {
            goodfit = true;
        }
      }

      return goodfit;
  }


  /** Fit a single peak w/o estimating original peak position
    */
  bool RefinePowderInstrumentParameters::fitSinglePeakSimple(API::CompositeFunction_sptr compfunction, CurveFitting::Bk2BkExpConvPV_sptr peak,
                                                             CurveFitting::BackgroundFunction_sptr background, double leftdev, double rightdev,
                                                             size_t workspaceindex, double prog, double deltaprog,
                                                             double& chi2, std::string& fitstatus)
  {
      // 2. Fit the background
      double tof_h = peak->centre();
      double leftbound = tof_h - leftdev;
      double rightbound = tof_h + rightdev;

      g_log.debug() << "Single peak fitting range: Left bound = " << leftbound << "  Right bound = "
                    << rightbound << std::endl;

      bool goodfit = fitLinearBackground(peak, workspaceindex, background, leftbound, rightbound);

      // 4. Set up fitting parameters.
      // FIXME This may not be the case all the time!
      peak->tie("Gamma", "0.0");

      // ii.  Tie the background if background fitting is good
      if (goodfit)
      {
          std::vector<std::string> bkgdparnames;
          bkgdparnames = background->getParameterNames();
          for (size_t ipm = 0; ipm < bkgdparnames.size(); ++ipm)
          {
              std::string paramname = bkgdparnames[ipm];
              double parvalue = background->getParameter(paramname);
              std::stringstream ss;
              ss << parvalue;
              background->tie(paramname, ss.str());

              g_log.debug() << "Tie Background Parameter " << paramname << " = " << parvalue << std::endl;
          }
      }

      // 5. Fit peak
      std::cout << "Fit Peak + Background Function: " << compfunction->asString() << std::endl;

      API::IAlgorithm_sptr fitalg = createSubAlgorithm("Fit", prog, prog+deltaprog, true);
      prog += deltaprog;
      fitalg->initialize();

      fitalg->setProperty("Function", boost::dynamic_pointer_cast<API::IFunction>(compfunction));
      fitalg->setProperty("InputWorkspace", dataWS);
      fitalg->setProperty("WorkspaceIndex", workspaceindex);
      fitalg->setProperty("StartX", leftbound);
      fitalg->setProperty("EndX", rightbound);
      fitalg->setProperty("Minimizer", "Levenberg-MarquardtMD");
      fitalg->setProperty("CostFunction", "Least squares");
      fitalg->setProperty("MaxIterations", 1000);

      // iv)  Result
      goodfit = false;

      bool successfulfit = fitalg->execute();
      if (!fitalg->isExecuted() || ! successfulfit)
      {
          // Early return due to bad fit
          g_log.warning() << "Fitting B2kBk-Exp-conv-PV + Background to Peak around "
                          << peak->getParameter("TOF_h") << " Failed. " << std::endl;
          goodfit = false;
      }

      if (goodfit)
      {
          chi2 = fitalg->getProperty("OutputChi2overDoF");
          std::string ts = fitalg->getProperty("OutputStatus");
          fitstatus = ts;

          g_log.information() << "Fit single peak @ " << peak->getParameter("TOF_h")
                          << " successful:  Chi^2 = " << chi2
                          << "; Fit Status = " << fitstatus
                          << "; Height = " << peak->getParameter("Height") << std::endl;

          if (fitstatus.compare("success"))
          {
              goodfit = false;
          }
          else
          {
              goodfit = true;
          }
      }

      return goodfit;
  }

  /** Search peak (max height) in the given range
    * Give out the range of peak center for constaint later
    */
  void RefinePowderInstrumentParameters::findMaxHeight(API::MatrixWorkspace_sptr dataws, size_t wsindex,
                                                       double xmin, double xmax, double& center, double& centerleftbound, double& centerrightbound)
  {
      // 1. Determine xmin, xmax range
      std::vector<double>::const_iterator viter;
      const MantidVec& X = dataws->readX(wsindex);
      viter = std::lower_bound(X.begin(), X.end(), xmin);
      size_t ixmin = size_t(viter-X.begin());
      if (ixmin != 0)
          -- ixmin;
      viter = std::lower_bound(X.begin(), X.end(), xmax);
      size_t ixmax = size_t(viter-X.begin());

      // 2. Search imax
      const MantidVec& Y = dataws->readY(wsindex);
      size_t imax = ixmin;
      double maxY = Y[ixmin];
      for (size_t i = ixmin+1; i <= ixmax; ++i)
      {
          if (Y[i] > maxY)
          {
              maxY = Y[i];
              imax = i;
          }
      }

      std::cout << "Find Max :  iMax = " << imax << std::endl;

      center = X[imax];

      // 3. Determine the range of the peaks by +/-? data points
      if (imax >= 1)
      {
          bool down3left = true;
          int iend = int(imax)-4;
          if (iend < 0)
          {
              iend = 0;
          }
          for (size_t i = imax; i > size_t(iend); --i)
          {
              if (Y[i] <= Y[i-1])
              {
                  down3left = false;
                  break;
              }
          }
          if (down3left)
          {
              centerleftbound = X[imax-1];
          }
          else
          {
              centerleftbound = X[iend];
          }
      }
      else
      {
          std::stringstream errmsg;
          errmsg << "A Peak Cannot Appear At The Low End of A Workspace. " << std::endl;
          throw std::runtime_error(errmsg.str());
      }

      if (imax < X.size()-1)
      {
          bool down3right = true;
          size_t iend = imax + 4;
          if (iend >= X.size())
          {
              iend = X.size()-1;
          }
          for (size_t i = imax; i < iend; ++i)
          {
              if (Y[i] <= Y[i+1])
              {
                  down3right = false;
                  break;
              }
          }
          if (down3right)
          {
              centerrightbound = X[imax+1];
          }
          else
          {
              centerrightbound = X[iend];
          }
      }
      else
      {
          std::stringstream errmsg;
          errmsg << "A Peak Cannot Appear At The Upper End of A Workspace. " << std::endl;
          throw std::runtime_error(errmsg.str());
      }

      return;
  }


  /** Fit linear background under a peak
   */
  bool RefinePowderInstrumentParameters::fitLinearBackground(CurveFitting::Bk2BkExpConvPV_sptr peak, size_t workspaceindex,
                                                             CurveFitting::BackgroundFunction_sptr bkgdfunc, double xmin, double xmax)
  {
      // 1. Re-construct a workspace for fitting the background
      double leftpeakbound = peak->centre() - 6.0*peak->fwhm();
      double rightpeakbound = peak->centre() + 6.0*peak->fwhm();

      std::vector<size_t> newxs;
      for (size_t i = 0; i < dataWS->readX(workspaceindex).size(); ++i)
      {
          double x = dataWS->readX(workspaceindex)[i];
          if ( (x > xmin && x < leftpeakbound) || (x > rightpeakbound && x < xmax) )
          {
              // region of peak
              newxs.push_back(i);
          }
          else if (x > xmax)
          {
              // out to right boundary
              break;
          }
      }

      DataObjects::Workspace2D_sptr bkgdws =
              boost::dynamic_pointer_cast<DataObjects::Workspace2D>
              (API::WorkspaceFactory::Instance().create("Workspace2D", 1, newxs.size(), newxs.size()));

      for (size_t i = 0; i < newxs.size(); ++i)
      {
          bkgdws->dataX(0)[i] = dataWS->dataX(workspaceindex)[newxs[i]];
          bkgdws->dataY(0)[i] = dataWS->dataY(workspaceindex)[newxs[i]];
          bkgdws->dataE(0)[i] = dataWS->dataE(workspaceindex)[newxs[i]];
          std::cout << bkgdws->dataX(0)[i] << "    " << bkgdws->dataY(0)[i] << std::endl;
      }

      // 2. Fit
      API::IAlgorithm_sptr fitalg = this->createSubAlgorithm("Fit", -1.0, -1.0, true);
      fitalg->initialize();

      g_log.information() << "Function To Fit: " << bkgdfunc->asString()
                          << ".  Number of points  to fit =  " << newxs.size() << std::endl;

      // b) Set property
      fitalg->setProperty("Function", boost::shared_ptr<API::IFunction>(bkgdfunc));
      fitalg->setProperty("InputWorkspace", bkgdws);
      fitalg->setProperty("WorkspaceIndex", 0);
      fitalg->setProperty("StartX", xmin);
      fitalg->setProperty("EndX", xmax);
      fitalg->setProperty("Minimizer", "Levenberg-MarquardtMD");
      fitalg->setProperty("CostFunction", "Least squares");
      fitalg->setProperty("MaxIterations", 1000);

      // c) Execute
      bool successfulfit = fitalg->execute();
      if (!fitalg->isExecuted() || ! successfulfit)
      {
          // Early return due to bad fit
          g_log.warning() << "Fitting background function failed. " << std::endl;
          return false;
      }

      double chi2 = fitalg->getProperty("OutputChi2overDoF");
      std::string fitstatus = fitalg->getProperty("OutputStatus");

      g_log.information() << "Fit Linear Background: result:  Chi^2 = " << chi2
                          << " Fit Status = " << fitstatus << std::endl;

      return true;
  }



  /** Fit linear background under a peak
    * with confidence on peak's profile parameters
   */
  bool RefinePowderInstrumentParameters::fitBackground(CurveFitting::Bk2BkExpConvPV_sptr peak, size_t workspaceindex,
                                                       CurveFitting::BackgroundFunction_sptr bkgdfunc, double xmin, double xmax)
  {
      // 1. Re-construct a workspace for fitting the background


      /* FIXME This disabled section will be rewritten for faster speed
      std::vector<size_t> newxs;
      for (size_t i = 0; i < dataWS->readX(workspaceindex).size(); ++i)
      {
          double x = dataWS->readX(workspaceindex)[i];
          if ( (x > xmin && x < leftpeakbound) || (x > rightpeakbound && x < xmax) )
          {
              // region of peak
              newxs.push_back(i);
          }
          else if (x > xmax)
          {
              // out to right boundary
              break;
          }
      }


      DataObjects::Workspace2D_sptr bkgdws =
              boost::dynamic_pointer_cast<DataObjects::Workspace2D>
              (API::WorkspaceFactory::Instance().create("Workspace2D", 1, newxs.size(), newxs.size()));

      for (size_t i = 0; i < newxs.size(); ++i)
      {
          bkgdws->dataX(0)[i] = dataWS->dataX(workspaceindex)[newxs[i]];
          bkgdws->dataY(0)[i] = dataWS->dataY(workspaceindex)[newxs[i]];
          bkgdws->dataE(0)[i] = dataWS->dataE(workspaceindex)[newxs[i]];
          std::cout << bkgdws->dataX(0)[i] << "    " << bkgdws->dataY(0)[i] << std::endl;
      }

      // 2. Fit
      API::IAlgorithm_sptr fitalg = this->createSubAlgorithm("Fit", -1.0, -1.0, true);
      fitalg->initialize();

      g_log.information() << "Function To Fit: " << bkgdfunc->asString()
                          << ".  Number of points  to fit =  " << newxs.size() << std::endl;

      // b) Set property
      fitalg->setProperty("Function", boost::shared_ptr<API::IFunction>(bkgdfunc));
      fitalg->setProperty("InputWorkspace", bkgdws);
      fitalg->setProperty("WorkspaceIndex", 0);
      fitalg->setProperty("StartX", xmin);
      fitalg->setProperty("EndX", xmax);
      fitalg->setProperty("Minimizer", "Levenberg-MarquardtMD");
      fitalg->setProperty("CostFunction", "Least squares");
      fitalg->setProperty("MaxIterations", 1000);

      // c) Execute
      bool successfulfit = fitalg->execute();
      if (!fitalg->isExecuted() || ! successfulfit)
      {
          // Early return due to bad fit
          g_log.warning() << "Fitting background function failed. " << std::endl;
          return false;
      }

      double chi2 = fitalg->getProperty("OutputChi2overDoF");
      std::string fitstatus = fitalg->getProperty("OutputStatus");

      g_log.information() << "Fit Linear Background: result:  Chi^2 = " << chi2
                          << " Fit Status = " << fitstatus << std::endl;

                          */
      return true;
  }

  /** Get peak positions from peak functions
    * Output: outWS
   */
  void RefinePowderInstrumentParameters::genPeakCentersWorkspace(
          std::map<std::vector<int>, CurveFitting::Bk2BkExpConvPV_sptr> peaks,
          std::vector<std::vector<int> > goodpeaks, std::vector<double> goodfitchi2)
  {
      // 1. Create output workspace
      size_t size = goodpeaks.size();
      outWS = boost::dynamic_pointer_cast<DataObjects::Workspace2D>
              (API::WorkspaceFactory::Instance().create("Workspace2D", 2, size, size));
      outWS->getAxis(0)->setUnit("dSpacing");

      // 2. Put values
      double lattice = mFuncParameters["LatticeConstant"];
      if (lattice < 1.0E-5)
      {
          std::stringstream errmsg;
          errmsg << "Input Lattice constant = " << lattice << " is wrong or not set up right. ";
          throw std::invalid_argument(errmsg.str());
      }

      std::map<std::vector<int>, CurveFitting::Bk2BkExpConvPV_sptr>::iterator peakiter;
      std::vector<std::pair<double, std::pair<double, double> > > peakcenters;

      for (size_t i = 0; i < goodpeaks.size(); ++i)
      {
          std::vector<int> peakhkl = goodpeaks[i];
          double chi2 = goodfitchi2[i];
          peakiter = peaks.find(peakhkl);
          if (peakiter == peaks.end())
          {
              std::stringstream errormsg;
              errormsg << "(Good fit) Peak [" << goodpeaks[i][0] << ", " << goodpeaks[i][1] << ", " << goodpeaks[i][2]
                       << "] is not in the input peaks. ";
              g_log.error() << errormsg.str() << std::endl;
              throw std::runtime_error(errormsg.str());
          }

          // Calculate d-spacing of the peak center (X value)
          int h, k, l;
          h = peakiter->first[0];
          k = peakiter->first[1];
          l = peakiter->first[2];
          double d_h = lattice/sqrt(h*h + k*k + l*l);

          // TOF_h (Y value observed)
          double tof_h = peakiter->second->centre();

          peakcenters.push_back(std::make_pair(d_h, std::make_pair(tof_h, chi2)));
      }

      std::sort(peakcenters.begin(), peakcenters.end());

      // 3. Put data to output workspace
      for (size_t i = 0; i < peakcenters.size(); ++i)
      {
          outWS->dataX(0)[i] = peakcenters[i].first;
          outWS->dataY(0)[i] = peakcenters[i].second.first;
          // outWS->dataE(0)[i] = sqrt(peakcenters[i].second);
          outWS->dataE(0)[i] = 1.0/peakcenters[i].second.second;
      }

      for (size_t i = 0; i < outWS->readX(0).size(); ++i)
      {
          g_log.debug() << "Peak-Index " << i << "  d = " << outWS->readX(0)[i]
                        << ", TOF_h = " << outWS->readY(0)[i] << std::endl;
      }

      return;
  }


  /** Fit instrument parameters
   */
  void RefinePowderInstrumentParameters::fitInstrumentParameters()
  {
      // 1. Set up and (possibly) tie parameters
      CurveFitting::ThermalNeutronDtoTOFFunction rawfunc;
      CurveFitting::ThermalNeutronDtoTOFFunction_sptr mfunc =
              boost::make_shared<CurveFitting::ThermalNeutronDtoTOFFunction>(rawfunc);
      mfunc->initialize();

      std::vector<std::string> funparamnames = mfunc->getParameterNames();
      std::vector<std::string> paramtofit = getProperty("ParametersToFit");
      std::sort(paramtofit.begin(), paramtofit.end());

      std::map<std::string, double>::iterator fiter;
      for (size_t i = 0; i < funparamnames.size(); ++i)
      {
          std::string parname = funparamnames[i];
          fiter = mFuncParameters.find(parname);
          if (fiter != mFuncParameters.end())
          {
              // Find the paramter in input parameters
              g_log.debug() << "Set instrument geometry function parameter " << parname
                            << " : " << fiter->second << std::endl;
              double parvalue = fiter->second;
              mfunc->setParameter(parname, parvalue);

              // Check whether it is to tie
              std::vector<std::string>::iterator viter;
              viter = std::find(paramtofit.begin(), paramtofit.end(), parname);
              if (viter == paramtofit.end())
              {
                  // Not in the fit list.  Tie it!
                  std::stringstream valuestr;
                  valuestr << parvalue;
                  mfunc->tie(parname, valuestr.str());
              }
          }
          else
          {
              // Cannot find the parameter in input parameters
              std::stringstream ss;
              ss << "Instrument Geometry function parameter " << parname << " is not initialized" << std::endl;
              throw std::invalid_argument(ss.str());
          }
      }

      /*
      mfunc->setParameter("Zero", mFuncParameters["Zero"]);
      mfunc->setParameter("Zerot", mFuncParameters["Zerot"]);
      mfunc->setParameter("Dtt1", mFuncParameters["Dtt1"]);
      mfunc->setParameter("Dtt1t", mFuncParameters["Dtt1t"]);
      mfunc->setParameter("Dtt2t", mFuncParameters["Dtt2t"]);
      mfunc->setParameter("Width", mFuncParameters["Width"]);
      mfunc->setParameter("Tcross", mFuncParameters["Tcross"]);
      */

      // 2. Create and setup fit algorithm
      std::cout << "Fit instrument geometry: " << mfunc->asString() << std::endl;

      API::IAlgorithm_sptr fitalg = createSubAlgorithm("Fit", 0.0, 0.2, true);
      fitalg->initialize();

      fitalg->setProperty("Function", boost::dynamic_pointer_cast<API::IFunction>(mfunc));
      fitalg->setProperty("InputWorkspace", outWS);
      fitalg->setProperty("WorkspaceIndex", 0);
      fitalg->setProperty("Minimizer", "Levenberg-MarquardtMD");
      fitalg->setProperty("CostFunction", "Least squares");
      fitalg->setProperty("MaxIterations", 1000);

      bool successfulfit = fitalg->execute();
      if (!fitalg->isExecuted() || ! successfulfit)
      {
          // Early return due to bad fit
          g_log.error() << "Fitting to instrument geometry function failed. " << std::endl;
          throw std::runtime_error("Fitting failed.");
      }

      double chi2 = fitalg->getProperty("OutputChi2overDoF");
      std::string fitstatus = fitalg->getProperty("OutputStatus");

      g_log.information() << "Fit geometry function result:  Chi^2 = " << chi2
                          << "; Fit Status = " << fitstatus << std::endl;

      API::IFunction_sptr fitfunc = fitalg->getProperty("Function");
      std::cout << "--------->  Fitted Zero = " << fitfunc->getParameter("Zero")
                << "  Zerot = " << fitfunc->getParameter("Zerot") << std::endl;

      // 3. Set the output
      API::FunctionDomain1DVector domain(outWS->readX(0));
      API::FunctionValues values(domain);
      mfunc->function(domain, values);

      for (size_t i = 0; i < domain.size(); ++i)
      {
          outWS->dataX(1)[i] = domain[i];
          outWS->dataY(1)[i] = values[i];
      }

      // 4. Set up the output parameters
      std::vector<std::string> parnames = mfunc->getParameterNames();
      for (size_t i = 0; i < parnames.size(); ++i)
      {
          std::string parname = parnames[i];
          double parvalue = mfunc->getParameter(parname);
          mFuncParameters[parname] = parvalue;
      }

      // 5. Pretty screen output
      std::map<std::string, double>::iterator par2iter;

      g_log.notice() << "**************************************" << std::endl;
      for (size_t i = 0; i < paramtofit.size(); ++i)
      {
          std::string parname = paramtofit[i];
          par2iter = mFuncParameters.find(parname);
          if (par2iter != mFuncParameters.end())
          {
              double parvalue = par2iter->second;
              double oldparvalue = mInputFuncParameters[parname];
              g_log.notice() << "Fit paramter:  " << parname << " = " << parvalue
                             << "  <----- " << oldparvalue << std::endl;
          }
      }
      g_log.notice() << "**************************************" << std::endl;

      return;
  }


  /// =========================  Import TableWorkspace  ===================== ///
  /** Genearte peaks from input workspace
    */
  void RefinePowderInstrumentParameters::generatePeaksFromInput(
          DataObjects::TableWorkspace_sptr peakparamws, std::map<std::vector<int>, CurveFitting::Bk2BkExpConvPV_sptr>& peaks)
  {
      if (!peakparamws)
      {
          g_log.error() << "Input tableworkspace for peak parameters is invalid!" << std::endl;
          throw std::invalid_argument("Invalid input table workspace for peak parameters");
      }

      /// Clear output
      peaks.clear();

      // FIXME Using fixed column name input table workspace now!
      size_t numrows =  peakparamws->rowCount();
      for (size_t ir = 0; ir < numrows; ++ir)
      {
          // a) Parse row
          API::TableRow paramrow = peakparamws->getRow(ir);
          double height, tof, alpha, beta, gamma, sigma2;
          int h, k, l;
          paramrow >> h >> k >> l >> height >> tof >> alpha >> beta >> sigma2 >> gamma;

          // b) Create function
          CurveFitting::Bk2BkExpConvPV newpeak;
          newpeak.initialize();
          newpeak.setParameter("Alpha", alpha);
          newpeak.setParameter("Beta", beta);
          newpeak.setParameter("Sigma2", sigma2);
          newpeak.setParameter("Gamma", gamma);

          newpeak.setParameter("TOF_h", tof);
          newpeak.setParameter("Height", height);

          CurveFitting::Bk2BkExpConvPV_sptr newpeakptr = boost::make_shared<CurveFitting::Bk2BkExpConvPV>(newpeak);

          // c) Create new entry in the map
          std::vector<int> hkl;
          hkl.push_back(h); hkl.push_back(k); hkl.push_back(l);

          peaks.insert(std::make_pair(hkl, newpeakptr));
      }

      return;
  }

  /** Import TableWorkspace containing the parameters for fitting
   * the diffrotometer geometry parameters
   */
  void RefinePowderInstrumentParameters::importParametersFromTable(
          DataObjects::TableWorkspace_sptr parameterWS, std::map<std::string, double>& parameters)
  {
      // 1. Check column orders
      std::vector<std::string> colnames = parameterWS->getColumnNames();
      if (colnames.size() < 2)
      {
          g_log.error() << "Input parameter table workspace does not have enough number of columns. "
                        << " Number of columns = " << colnames.size() << " < 3 as required. " << std::endl;
          throw std::runtime_error("Input parameter workspace is wrong. ");
      }

      if (colnames[0].compare("Name") != 0 ||
              colnames[1].compare("Value") != 0)
      {
          g_log.error() << "Input parameter table workspace does not have the columns in order.  "
                        << " It must be Name, Value, FitOrTie." << std::endl;
          throw std::runtime_error("Input parameter workspace is wrong. ");
      }

      // 2. Import data to maps
      std::string parname;
      double value;

      size_t numrows = parameterWS->rowCount();

      for (size_t ir = 0; ir < numrows; ++ir)
      {
          API::TableRow trow = parameterWS->getRow(ir);
          trow >> parname >> value;
          parameters.insert(std::make_pair(parname, value));
      }

      return;
  }

  /** Calculate thermal neutron's d-spacing
    */
  double RefinePowderInstrumentParameters::calculateDspaceValue(std::vector<int> hkl)
  {
      throw std::runtime_error("To be implemented soon!");

      return -1.0;
  }

} // namespace CurveFitting
} // namespace Mantid
