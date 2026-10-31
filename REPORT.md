# Motivation behind Product Importance Sampling

The primary goal of path tracing is to quickly and accurately approximate the rendering equation. In particular, we are interested in the integral over the hemisphere $\Omega$, as $L_e$ is usually a constant.

$$
L_o(x \rightarrow \omega_o) = L_e(x \rightarrow \omega_o) + \int_{\Omega} f_r(x, \omega_i \rightarrow \omega_o) \, L_i(x \leftarrow \omega_i) \, \cos\theta \, d\omega_i
$$

For this approximation, Monte Carlo Integration is commonly used. This method allows us to estimate the integral by calculating a weighted average of many random samples.

$$
L_o(x \rightarrow \omega_o) \approx L_e(x \rightarrow \omega_o) + \frac{1}{N} \sum_{n=1}^{N} \frac{f_r(x, \omega_{i,n} \rightarrow \omega_o) \, L_i(x \leftarrow \omega_{i,n}) \, \cos\theta_n}{p(\omega_{i,n})}
$$

TODO: Explain Monte Carlo Integration, ray intersection, the choice of sampling direction,
light sampling and brdf sampling are heuristics, $L_e$ is just addition of constant, 

Multiple Importance Sampling approximates the rendering equation's integral (See figure below) by sampling the incoming light ($L_i$) and the BRDF ($f_r$) independently. It then takes a weighted sum of both these values for the final output.

Product Importance Sampling instead attempts to sample the product of these two components directly. 
TODO: Product should be optimal


# Methodology

- Setup of the experiment
  - 2D
  - Microfacet Model
  - Equal Time, Equal Samples tests

# Implementation

- Problems encountered
  - Integral calculation
  - Mapping of uniform samples (to limit sampling area)
- w_i and w_o swapped

# Results

- Good in limited experiment
- Explain strengths and weaknesses of PIS

TODO: PDF, Results in Tables & Plots, with different cases shown