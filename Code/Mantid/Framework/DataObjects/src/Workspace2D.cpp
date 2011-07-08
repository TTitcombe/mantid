#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/Exception.h"
#include "MantidAPI/RefAxis.h"
#include "MantidAPI/SpectraAxis.h"
#include "MantidAPI/LocatedDataRef.h"
#include "MantidAPI/WorkspaceIterator.h"
#include "MantidAPI/WorkspaceIteratorCode.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/ISpectrum.h"

using Mantid::API::ISpectrum;

namespace Mantid
{
  namespace DataObjects
  {
    using std::size_t;

    DECLARE_WORKSPACE(Workspace2D)

    // Get a reference to the logger
    Kernel::Logger& Workspace2D::g_log = Kernel::Logger::get("Workspace2D");

    /// Constructor
    Workspace2D::Workspace2D()
    {}

    ///Destructor
    Workspace2D::~Workspace2D()
    {}

    /** Sets the size of the workspace and initializes arrays to zero
    *  @param NVectors :: The number of vectors/histograms/detectors in the workspace
    *  @param XLength :: The number of X data points/bin boundaries in each vector (must all be the same)
    *  @param YLength :: The number of data/error points in each vector (must all be the same)
    */
    void Workspace2D::init(const size_t &NVectors, const size_t &XLength, const size_t &YLength)
    {
      m_noVectors = NVectors;
      data.resize(m_noVectors);
      m_axes.resize(2);
      m_axes[0] = new API::RefAxis(XLength, this);
      // This axis is always a spectra one for now. By default its values run from 1->NVectors
      m_axes[1] = new API::SpectraAxis(m_noVectors);

      MantidVecPtr t1,t2;
      t1.access().resize(XLength); //this call initializes array to zero
      t2.access().resize(YLength);
      for (size_t i=0;i<m_noVectors;i++)
      {
        ISpectrum * spec = this->getSpectrum(i);
        spec->setX(t1);
        spec->setDx(t1);
        // Y,E arrays populated
        spec->setData(t2,t2);
        // Default spectrum number = starts at 1, for workspace index 0.
        spec->setSpectrumNo(specid_t(i+1));
        spec->setDetectorID(detid_t(i+1));
      }
      this->generateSpectraMap();
    }

    
    /** Gets the number of histograms
    @return Integer
    */
    size_t Workspace2D::getNumberHistograms() const
    {
      return getHistogramNumberHelper();
    }

    /// get pseudo size
    size_t Workspace2D::size() const
    {
      return data.size() * blocksize();
    }

    ///get the size of each vector
    size_t Workspace2D::blocksize() const
    {
      return (data.size() > 0) ? data[0].size() : 0;
    }


    //--------------------------------------------------------------------------------------------
    /// Return the underlying ISpectrum ptr at the given workspace index.
    ISpectrum * Workspace2D::getSpectrum(const size_t index)
    {
      if (index>=m_noVectors)
        throw std::range_error("Workspace2D::getSpectrum, histogram number out of range");
      return &data[index];
    }

    const ISpectrum * Workspace2D::getSpectrum(const size_t index) const
    {
      if (index>=m_noVectors)
        throw std::range_error("Workspace2D::getSpectrum, histogram number out of range");
      return &data[index];
    }


    //--------------------------------------------------------------------------------------------
    /** Returns the number of histograms.
     *  For some reason Visual Studio couldn't deal with the main getHistogramNumber() method
     *  being virtual so it now just calls this private (and virtual) method which does the work.
     *  @return the number of histograms associated with the workspace
     */
    size_t Workspace2D::getHistogramNumberHelper() const
    {
      return data.size();
    }

  } // namespace DataObjects
} //NamespaceMantid






///\cond TEMPLATE
template DLLExport class Mantid::API::workspace_iterator<Mantid::API::LocatedDataRef, Mantid::DataObjects::Workspace2D>;
template DLLExport class Mantid::API::workspace_iterator<const Mantid::API::LocatedDataRef, const Mantid::DataObjects::Workspace2D>;

template DLLExport class Mantid::API::WorkspaceProperty<Mantid::DataObjects::Workspace2D>;

namespace Mantid
{
  namespace Kernel
  {
    template<> DLLExport
    Mantid::DataObjects::Workspace2D_sptr IPropertyManager::getValue<Mantid::DataObjects::Workspace2D_sptr>(const std::string &name) const
    {
      PropertyWithValue<Mantid::DataObjects::Workspace2D_sptr>* prop =
        dynamic_cast<PropertyWithValue<Mantid::DataObjects::Workspace2D_sptr>*>(getPointerToProperty(name));
      if (prop)
      {
        return *prop;
      }
      else
      {
        std::string message = "Attempt to assign property "+ name +" to incorrect type. Expected Workspace2D.";
        throw std::runtime_error(message);
      }
    }

    template<> DLLExport
    Mantid::DataObjects::Workspace2D_const_sptr IPropertyManager::getValue<Mantid::DataObjects::Workspace2D_const_sptr>(const std::string &name) const
    {
      PropertyWithValue<Mantid::DataObjects::Workspace2D_sptr>* prop =
        dynamic_cast<PropertyWithValue<Mantid::DataObjects::Workspace2D_sptr>*>(getPointerToProperty(name));
      if (prop)
      {
        return prop->operator()();
      }
      else
      {
        std::string message = "Attempt to assign property "+ name +" to incorrect type. Expected const Workspace2D.";
        throw std::runtime_error(message);
      }
    }
  } // namespace Kernel
} // namespace Mantid

///\endcond TEMPLATE
