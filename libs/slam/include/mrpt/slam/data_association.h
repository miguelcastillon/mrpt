/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */
#pragma once

#include <mrpt/math/CMatrixDynamic.h>  // mrpt::math::CMatrixBool
#include <mrpt/poses/CPoint2DPDFGaussian.h>
#include <mrpt/poses/CPointPDFGaussian.h>
#include <mrpt/typemeta/TEnumType.h>

namespace mrpt::slam
{
/** \addtogroup data_assoc_grp Data association
 * \ingroup mrpt_slam_grp
 *  @{ */

/** @name Data association
	@{
  */

/** Different algorithms for data association, used in
 * mrpt::slam::data_association
 */
enum TDataAssociationMethod
{
	/** Nearest-neighbor. */
	assocNN = 0,
	/** JCBB: Joint Compatibility Branch & Bound \cite neira2001data */
	assocJCBB
};

/** Different metrics for data association, used in mrpt::slam::data_association
 *  For a comparison of both methods see paper \cite blanco2012amd
 */
enum TDataAssociationMetric
{
	/** Mahalanobis distance */
	metricMaha = 0,
	/** Matching likelihood (See TDataAssociationMetric for a paper explaining
	   this metric) */
	metricML
};

/** Used in mrpt::slam::TDataAssociationResults */
using observation_index_t = size_t;
/** Used in mrpt::slam::TDataAssociationResults */
using prediction_index_t = size_t;

/** The results from mrpt::slam::data_association_independent_predictions()
 */
struct TDataAssociationResults
{
	TDataAssociationResults()
		: associations(),

		  indiv_distances(0, 0),
		  indiv_compatibility(0, 0),
		  indiv_compatibility_counts()

	{
	}

	void clear()
	{
		associations.clear();
		distance = 0;
		indiv_distances.setSize(0, 0);
		indiv_compatibility.setSize(0, 0);
		indiv_compatibility_counts.clear();
		nNodesExploredInJCBB = 0;
	}

	/** For each observation (with row index IDX_obs in the input
	 * "Z_observations"), its association in the predictions, as the row index
	 * in the "Y_predictions_mean" input (or it's mapping to a custom ID, if it
	 * was provided).
	 * Note that not all observations may have an associated prediction.
	 * An observation with index "IDX_obs" corresponds to the prediction number
	 * "associations[IDX_obs]", or it may not correspond to anyone if it's not
	 * present
	 *  in the std::map (Tip: Use
	 * associations.find(IDX_obs)!=associations.end() )
	 * \note The types observation_index_t and prediction_index_t are just used
	 * for clarity, use normal size_t's.
	 */
	std::map<observation_index_t, prediction_index_t> associations;
	/** The Joint Mahalanobis distance or matching likelihood of the best
	 * associations found. */
	double distance{0};

	/** Individual mahalanobis distances (or matching likelihood, depending on
	 * the selected metric) between predictions (row indices) & observations
	 * (column indices).
	 *  Indices are for the appearing order in the arguments
	 * "Y_predictions_mean" & "Z_observations", they are NOT landmark IDs.
	 */
	mrpt::math::CMatrixDouble indiv_distances;
	/** The result of a chi2 test for compatibility using mahalanobis distance -
	 * Indices are like in "indiv_distances". */
	mrpt::math::CMatrixBool indiv_compatibility;
	/** The sum of each column of indiv_compatibility, that is, the number of
	 * compatible pairings for each observation. */
	std::vector<uint32_t> indiv_compatibility_counts;

