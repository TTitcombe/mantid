#ifndef COMPONENTCREATIONHELPERS_H_
#define COMPONENTCREATIONHELPERS_H_

#include "MantidGeometry/Instrument/CompAssembly.h"
#include "MantidGeometry/Instrument/ObjComponent.h"
#include "MantidGeometry/Instrument/DetectorGroup.h"
#include "MantidGeometry/Instrument/Detector.h"
#include "MantidGeometry/Instrument/Instrument.h"
#include "MantidGeometry/Objects/Object.h"
#include "MantidGeometry/Objects/ShapeFactory.h"

namespace ComponentCreationHelper
{

  using namespace Mantid::Geometry;

  /** 
  A set of helper functions for creating various component structures for the unit tests.

  Copyright &copy; 2010 ISIS Rutherford Appleton Laboratory & NScD Oak Ridge National Laboratory

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


  //----------------------------------------------------------------------------------------------
  /**
   * Create a capped cylinder object
   */
  static Object_sptr createCappedCylinder(double radius, double height, const V3D & baseCentre, const V3D & axis, const std::string & id)
  {
    std::ostringstream xml;
    xml << "<cylinder id=\"" << id << "\">" 
      << "<centre-of-bottom-base x=\"" << baseCentre.X() << "\" y=\"" << baseCentre.Y() << "\" z=\"" << baseCentre.Z() << "\"/>"
      << "<axis x=\"" << axis.X() << "\" y=\"" << axis.Y() << "\" z=\"" << axis.Z() << "\"/>"
      << "<radius val=\"" << radius << "\" />"
      << "<height val=\"" << height << "\" />"
      << "</cylinder>";

    ShapeFactory shapeMaker;
    return shapeMaker.createShape(xml.str());
  }
  

  //----------------------------------------------------------------------------------------------

  /**
   * Return the XML for a sphere.
   */
  std::string sphereXML(double radius, const V3D & centre, const std::string & id)
  {
    std::ostringstream xml;
    xml << "<sphere id=\"" << id <<  "\">"
    << "<centre x=\"" << centre.X() << "\"  y=\"" << centre.Y() << "\" z=\"" << centre.Z() << "\" />"
    << "<radius val=\"" << radius << "\" />"
    << "</sphere>";
    return xml.str();
  }

  /**
   * Create a sphere object
   */
  static Object_sptr createSphere(double radius, const V3D & centre, const std::string & id)
  {
    ShapeFactory shapeMaker;
    return shapeMaker.createShape(sphereXML(radius, centre, id));
  }


  //----------------------------------------------------------------------------------------------
  /** Create a cuboid shape for your pixels */
  Object_sptr createCuboid(double side_length)
  {
    double szX=side_length;
    double szY=szX;
    double szZ=szX;
    std::ostringstream xmlShapeStream;
    xmlShapeStream
    << " <cuboid id=\"detector-shape\"> "
    << "<left-front-bottom-point x=\""<<szX<<"\" y=\""<<-szY<<"\" z=\""<<-szZ<<"\"  /> "
    << "<left-front-top-point  x=\""<<szX<<"\" y=\""<<-szY<<"\" z=\""<<szZ<<"\"  /> "
    << "<left-back-bottom-point  x=\""<<-szX<<"\" y=\""<<-szY<<"\" z=\""<<-szZ<<"\"  /> "
    << "<right-front-bottom-point  x=\""<<szX<<"\" y=\""<<szY<<"\" z=\""<<-szZ<<"\"  /> "
    << "</cuboid>";

    std::string xmlCuboidShape(xmlShapeStream.str());
    ShapeFactory shapeCreator;
    Object_sptr cuboidShape = shapeCreator.createShape(xmlCuboidShape);
    return cuboidShape;
  }


  //----------------------------------------------------------------------------------------------
  /**
  * Create a component assembly at the origin made up of 4 cylindrical detectors
  */
  static boost::shared_ptr<CompAssembly> createTestAssemblyOfFourCylinders()
  {
    boost::shared_ptr<CompAssembly> bank = boost::shared_ptr<CompAssembly>(new CompAssembly("BankName"));
    // One object
    Object_sptr pixelShape = ComponentCreationHelper::createCappedCylinder(0.5, 1.5, V3D(0.0,0.0,0.0), V3D(0.,1.0,0.), "tube"); 
    // Four object components
    for( size_t i = 1; i < 5; ++i )
    {
      ObjComponent * physicalPixel = new ObjComponent("pixel", pixelShape);
      physicalPixel->setPos(static_cast<double>(i),0.0,0.0);
      bank->add(physicalPixel);
    }
    
    return bank;
  }

  /**
   * Create an object component that has a defined shape
   */
  static ObjComponent * createSingleObjectComponent()
  {
    Object_sptr pixelShape = ComponentCreationHelper::createCappedCylinder(0.5, 1.5, V3D(0.0,0.0,0.0), V3D(0.,1.0,0.), "tube");
    return new ObjComponent("pixel", pixelShape);
  }

  /**
   * Create a hollow shell, i.e. the intersection of two spheres or radius r1 and r2
   */
  static Object_sptr createHollowShell(double innerRadius, double outerRadius, const V3D & centre = V3D())
  {
    std::string wholeXML = 
      sphereXML(innerRadius, centre, "inner") + "\n" + 
      sphereXML(outerRadius, centre, "outer") + "\n" + 
      "<algebra val=\"(outer (# inner))\" />";
    
    ShapeFactory shapeMaker;
    return shapeMaker.createShape(wholeXML);
  }

