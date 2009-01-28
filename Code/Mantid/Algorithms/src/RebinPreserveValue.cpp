//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/RebinPreserveValue.h"
#include "MantidAPI/Workspace.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/ArrayProperty.h"

#include <sstream>
#include <numeric>
#include <math.h>

namespace Mantid
{
  namespace Algorithms
  {

    // Register the class into the algorithm factory
    DECLARE_ALGORITHM(RebinPreserveValue)

    using namespace Kernel;
    using namespace API;
    using DataObjects::Workspace2D;
    using DataObjects::Workspace2D_sptr;

    // Get a reference to the logger
    Logger& RebinPreserveValue::g_log = Logger::get("RebinPreserveValue");

    /** Initialisation method. Declares properties to be used in algorithm.
    *
    */
    void RebinPreserveValue::init()
    {
      declareProperty(new WorkspaceProperty<MatrixWorkspace>("InputWorkspace","",Direction::Input,new HistogramValidator<>));
      declareProperty(new WorkspaceProperty<MatrixWorkspace>("OutputWorkspace","",Direction::Output));

      declareProperty(new ArrayProperty<double>("params", new MandatoryValidator<std::vector<double> >));
    }

    /** Executes the rebin algorithm
    *
    *  @throw runtime_error Thrown if
    */
    void RebinPreserveValue::exec()
    {
      // retrieve the properties
      std::vector<double> rb_params=getProperty("params");

      // Get the input workspace
      MatrixWorkspace_const_sptr inputW = getProperty("InputWorkspace");

      bool dist = inputW->isDistribution();

      // workspace independent determination of length
      int histnumber = inputW->size()/inputW->blocksize();
      DataObjects::Histogram1D::RCtype XValues_new;
      // create new output X axis
      int ntcnew = newAxis(rb_params,XValues_new.access());

      // make output Workspace the same type is the input, but with new length of signal array
      API::MatrixWorkspace_sptr outputW = API::WorkspaceFactory::Instance().create(inputW,histnumber,ntcnew,ntcnew-1);
      // Try to cast it to a Workspace2D for use later
      Workspace2D_sptr outputW_2D = boost::dynamic_pointer_cast<Workspace2D>(outputW);

      int progress_step = histnumber / 100;
      if (progress_step == 0) progress_step = 1;
      for (int hist=0; hist <  histnumber;++hist)
      {
        const API::IErrorHelper* e_ptr= inputW->errorHelper(hist);
        if(dynamic_cast<const API::GaussianErrorHelper*>(e_ptr) ==0)
        {
          g_log.error("Can only rebin Gaussian data");
          throw std::invalid_argument("Invalid input Workspace");
        }


        // get const references to input Workspace arrays (no copying)
        const std::vector<double>& XValues = inputW->dataX(hist);
        const std::vector<double>& YValues = inputW->dataY(hist);
        const std::vector<double>& YErrors = inputW->dataE(hist);

        //get references to output workspace data (no copying)
        std::vector<double>& YValues_new=outputW->dataY(hist);
        std::vector<double>& YErrors_new=outputW->dataE(hist);

        // output data arrays are implicitly filled by function
        rebin(XValues,YValues,YErrors,XValues_new,YValues_new,YErrors_new, dist);
        // Populate the output workspace X values
        if (outputW_2D)
        {
          outputW_2D->setX(hist,XValues_new);
        }
        else
        {
          outputW->dataX(hist)=XValues_new.access();
        }
        //copy oer the spectrum No and ErrorHelper
        try {
          outputW->getAxis(1)->spectraNo(hist)=inputW->getAxis(1)->spectraNo(hist);
        } catch (Exception::IndexError) {
          // OK, so this isn't a Workspace2D
        }
        outputW->setErrorHelper(hist,inputW->errorHelper(hist));
        if (hist % progress_step == 0)
        {
            progress(double(hist)/histnumber);
            interruption_point();
        }
      }
      outputW->isDistribution(dist);

      // Copy units
      if (outputW->getAxis(0)->unit().get())
          outputW->getAxis(0)->unit() = inputW->getAxis(0)->unit();
      try
      {
          if (inputW->getAxis(1)->unit().get())
              outputW->getAxis(1)->unit() = inputW->getAxis(1)->unit();
      }
      catch(Exception::IndexError) {
          // OK, so this isn't a Workspace2D
      }

      // Assign it to the output workspace property
      setProperty("OutputWorkspace",outputW);

      return;
    }

