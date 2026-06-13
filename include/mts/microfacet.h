//
// Created by Saman on 14.03.26.
//

#ifndef PIS_MICROFACET_H
#define PIS_MICROFACET_H

#include "coords.h"
#include <numbers>

#include "utils.h"

// Ported from: https://github.com/mitsuba-renderer/mitsuba3/blob/master/include/mitsuba/render/microfacet.h
// sample_visible = True, type = GGX
constexpr double PI = std::numbers::pi_v<double>;

#define print_zero(x, name) const double x_##name = x; if(x_##name == 0.0) printf("%s: %f\n", #name, x_##name);
#define print_n_zero(x, name) const double x_##name = x; if(x_##name != 0.0) printf("%s: %f\n", #name, x_##name);

namespace mts {
    inline Point2f square_to_uniform_disk_concentric(const Point2f &sample) {
        const double x = 2.0 * sample.x - 1.0;
        const double y = 2.0 * sample.y - 1.0;

        double phi, r;
        if (x == 0 && y == 0) {
            r = phi = 0;
        } else if (x * x > y * y) {
            r = x;
            phi = PI / 4.0 * (y / x);
        } else {
            r = y;
            phi = PI / 2.0 - x / y * (PI / 4.0);
        }

        return {r * std::cos(phi), r * std::sin(phi)};
    }

    class MicrofacetDistribution {
    public:
        explicit MicrofacetDistribution(const double alpha)
            : m_alpha_u(alpha), m_alpha_v(alpha) {
            m_alpha_u = std::max(m_alpha_u, 1e-4);
            m_alpha_v = std::max(m_alpha_v, 1e-4);
        }

        MicrofacetDistribution(const double alpha_u, const double alpha_v)
            : m_alpha_u(alpha_u), m_alpha_v(alpha_v) {
            m_alpha_u = std::max(m_alpha_u, 1e-4);
            m_alpha_v = std::max(m_alpha_v, 1e-4);
        }

        [[nodiscard]] double eval(const Vec3f &m) const {
            const double alpha_uv = m_alpha_u * m_alpha_v;
            const double cos_theta = utils::cosTheta(m);

            const double result = 1.0 / (PI * alpha_uv * std::pow(
                                             std::pow(m.x / m_alpha_u, 2.0) +
                                             std::pow(m.y / m_alpha_v, 2.0) +
                                             std::pow(m.z, 2.0), 2.0)
                                  );

            return result * cos_theta > 1e-20f ? result : 0.0;
        }

        [[nodiscard]] double solid_angle_density(const Vec3f &wi, const Vec3f &m) const {
            return eval(m) * smith_g1(wi, m) * std::abs(utils::dot(wi, m)) / utils::cosTheta(wi);
        }

        [[nodiscard]] double reflected_pdf(const Vec3f &wi, const Vec3f &m) const {
            // https://github.com/mitsuba-renderer/mitsuba3/blob/9067366f4e7d398a2971efd46ec63944264dfb27/src/bsdfs/roughconductor.cpp#L421
            return eval(m) * smith_g1(wi, m) / (4.0 * utils::cosTheta(wi));
        }

        [[nodiscard]] Normal3f sample(const Vec3f &wi, const Point2f &sample) const {
            // Step 1: stretch wi
            const Vec3f wi_p = utils::normalize(Vec3f(
                m_alpha_u * wi.x,
                m_alpha_v * wi.y,
                wi.z
            ));

            const double sin_phi = utils::sinPhi(wi_p);
            const double cos_phi = utils::cosPhi(wi_p);
            const double cos_theta = utils::cosTheta(wi_p);

            // Step 2: simulate P22_{wi}(slope.x, slope.y, 1, 1)
            Vec2f slope = sample_visible_11(cos_theta, sample);

            // Step 3: rotate & unstretch
            slope = Vec2f(
                (cos_phi * slope.x - sin_phi * slope.y) * m_alpha_u,
                (sin_phi * slope.x + cos_phi * slope.y) * m_alpha_v);

            // Step 4: compute normal
            const Normal3f m = utils::normalize(Vec3f(-slope.x, -slope.y, 1));
            return m;
        }

        /// \brief Smith's shadowing-masking function for a single direction
        [[nodiscard]] double smith_g1(const Vec3f &v, const Vec3f &m) const {
            const double xy_alpha_2 = std::pow(m_alpha_u * v.x, 2.0) + std::pow(m_alpha_v * v.y, 2.0);
            const double tan_theta_alpha_2 = xy_alpha_2 / std::pow(v.z, 2.0);

            double result = 2.0 / (1.0 + std::sqrt(1.0 + tan_theta_alpha_2));

            // Perpendicular incidence -- no shadowing/masking
            if (xy_alpha_2 == 0.0) {
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
        static Vec2f sample_visible_11(const double cos_theta_i, const Point2f sample) {
            // Choose a projection direction and re-scale the sample
            Point2f p = square_to_uniform_disk_concentric(sample);

            const double s = 0.5f * (1.0 + cos_theta_i);
            p.y = utils::lerp(utils::safeSqrt(1.0 - std::pow(p.x, 2.0)), p.y, s);

            const double x = p.x;
            // Project onto chosen side of the hemisphere
            const double y = p.y;
            const double z = utils::safeSqrt(1.0 - utils::dot(p, p));

            // Convert to slope
            const double sin_theta_i = utils::safeSqrt(1.0 - std::pow(cos_theta_i, 2.0));
            const double norm = 1.0 / (sin_theta_i * y + cos_theta_i * z);
            return Vec2f(cos_theta_i * y - sin_theta_i * z, x) * norm;
        }

    protected:
        double m_alpha_u, m_alpha_v;
    };
}

#endif //PIS_MICROFACET_H
