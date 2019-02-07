# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantid workbench.
import unittest

from qtpy.QtWidgets import QApplication

from mantidqt.widgets.workspacedisplay.test_helper.qt_widget_finder import QtWidgetFinder

try:
    from unittest import MagicMock, patch
except ImportError:
    from mock import MagicMock, patch

from mantidqt.utils.qt.test import GuiTest

from workbench.plotting.figuremanager import FigureCanvasQTAgg, FigureManagerWorkbench


class FigureManagerWorkbenchTest(GuiTest, QtWidgetFinder):

    @patch("workbench.plotting.qappthreadcall.QAppThreadCall")
    def test_construction(self, mock_qappthread):
        mock_qappthread.return_value = mock_qappthread
        fig = MagicMock()
        canvas = FigureCanvasQTAgg(fig)
        fig_mgr = FigureManagerWorkbench(canvas, 1)
        self.assertTrue(fig_mgr is not None)

    @patch("workbench.plotting.qappthreadcall.QAppThreadCall")
    def test_closing_deletion(self, mock_qappthread):
        fig = MagicMock()
        canvas = FigureCanvasQTAgg(fig)
        fig_mgr = FigureManagerWorkbench(canvas, 1)
        self.assertTrue(fig_mgr is not None)
        self.assert_window_created()
        canvas.close()
        fig_mgr.destroy()

        QApplication.processEvents()
        self.find_qt_widget("fig")


if __name__ == "__main__":
    unittest.main()
