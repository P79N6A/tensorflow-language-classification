/*****************************************************************************
 *   Copyright (C) 2004-2015 The PaGMO development team,                     *
 *   Advanced Concepts Team (ACT), European Space Agency (ESA)               *
 *                                                                           *
 *   https://github.com/esa/pagmo                                            *
 *                                                                           *
 *   act@esa.int                                                             *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.               *
 *****************************************************************************/


#include "hv2d.h"
#include "wfg.h"
#include "base.h"
#include <algorithm>
#include <boost/bind.hpp>

namespace pagmo { namespace util { namespace hv_algorithm {

/// Comparator function for sorting
/**
 * Comparison function for WFG. Can't be static in order to have access to member variable m_current_slice.
 */
bool wfg::cmp_points(double* a, double* b) const
{
	for(int i = m_current_slice - 1; i >= 0 ; --i){
		if (a[i] > b[i]) {
			return true;
		} else if(a[i] < b[i]) {
			return false;
		}
	}
	return false;
}

/// Constructor
wfg::wfg(const unsigned int stop_dimension) : m_current_slice(0), m_stop_dimension(stop_dimension)
{
	if (stop_dimension < 2 ) {
		pagmo_throw(value_error, "Stop dimension for WFG must be greater than or equal to 2");
	}
}

/// Compute hypervolume
/**
 * Computes the hypervolume using the WFG algorithm.
 *
 * @param[in] points vector of points containing the D-dimensional points for which we compute the hypervolume
 * @param[in] r_point reference point for the points
 *
 * @return hypervolume.
 */
double wfg::compute(std::vector<fitness_vector> &points, const fitness_vector &r_point) const
{
	allocate_wfg_members(points, r_point);
	double hv = compute_hv(1);
	free_wfg_members();
	return hv;
}

/// Contributions method
/**
 * This method employs a slightly modified version of the original WFG algorithm to suit the computation of the exclusive contributions.
 * It differs from the IWFG algorithm (referenced below), as we do not use the priority-queueing mechanism, but compute every exclusive contribution instead.
 * This may suggest that the algorithm for the extreme contributor itself reduces to the 'naive' approach. It is not the case however,
 * as we utilize the benefits of the 'limitset', before we begin the recursion.
 * This simplifies the sub problems for each exclusive computation right away, which makes the whole algorithm much faster, and in many cases only slower than regular WFG algorithm by a constant factor.
 *
 * @see "Lyndon While and Lucas Bradstreet. Applying the WFG Algorithm To Calculate Incremental Hypervolumes. 2012 IEEE Congress on Evolutionary Computation. CEC 2012, pages 489-496. IEEE, June 2012."
 *
 * @param[in] points vector of points containing the D-dimensional points for which we compute the hypervolume
 * @param[in] r_point reference point for the points
 */
std::vector<double> wfg::contributions(std::vector<fitness_vector> &points, const fitness_vector &r_point) const
{
	std::vector<double> c;
	c.reserve(points.size());

	// Allocate the same members as for 'compute' method
	allocate_wfg_members(points, r_point);

	// Prepare the memory for first front
	double** fr = new double*[m_max_points];
	for(unsigned int i = 0 ; i < m_max_points ; ++i) {
		fr[i]=new double[m_current_slice];
	}
	m_frames[m_n_frames] = fr;
	m_frames_size[m_n_frames] = 0;
	++m_n_frames;

	for(unsigned int p_idx = 0 ; p_idx < m_max_points ; ++p_idx) {
		limitset(0, p_idx, 1);
		c.push_back(exclusive_hv(p_idx, 1));
	}

	// Free the contributions and the remaining WFG members
	free_wfg_members();

	return c;
}

/// Allocate the memory for the 'compute' method
void wfg::allocate_wfg_members(std::vector<fitness_vector> &points, const fitness_vector &r_point) const
{
	m_max_points = points.size();
	m_max_dim = r_point.size();

	m_refpoint = new double[m_max_dim];
	for(unsigned int d_idx = 0 ; d_idx < m_max_dim ; ++d_idx) {
		m_refpoint[d_idx] = r_point[d_idx];
	}

	// Reserve the space beforehand for each level or recursion.
	// WFG with slicing feature will not go recursively deeper than the dimension size.
	m_frames = new double**[m_max_dim];
	m_frames_size = new unsigned int[m_max_dim];

	// Copy the initial set into the frame at index 0.
	double** fr = new double*[m_max_points];
	for(unsigned int p_idx = 0 ; p_idx < m_max_points ; ++p_idx) {
		fr[p_idx] = new double[m_max_dim];
		for(unsigned int d_idx = 0 ; d_idx < m_max_dim ; ++d_idx) {
			fr[p_idx][d_idx] = points[p_idx][d_idx];
		}
	}
	m_frames[0] = fr;
	m_frames_size[0] = m_max_points;
	m_n_frames = 1;

	// Variable holding the current "depth" of dimension slicing. We progress by slicing dimensions from the end.
	m_current_slice = m_max_dim;
}

/// Free the previously allocated memory
void wfg::free_wfg_members() const
{
	// Free the memory.
	delete[] m_refpoint;

	for(unsigned int fr_idx = 0 ; fr_idx < m_n_frames ; ++fr_idx) {
		for(unsigned int p_idx = 0; p_idx < m_max_points ; ++p_idx) {
			delete[] m_frames[fr_idx][p_idx];
		}
		delete[] m_frames[fr_idx];
	}
	delete[] m_frames;
	delete[] m_frames_size;
}

/// Limit the set of points to point at p_idx
void wfg::limitset(const unsigned int begin_idx, const unsigned int p_idx, const unsigned int rec_level) const
{
	double **points = m_frames[rec_level - 1];
	unsigned int n_points = m_frames_size[rec_level - 1];

	int no_points = 0;

	double* p = points[p_idx];
	double** frame = m_frames[rec_level];

	for(unsigned int idx = begin_idx; idx < n_points; ++idx) {
		if (idx == p_idx) {
			continue;
		}

		for(fitness_vector::size_type f_idx = 0; f_idx < m_current_slice; ++f_idx) {
			frame[no_points][f_idx] = std::max(points[idx][f_idx], p[f_idx]);
		}

		std::vector<int> cmp_results;
		cmp_results.resize(no_points);
		double* s = frame[no_points];

		bool keep_s = true;

		// Check whether any point is dominating the point 's'.
		for(int q_idx = 0; q_idx < no_points; ++q_idx) {
			cmp_results[q_idx] = base::dom_cmp(s, frame[q_idx], m_current_slice);
			if (cmp_results[q_idx] == base::DOM_CMP_B_DOMINATES_A) {
				keep_s = false;
				break;
			}
		}
		// If neither is, remove points dominated by 's' (we store that during the first loop).
		if( keep_s ) {
			int prev = 0;
			int next = 0;
			while(next < no_points) {
				if( cmp_results[next] != base::DOM_CMP_A_DOMINATES_B && cmp_results[next] != base::DOM_CMP_A_B_EQUAL) {
					if(prev < next) {
						for(unsigned int d_idx = 0; d_idx < m_current_slice ; ++d_idx) {
							frame[prev][d_idx] = frame[next][d_idx];
						}
					}
					++prev;
				}
				++next;
			}
			// Append 's' at the end, if prev==next it's not necessary as it's already there.
			if(prev < next) {
				for(unsigned int d_idx = 0; d_idx < m_current_slice ; ++d_idx) {
					frame[prev][d_idx] = s[d_idx];
				}
			}
			no_points = prev + 1;
		}
	}

	m_frames_size[rec_level] = no_points;
}

/// Compute the hypervolume recursively
double wfg::compute_hv(const unsigned int rec_level) const
{
	double **points = m_frames[rec_level - 1];
	unsigned int n_points = m_frames_size[rec_level - 1];

	// Simple inclusion-exclusion for one and two points
	if (n_points == 1) {
		return base::volume_between(points[0], m_refpoint, m_current_slice);
	}
	else if (n_points == 2) {
		double hv = base::volume_between(points[0], m_refpoint, m_current_slice)
			+ base::volume_between(points[1], m_refpoint, m_current_slice);
		double isect = 1.0;
		for(unsigned int i=0;i<m_current_slice;++i) {
			isect *= (m_refpoint[i] - std::max(points[0][i], points[1][i]));
		}
		return hv - isect;
	}

	// If already sliced to dimension at which we use another algorithm.
	if (m_current_slice == m_stop_dimension) {

		if (m_stop_dimension == 2) {
			// Use a very efficient version of hv2d
			return hv2d().compute(points, n_points, m_refpoint);
		} else {
			// Let hypervolume object pick the best method otherwise.
			std::vector<fitness_vector> points_cpy;
			points_cpy.reserve(n_points);
			for(unsigned int i = 0 ; i < n_points ; ++i) {
				points_cpy.push_back(fitness_vector(points[i], points[i] + m_current_slice));
			}
			fitness_vector r_cpy(m_refpoint, m_refpoint + m_current_slice);

			hypervolume hv = hypervolume(points_cpy, false);
			hv.set_copy_points(false);
			return hv.compute(r_cpy);
		}
	} else {
		// Otherwise, sort the points in preparation for the next recursive step
		// Bind the object under "this" pointer to the cmp_points method so it can be used as a valid comparator function for std::sort
		// We need that in order for the cmp_points to have acces to the m_current_slice member variable.
		std::sort(points, points + n_points, boost::bind(&wfg::cmp_points, this, _1, _2));
	}

	double H = 0.0;
	--m_current_slice;

	if(rec_level >= m_n_frames) {
		double** fr = new double*[m_max_points];
		for(unsigned int i = 0 ; i < m_max_points ; ++i) {
			fr[i]=new double[m_current_slice];
		}
		m_frames[m_n_frames] = fr;
		m_frames_size[m_n_frames] = 0;
		++m_n_frames;
	}

	for(unsigned int p_idx = 0 ; p_idx < n_points ; ++p_idx) {
		limitset(p_idx + 1, p_idx, rec_level);

		H += fabs((points[p_idx][m_current_slice] - m_refpoint[m_current_slice]) * exclusive_hv(p_idx, rec_level));
	}
	++m_current_slice;
	return H;
}

/// Compute the exclusive hypervolume of point at p_idx
double wfg::exclusive_hv(const unsigned int p_idx, const unsigned int rec_level) const
{
	//double H = base::volume_between(points[p_idx], m_refpoint, m_current_slice);
	double H = base::volume_between(m_frames[rec_level - 1][p_idx], m_refpoint, m_current_slice);

	if (m_frames_size[rec_level] == 1) {
		H -= base::volume_between(m_frames[rec_level][0], m_refpoint, m_current_slice);
	} else if (m_frames_size[rec_level] > 1) {
		H -= compute_hv(rec_level + 1);
	}

	return H;
}

/// Verify before compute method
/**
 * Verifies whether given algorithm suits the requested data.
 *
 * @param[in] points vector of points containing the D-dimensional points for which we compute the hypervolume
 * @param[in] r_point reference point for the vector of points
 *
 * @throws value_error when trying to compute the hypervolume for the non-maximal reference point
 */
void wfg::verify_before_compute(const std::vector<fitness_vector> &points, const fitness_vector &r_point) const
{
	base::assert_minimisation(points, r_point);
}

/// Clone method.
base_ptr wfg::clone() const
{
	return base_ptr(new wfg(*this));
}

/// Algorithm name
std::string wfg::get_name() const
{
	return "WFG algorithm";
}

} } }

BOOST_CLASS_EXPORT_IMPLEMENT(pagmo::util::hv_algorithm::wfg)
