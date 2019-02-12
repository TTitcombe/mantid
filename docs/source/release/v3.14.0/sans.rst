============
SANS Changes
============

.. contents:: Table of Contents
   :local:

New
###
* Added manual saving functionality to the new GUI.

- :ref:`SANSILLReduction <algm-SANSILLReduction>` performs SANS data reduction for ILL instruments D11, D22, D33.
- :ref:`SANSILLIntegration <algm-SANSILLIntegration>` performs integration of corrected SANS data to produce I(Q), I(Phi,Q) or I(Qx,Qy).

ISIS SANS Interface
-------------------

Improved
########
* Updated workspace naming scheme for new backend.
* Added shortcut keys to copy/paste/cut rows of data.
* Added shortcut keys to delete or add rows.
* Added tabbing support to table.
* Added error notifications on a row by row basis.
* Updated file adding to prefix the instrument name
* Updated file finding to be able to find added runs without instrument name prefix
* Updated GUI code so calculated merge scale and shift are shown after reduction.
* Removed instrument selection box. Instrument is now determined by user file.
* Automatically remembers last loaded user file
* Added display of current save directory
* Added a "process all" and "process selected" button to the batch table in place of "process" button.
* Added a load button to load selected workspaces without processing.
* Added save_can option to output unsubtracted can and sample workspaces.
* Autocomplete for Sample shape column in the table.
* Can separate items in variable q binning with commas or spaces. E.g. L/Q 0.0, 0.02 0.3 0.05, 0.8
* Can export table as a csv, which can be re-loaded as a batch file.
* File path to batch file will be added to your directories automatically upon loading
* If loading of user file fails, user file field will remain empty to make it clear it has not be loaded successfully.
* Workspaces are centred upon loading.
* All limit strings in the user file (L/ ) are now space-separable to allow for uniform structure in the user file. For backwards compatibility, any string which was comma separable remains comma separable as well.
* For ZOOM, SHIFT user file command now moves monitor 5.

Bug fixes
#########
* Fixed an issue where the run tab was difficult to select.
* Changed the geometry names from CylinderAxisUP, Cuboid and CylinderAxisAlong to Cylinder, FlatPlate and Disc
* The GUI now correctly resets to default values when a new user file is loaded.
* The GUI no longer hangs whilst searching the archive for files.
* Updated the options and units displayed in wavelength and momentum range combo boxes.
* Fixed a bug which crashed the beam centre finder if a phi mask was set.
* Removed option to process in non-compatibility mode to avoid calculation issues.
* GUI can correctly read user files with variable step sizes, in /LOG and /LIN modes.
* Fixed occasional crash when entering data into table.
* Fixed error message when trying to load or process table with empty rows.
* Removed option to process in non-compatibility mode to avoid calculation issues.
* Default name for added runs has correct number of digits.
* RKH files no longer append to existing files, but overwrite instead.

Improvements
############

- :ref:`Q1DWeighted <algm-Q1DWeighted>` now supports the option of asymmetric wedges for unisotropic scatterer.

Removed
#######

- Obsolete *SetupILLD33Reduction* algorithm was removed.


ORNL SANS
---------

Improvements
############

- ORNL HFIR SANS instruments have new geometries. The monitors have now a shape associated to them. Detector will move to the right position based on log values.


:ref:`Release 3.14.0 <v3.14.0>`