    /** Rebins the data according to new output X array
    *
    * @param xold - old x array of data
    * @param xnew - new x array of data
    * @param yold - old y array of data
    * @param ynew - new y array of data
    * @param eold - old error array of data
    * @param enew - new error array of data
    * @param distribution - flag defining if distribution data (1) or not (0)
    * @throw runtime_error Thrown if algorithm cannot execute
    * @throw invalid_argument Thrown if input to function is incorrect
    **/
    void RebinPreserveValue::rebin(const std::vector<double>& xold, const std::vector<double>& yold, const std::vector<double>& eold,
      const DataObjects::Histogram1D::RCtype& xnew, std::vector<double>& ynew, std::vector<double>& enew, bool distribution)

    {
      int i,iold = 0,inew = 0;
      double xo_low, xo_high, xn_low, xn_high, delta(0.0), width;
      int size_yold=yold.size();
      int size_ynew=ynew.size();

      while((inew < size_ynew) && (iold < size_yold))
      {
        xo_low = xold[iold];
        xo_high = xold[iold+1];
        xn_low = (*xnew)[inew];
        xn_high = (*xnew)[inew+1];
        if ( xn_high <= xo_low )
        {
          inew++;		/* old and new bins do not overlap */
        }
        else if ( xo_high <= xn_low )
        {
          iold++;		/* old and new bins do not overlap */
        }
        else
        {
          /*/        delta is the overlap of the bins on the x axis
          //delta = std::min(xo_high, xn_high) - std::max(xo_low, xn_low);
          delta = xo_high<xn_high?xo_high:xn_high;
          delta -= xo_low>xn_low?xo_low:xn_low;
          width = xo_high - xo_low;
          if ( (delta <= 0.0) || (width <= 0.0) )
          {
            g_log.error("SimpleRebin: no bin overlap detected");
            throw std::runtime_error("no bin overlap detected");
          }//*/

          ynew[inew] = yold[iold];
          enew[inew] = eold[iold];

          if ( xn_high > xo_high )
          {
            iold++;
          }
          //else
          //{
            inew++;
          //}
        }
      }

      return; //without problems
    }

    /** Creates a new  output X array  according to specific boundary defnitions
    *
    * @param params - rebin parameters input [x_1, delta_1,x_2, ... ,x_n-1,delta_n-1,x_n)
    * @param xnew - new output workspace x array
    **/
    int RebinPreserveValue::newAxis(const std::vector<double>& params,
      std::vector<double>& xnew)
    {
      double xcurr, xs;
      int ibound(2), istep(1), inew(1);
      int ibounds=params.size(); //highest index in params array containing a bin boundary
      int isteps=ibounds-1; // highest index in params array containing a step

      xcurr = params[0];
      xnew.push_back(xcurr);

      while( (ibound <= ibounds) && (istep <= isteps) )
      {
        // if step is negative then it is logarithmic step
        if ( params[istep] >= 0.0)
          xs = params[istep];
        else
          xs = xcurr * fabs(params[istep]);
        /* continue stepping unless we get to almost where we want to */
        if ( (xcurr + xs) < (params[ibound] - (xs * 1.E-6)) )
        {
          xcurr += xs;
        }
        else
        {
          xcurr = params[ibound];
          ibound+=2;
          istep+=2;
        }
        xnew.push_back(xcurr);
        inew++;
      }
      //returns length of new x array or -1 if failure
      return inew;
      //return( (ibound == ibounds) && (istep == isteps) ? inew : -1 );
    }

  } // namespace Algorithm
} // namespace Mantid
