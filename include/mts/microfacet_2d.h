//
// Created by Saman on 14.03.26.
//

#ifndef PIS_MICROFACET_2D_H
#define PIS_MICROFACET_2D_H

#include "coords.h"

#include "utils.h"

namespace mts {
    class MicrofacetDistribution2D {
    public:
        explicit MicrofacetDistribution2D(const double alpha)
            : m_alpha(alpha) {
            m_alpha = std::max(m_alpha, 1e-4);
        }

        [[nodiscard]] double eval(const Vec2f &m) const {
            const double cos_theta = utils::cosTheta(m);

            const double result = 1.0 / (2.0 * m_alpha * std::pow(
                                             std::pow(m.x / m_alpha, 2.0) +
                                             std::pow(m.y, 2.0), 1.5)
                                  );

            return result * cos_theta > 1e-20f ? result : 0.0;
        }

        [[nodiscard]] double solid_angle_density(const Vec2f &wi, const Vec2f &m) const {
            return eval(m) * smith_g1(wi, m) * std::abs(utils::dot(wi, m)) / utils::cosTheta(wi);
        }

        [[nodiscard]] double reflected_pdf(const Vec2f &wi, const Vec2f &m) const {
            // uses 2D half-vector Jacobian
            return eval(m) * smith_g1(wi, m) * std::abs(utils::dot(wi, m)) / (2.0 * utils::cosTheta(wi));
        }

        [[nodiscard]] Normal2f sample(const Vec2f &wi, const double sample) const {
            // Step 1: stretch wi
            const Vec2f wi_p = utils::normalize(Vec2f(
                m_alpha * wi.x,
                wi.y
            ));

            const double cos_theta = utils::cosTheta(wi_p);
            const double sign = wi_p.x < 0.0 ? -1.0 : 1.0;

            // Step 2: simulate P11_{wi}(slope, 1) for the alpha = 1 case
            double slope = sample_visible_11(cos_theta, sample);

            // Step 3: rotate & unstretch
            slope = sign * slope * m_alpha;

            // Step 4: compute normal
            const Normal2f m = utils::normalize(Vec2f(-slope, 1.0));
            return m;
        }

        /// \brief Smith's shadowing-masking function for a single direction
        [[nodiscard]] double smith_g1(const Vec2f &v, const Vec2f &m) const {
            const double x_alpha_2 = std::pow(m_alpha * v.x, 2.0);

            // 2D Smith-GGX masking term has a different closed form
            // 3D: G1 = 2 / (1 + sqrt(1 + a^2 tan^2(theta)))
            // 2D: G1 = 2 cos(theta) / (cos(theta) + sqrt(cos^2(theta) + a^2 sin^2(theta)))
            double result = 2.0 * v.y / (v.y + std::sqrt(std::pow(v.y, 2.0) + x_alpha_2));

            // Perpendicular incidence -- no shadowing/masking
            if (x_alpha_2 == 0.0) {
                result = 1.0;
            }

            /* Ensure consistent orientation (can't see the back
               of the microfacet from the front and vice versa) */
            if (utils::dot(v, m) * utils::cosTheta(v) <= 0.0) {
                result = 0.0;
            }

            return result;
        }

        /// \brief Visible normal sampling code for the alpha=1 case
        static double sample_visible_11(const double cos_theta_i, const double sample) {
            const double sin_theta_i = utils::safeSqrt(1.0 - cos_theta_i * cos_theta_i);

            const double sin_delta = sample * (1.0 + cos_theta_i) - 1.0;
            const double cos_delta = utils::safeSqrt(1.0 - sin_delta * sin_delta);

            // Convert to slope
            const double sin_theta_m = sin_theta_i * cos_delta + cos_theta_i * sin_delta;
            const double cos_theta_m = cos_theta_i * cos_delta - sin_theta_i * sin_delta;
            return -sin_theta_m / cos_theta_m;
        }

    protected:
        double m_alpha;
    };
}

#endif //PIS_MICROFACET_2D_H
