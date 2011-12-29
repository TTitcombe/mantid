#include "MantidMDCurve.h"
#include "MantidAPI/IMDWorkspace.h"
#include "MantidAPI/IMDIterator.h"
#include <qpainter.h>
#include <qwt_symbol.h>
#include "MantidAPI/AnalysisDataService.h"
#include "../Graph.h"
#include "../ApplicationWindow.h"
#include "../MultiLayer.h"


using namespace Mantid::API;
using namespace MantidQt::API;

/**
 *  @param wsName :: The workspace name.
 *  @param g :: The Graph widget which will display the curve
 *  @param err :: True if the errors are to be plotted
 *  @param distr :: True if this is a distribution
 *  @param style :: Graph style
 *  @throw std::invalid_argument if the index is out of range for the given workspace
 */
MantidMDCurve::MantidMDCurve(const QString& wsName,Graph* g,bool err,bool distr,Graph::CurveType style)
  :MantidCurve(wsName, err),
  m_wsName(wsName)
{
  init(g,distr,style);
}


MantidMDCurve::MantidMDCurve(const MantidMDCurve& c)
  :MantidCurve(createCopyName(c.title().text()), c.m_drawErrorBars, c.m_drawAllErrorBars),
  m_wsName(c.m_wsName)
{
  setData(c.data());
  observeDelete();
  connect( this, SIGNAL(resetData(const QString&)), this, SLOT(dataReset(const QString&)) );
  observeAfterReplace();
  observeADSClear();
}

/**
 *  @param g :: The Graph widget which will display the curve
 *  @param distr :: True if this is a distribution
 *  @param style :: The graph style to use
 */
void MantidMDCurve::init(Graph* g, bool distr, Graph::CurveType style)
{
  UNUSED_ARG(distr);
  IMDWorkspace_const_sptr ws = boost::dynamic_pointer_cast<IMDWorkspace>(
              AnalysisDataService::Instance().retrieve(m_wsName.toStdString()) );
  if(!ws)
  {
    std::string message = "Could not extract IMDWorkspace of name: " + m_wsName.toStdString();
    throw std::runtime_error(message);
  }
  if(ws->getNonIntegratedDimensions().size() != 1)
  {
    std::string message = "This plot only applies to MD Workspaces with a single expanded dimension";
    throw std::invalid_argument(message);
  }

  this->setTitle(m_wsName + "-signal");

  const bool log = g->isLog(QwtPlot::yLeft);
  MantidQwtIMDWorkspaceData data(ws,log);
  setData(data);

  int lineWidth = 1;
  MultiLayer* ml = (MultiLayer*)(g->parent()->parent()->parent());
  if (style == Graph::Unspecified || (ml && ml->applicationWindow()->applyCurveStyleToMantid) )
  {
    applyStyleChoice(style, ml, lineWidth);
  }
  else
  {
    setStyle(QwtPlotCurve::Lines);
  }
  if (g)
  {
    g->insertCurve(this,lineWidth);
  }
  connect(g,SIGNAL(axisScaleChanged(int,bool)),this,SLOT(axisScaleChanged(int,bool)));
  observeDelete();
  connect( this, SIGNAL(resetData(const QString&)), this, SLOT(dataReset(const QString&)) );
  observeAfterReplace();
  observeADSClear();
}


MantidMDCurve::~MantidMDCurve()
{
}

/**
 * Clone the curve for the use by a particular Graph
 */
MantidMDCurve* MantidMDCurve::clone(const Graph*)const
{
  MantidMDCurve* mc = new MantidMDCurve(*this);/*
  if (g)
  {
    mc->setDrawAsDistribution(g->isDistribution());
  }*/
  return mc;
}


void MantidMDCurve::setData(const QwtData &data)
{
  if (!dynamic_cast<const MantidQwtIMDWorkspaceData*>(&data)) 
    throw std::runtime_error("Only MantidQwtIMDWorkspaceData can be set to a MantidMDCurve");
  PlotCurve::setData(data);
}

QwtDoubleRect MantidMDCurve::boundingRect() const
{
  return MantidCurve::boundingRect();
}

void MantidMDCurve::draw(QPainter *p, 
          const QwtScaleMap &xMap, const QwtScaleMap &yMap,
          const QRect &rect) const
{
  PlotCurve::draw(p,xMap,yMap,rect);

  if (m_drawErrorBars)// drawing error bars
  {
    const MantidQwtIMDWorkspaceData* d = dynamic_cast<const MantidQwtIMDWorkspaceData*>(&data());
    if (!d)
    {
      throw std::runtime_error("Only MantidQwtIMDWorkspaceData can be set to a MantidMDCurve");
    }
    doDraw(p, xMap, yMap, rect, d);
  }
}

/**  Resets the data if wsName is the name of this workspace
 *  @param wsName :: The name of a workspace which data has been changed in the data service.
 */
void MantidMDCurve::dataReset(const QString& wsName)
{
  if (m_wsName != wsName) return;
  const std::string wsNameStd = wsName.toStdString();
  Mantid::API::IMDWorkspace_sptr mws;
  try
  {
    Mantid::API::Workspace_sptr base =  Mantid::API::AnalysisDataService::Instance().retrieve(wsNameStd);
    mws = boost::dynamic_pointer_cast<Mantid::API::IMDWorkspace>(base);
  }
  catch(std::runtime_error&)
  {
    Mantid::Kernel::Logger::get("MantidMDCurve").information() << "Workspace " << wsNameStd
        << " could not be found - plotted curve(s) deleted\n";
    mws = Mantid::API::IMDWorkspace_sptr();
  }

  if (!mws) return;
  const MantidQwtIMDWorkspaceData * new_mantidData(NULL);
  try {
    new_mantidData = mantidData()->copy(mws);
    setData(*new_mantidData);
    setStyle(QwtPlotCurve::Lines);
    // Queue this plot to be updated once all MantidQwtIMDWorkspaceData objects for this workspace have been
    emit dataUpdated();
  } catch(std::range_error &) {
    // Get here if the new workspace has fewer spectra and the plotted one no longer exists
    Mantid::Kernel::Logger::get("MantidMDCurve").information() << "Workspace " << wsNameStd
        << " now has fewer spectra - plotted curve(s) deleted\n";
    deleteHandle(wsNameStd,mws);
  }
  delete new_mantidData;
}


/* This method saves the curve details to a string.
 * Useful for loading/saving mantid project.
*/
QString MantidMDCurve::saveToString()
{
	QString s;
	s="MantidMDCurve\t"+m_wsName+"\t"+QString::number(m_drawErrorBars)+"\n";
	return s;
}

void MantidMDCurve::afterReplaceHandle(const std::string& wsName, const boost::shared_ptr<Mantid::API::Workspace> ws)
{
  (void) ws;

  invalidateBoundingRect();
  emit resetData(QString::fromStdString(wsName));
}


MantidQwtIMDWorkspaceData* MantidMDCurve::mantidData() 
{
  MantidQwtIMDWorkspaceData* d = dynamic_cast<MantidQwtIMDWorkspaceData*>(&data());
  return d;
}

const MantidQwtIMDWorkspaceData* MantidMDCurve::mantidData()const
{
  const MantidQwtIMDWorkspaceData* d = dynamic_cast<const MantidQwtIMDWorkspaceData*>(&data());
  return d;
}

