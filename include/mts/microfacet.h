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
    /// Low-distortion concentric square to disk mapping by Peter Shirley
    inline float lerp(const float v0, const float v1, const float t) {
        return v0 + t * (v1 - v0);
    }

    inline Point2f square_to_uniform_disk_concentric(const Point2f &sample) {
        const float x = 2.f * sample.x - 1.f;
        const float y = 2.f * sample.y - 1.f;

        float phi, r;
        if (x == 0 && y == 0) {
            r = phi = 0;
        } else if (x * x > y * y) {
            r = x;
            phi = PI / 4.f * (y / x);
        } else {
            r = y;
            phi = PI / 2.f - x / y * (PI / 4.f);
        }

        return {r * std::cos(phi), r * std::sin(phi)};
    }

    class MicrofacetDistribution {
    public:
        explicit MicrofacetDistribution(const float alpha)
            : m_alpha_u(alpha), m_alpha_v(alpha) {
            configure();
        }

        MicrofacetDistribution(const float alpha_u, const float alpha_v)
            : m_alpha_u(alpha_u), m_alpha_v(alpha_v) {
            configure();
        }

        /// Return the roughness (isotropic case)
        float alpha() const { return m_alpha_u; }

        /// Return the roughness along the tangent direction
        float alpha_u() const { return m_alpha_u; }

        /// Return the roughness along the bitangent direction
        float alpha_v() const { return m_alpha_v; }

        /// Is this an isotropic microfacet distribution?
        bool is_isotropic() const {
            return m_alpha_u == m_alpha_v;
        }

        /// Scale the roughness values by some constant
        void scale_alpha(const float value) {
            m_alpha_u *= value;
            m_alpha_v *= value;
        }

        float eval(const Vec3f &m) const {
            const float alpha_uv = m_alpha_u * m_alpha_v;
            const float
                    cos_theta = utils::cosTheta(m);

            const float result = 1.0f / (PI * alpha_uv * std::pow(
                                             std::pow(m.x / m_alpha_u, 2.0f) +
                                             std::pow(m.y / m_alpha_v, 2.0f) +
                                             std::pow(m.z, 2.0f), 2.0f)
                                 );


            // Prevent potential numerical issues in other stages of the model
            return result * cos_theta > 1e-20f ? result : 0.f;
        }

        float pdf(const Vec3f &wi, const Vec3f &m) const {
            float result = eval(m);

            result *= smith_g1(wi, m) * std::abs(utils::dot(wi, m)) / utils::cosTheta(wi);

            return result;
        }

        std::pair<Normal3f, float> sample(const Vec3f &wi,
                                          const Point2f &sample) const {
            // Visible normal sampling.

            // Step 1: stretch wi
            const Vec3f wi_p = utils::normalize(Vec3f(
                m_alpha_u * wi.x,
                m_alpha_v * wi.y,
                wi.z
            ));

            const float sin_phi = utils::sinPhi(wi_p);
            const float cos_phi = utils::cosPhi(wi_p);
            const float cos_theta = utils::cosTheta(wi_p);

            // Step 2: simulate P22_{wi}(slope.x, slope.y, 1, 1)
            Vec2f slope = sample_visible_11(cos_theta, sample);

            // Step 3: rotate & unstretch
            slope = Vec2f(
                (cos_phi * slope.x - sin_phi * slope.y) * m_alpha_u,
                (sin_phi * slope.x + cos_phi * slope.y) * m_alpha_v);

            // Step 4: compute normal & PDF
            Normal3f m = utils::normalize(Vec3f(-slope.x, -slope.y, 1));

            float pdf = eval(m) * smith_g1(wi, m) * std::abs(utils::dot(wi, m)) /
                        utils::cosTheta(wi);

            return {m, pdf};
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
            const float xy_alpha_2 = std::pow(m_alpha_u * v.x, 2.0f) + std::pow(m_alpha_v * v.y, 2.0f);
            const float tan_theta_alpha_2 = xy_alpha_2 / std::pow(v.z, 2.0f);

            float result = 2.f / (1.f + std::sqrt(1.f + tan_theta_alpha_2));

            // Perpendicular incidence -- no shadowing/masking
            if (xy_alpha_2 == 0.f)
                result = 1.f;

            /* Ensure consistent orientation (can't see the back
               of the microfacet from the front and vice versa) */
            if (utils::dot(v, m) * utils::cosTheta(v) <= 0.f)
                result = 0.f;

            return result;
        }

        /// \brief Visible normal sampling code for the alpha=1 case
        static Vec2f sample_visible_11(const float cos_theta_i, const Point2f sample) {
            // Choose a projection direction and re-scale the sample
            Point2f p = square_to_uniform_disk_concentric(sample);

            const float s = 0.5f * (1.f + cos_theta_i);
            p.y = lerp(utils::safeSqrt(1.f - std::pow(p.x, 2.0f)), p.y, s);

            const float x = p.x;
            // Project onto chosen side of the hemisphere
            const float y = p.y;
            const float z = utils::safeSqrt(1.f - utils::dot(p, p));

            // Convert to slope
            const float sin_theta_i = utils::safeSqrt(1.f - std::pow(cos_theta_i, 2.0f));
            const float norm = 1.0f / (sin_theta_i * y + cos_theta_i * z);
            return Vec2f(cos_theta_i * y - sin_theta_i * z, x) * norm;
        }

    protected:
        void configure() {
            m_alpha_u = std::max(m_alpha_u, 1e-4f);
            m_alpha_v = std::max(m_alpha_v, 1e-4f);
        }

        float m_alpha_u, m_alpha_v;
    };
}
#endif //PIS_MICROFACET_H
