/**
 *  File: EventWSDataSource.cpp
 */

#include <iostream>
#include <math.h>

#include <QThread>

#include "MantidQtImageViewer/EventWSDataSource.h"
#include "MantidQtImageViewer/IVUtils.h"
#include "MantidAPI/ISpectrum.h"
#include "MantidAPI/IEventList.h"

using namespace Mantid;
using namespace Kernel;
using namespace API;

namespace MantidQt
{
namespace ImageView
{

/**
 * Construct a DataSource object around the specifed EventWorkspace.
 *
 * @param ev_ws  Shared pointer to the event workspace being "wrapped"
 */
EventWSDataSource::EventWSDataSource( IEventWorkspace_sptr ev_ws )
                 :ImageDataSource( 0.0, 1.0, 0.0, 1.0, 0, 0 )  // some defaults
{
  this->ev_ws = ev_ws;
  total_xmin = ev_ws->getTofMin(); 
  total_xmax = ev_ws->getTofMax(); 
  total_ymin = 0;                 // y direction is spectrum index
  total_ymax = (double)ev_ws->getNumberHistograms();
  total_rows = ev_ws->getNumberHistograms();
//  total_cols = 2000;             // initially use 2000 bins for event data
  total_cols = (int)(total_xmax-total_xmin)/1;    // initially use >= 1 us 
                                                  // bins for event data

//  std::cout << "total_xmin = " << total_xmin << std::endl;
//  std::cout << "total_xmax = " << total_xmax << std::endl;

  if ( total_xmax > 120000 )   
  {
    total_xmax = 120000;          // hack for now
    std::cout << "WARNING: max tof too large, set to " 
              << total_xmax << std::endl;
  }

  x_scale = new MantidVec();      // keep one x_scale, just reset it to
                                  // different binnings.
}


EventWSDataSource::~EventWSDataSource()
{
  if ( x_scale )
  {
    delete x_scale;
  }
}


/**
 * Get a data array covering the specified range of data, at the specified
 * resolution.  NOTE: The calling code is responsible for deleting the 
 * DataArray that is constructed in and returned by this method.
 *
 * @param xmin      Left edge of region to be covered.
 * @param xmax      Right edge of region to be covered.
 * @param ymin      Bottom edge of region to be covered.
 * @param ymax      Top edge of region to be covered.
 * @param n_rows    Number of rows to return. If the number of rows is less
 *                  than the actual number of data rows in [ymin,ymax], the 
 *                  data will be subsampled, and only the specified number 
 *                  of rows will be returned.
 * @param n_cols    The event data will be rebinned using the specified
 *                  number of colums.
 * @param is_log_x  Flag indicating whether or not the data should be
 *                  binned logarithmically.  (NOT USED YET)
 */
DataArray* EventWSDataSource::GetDataArray( double xmin,   double  xmax,
                                            double ymin,   double  ymax,
                                            size_t n_rows, size_t  n_cols,
                                            bool   is_log_x )
{
/*
  std::cout << "Start EventWSDataSource::GetDataArray " << std::endl;
  std::cout << "  xmin   = " << xmin 
            << "  xmax   = " << xmax 
            << "  ymin   = " << ymin 
            << "  ymax   = " << ymax 
            << "  n_rows = " << n_rows
            << "  n_cols = " << n_cols << std::endl;
*/
  size_t first_col;
  IVUtils::CalculateInterval( total_xmin, total_xmax, total_cols,
                              first_col, xmin, xmax, n_cols );

  size_t first_row;
  IVUtils::CalculateInterval( total_ymin, total_ymax, total_rows,
                              first_row, ymin, ymax, n_rows );

  float* new_data = new float[n_rows * n_cols];   // this is deleted in the
                                                  // DataArrray destructor
  x_scale->resize(n_cols+1);
  double dx = (xmax - xmin)/((double)n_cols + 1.0);
  for ( size_t i = 0; i < n_cols+1; i++ )
  {
    (*x_scale)[i] = xmin + (double)i * dx;;
  }

  size_t index = 0;
  for ( size_t i = first_row; i < first_row + n_rows; i++ )
  {
    IEventList * list = ev_ws->getEventListPtr(i);
    list->setX( *x_scale );
    list->setTofs( *x_scale );
    const MantidVec & y_vals = ev_ws->readY(i);
    for ( size_t col = 0; col < n_cols; col++ )
    {
      new_data[index] = (float)y_vals[col];
      index++;
    }
  }
                                // The calling code is responsible for deleting 
                                // the DataArray when it is done with it      
  DataArray* new_data_array = new DataArray( xmin, xmax, ymin, ymax,
                                           is_log_x, n_rows, n_cols, new_data);
  return new_data_array;
}


/**
 * Get a data array covering the full range of data.
 *
 * @param is_log_x  Flag indicating whether or not the data should be
 *                  binned logarithmically.  (NOT USED YET)
 */
DataArray * EventWSDataSource::GetDataArray( bool is_log_x )
{
  return GetDataArray( total_xmin, total_xmax, total_ymin, total_ymax,
                       total_rows, total_cols, is_log_x );
}

/**
 * Clear the vector of strings and then add pairs of strings giving information
 * about the specified point, x, y.  The first string in a pair should 
 * generally be a string describing the value being presented and the second
 * string should contain the value.
 *  
 * @param x    The x-coordinate of the point of interest in the data.
 * @param y    The y-coordinate of the point of interest in the data.
 * @param list Vector that will be filled out with the information strings.
 */
void EventWSDataSource::GetInfoList( double x, 
                                     double y,
                                     std::vector<std::string> &list )
{
  list.clear();

  list.push_back("Test X:");
  std::string x_str;
  IVUtils::Format(8,3,x,x_str);
  list.push_back(x_str);

  list.push_back("Test Y:");
  std::string y_str;
  IVUtils::Format(8,3,y,y_str);
  list.push_back(y_str);

  ISpectrum* spec = ev_ws->getSpectrum( (int)y );
  double spec_id = spec->getSpectrumNo();

  list.push_back("Spec ID");
  std::string spec_id_str;
  IVUtils::Format(8,0,spec_id,spec_id_str);
  list.push_back( spec_id_str );
}

} // namespace MantidQt 
} // namespace ImageView
