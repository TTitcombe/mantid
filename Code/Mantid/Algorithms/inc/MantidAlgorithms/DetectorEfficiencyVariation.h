#ifndef MANTID_ALGORITHM_DETECTOREFFICIENCYVARIATION_H_
#define MANTID_ALGORITHM_DETECTOREFFICIENCYVARIATION_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/Algorithm.h"
#include "MantidDataObjects/Workspace2D.h"
#include <Poco/NObserver.h>
#include <climits>
#include <string>
#include <vector>

namespace Mantid
{
  namespace Algorithms
  {
    using namespace API;
    /**
    


    @author Steve D Williams, ISIS Facility Rutherford Appleton Laboratory
    @date 15/06/2009

    Copyright &copy; 2009 STFC Rutherford Appleton Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>
    Code Documentation is available at: <http://doxygen.mantidproject.org>
    */
    class DLLExport DetectorEfficiencyVariation : public Algorithm
    {
    public:
      /// Default constructor initialises all values to zero and runs the base class constructor
      DetectorEfficiencyVariation() :
          Algorithm(),                                                        //call the base class constructor
          m_PercentDone(0.0), m_TotalTime(RTTotal), m_usableMaskMap(true)
      {};
      /// Destructor
      virtual ~DetectorEfficiencyVariation() {};
      /// Algorithm's name for identification overriding a virtual method
      virtual const std::string name() const { return "DetectorEfficiencyVariation";}
      /// Algorithm's version for identification overriding a virtual method
      virtual const int version() const { return (1);}
      /// Algorithm's category for identification overriding a virtual method
      virtual const std::string category() const { return "Diagnostics";}

    protected:
      // Overridden Algorithm methods
      void init();
      void exec();
      
      // The different steps of the calculation, all called by exec()
      /// Loads and checks the values passed to the algorithm
      void retrieveProperties( MatrixWorkspace_sptr &whiteBeam1,
        MatrixWorkspace_sptr &whiteBeam2, double &vari,
        int &minSpec, int &maxSpec );
      /// Calculates the sum counts in each histogram
      MatrixWorkspace_sptr getTotalCounts( MatrixWorkspace_sptr input,
        int firstSpec, int lastSpec );
      /// Finds the median of values in single bin histograms
      double getMedian(MatrixWorkspace_const_sptr input) const;
      /// Overwrites the first workspace with bad spectrum information, also outputs an array and a file
      std::vector<int> markBad( MatrixWorkspace_sptr a,
        MatrixWorkspace_const_sptr b, double average, double variation,
        std::string fileName);

      /// Value written to the output workspace where bad spectra are found
      static const int BadVal = 100;
      /// Marks accepted spectra the output workspace
      static const int GoodVal = 0;
      ///a flag int value to indicate that the value wasn't set by users
      static const int UNSETINT = INT_MAX-15;

      //a lot of data and functions for the progress bar 
      /// An estimate of the percentage of the algorithm runtimes that has been completed 
      float m_PercentDone;
      /// For the progress bar, estimates of how many additions (and equilivent) member functions will do for each spectrum assuming large spectra where progressing times are likely to be long
      enum RunTime
      {
        /// Estimate of the work required from Integtrate for each spectrum in _each_ workspace
        RTGetTotalCounts = 5000,
        /// Time taken to find failing detectors
        RTMarkDetects = 100,
        /// The total of all run times
        RTTotal = 2*RTGetTotalCounts + RTMarkDetects
      };
      /// An estimate total number of additions or equilivent are require to compute a spectrum 
      int m_TotalTime;
      /// Update the percentage complete estimate assuming that the algorithm has completed a task with estimated RunTime toAdd
      float advanceProgress(int toAdd);
    private:
      /// when this is set to false reading and writing to the detector map is disabled, this is done if there is no map in the workspace
      bool m_usableMaskMap;
    };

  } // namespace Algorithm
} // namespace Mantid

#endif /*MANTID_ALGORITHM_DETECTOREFFICIENCYVARIATION_H_*/
