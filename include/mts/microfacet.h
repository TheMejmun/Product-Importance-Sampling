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
            print_zero(eval(m), eva)
            print_zero(smith_g1(wi, m), smi)
            print_zero(std::abs(utils::dot(wi, m)), dot)
            print_zero(1.0 / utils::cosTheta(wi), cos)
            return eval(m) * smith_g1(wi, m) * std::abs(utils::dot(wi, m)) / utils::cosTheta(wi);
        }

        [[nodiscard]] double reflected_pdf(const Vec3f &wi, const Vec3f &m) const {
            print_zero(1.0 / (4.0 * std::abs(utils::dot(wi, m))), jac)
            return solid_angle_density(wi, m) / (4.0 * std::abs(utils::dot(wi, m)));
        }

        // [[nodiscard]] double solid_angle_density(const Polar &wi, const Polar &m) const {
        //     return eval(m) * smith_g1(wi, m) * std::abs(utils::dot(wi, m)) / utils::cosTheta(wi);
        // }
        //
        // [[nodiscard]] double reflected_pdf(const Polar &wi, const Polar &m) const {
        //     return solid_angle_density(wi, m) / (4.0 * std::abs(utils::dot(wi, m)));
        // }

        [[nodiscard]] Normal3f sample(const Vec3f &wi, const Point2f &sample) const {
            // Visible normal sampling.

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

        /**
         * \brief Smith's shadowing-masking function for a single direction
         *
         * \param v
         *     An arbitrary direction
         * \param m
         *     The microfacet normal
         */

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
                printf("misoriented %f, %f\n", utils::dot(v, m), utils::cosTheta(v));
                printf("v: [%f,%f,%f]\tm: [%f,%f,%f]\n", v.x, v.y, v.z, m.x, m.y, m.z);
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


// double eval_reflectance(const mts::MicrofacetDistribution &distr,
//                        const Vec3f &wi,double eta) {
//     MI_IMPORT_CORE_TYPES()
//
//     if (!distr.sample_visible())
//         Throw("eval_reflectance(): requires visible normal sampling!");
//
//     int res = eta > 1 ? 32 : 128;
//
//     using FloatX = dr::DynamicArray<dr::scalar_t<double>>;
//     auto [nodes, weights] = quad::gauss_legendre<FloatX>(res);
//     double result = dr::zeros<double>(dr::width(wi));
//
//     auto [nodes_x, nodes_y]     = dr::meshgrid(nodes, nodes);
//     auto [weights_x, weights_y] = dr::meshgrid(weights, weights);
//
//     using FloatP = dr::Packet<dr::scalar_t<double>>;
//     using Normal3fP = Normal<FloatP, 3>;
//     using Vector3fP = Vector<FloatP, 3>;
//
//     uint32_t packet_count = dr::width(wi) / FloatP::Size;
//
//     Assert(dr::width(wi) % FloatP::Size == 0);
//
//     for (uint32_t i = 0; i < packet_count; ++i) {
//         Vector3fP wi_p;
//         wi_p.x() = dr::load<FloatP>(wi.x().data() + i * FloatP::Size);
//         wi_p.y() = dr::load<FloatP>(wi.y().data() + i * FloatP::Size);
//         wi_p.z() = dr::load<FloatP>(wi.z().data() + i * FloatP::Size);
//
//         FloatP result_p = 0.0;
//
//         for (uint32_t j = 0; j < dr::width(nodes_x); ++j) {
//             ScalarVector2f node = { nodes_x[j], nodes_y[j] };
//             ScalarVector2f weight = { weights_x[j], weights_y[j] };
//             node = dr::fmadd(node, 0.5f, 0.5f);
//
//             Normal3fP m = std::get<0>(distr.sample(wi_p, node));
//             Vector3fP wo = reflect(wi_p, m);
//             FloatP f = std::get<0>(fresnel(dr::dot(wi_p, m), FloatP(eta)));
//             FloatP smith = distr.smith_g1(wo, m) * f;
//             dr::masked(smith, wo.z() <= 0.0 || wi_p.z() <= 0.0) = 0.0;
//             result_p += smith * dr::prod(weight) * 0.25f;
//
//
//             // double pdf = eval(m) * smith_g1(wi, m) * dr::abs_dot(wi, m) /
//             //             Frame3f::cos_theta(wi);
//         }
//
//         dr::store(result.data() + i * FloatP::Size, result_p);
//     }
//
//     return result;
// }

#endif //PIS_MICROFACET_H
