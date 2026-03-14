//
// Created by Saman on 14.03.26.
//

#ifndef PIS_MICROFACET_H
#define PIS_MICROFACET_H

#include "coords.h"
#include <numbers>

#include "utils.h"
constexpr float PI = std::numbers::pi_v<float>;

namespace mts {
    class MicrofacetDistribution {
    public:
        MicrofacetDistribution(float alpha, bool sample_visible = true)
            : m_alpha_u(alpha), m_alpha_v(alpha),
              m_sample_visible(sample_visible) {
            configure();
        }

        MicrofacetDistribution(float alpha_u, float alpha_v,
                               bool sample_visible = true)
            : m_alpha_u(alpha_u), m_alpha_v(alpha_v),
              m_sample_visible(sample_visible) {
            configure();
        }

        /// Return the roughness (isotropic case)
        float alpha() const { return m_alpha_u; }

        /// Return the roughness along the tangent direction
        float alpha_u() const { return m_alpha_u; }

        /// Return the roughness along the bitangent direction
        float alpha_v() const { return m_alpha_v; }

        /// Return whether or not only visible normals are sampled?
        bool sample_visible() const { return m_sample_visible; }

        /// Is this an isotropic microfacet distribution?
        bool is_isotropic() const {
            return m_alpha_u == m_alpha_v;
        }

        /// Scale the roughness values by some constant
        void scale_alpha(float value) {
            m_alpha_u *= value;
            m_alpha_v *= value;
        }

        float eval(const Vec3f &m) const {
            float alpha_uv = m_alpha_u * m_alpha_v,
                    cos_theta = utils::cosTheta(m),
                    cos_theta_2 = std::pow(cos_theta, 2),
                    result;

            // Beckmann distribution function for Gaussian random surfaces
            result = std::exp(-(std::pow(m.x / m_alpha_u, 2) +
                                std::pow(m.y / m_alpha_v, 2)) /
                              cos_theta_2) /
                     (PI * alpha_uv * std::pow(cos_theta_2, 2));

            // Prevent potential numerical issues in other stages of the model
            return result * cos_theta > 1e-20f ? result : 0.f;
        }

        float pdf(const Vec3f &wi, const Vec3f &m) const {
            float result = eval(m);

            if (m_sample_visible)
                result *= smith_g1(wi, m) * std::abs(utils::dot(wi, m)) / utils::cosTheta(wi);
            else
                result *= utils::cosTheta(m);

            return result;
        }

        std::pair<Normal3f, float> sample(const Vec3f &wi,
                                          const Point2f &sample) const {
            float sin_phi, cos_phi, cos_theta, cos_theta_2, alpha_2, pdf;

            // Sample azimuth component (identical for Beckmann & GGX)
            if (is_isotropic()) {
                sin_phi = std::sin((2.f * PI) * sample.y);
                cos_phi = std::cos((2.f * PI) * sample.y);

                alpha_2 = m_alpha_u * m_alpha_u;
            } else {
                const float ratio = m_alpha_v / m_alpha_u;
                const float tmp = ratio * std::tan((2.f * PI) * sample.y);

                cos_phi = 1.0f / std::sqrt(tmp * tmp + 1.f);
                cos_phi = copysign(cos_phi, std::abs(sample.y - .5f) - .25f);

                sin_phi = cos_phi * tmp;

                alpha_2 = 1.0f / (std::pow(cos_phi / m_alpha_u, 2) +
                                  std::pow(sin_phi / m_alpha_v, 2));
            }

            // Beckmann distribution function for Gaussian random surfaces
            cos_theta = 1.0f / std::sqrt(-(alpha_2 * std::log(1.f - sample.x) + 1.f));
            cos_theta_2 = cos_theta * cos_theta;

            // Compute probability density of the sampled position
            float cos_theta_3 = std::max(cos_theta_2 * cos_theta, 1e-20f);
            pdf = (1.f - sample.x) / (PI * m_alpha_u * m_alpha_v * cos_theta_3);

            float sin_theta = std::sqrt(1.f - cos_theta_2);

            return {
                Normal3f(cos_phi * sin_theta,
                         sin_phi * sin_theta,
                         cos_theta),
                pdf
            };
        }

        /**
         * \brief Smith's shadowing-masking function for a single direction
         *
         * \param v
         *     An arbitrary direction
         * \param m
         *     The microfacet normal
         */
        float smith_g1(const Vec3f &v, const Vec3f &m) const {
            float xy_alpha_2 = std::pow(m_alpha_u * v.x, 2.0f) + std::pow(m_alpha_v * v.y, 2.0f),
                    tan_theta_alpha_2 = xy_alpha_2 / std::pow(v.z, 2.0f),
                    result;

            float a = 1.0f / std::sqrt(tan_theta_alpha_2), a_sqr = a * a;
            /* Use a fast and accurate (<0.35% rel. error) rational
               approximation to the shadowing-masking function */
            result = a >= 1.6f
                         ? 1.f
                         : (3.535f * a + 2.181f * a_sqr) /
                           (1.f + 2.276f * a + 2.577f * a_sqr);

            // Perpendicular incidence -- no shadowing/masking
            dr::masked(result, xy_alpha_2 == 0.f) = 1.f;

            /* Ensure consistent orientation (can't see the back
               of the microfacet from the front and vice versa) */
            dr::masked(result, utils::dot(v, m) * utils::cosTheta(v) <= 0.f) = 0.f;

            return result;
        }

        /// \brief Visible normal sampling code for the alpha=1 case
        Vec2f sample_visible_11(float cos_theta_i, Point2f sample) const {
            /* The original inversion routine from the paper contained
               discontinuities, which causes issues for QMC integration
               and techniques like Kelemen-style MLT. The following code
               performs a numerical inversion with better behavior */

            float tan_theta_i =
                    utils::safeSqrt(dr::fnmadd(cos_theta_i, cos_theta_i, 1.f)) /
                    cos_theta_i;
            float cot_theta_i = dr::rcp(tan_theta_i);

            /* Search interval -- everything is parameterized
               in the erf() domain */
            float maxval = dr::erf(cot_theta_i);

            /* Start with a good initial guess (analytic solution for
               theta_i = pi/2, which is the most nonlinear case) */
            sample = dr::maximum(dr::minimum(sample, 1.f - 1e-6f), 1e-6f);
            float x = maxval - (maxval + 1.f) * dr::erf(dr::sqrt(-dr::log(sample.x())));

            // Normalization factor for the CDF
            sample.x() *= 1.f + maxval + dr::InvSqrtPi<float> *
                    tan_theta_i * dr::exp(-dr::square(cot_theta_i));

            // Three Newton iterations
            for (size_t i = 0; i < 3; ++i) {
                float slope = dr::erfinv(x),
                        value = 1.f + x + dr::InvSqrtPi<float> * tan_theta_i *
                                dr::exp(-dr::square(slope)) - sample.x(),
                        derivative = 1.f - slope * tan_theta_i;

                x -= value / derivative;
            }

            // Now convert back into a slope value
            return dr::erfinv(Vec2f(x, 2.f * sample.y() - 1.f));
        }

    protected:
        void configure() {
            m_alpha_u = std::max(m_alpha_u, 1e-4f);
            m_alpha_v = std::max(m_alpha_v, 1e-4f);
        }

        float m_alpha_u, m_alpha_v;
        bool m_sample_visible;
    };
}
#endif //PIS_MICROFACET_H
