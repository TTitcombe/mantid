#pylint: disable=W0403,R0902
from fourcircle_utility import *

import sys
sys.path.append('/Users/wzz/MantidBuild/debug/bin/')

from mantid.api import AnalysisDataService
from mantid.kernel import V3D

__author__ = 'wzz'


class PeakProcessHelper(object):
    """ Class containing a peak's information for GUI
    In order to manage some operations for a peak
    It does not contain peak workspace but will hold
    """
    def __init__(self, exp_number, scan_number, peak_ws_name):
        """ Initialization
        Purpose: set up unchanged parameters including experiment number, scan number and peak workspace's name
        """
        # check
        assert isinstance(exp_number, int) and isinstance(scan_number, int)
        assert isinstance(peak_ws_name, str), 'Peak workspace name %s must be a string.' \
                                              'but not %s.' % (str(peak_ws_name),
                                                               str(type(peak_ws_name)))
        assert AnalysisDataService.doesExist(peak_ws_name), 'Peak workspace %s does not' \
                                                            'exist.' % peak_ws_name

        # set
        self._myExpNumber = exp_number
        self._myScanNumber = scan_number
        self._myPeakWorkspaceName = peak_ws_name

        #
        self._myDataMDWorkspaceName = None

        # Define class variable
        self._userHKL = None
        self._currHKL = None

        self._calculatedHKL = numpy.array([0., 0., 0.])
        self._avgPeakCenter = None

        self._myPeakWSKey = (None, None, None)
        self._myPeakIndex = None

        self._myLastPeakUB = None

        self._myIntensity = 0.
        self._mySigma = 0.

        return

    def calculate_peak_center(self):
        """ Calculate peak's center by averaging the peaks found and stored in PeakWorkspace
        :return:
        """
        # Go through the peak workspaces to calculate peak center with weight (monitor and counts)
        peak_ws = AnalysisDataService.retrieve(self._myPeakWorkspaceName)

        # spice table workspace
        spice_table_name = get_spice_table_name(self._myExpNumber, self._myScanNumber)
        spice_table_ws = AnalysisDataService.retrieve(spice_table_name)

        pt_spice_row_dict = build_pt_spice_table_row_map(spice_table_ws)
        det_col_index = spice_table_ws.getColumnNames().index('detector')
        monitor_col_index = spice_table_ws.getColumnNames().index('monitor')

        num_found_peaks = peak_ws.rowCount()

        q_sample_sum = numpy.array([0., 0., 0.])
        weight_sum = 0.

        for i_peak in xrange(num_found_peaks):
            # get peak
            peak_i = peak_ws.getPeak(i_peak)
            run_number = peak_i.getRunNumber()
            # get Pt. number
            pt_number = run_number % self._myScanNumber
            # get row number and then detector counts and monitor counts
            if pt_number not in pt_spice_row_dict:
                # skip
                print '[Error] Scan %d Peak %d Pt %d cannot be located.' % (self._myScanNumber, i_peak, pt_number)
                continue

            row_index = pt_spice_row_dict[pt_number]
            det_counts = spice_table_ws.cell(row_index, det_col_index)
            monitor_counts = spice_table_ws.cell(row_index, monitor_col_index)
            if monitor_counts < 1.:
                # skip zero-count
                continue
            # convert q sample from V3D to ndarray
            q_i = peak_i.getQSampleFrame()
            q_array = numpy.array([q_i.X(), q_i.Y(), q_i.Z()])
            # calculate weight
            weight_i = float(det_counts)/float(monitor_counts)
            # contribute to total
            weight_sum += weight_i
            q_sample_sum += q_array * weight_i
            # set non-normalized peak intensity as detector counts (roughly)
            peak_i.setIntensity(det_counts)
        # END-FOR (i_peak)

        self._avgPeakCenter = q_sample_sum/weight_sum
        print '[DB] Weighted peak center = %s' % str(self._avgPeakCenter)

        return

    def get_peak_centre(self):
        """ get weighted peak centre
        :return: Qx, Qy, Qz (3-double-tuple)
        """
        assert isinstance(self._avgPeakCenter, numpy.ndarray)
        return self._avgPeakCenter[0], self._avgPeakCenter[1], self._avgPeakCenter[2]

    def get_peak_centre_v3d(self):
        """ Returned the statistically averaged peak center in V3D
        :return:
        """
        q_x, q_y, q_z = self.get_peak_centre()
        q_3d = V3D(q_x, q_y, q_z)

        return q_3d

    def get_peak_workspace(self):
        """
        Get peak workspace related
        :return:
        """
        peak_ws = AnalysisDataService.retrieve(self._myPeakWorkspaceName)
        assert peak_ws
        return peak_ws

    def get_current_hkl(self):
        """ Get user's last setup HKL
        :return:
        """
        if self._currHKL is None:
            raise RuntimeError('Exp %d Scan %d does not have CURRENT HKL set up yet.' % (self._myExpNumber,
                                                                                         self._myScanNumber))

        return self._currHKL

    def get_spice_hkl(self):
        """
        Get HKL set to this object by client
        :return: 3-tuple of float as (H, K, L)
        """
        # get HKL from SPICE table if not set up yet.
        if self._userHKL is None:
            self.retrieve_hkl_from_spice_table()

        # check
        assert self._userHKL is not None, 'Spice/user HKL has not been set.'

        return self._userHKL[0], self._userHKL[1], self._userHKL[2]

    def get_weighted_peak_centres(self):
        """ Get the peak centers found in peak workspace.
        Guarantees: the peak centers and its weight (detector counts) are exported
        :return: 2 lists: list of 3-tuple (Qx, Qy, Qz) and list of double (Det_Counts)
        """
        peak_ws = AnalysisDataService.retrieve(self._myPeakWorkspaceName)

        peak_center_list = list()
        peak_intensity_list = list()
        num_peaks = peak_ws.getNumberPeaks()
        for i_peak in xrange(num_peaks):
            peak_i = peak_ws.getPeak(i_peak)
            center_i = peak_i.getQSampleFrame()
            intensity_i = peak_i.getIntensity()
            peak_center_list.append((center_i.X(), center_i.Y(), center_i.Z()))
            peak_intensity_list.append(intensity_i)
        # END-FOR

        return peak_center_list, peak_intensity_list

    def retrieve_hkl_from_spice_table(self):
        """ Get averaged HKL from SPICE table
        HKL will be averaged from SPICE table by assuming the value in SPICE might be right
        :return:
        """
        # get SPICE table
        spice_table_name = get_spice_table_name(self._myExpNumber, self._myScanNumber)
        assert AnalysisDataService.doesExist(spice_table_name), 'Spice table for exp %d scan %d cannot be found.' \
                                                              '' % (self._myExpNumber, self._myScanNumber)

        spice_table_ws = AnalysisDataService.retrieve(spice_table_name)

        # get HKL column indexes
        h_col_index = spice_table_ws.getColumnNames().index('h')
        k_col_index = spice_table_ws.getColumnNames().index('k')
        l_col_index = spice_table_ws.getColumnNames().index('l')

        # scan each Pt.
        hkl = numpy.array([0., 0., 0.])

        num_rows = spice_table_ws.rowCount()
        for row_index in xrange(num_rows):
            mi_h = spice_table_ws.cell(row_index, h_col_index)
            mi_k = spice_table_ws.cell(row_index, k_col_index)
            mi_l = spice_table_ws.cell(row_index, l_col_index)
            hkl += numpy.array([mi_h, mi_k, mi_l])
        # END-FOR

        self._userHKL = hkl/num_rows

        return self._userHKL

    def set_data_ws_name(self, md_ws_name):
        """ Set the name of MDEventWorkspace with merged Pts.
        :param md_ws_name:
        :return:
        """
        assert isinstance(md_ws_name, str)
        assert AnalysisDataService.doesExist(md_ws_name)

        self._myDataMDWorkspaceName = md_ws_name

        return

    def set_hkl(self, hkl):
        """ Set current HKL which may come from any source, such as user, spice or calculation
        :param hkl: 3-item-list or 3-tuple for HKL
        :return:
        """
        # check
        assert len(hkl) == 3

        self._currHKL = hkl

        return

    def set_indexed_hkl(self, hkl):
        """
        Set HKL to PeakInfo.  HKL is calculated via peak indexing
        Requirements: input HKL is a list of 3 elements
        :param hkl:
        :return:
        """
        assert isinstance(hkl, list) and len(hkl) == 3

        for i in xrange(3):
            self._calculatedHKL[i] = hkl[i]

        return

    def get_intensity(self):
        """ Get current peak intensity
        :return:
        """
        return self._myIntensity

    def set_intensity(self, peak_intensity):
        """ Set peak intensity
        :param peak_intensity:
        :return:
        """
        assert isinstance(peak_intensity, float) and peak_intensity >= -0.

        self._myIntensity = peak_intensity

        return

    def get_sigma(self):
        """ Get peak intensity's sigma
        :return:
        """
        return self._mySigma

    def set_sigma(self, sigma):
        """ set peak intensity's sigma
        :return:
        """
        assert isinstance(sigma, float) and sigma > -0.

        self._mySigma = sigma

        return

    def set_user_hkl(self, mi_h, mi_k, mi_l):
        """
        Set HKL to this peak Info
        :param mi_h:
        :param mi_k:
        :param mi_l:
        :return:
        """
        assert isinstance(mi_h, float)
        assert isinstance(mi_k, float)
        assert isinstance(mi_l, float)

        self._userHKL[0] = mi_h
        self._userHKL[1] = mi_k
        self._userHKL[2] = mi_l

        print '[DB] PeakInfo Set User HKL to (%f, %f, %f) ' % (self._userHKL[0], self._userHKL[1],
                                                               self._userHKL[2])

        return

    def get_experiment_info(self):
        """

        :return: 2-tuple of integer as experiment number
        """
        return self._myExpNumber, self._myScanNumber

    def get_sample_frame_q(self, peak_index):
        """
        Get Q in sample frame
        :return: 3-tuple of floats as Qx, Qy, Qz
        """
        peak_ws = AnalysisDataService.retrieve(self._myPeakWorkspaceName)
        peak = peak_ws.getPeak(peak_index)
        q_sample = peak.getQSampleFrame()

        return q_sample.getX(), q_sample.getY(), q_sample.getZ()


def build_pt_spice_table_row_map(spice_table_ws):
    """
    Build a lookup dictionary for Pt number and row number
    :param spice_table_ws:
    :return:
    """
    pt_spice_row_dict = dict()
    num_rows = spice_table_ws.rowCount()
    pt_col_index = spice_table_ws.getColumnNames().index('Pt.')

    for i_row in xrange(num_rows):
        pt_number = int(spice_table_ws.cell(i_row, pt_col_index))
        pt_spice_row_dict[pt_number] = i_row

    return pt_spice_row_dict
