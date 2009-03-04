//---------------------------------------------------
// Includes
//---------------------------------------------------
#include "MantidDataHandling/SaveFocusedXYE.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"

#include <fstream>
#include <iomanip>

using namespace Mantid::DataHandling;

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(SaveFocusedXYE)

// Get a reference to the logger. It is used to print out information, warning and error messages
Mantid::Kernel::Logger& SaveFocusedXYE::g_log = Mantid::Kernel::Logger::get("SaveFocusedXYE");

//---------------------------------------------------
// Private member functions
//---------------------------------------------------
/**
 * Initialise the algorithm
 */
void SaveFocusedXYE::init()
{
  declareProperty(new API::WorkspaceProperty<>("InputWorkspace", "", Kernel::Direction::Input));
  declareProperty("Filename","",Kernel::Direction::Input);

  std::vector<std::string> Split(2);
  Split.push_back("True");Split.push_back("False");
  declareProperty("SplitFiles","True",new Kernel::ListValidator(Split));

}

/**
 * Execute the algorithm
 */
void SaveFocusedXYE::exec()
{
  using namespace Mantid::API;
  //Retrieve the input workspace
  MatrixWorkspace_sptr inputWS = getProperty("InputWorkspace");


  const int nHist=inputWS->getNumberHistograms();
  std::string filename = getProperty("Filename");
  std::string split=getProperty("SplitFiles");
  std::ostringstream number;
  std::fstream out;
  const int size=inputWS->readX(0).size();

	for (int i=0;i<nHist;i++)
	{
		const std::vector<double>& X=inputWS->readX(i);
		const std::vector<double>& Y=inputWS->readY(i);
		const std::vector<double>& E=inputWS->readE(i);
		if (split!="True" || nHist==1) // Assign only one file
		{
			out.open(filename.c_str(),std::ios::out);
		}
		else //Several files will be created
		{
			number << i << ".xye";
			filename+=number.str();
			number.str("");
			out.open(filename.c_str(),std::ios::out);
		}
		{
		if (!out.is_open())
		{
			g_log.information("Could not open filename: "+filename);
			throw std::runtime_error("Could not open filename: "+filename);
		}
		out <<"# File generated by Mantid for Group:" << std::endl;
		for (int j=0;j<size;j++)
			out << std::fixed << std::setprecision(4) << std::setw(15) << X[j]
			    << std::fixed << std::setprecision(4) << std::setw(15) << Y[j]
			    << std::fixed << std::setprecision(4) << std::setw(15) << E[j] << "\n";
		}
		out.close();
	}
	return;
}

