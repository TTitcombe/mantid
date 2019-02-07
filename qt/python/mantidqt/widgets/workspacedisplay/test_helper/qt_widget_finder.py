# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.
from qtpy.QtWidgets import QApplication


class QtWidgetFinder(object):
    """
    This class provides common functions for finding widgets within all currently existing ones.
    """

    def find_qt_widget(self, name):
        a = QApplication.topLevelWidgets()
        all = [x for x in a if name.lower() in str(type(x)).lower()]

        self.assertEqual(0, len(all),
                         "Alive widgets were detected in the QApplication. Something has not been deleted: {}".format(all))

    def assert_window_created(self):
        self.assertGreater(QApplication.topLevelWidgets(), 0)
