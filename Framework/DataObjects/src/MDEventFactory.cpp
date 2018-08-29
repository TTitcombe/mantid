#include "MantidDataObjects/MDEventFactory.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidDataObjects/MDEventWorkspace.h"
#include <boost/shared_ptr.hpp>

#include "MantidDataObjects/MDBin.h"
#include "MantidDataObjects/MDBox.h"
#include "MantidDataObjects/MDBoxBase.h"
#include "MantidDataObjects/MDBoxIterator.h"
#include "MantidDataObjects/MDEvent.h"
#include "MantidDataObjects/MDEventWorkspace.h"
#include "MantidDataObjects/MDGridBox.h"
#include "MantidDataObjects/MDLeanEvent.h"

// We need to include the .cpp files so that the declarations are picked up
// correctly. Weird, I know.
// See http://www.parashift.com/c++-faq-lite/templates.html#faq-35.13
#include "MantidDataObjects/MDBin.tcc"
#include "MantidDataObjects/MDBox.tcc"
#include "MantidDataObjects/MDBoxBase.tcc"
#include "MantidDataObjects/MDBoxIterator.tcc"
#include "MantidDataObjects/MDEventWorkspace.tcc"
#include "MantidDataObjects/MDGridBox.tcc"

namespace Mantid {
namespace DataObjects {
//### BEGIN AUTO-GENERATED CODE
/* Code below Auto-generated by 'generate_mdevent_declarations.py'
 *     on 2016-06-03 10:28:44.608989
 *
 * DO NOT EDIT!
 */

// Instantiations for MDLeanEvent
template class DLLExport MDLeanEvent<1>;
template class DLLExport MDLeanEvent<2>;
template class DLLExport MDLeanEvent<3>;
template class DLLExport MDLeanEvent<4>;
template class DLLExport MDLeanEvent<5>;
template class DLLExport MDLeanEvent<6>;
template class DLLExport MDLeanEvent<7>;
template class DLLExport MDLeanEvent<8>;
template class DLLExport MDLeanEvent<9>;
// Instantiations for MDEvent
template class DLLExport MDEvent<1>;
template class DLLExport MDEvent<2>;
template class DLLExport MDEvent<3>;
template class DLLExport MDEvent<4>;
template class DLLExport MDEvent<5>;
template class DLLExport MDEvent<6>;
template class DLLExport MDEvent<7>;
template class DLLExport MDEvent<8>;
template class DLLExport MDEvent<9>;
// Instantiations for MDBoxBase
template class DLLExport MDBoxBase<MDLeanEvent<1>, 1>;
template class DLLExport MDBoxBase<MDLeanEvent<2>, 2>;
template class DLLExport MDBoxBase<MDLeanEvent<3>, 3>;
template class DLLExport MDBoxBase<MDLeanEvent<4>, 4>;
template class DLLExport MDBoxBase<MDLeanEvent<5>, 5>;
template class DLLExport MDBoxBase<MDLeanEvent<6>, 6>;
template class DLLExport MDBoxBase<MDLeanEvent<7>, 7>;
template class DLLExport MDBoxBase<MDLeanEvent<8>, 8>;
template class DLLExport MDBoxBase<MDLeanEvent<9>, 9>;
template class DLLExport MDBoxBase<MDEvent<1>, 1>;
template class DLLExport MDBoxBase<MDEvent<2>, 2>;
template class DLLExport MDBoxBase<MDEvent<3>, 3>;
template class DLLExport MDBoxBase<MDEvent<4>, 4>;
template class DLLExport MDBoxBase<MDEvent<5>, 5>;
template class DLLExport MDBoxBase<MDEvent<6>, 6>;
template class DLLExport MDBoxBase<MDEvent<7>, 7>;
template class DLLExport MDBoxBase<MDEvent<8>, 8>;
template class DLLExport MDBoxBase<MDEvent<9>, 9>;

// Instantiations for MDBox
template class DLLExport MDBox<MDLeanEvent<1>, 1>;
template class DLLExport MDBox<MDLeanEvent<2>, 2>;
template class DLLExport MDBox<MDLeanEvent<3>, 3>;
template class DLLExport MDBox<MDLeanEvent<4>, 4>;
template class DLLExport MDBox<MDLeanEvent<5>, 5>;
template class DLLExport MDBox<MDLeanEvent<6>, 6>;
template class DLLExport MDBox<MDLeanEvent<7>, 7>;
template class DLLExport MDBox<MDLeanEvent<8>, 8>;
template class DLLExport MDBox<MDLeanEvent<9>, 9>;
template class DLLExport MDBox<MDEvent<1>, 1>;
template class DLLExport MDBox<MDEvent<2>, 2>;
template class DLLExport MDBox<MDEvent<3>, 3>;
template class DLLExport MDBox<MDEvent<4>, 4>;
template class DLLExport MDBox<MDEvent<5>, 5>;
template class DLLExport MDBox<MDEvent<6>, 6>;
template class DLLExport MDBox<MDEvent<7>, 7>;
template class DLLExport MDBox<MDEvent<8>, 8>;
template class DLLExport MDBox<MDEvent<9>, 9>;

// Instantiations for MDEventWorkspace
template class DLLExport MDEventWorkspace<MDLeanEvent<1>, 1>;
template class DLLExport MDEventWorkspace<MDLeanEvent<2>, 2>;
template class DLLExport MDEventWorkspace<MDLeanEvent<3>, 3>;
template class DLLExport MDEventWorkspace<MDLeanEvent<4>, 4>;
template class DLLExport MDEventWorkspace<MDLeanEvent<5>, 5>;
template class DLLExport MDEventWorkspace<MDLeanEvent<6>, 6>;
template class DLLExport MDEventWorkspace<MDLeanEvent<7>, 7>;
template class DLLExport MDEventWorkspace<MDLeanEvent<8>, 8>;
template class DLLExport MDEventWorkspace<MDLeanEvent<9>, 9>;
template class DLLExport MDEventWorkspace<MDEvent<1>, 1>;
template class DLLExport MDEventWorkspace<MDEvent<2>, 2>;
template class DLLExport MDEventWorkspace<MDEvent<3>, 3>;
template class DLLExport MDEventWorkspace<MDEvent<4>, 4>;
template class DLLExport MDEventWorkspace<MDEvent<5>, 5>;
template class DLLExport MDEventWorkspace<MDEvent<6>, 6>;
template class DLLExport MDEventWorkspace<MDEvent<7>, 7>;
template class DLLExport MDEventWorkspace<MDEvent<8>, 8>;
template class DLLExport MDEventWorkspace<MDEvent<9>, 9>;

// Instantiations for MDGridBox
template class DLLExport MDGridBox<MDLeanEvent<1>, 1>;
template class DLLExport MDGridBox<MDLeanEvent<2>, 2>;
template class DLLExport MDGridBox<MDLeanEvent<3>, 3>;
template class DLLExport MDGridBox<MDLeanEvent<4>, 4>;
template class DLLExport MDGridBox<MDLeanEvent<5>, 5>;
template class DLLExport MDGridBox<MDLeanEvent<6>, 6>;
template class DLLExport MDGridBox<MDLeanEvent<7>, 7>;
template class DLLExport MDGridBox<MDLeanEvent<8>, 8>;
template class DLLExport MDGridBox<MDLeanEvent<9>, 9>;
template class DLLExport MDGridBox<MDEvent<1>, 1>;
template class DLLExport MDGridBox<MDEvent<2>, 2>;
template class DLLExport MDGridBox<MDEvent<3>, 3>;
template class DLLExport MDGridBox<MDEvent<4>, 4>;
template class DLLExport MDGridBox<MDEvent<5>, 5>;
template class DLLExport MDGridBox<MDEvent<6>, 6>;
template class DLLExport MDGridBox<MDEvent<7>, 7>;
template class DLLExport MDGridBox<MDEvent<8>, 8>;
template class DLLExport MDGridBox<MDEvent<9>, 9>;

// Instantiations for MDBin
template class DLLExport MDBin<MDLeanEvent<1>, 1>;
template class DLLExport MDBin<MDLeanEvent<2>, 2>;
template class DLLExport MDBin<MDLeanEvent<3>, 3>;
template class DLLExport MDBin<MDLeanEvent<4>, 4>;
template class DLLExport MDBin<MDLeanEvent<5>, 5>;
template class DLLExport MDBin<MDLeanEvent<6>, 6>;
template class DLLExport MDBin<MDLeanEvent<7>, 7>;
template class DLLExport MDBin<MDLeanEvent<8>, 8>;
template class DLLExport MDBin<MDLeanEvent<9>, 9>;
template class DLLExport MDBin<MDEvent<1>, 1>;
template class DLLExport MDBin<MDEvent<2>, 2>;
template class DLLExport MDBin<MDEvent<3>, 3>;
template class DLLExport MDBin<MDEvent<4>, 4>;
template class DLLExport MDBin<MDEvent<5>, 5>;
template class DLLExport MDBin<MDEvent<6>, 6>;
template class DLLExport MDBin<MDEvent<7>, 7>;
template class DLLExport MDBin<MDEvent<8>, 8>;
template class DLLExport MDBin<MDEvent<9>, 9>;

// Instantiations for MDBoxIterator
template class DLLExport MDBoxIterator<MDLeanEvent<1>, 1>;
template class DLLExport MDBoxIterator<MDLeanEvent<2>, 2>;
template class DLLExport MDBoxIterator<MDLeanEvent<3>, 3>;
template class DLLExport MDBoxIterator<MDLeanEvent<4>, 4>;
template class DLLExport MDBoxIterator<MDLeanEvent<5>, 5>;
template class DLLExport MDBoxIterator<MDLeanEvent<6>, 6>;
template class DLLExport MDBoxIterator<MDLeanEvent<7>, 7>;
template class DLLExport MDBoxIterator<MDLeanEvent<8>, 8>;
template class DLLExport MDBoxIterator<MDLeanEvent<9>, 9>;
template class DLLExport MDBoxIterator<MDEvent<1>, 1>;
template class DLLExport MDBoxIterator<MDEvent<2>, 2>;
template class DLLExport MDBoxIterator<MDEvent<3>, 3>;
template class DLLExport MDBoxIterator<MDEvent<4>, 4>;
template class DLLExport MDBoxIterator<MDEvent<5>, 5>;
template class DLLExport MDBoxIterator<MDEvent<6>, 6>;
template class DLLExport MDBoxIterator<MDEvent<7>, 7>;
template class DLLExport MDBoxIterator<MDEvent<8>, 8>;
template class DLLExport MDBoxIterator<MDEvent<9>, 9>;

/* CODE ABOWE WAS AUTO-GENERATED BY generate_mdevent_declarations.py - DO NOT
 * EDIT! */

//### END AUTO-GENERATED CODE
//##################################################################

//------------------------------- FACTORY METHODS
//------------------------------------------------------------------------------------------------------------------

/** Create a MDEventWorkspace of the given type
@param nd :: number of dimensions
@param eventType :: string describing the event type (MDEvent or MDLeanEvent)
@param preferredNormalization: the preferred normalization for the event
workspace
@param preferredNormalizationHisto: preferred normalization for histo workspaces
which derive
                                    from this event workspace
@return shared pointer to the MDEventWorkspace created (as a IMDEventWorkspace).
*/
API::IMDEventWorkspace_sptr MDEventFactory::CreateMDWorkspace(
    size_t nd, const std::string &eventType,
    const Mantid::API::MDNormalization &preferredNormalization,
    const Mantid::API::MDNormalization &preferredNormalizationHisto) {
  if (nd > MAX_MD_DIMENSIONS_NUM)
    throw std::invalid_argument(
        " there are more dimensions requested then instantiated");
  API::IMDEventWorkspace *pWs = (*(wsCreatorFP[nd]))(
      eventType, preferredNormalization, preferredNormalizationHisto);
  return boost::shared_ptr<API::IMDEventWorkspace>(pWs);
}

/** Create a MDBox or MDGridBoxof the given type
@param nDimensions  :: number of dimensions
@param Type         :: enum descibing the box (MDBox or MDGridBox) and the event
type (MDEvent or MDLeanEvent)
@param splitter     :: shared pointer to the box controller responsible for
splitting boxes. The BC is not incremented as boxes take usual pointer from this
pointer
@param extentsVector:: box extents in all n-dimensions (min-max)
@param depth        :: the depth of the box within the box tree
@param nBoxEvents   :: if defined, specify the memory the box should allocate to
accept events -- not used for MDGridBox
@param boxID        :: the unique identifier, referencing location of the box in
1D linked list of boxes.  -- not used for MDGridBox

@return pointer to the IMDNode with proper box created.
*/

API::IMDNode *MDEventFactory::createBox(
    size_t nDimensions, MDEventFactory::BoxType Type,
    API::BoxController_sptr &splitter,
    const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t>>
        &extentsVector,
    const uint32_t depth, const size_t nBoxEvents, const size_t boxID) {

  if (nDimensions > MAX_MD_DIMENSIONS_NUM)
    throw std::invalid_argument(
        " there are more dimensions requested then instantiated");

  size_t id = nDimensions * MDEventFactory::NumBoxTypes + Type;
  if (extentsVector.size() != nDimensions) // set defaults
  {
    std::vector<Mantid::Geometry::MDDimensionExtents<coord_t>> defaultExtents(
        nDimensions);
    for (size_t i = 0; i < nDimensions; i++) {
      // set to smaller than float max, so the entire range fits in a float.
      defaultExtents[i].setExtents(-1e30f, 1e30f);
    }
    return (*(boxCreatorFP[id]))(splitter.get(), defaultExtents, depth,
                                 nBoxEvents, boxID);
  }

  return (*(boxCreatorFP[id]))(splitter.get(), extentsVector, depth, nBoxEvents,
                               boxID);
}

//------------------------------- FACTORY METHODS END
//--------------------------------------------------------------------------------------------------------------

/// static vector, conaining the pointers to the functions creating MD boxes
std::vector<MDEventFactory::fpCreateBox> MDEventFactory::boxCreatorFP(
    MDEventFactory::NumBoxTypes *(MDEventFactory::MAX_MD_DIMENSIONS_NUM + 1),
    nullptr);
// static vector, conaining the pointers to the functions creating MD Workspaces
std::vector<MDEventFactory::fpCreateMDWS>
    MDEventFactory::wsCreatorFP(MDEventFactory::MAX_MD_DIMENSIONS_NUM + 1,
                                nullptr);

//########### Teplate methaprogrammed CODE SOURCE start:
//-------------------------------------

//-------------------------------------------------------------- MD Workspace
// constructor wrapper
/** Template to create md workspace with specific number of dimensions
 * @param eventType -- type of event (lean or full) to generate workspace for
 * @param preferredNormalization: the preferred normalization for the event
 * workspace
 * @param preferredNormalizationHisto: the preferred normalization for a derived
 * histo workspace
 */
template <size_t nd>
API::IMDEventWorkspace *MDEventFactory::createMDWorkspaceND(
    const std::string &eventType,
    const Mantid::API::MDNormalization &preferredNormalization,
    const Mantid::API::MDNormalization &preferredNormalizationHisto) {
  if (eventType == "MDEvent")
    return new MDEventWorkspace<MDEvent<nd>, nd>(preferredNormalization,
                                                 preferredNormalizationHisto);
  else if (eventType == "MDLeanEvent")
    return new MDEventWorkspace<MDLeanEvent<nd>, nd>(
        preferredNormalization, preferredNormalizationHisto);
  else
    throw std::invalid_argument("Unknown event type " + eventType +
                                " passed to CreateMDWorkspace.");
}
/** Template to create md workspace w ith 0 number of dimentisons. As this is
 * wrong, just throws. Used as terminator and check for the wrong dimensions
 * number
 * @param eventType -- type of event (lean or full) to generate workspace for -
 * -does not actually used.
 * @param preferredNormalization: the preferred normalization of the workspace
 * @param preferredNormalizationHisto: the preferred normalization of the
 * derived histo workspace
 */
template <>
API::IMDEventWorkspace *MDEventFactory::createMDWorkspaceND<0>(
    const std::string &eventType,
    const Mantid::API::MDNormalization &preferredNormalization,
    const Mantid::API::MDNormalization &preferredNormalizationHisto) {
  UNUSED_ARG(eventType);
  UNUSED_ARG(preferredNormalization);
  UNUSED_ARG(preferredNormalizationHisto);
  throw std::invalid_argument("Workspace can not have 0 dimensions");
}

//-------------------------------------------------------------- MD BOX
// constructor wrapper
/** Method to create any MDBox type with 0 number of dimensions. As it is wrong,
 * just throws */
API::IMDNode *MDEventFactory::createMDBoxWrong(
    API::BoxController *,
    const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t>> &,
    const uint32_t, const size_t, const size_t) {
  throw std::invalid_argument("MDBox/MDGridBox can not have 0 dimensions");
}
/**Method to create MDBox for lean events (Constructor wrapper) with given
 * number of dimensions
 * @param splitter :: BoxController that controls how boxes split
 * @param extentsVector :: vector defining the extents of the box in all
 * n-dimensions
 * @param depth :: splitting depth of the new box.
 * @param nBoxEvents :: number of events to reserve memory for (if needed).
 * @param boxID :: id for the given box
 */
template <size_t nd>
API::IMDNode *MDEventFactory::createMDBoxLean(
    API::BoxController *splitter,
    const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t>>
        &extentsVector,
    const uint32_t depth, const size_t nBoxEvents, const size_t boxID) {
  return new MDBox<MDLeanEvent<nd>, nd>(splitter, depth, extentsVector,
                                        nBoxEvents, boxID);
}
/**Method to create MDBox for events (Constructor wrapper) with given number of
 * dimensions
 * @param splitter :: BoxController that controls how boxes split
 * @param extentsVector :: vector defining the extents of the box in all
 * n-dimensions
 * @param depth :: splitting depth of the new box.
 * @param nBoxEvents :: number of events to reserve memory for (if needed).
 * @param boxID :: id for the given box
 */
template <size_t nd>
API::IMDNode *MDEventFactory::createMDBoxFat(
    API::BoxController *splitter,
    const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t>>
        &extentsVector,
    const uint32_t depth, const size_t nBoxEvents, const size_t boxID) {
  return new MDBox<MDEvent<nd>, nd>(splitter, depth, extentsVector, nBoxEvents,
                                    boxID);
}
/**Method to create MDGridBox for lean events (Constructor wrapper) with given
 * number of dimensions
 * @param splitter :: BoxController that controls how boxes split
 * @param extentsVector :: vector defining the extents of the box in all
 * n-dimensions
 * @param depth :: splitting depth of the new box.
 * @param nBoxEvents  -- not used
 * @param boxID ::   --- not used
 */
template <size_t nd>
API::IMDNode *MDEventFactory::createMDGridBoxLean(
    API::BoxController *splitter,
    const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t>>
        &extentsVector,
    const uint32_t depth, const size_t /*nBoxEvents*/, const size_t /*boxID*/) {
  return new MDGridBox<MDLeanEvent<nd>, nd>(splitter, depth, extentsVector);
}
/**Method to create MDGridBox for events (Constructor wrapper) with given number
 * of dimensions
 * @param splitter :: BoxController that controls how boxes split
 * @param extentsVector :: vector defining the extents of the box in all
 * n-dimensions
 * @param depth :: splitting depth of the new box.
 * @param nBoxEvents  -- not used
 * @param boxID ::   --- not used
 */
template <size_t nd>
API::IMDNode *MDEventFactory::createMDGridBoxFat(
    API::BoxController *splitter,
    const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t>>
        &extentsVector,
    const uint32_t depth, const size_t /*nBoxEvents*/, const size_t /*boxID*/) {
  return new MDGridBox<MDEvent<nd>, nd>(splitter, depth, extentsVector);
}
//-------------------------------------------------------------- MD BOX
// constructor wrapper -- END

// the class instantiated by compiler at compilation time and generates the map,
// between the number of dimensions and the function, which process this number
// of dimensions
template <size_t nd> class LOOP {
public:
  LOOP() { EXEC(); }
  static inline void EXEC() {
    LOOP<nd - 1>::EXEC();
    MDEventFactory::wsCreatorFP[nd] = &MDEventFactory::createMDWorkspaceND<nd>;

    MDEventFactory::boxCreatorFP[MDEventFactory::NumBoxTypes * nd +
                                 MDEventFactory::MDBoxWithLean] =
        &MDEventFactory::createMDBoxLean<nd>;
    MDEventFactory::boxCreatorFP[MDEventFactory::NumBoxTypes * nd +
                                 MDEventFactory::MDBoxWithFat] =
        &MDEventFactory::createMDBoxFat<nd>;
    MDEventFactory::boxCreatorFP[MDEventFactory::NumBoxTypes * nd +
                                 MDEventFactory::MDGridBoxWithLean] =
        &MDEventFactory::createMDGridBoxLean<nd>;
    MDEventFactory::boxCreatorFP[MDEventFactory::NumBoxTypes * nd +
                                 MDEventFactory::MDGridBoxWithFat] =
        &MDEventFactory::createMDGridBoxFat<nd>;
  }
};
// the class terminates the compitlation-time metaloop and sets up functions
// which process 0-dimension workspace operations (throw invalid argument)
template <> class LOOP<0> {
public:
  static inline void EXEC() {
    MDEventFactory::wsCreatorFP[0] = &MDEventFactory::createMDWorkspaceND<0>;

    MDEventFactory::boxCreatorFP[MDEventFactory::MDBoxWithLean] =
        &MDEventFactory::createMDBoxWrong;
    MDEventFactory::boxCreatorFP[MDEventFactory::MDBoxWithFat] =
        &MDEventFactory::createMDBoxWrong;
    MDEventFactory::boxCreatorFP[MDEventFactory::MDGridBoxWithLean] =
        &MDEventFactory::createMDBoxWrong;
    MDEventFactory::boxCreatorFP[MDEventFactory::MDGridBoxWithFat] =
        &MDEventFactory::createMDBoxWrong;
  }
};
//########### Teplate methaprogrammed CODE SOURCE END:
//-------------------------------------
// statically instantiate the code, defined by the class above and assocoate it
// with events factory
LOOP<MDEventFactory::MAX_MD_DIMENSIONS_NUM> MDEventFactory::CODE_GENERATOR;

} // namespace DataObjects
} // namespace Mantid
