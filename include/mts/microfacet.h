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
constexpr float PI = std::numbers::pi_v<float>;

namespace mts {
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
            m_alpha_u = std::max(m_alpha_u, 1e-4f);
            m_alpha_v = std::max(m_alpha_v, 1e-4f);
        }

        MicrofacetDistribution(const float alpha_u, const float alpha_v)
            : m_alpha_u(alpha_u), m_alpha_v(alpha_v) {
            m_alpha_u = std::max(m_alpha_u, 1e-4f);
            m_alpha_v = std::max(m_alpha_v, 1e-4f);
        }

        [[nodiscard]] float eval(const Vec3f &m) const {
            const float alpha_uv = m_alpha_u * m_alpha_v;
            const float cos_theta = utils::cosTheta(m);

            const float result = 1.0f / (PI * alpha_uv * std::pow(
                                             std::pow(m.x / m_alpha_u, 2.0f) +
                                             std::pow(m.y / m_alpha_v, 2.0f) +
                                             std::pow(m.z, 2.0f), 2.0f)
                                 );

            // Prevent potential numerical issues in other stages of the model
            return result * cos_theta > 1e-20f ? result : 0.f;
        }

        [[nodiscard]] float pdf(const Vec3f &wi, const Vec3f &m) const {
            float result = eval(m);

            result *= smith_g1(wi, m) * std::abs(utils::dot(wi, m)) / utils::cosTheta(wi);

            return result;
        }

        [[nodiscard]] std::pair<Normal3f, float> sample(const Vec3f &wi,
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
        [[nodiscard]] float smith_g1(const Vec3f &v, const Vec3f &m) const {
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
            p.y = utils::lerp(utils::safeSqrt(1.f - std::pow(p.x, 2.0f)), p.y, s);

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
        float m_alpha_u, m_alpha_v;
    };
}


// float eval_reflectance(const mts::MicrofacetDistribution &distr,
//                        const Vec3f &wi,float eta) {
//     MI_IMPORT_CORE_TYPES()
//
//     if (!distr.sample_visible())
//         Throw("eval_reflectance(): requires visible normal sampling!");
//
//     int res = eta > 1 ? 32 : 128;
//
//     using FloatX = dr::DynamicArray<dr::scalar_t<float>>;
//     auto [nodes, weights] = quad::gauss_legendre<FloatX>(res);
//     float result = dr::zeros<float>(dr::width(wi));
//
//     auto [nodes_x, nodes_y]     = dr::meshgrid(nodes, nodes);
//     auto [weights_x, weights_y] = dr::meshgrid(weights, weights);
//
//     using FloatP = dr::Packet<dr::scalar_t<float>>;
//     using Normal3fP = Normal<FloatP, 3>;
//     using Vector3fP = Vector<FloatP, 3>;
//
//     size_t packet_count = dr::width(wi) / FloatP::Size;
//
//     Assert(dr::width(wi) % FloatP::Size == 0);
//
//     for (size_t i = 0; i < packet_count; ++i) {
//         Vector3fP wi_p;
//         wi_p.x() = dr::load<FloatP>(wi.x().data() + i * FloatP::Size);
//         wi_p.y() = dr::load<FloatP>(wi.y().data() + i * FloatP::Size);
//         wi_p.z() = dr::load<FloatP>(wi.z().data() + i * FloatP::Size);
//
//         FloatP result_p = 0.f;
//
//         for (size_t j = 0; j < dr::width(nodes_x); ++j) {
//             ScalarVector2f node = { nodes_x[j], nodes_y[j] };
//             ScalarVector2f weight = { weights_x[j], weights_y[j] };
//             node = dr::fmadd(node, 0.5f, 0.5f);
//
//             Normal3fP m = std::get<0>(distr.sample(wi_p, node));
//             Vector3fP wo = reflect(wi_p, m);
//             FloatP f = std::get<0>(fresnel(dr::dot(wi_p, m), FloatP(eta)));
//             FloatP smith = distr.smith_g1(wo, m) * f;
//             dr::masked(smith, wo.z() <= 0.f || wi_p.z() <= 0.f) = 0.f;
//             result_p += smith * dr::prod(weight) * 0.25f;
//
//
//             // float pdf = eval(m) * smith_g1(wi, m) * dr::abs_dot(wi, m) /
//             //             Frame3f::cos_theta(wi);
//         }
//
//         dr::store(result.data() + i * FloatP::Size, result_p);
//     }
//
//     return result;
// }

#endif //PIS_MICROFACET_H