	/** Only for the JCBB method,the number of recursive calls expent in the
	 * algorithm. */
	size_t nNodesExploredInJCBB{0};
};

/** Computes the data-association between the prediction of a set of landmarks
 *and their observations, all of them with covariance matrices - Generic
 *version with prediction full cross-covariances.
 * Implemented methods include (see TDataAssociation)
 *		- NN: Nearest-neighbor
 *		- JCBB: Joint Compatibility Branch & Bound \cite neira2001data
 *
 *  With both a Mahalanobis-distance or Matching-likelihood metric. For a
 *comparison of both methods, see paper \cite blanco2012amd
 *
 * \param Z_observations_mean [IN] An MxO matrix with the M observations, each
 *row containing the observation "mean".
 * \param Y_predictions_mean [IN] An NxO matrix with the N predictions, each
 *row containing the mean of one prediction.
 * \param Y_predictions_cov [IN] An N*OxN*O matrix with the full covariance
 *matrix of all the N predictions.
 * \param results [OUT] The output data association hypothesis, and other
 *useful information.
 * \param method [IN, optional] The selected method to make the associations.
 * \param chi2quantile [IN, optional] The threshold for considering a match
 *between two close Gaussians for two landmarks, in the range [0,1]. It is used
 *to call mrpt::math::chi2inv
 * \param use_kd_tree [IN, optional] Build a KD-tree to speed-up the evaluation
 *of individual compatibility (IC). It's perhaps more efficient to disable it
 *for a small number of features. (default=true).
 * \param predictions_IDs [IN, optional] (default:none) An N-vector. If
 *provided, the resulting associations in "results.associations" will not
 *contain prediction indices "i", but "predictions_IDs[i]".
 *
 * \sa data_association_independent_predictions,
 *data_association_independent_2d_points,
 *data_association_independent_3d_points
 */
void data_association_full_covariance(
	const mrpt::math::CMatrixDouble& Z_observations_mean,
	const mrpt::math::CMatrixDouble& Y_predictions_mean,
	const mrpt::math::CMatrixDouble& Y_predictions_cov,
	TDataAssociationResults& results,
	const TDataAssociationMethod method = assocJCBB,
	const TDataAssociationMetric metric = metricMaha,
	const double chi2quantile = 0.99, const bool DAT_ASOC_USE_KDTREE = true,
	const std::vector<prediction_index_t>& predictions_IDs =
		std::vector<prediction_index_t>(),
	const TDataAssociationMetric compatibilityTestMetric = metricMaha,
	const double log_ML_compat_test_threshold = 0.0);

/** Computes the data-association between the prediction of a set of landmarks
 *and their observations, all of them with covariance matrices - Generic
 *version with NO prediction cross-covariances.
 * Implemented methods include (see TDataAssociation)
 *		- NN: Nearest-neighbor
 *		- JCBB: Joint Compatibility Branch & Bound \cite neira2001data
 *
 *  With both a Mahalanobis-distance or Matching-likelihood metric. For a
 *comparison of both methods, see paper \cite blanco2012amd :
 *
 * \param Z_observations_mean [IN] An MxO matrix with the M observations, each
 *row containing the observation "mean".
 * \param Y_predictions_mean [IN] An NxO matrix with the N predictions, each
 *row containing the mean of one prediction.
 * \param Y_predictions_cov [IN] An N*OxO matrix: A vertical stack of N
 *covariance matrix, one for each of the N prediction.
 * \param results [OUT] The output data association hypothesis, and other
 *useful information.
 * \param method [IN, optional] The selected method to make the associations.
 * \param chi2quantile [IN, optional] The threshold for considering a match
 *between two close Gaussians for two landmarks, in the range [0,1]. It is used
 *to call mrpt::math::chi2inv
 * \param use_kd_tree [IN, optional] Build a KD-tree to speed-up the evaluation
 *of individual compatibility (IC). It's perhaps more efficient to disable it
 *for a small number of features. (default=true).
 * \param predictions_IDs [IN, optional] (default:none) An N-vector. If
 *provided, the resulting associations in "results.associations" will not
 *contain prediction indices "i", but "predictions_IDs[i]".
 *
 * \sa data_association_full_covariance,
 *data_association_independent_2d_points,
 *data_association_independent_3d_points
 */
void data_association_independent_predictions(
	const mrpt::math::CMatrixDouble& Z_observations_mean,
	const mrpt::math::CMatrixDouble& Y_predictions_mean,
	const mrpt::math::CMatrixDouble& Y_predictions_cov,
	TDataAssociationResults& results,
	const TDataAssociationMethod method = assocJCBB,
	const TDataAssociationMetric metric = metricMaha,
	const double chi2quantile = 0.99, const bool DAT_ASOC_USE_KDTREE = true,
	const std::vector<prediction_index_t>& predictions_IDs =
		std::vector<prediction_index_t>(),
	const TDataAssociationMetric compatibilityTestMetric = metricMaha,
	const double log_ML_compat_test_threshold = 0.0);

/** @} */

/** @} */  // end of grouping

}  // namespace mrpt::slam
MRPT_ENUM_TYPE_BEGIN(mrpt::slam::TDataAssociationMethod)
using namespace mrpt::slam;
MRPT_FILL_ENUM(assocNN);
MRPT_FILL_ENUM(assocJCBB);
MRPT_ENUM_TYPE_END()

MRPT_ENUM_TYPE_BEGIN(mrpt::slam::TDataAssociationMetric)
using namespace mrpt::slam;
MRPT_FILL_ENUM(metricMaha);
MRPT_FILL_ENUM(metricML);
MRPT_ENUM_TYPE_END()
