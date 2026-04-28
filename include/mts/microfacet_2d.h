// //
// // Created by Saman on 14.03.26.
// //
//
// #ifndef PIS_MICROFACET_H
// #define PIS_MICROFACET_H
//
// #include "coords.h"
// #include <numbers>
//
// #include "utils.h"
//
// // Ported from: https://github.com/mitsuba-renderer/mitsuba3/blob/master/include/mitsuba/render/microfacet.h
// // sample_visible = True, type = GGX
// constexpr double PI = std::numbers::pi_v<double>;
//
// namespace mts {
//     class MicrofacetDistribution2D {
//     public:
//         explicit MicrofacetDistribution2D(const double alpha)
//             : m_alpha(alpha) {
//             m_alpha = std::max(m_alpha, 1e-4);
//         }
//
//         [[nodiscard]] double eval(const Vec2f &m) const {
//             const double cos_theta = utils::cosTheta(m);
//
//             const double result = 1.0 / (PI * m_alpha * m_alpha * std::pow(
//                                              std::pow(m.x / m_alpha, 2.0) +
//                                              std::pow(m.y / m_alpha, 2.0) +
//                                              std::pow(m.z, 2.0), 2.0)
//                                   );
//
//             // Prevent potential numerical issues in other stages of the model
//
//             return result * cos_theta > 1e-20f ? result : 0.0;
//         }
//
//         [[nodiscard]] double solid_angle_density(const Vec2f &wi, const Vec2f &m) const {
//             return eval(m) * smith_g1(wi, m) * std::abs(utils::dot(wi, m)) / utils::cosTheta(wi);
//         }
//
//         [[nodiscard]] double reflected_pdf(const Vec2f &wi, const Vec2f &m) const {
//             return solid_angle_density(wi, m) / (4.0 * std::abs(utils::dot(wi, m)));
//         }
//
//         [[nodiscard]] Normal3f sample(const Vec2f &wi, const Point2f &sample) const {
//             // Visible normal sampling.
//
//             // Step 1: stretch wi
//             const Vec2f wi_p = utils::normalize(Vec2f(
//                 m_alpha * wi.x,
//                 m_alpha * wi.y,
//                 wi.z
//             ));
//
//             const double sin_phi = utils::sinPhi(wi_p);
//             const double cos_phi = utils::cosPhi(wi_p);
//             const double cos_theta = utils::cosTheta(wi_p);
//
//             // Step 2: simulate P22_{wi}(slope.x, slope.y, 1, 1)
//             Vec2f slope = sample_visible_11(cos_theta, sample);
//
//             // Step 3: rotate & unstretch
//             slope = Vec2f(
//                 (cos_phi * slope.x - sin_phi * slope.y) * m_alpha,
//                 (sin_phi * slope.x + cos_phi * slope.y) * m_alpha);
//
//             // Step 4: compute normal
//             const Normal3f m = utils::normalize(Vec2f(-slope.x, -slope.y, 1));
//             return m;
//         }
//
//         /**
//          * \brief Smith's shadowing-masking function for a single direction
//          *
//          * \param v
//          *     An arbitrary direction
//          * \param m
//          *     The microfacet normal
//          */
//
//         [[nodiscard]] double smith_g1(const Vec2f &v, const Vec2f &m) const {
//             const double xy_alpha_2 = std::pow(m_alpha * v.x, 2.0) + std::pow(m_alpha * v.y, 2.0);
//             const double tan_theta_alpha_2 = xy_alpha_2 / std::pow(v.z, 2.0);
//
//             double result = 2.0 / (1.0 + std::sqrt(1.0 + tan_theta_alpha_2));
//
//             // Perpendicular incidence -- no shadowing/masking
//             if (xy_alpha_2 == 0.0)
//                 result = 1.0;
//
//             /* Ensure consistent orientation (can't see the back
//                of the microfacet from the front and vice versa) */
//             if (utils::dot(v, m) * utils::cosTheta(v) <= 0.0)
//                 result = 0.0;
//
//             return result;
//         }
//
//         /// \brief Visible normal sampling code for the alpha=1 case
//         static Vec2f sample_visible_11(const double cos_theta_i, const double sample) {
//             const double s = 0.5f * (1.0 + cos_theta_i);
//             p.y = utils::lerp(utils::safeSqrt(1.0 - std::pow(p.x, 2.0)), p.y, s);
//
//             const double x = p.x;
//             // Project onto chosen side of the hemisphere
//             const double y = p.y;
//             const double z = utils::safeSqrt(1.0 - utils::dot(p, p));
//
//             // Convert to slope
//             const double sin_theta_i = utils::safeSqrt(1.0 - std::pow(cos_theta_i, 2.0));
//             const double norm = 1.0 / (sin_theta_i * y + cos_theta_i * z);
//             return Vec2f(cos_theta_i * y - sin_theta_i * z, x) * norm;
//         }
//
//     protected:
//         double m_alpha;
//     };
// }
//
//
// // double eval_reflectance(const mts::MicrofacetDistribution2D &distr,
// //                        const Vec2f &wi,double eta) {
// //     MI_IMPORT_CORE_TYPES()
// //
// //     if (!distr.sample_visible())
// //         Throw("eval_reflectance(): requires visible normal sampling!");
// //
// //     int res = eta > 1 ? 32 : 128;
// //
// //     using FloatX = dr::DynamicArray<dr::scalar_t<double>>;
// //     auto [nodes, weights] = quad::gauss_legendre<FloatX>(res);
// //     double result = dr::zeros<double>(dr::width(wi));
// //
// //     auto [nodes_x, nodes_y]     = dr::meshgrid(nodes, nodes);
// //     auto [weights_x, weights_y] = dr::meshgrid(weights, weights);
// //
// //     using FloatP = dr::Packet<dr::scalar_t<double>>;
// //     using Normal3fP = Normal<FloatP, 3>;
// //     using Vector3fP = Vector<FloatP, 3>;
// //
// //     uint32_t packet_count = dr::width(wi) / FloatP::Size;
// //
// //     Assert(dr::width(wi) % FloatP::Size == 0);
// //
// //     for (uint32_t i = 0; i < packet_count; ++i) {
// //         Vector3fP wi_p;
// //         wi_p.x() = dr::load<FloatP>(wi.x().data() + i * FloatP::Size);
// //         wi_p.y() = dr::load<FloatP>(wi.y().data() + i * FloatP::Size);
// //         wi_p.z() = dr::load<FloatP>(wi.z().data() + i * FloatP::Size);
// //
// //         FloatP result_p = 0.0;
// //
// //         for (uint32_t j = 0; j < dr::width(nodes_x); ++j) {
// //             ScalarVector2f node = { nodes_x[j], nodes_y[j] };
// //             ScalarVector2f weight = { weights_x[j], weights_y[j] };
// //             node = dr::fmadd(node, 0.5f, 0.5f);
// //
// //             Normal3fP m = std::get<0>(distr.sample(wi_p, node));
// //             Vector3fP wo = reflect(wi_p, m);
// //             FloatP f = std::get<0>(fresnel(dr::dot(wi_p, m), FloatP(eta)));
// //             FloatP smith = distr.smith_g1(wo, m) * f;
// //             dr::masked(smith, wo.z() <= 0.0 || wi_p.z() <= 0.0) = 0.0;
// //             result_p += smith * dr::prod(weight) * 0.25f;
// //
// //
// //             // double pdf = eval(m) * smith_g1(wi, m) * dr::abs_dot(wi, m) /
// //             //             Frame3f::cos_theta(wi);
// //         }
// //
// //         dr::store(result.data() + i * FloatP::Size, result_p);
// //     }
// //
// //     return result;
// // }
//
// #endif //PIS_MICROFACET_H
