/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */
#pragma once

#include <mrpt/math/CMatrixDynamic.h>
#include <mrpt/math/CMatrixFixed.h>
#include <mrpt/math/CVectorDynamic.h>
#include <mrpt/math/math_frwds.h>

#include <limits>  // numeric_limits

namespace mrpt::math
{
/** A generic template for probability density distributions (PDFs).
 * This template is used as base for many classes in mrpt::poses
 *  Any derived class must implement \a getMean() and a getCovarianceAndMean().
 *  Other methods such as \a getMean() or \a getCovariance() are implemented
 * here for convenience.
 * \sa mprt::poses::CPosePDF, mprt::poses::CPose3DPDF, mprt::poses::CPointPDF
 * \ingroup mrpt_math_grp
 */
template <class TDATA, size_t STATE_LEN>
class CProbabilityDensityFunction
{
   public:
	/** The length of the variable, for example, 3 for a 3D point, 6 for a 3D
	 * pose (x y z yaw pitch roll). */
	static constexpr size_t state_length = STATE_LEN;
	/** The type of the state the PDF represents */
	using type_value = TDATA;
	using self_t = CProbabilityDensityFunction<TDATA, STATE_LEN>;
	/** Covariance matrix type */
	using cov_mat_t = mrpt::math::CMatrixFixed<double, STATE_LEN, STATE_LEN>;
	/** Information matrix type */
	using inf_mat_t = cov_mat_t;

	/** Returns the mean, or mathematical expectation of the probability density
	 * distribution (PDF).
	 * \sa getCovarianceAndMean, getInformationMatrix
	 */
	virtual void getMean(type_value& mean_point) const = 0;

	/** Returns an estimate of the pose covariance matrix (STATE_LENxSTATE_LEN
	 * cov matrix) and the mean, both at once.
	 * \sa getMean, getInformationMatrix
	 */
	virtual std::tuple<cov_mat_t, type_value> getCovarianceAndMean() const = 0;

	/// \overload
	virtual void getCovarianceAndMean(cov_mat_t& c, TDATA& mean) const final
	{
		const auto [C, M] = getCovarianceAndMean();
		c = C;
		mean = M;
	}

	/** Returns an estimate of the pose covariance matrix (STATE_LENxSTATE_LEN
	 * cov matrix) and the mean, both at once.
	 * \sa getMean, getInformationMatrix
	 */
	inline void getCovarianceDynAndMean(
		mrpt::math::CMatrixDouble& cov, type_value& mean_point) const
	{
		cov_mat_t C(mrpt::math::UNINITIALIZED_MATRIX);
		this->getCovarianceAndMean(C, mean_point);
		cov = C;  // Convert to dynamic size matrix
	}

	/** Returns the mean, or mathematical expectation of the probability density
	 * distribution (PDF).
	 * \sa getCovariance, getInformationMatrix
	 */
	inline type_value getMeanVal() const
	{
		type_value p;
		getMean(p);
		return p;
	}

	/** Returns the estimate of the covariance matrix (STATE_LEN x STATE_LEN
	 * covariance matrix)
	 * \sa getMean, getCovarianceAndMean, getInformationMatrix
	 */
	inline void getCovariance(mrpt::math::CMatrixDouble& cov) const
	{
		TDATA p;
		this->getCovarianceDynAndMean(cov, p);
	}

	/** Returns the estimate of the covariance matrix (STATE_LEN x STATE_LEN
	 * covariance matrix)
	 * \sa getMean, getCovarianceAndMean, getInformationMatrix
	 */
	inline void getCovariance(cov_mat_t& cov) const
	{
		TDATA p;
		this->getCovarianceAndMean(cov, p);
	}

	/** Returns the estimate of the covariance matrix (STATE_LEN x STATE_LEN
	 * covariance matrix)
	 * \sa getMean, getInformationMatrix
	 */
	inline cov_mat_t getCovariance() const
	{
		cov_mat_t cov(mrpt::math::UNINITIALIZED_MATRIX);
		TDATA p;
		this->getCovarianceAndMean(cov, p);
		return cov;
	}

	/** Returns whether the class instance holds the uncertainty in covariance
	 * or information form.
	 * \note By default this is going to be covariance form. *Inf classes
	 * (e.g. CPosePDFGaussianInf) store it in information form.
	 *
	 * \sa mrpt::traits::is_inf_type
	 */
	virtual bool isInfType() const { return false; }
	/** Returns the information (inverse covariance) matrix (a STATE_LEN x
	 * STATE_LEN matrix)
	 *  Unless reimplemented in derived classes, this method first reads the
	 * covariance, then invert it.
	 * \sa getMean, getCovarianceAndMean
	 */
	virtual void getInformationMatrix(inf_mat_t& inf) const
	{
		inf = getCovariance().inverse_LLt();
	}

	/** Save PDF's particles to a text file. See derived classes for more
	 * information about the format of generated files.
	 * \return false on error
	 */
	virtual bool saveToTextFile(const std::string& file) const = 0;

	/** Draws a single sample from the distribution
	 */
	virtual void drawSingleSample(TDATA& outPart) const = 0;

	/** Draws a number of samples from the distribution, and saves as a list of
	 * 1xSTATE_LEN vectors, where each row contains a (x,y,z,yaw,pitch,roll)
	 * datum.
	 * This base method just call N times to drawSingleSample, but derived
	 * classes should implemented optimized method for each particular PDF.
	 */
	virtual void drawManySamples(
		size_t N, std::vector<mrpt::math::CVectorDouble>& outSamples) const
	{
		outSamples.resize(N);
		TDATA pnt;
		for (size_t i = 0; i < N; i++)
		{
			this->drawSingleSample(pnt);
			outSamples[i] = mrpt::math::CVectorDouble(pnt.asVectorVal());
		}
	}

	/** Compute the entropy of the estimated covariance matrix.
	 * \sa
	 * http://en.wikipedia.org/wiki/Multivariate_normal_distribution#Entropy
	 */
	double getCovarianceEntropy() const
	{
		static const double ln_2PI = 1.8378770664093454835606594728112;
		return 0.5 *
			(STATE_LEN + STATE_LEN * ln_2PI +
			 log(std::max(
				 getCovariance().det(),
				 std::numeric_limits<double>::epsilon())));
	}

};	// End of class def.

}  // namespace mrpt::math