  //----------------------------------------------------------------------------------------------
  /**
   * Create a detector group containing 5 detectors
   */
  static boost::shared_ptr<DetectorGroup> createDetectorGroupWith5CylindricalDetectors()
  {
    const int ndets = 5;
    std::vector<boost::shared_ptr<IDetector> > groupMembers(ndets);
    // One object
    Object_sptr detShape = ComponentCreationHelper::createCappedCylinder(0.5, 1.5, V3D(0.0,0.0,0.0), V3D(0.,1.0,0.), "tube"); 
    for( int i = 0; i < ndets; ++i )
    {
      std::ostringstream os;
      os << "d" << i;
      boost::shared_ptr<Detector> det(new Detector(os.str(), detShape, NULL));
      det->setID(i+1);
      det->setPos((double)(i+1), 2.0, 2.0);
      groupMembers[i] = det;
    }

    return boost::shared_ptr<DetectorGroup>(new DetectorGroup(groupMembers, false));
  }


  //----------------------------------------------------------------------------------------------
  /**
   * Create a group of two monitors
   */
  static boost::shared_ptr<DetectorGroup> createGroupOfTwoMonitors()
  {
    const int ndets(2);
    std::vector<boost::shared_ptr<IDetector> > groupMembers(ndets);
    for( int i = 0; i < ndets; ++i )
    {
      std::ostringstream os;
      os << "m" << i;
      boost::shared_ptr<Detector> det(new Detector(os.str(), NULL));
      det->setID(i+1);
      det->setPos((double)(i+1), 2.0, 2.0);
      det->markAsMonitor();
      groupMembers[i] = det;
    }
    return boost::shared_ptr<DetectorGroup>(new DetectorGroup(groupMembers, false));
  }


  //----------------------------------------------------------------------------------------------
  /**
   * Create an test instrument with n panels of 9 cylindrical detectors, a source and spherical sample shape.
   *
   * @param num_banks: number of 9-cylinder banks to create
   * @param verbose: prints out the instrument after creation.
   */
  static IInstrument_sptr createTestInstrumentCylindrical(int num_banks,
      bool verbose = false)
  {
    boost::shared_ptr<Instrument> testInst(new Instrument("basic"));

    const double cylRadius(0.004);
    const double cylHeight(0.0002);
    // One object
    Object_sptr pixelShape = ComponentCreationHelper::createCappedCylinder(cylRadius, cylHeight, V3D(0.0,-cylHeight/2.0,0.0), V3D(0.,1.0,0.), "pixel-shape");

    //Just increment pixel ID's
    int pixelID = 1;

    for (int banknum=1; banknum <= num_banks; banknum++)
    {
      //Make a new bank
      std::ostringstream bankname;
      bankname << "bank" << banknum;
      CompAssembly *bank = new CompAssembly(bankname.str());

      // Four object components
      for( int i = -1; i < 2; ++i )
      {
        for( int j = -1; j < 2; ++j )
        {
          std::ostringstream lexer;
          lexer << "pixel-(" << j << "," << i << ")";
          Detector * physicalPixel = new Detector(lexer.str(), pixelShape, bank);
          const double xpos = j*cylRadius*2.0;
          const double ypos = i*cylHeight;
          physicalPixel->setPos(xpos, ypos,0.0);
          physicalPixel->setID(pixelID);
          pixelID++;
          bank->add(physicalPixel);
          testInst->markAsDetector(physicalPixel);
        }
      }

      testInst->add(bank);
      bank->setPos(V3D(0.0, 0.0, 5.0*banknum));
    }

    //Define a source component
    ObjComponent *source = new ObjComponent("moderator", Object_sptr(new Object), testInst.get());
    source->setPos(V3D(0.0, 0.0, -10.));
    testInst->add(source);
    testInst->markAsSource(source);

    // Define a sample as a simple sphere
    Object_sptr sampleSphere = createSphere(0.001, V3D(0.0, 0.0, 0.0), "sample-shape");
    ObjComponent *sample = new ObjComponent("sample", sampleSphere, testInst.get());
    testInst->setPos(0.0, 0.0, 0.0);
    testInst->add(sample);
    testInst->markAsSamplePos(sample);

    if( verbose )
    {
      std::cout << "\n\n=== Testing bank positions ==\n";
      const int nchilds = testInst->nelements();
      for(int i = 0; i < nchilds; ++i )
      {
        boost::shared_ptr<IComponent> child = testInst->getChild(i);
        std::cout << "Component " << i << " at pos " << child->getPos() << "\n";
        if( boost::shared_ptr<ICompAssembly> assem = boost::dynamic_pointer_cast<ICompAssembly>(child) )
        {
          for(int j = 0; j < assem->nelements(); ++j )
          {
            boost::shared_ptr<IComponent> comp = assem->getChild(j);
            std::cout << "Child " << j << " at pos " << comp->getPos() << "\n";
          }
        }
      }
      std::cout << "==================================\n";
    }
    
    return testInst;
  }
}

#endif //COMPONENTCREATIONHELPERS_H_
