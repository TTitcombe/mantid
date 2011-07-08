#ifndef MANTID_DATAOBJECTS_WORKSPACE2D_H_
#define MANTID_DATAOBJECTS_WORKSPACE2D_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidDataObjects/Histogram1D.h"

namespace Mantid
{

//----------------------------------------------------------------------
// Forward declarations
//----------------------------------------------------------------------
namespace Kernel
{
  class Logger;
}

namespace DataObjects
{
/** \class Workspace2D

    Concrete workspace implementation. Data is a vector of Histogram1D.
    Since Histogram1D have share ownership of X, Y or E arrays,
    duplication is avoided for workspaces for example with identical time bins.

    \author Laurent C Chapon, ISIS, RAL
    \date 26/09/2007

    Copyright &copy; 2007-9 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>.
    Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport Workspace2D : public API::MatrixWorkspace
{
public:
  /// Typedef for the workspace_iterator to use with a Workspace2D
  typedef API::workspace_iterator<API::LocatedDataRef, Workspace2D> iterator;
  /// Typedef for the const workspace_iterator to use with a Workspace2D
  typedef API::workspace_iterator<const API::LocatedDataRef, const Workspace2D> const_iterator;

  /**
  Gets the name of the workspace type
  @return Standard string name
   */
  virtual const std::string id() const {return "Workspace2D";}

  Workspace2D();
  virtual ~Workspace2D();

  /// Returns the histogram number
  std::size_t getNumberHistograms() const;

  //section required for iteration
  virtual std::size_t size() const;
  virtual std::size_t blocksize() const;

  /// Return the underlying ISpectrum ptr at the given workspace index.
  virtual Mantid::API::ISpectrum * getSpectrum(const size_t index);

  /// Return the underlying ISpectrum ptr (const version) at the given workspace index.
  virtual const Mantid::API::ISpectrum * getSpectrum(const size_t index) const;


  /** sets the monitorWorkspace indexlist
	@param mList :: a vector holding the monitor workspace indexes
  */
  void setMonitorList(std::vector<specid_t>& mList){m_monitorList=mList;}

   /** sets the number of histograms.This method is mainly useful when the user selects 
   monitor 'select' or 'exclude' options from loadraw UI
	@param nhistograms :: is the number of histograms
  */
  void sethistogramNumbers(const int64_t &nhistograms ){m_noVectors=static_cast<std::size_t>(nhistograms);}
  void sethistogramNumbers(const std::size_t &nhistograms ){m_noVectors=nhistograms;}

protected:
  /// Called by initialize()
  virtual void init(const std::size_t &NVectors, const std::size_t &XLength, const std::size_t &YLength);

  /// The number of vectors in the workspace
  std::size_t m_noVectors;

  /// a vector holding workspace index of monitors in the workspace
  std::vector<specid_t> m_monitorList;

  /// A vector that holds the 1D histograms
  std::vector<Histogram1D> data;

private:
  /// Private copy constructor. NO COPY ALLOWED
  Workspace2D(const Workspace2D&);
  /// Private copy assignment operator. NO ASSIGNMENT ALLOWED
  Workspace2D& operator=(const Workspace2D&);

  virtual std::size_t getHistogramNumberHelper() const;

  /// Static reference to the logger class
  static Kernel::Logger &g_log;
};

///shared pointer to the Workspace2D class
typedef boost::shared_ptr<Workspace2D> Workspace2D_sptr;
///shared pointer to a const Workspace2D
typedef boost::shared_ptr<const Workspace2D> Workspace2D_const_sptr;

} // namespace DataObjects
} // Namespace Mantid
#endif /*MANTID_DATAOBJECTS_WORKSPACE2D_H_*/
