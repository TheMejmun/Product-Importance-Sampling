# Motivation behind Product Importance Sampling

The primary goal of path tracing is to quickly and accurately approximate the rendering equation. 
In particular, we are interested in the integral over the hemisphere $\Omega$, as $L_e$ is usually known.

$$
L_o(x \rightarrow \omega_o) = L_e(x \rightarrow \omega_o) + \int_{\Omega} f_r(x, \omega_i \rightarrow \omega_o) \, L_i(x \leftarrow \omega_i) \, \cos\theta \, d\omega_i
$$

For this approximation, Monte Carlo Integration is commonly used. This method allows us to estimate the integral by calculating a weighted average of many random samples.

$$
L_o(x \rightarrow \omega_o) \approx L_e(x \rightarrow \omega_o) + \frac{1}{N} \sum_{n=1}^{N} \frac{f_r(x, \omega_{i,n} \rightarrow \omega_o) \, L_i(x \leftarrow \omega_{i,n}) \, \cos\theta_n}{p(\omega_{i,n})}
$$

By dividing the integrand by the probability density function $p(\omega_{i,n})$, we ensure that we do not introduce bias into our estimate. 
This PDF represents the likelihood of a particular sample being chosen, and dividing by it normalizes the contribution of each sample to the final result. 
The choice of the sampling direction $\omega_{i,n}$ 

TODO: Explain ray intersection, the choice of sampling direction, light sampling and brdf sampling are heuristics

Multiple Importance Sampling approximates the rendering equation's integral (See figure below) by sampling the incoming light ($L_i$) and the BRDF ($f_r$) independently. 
It then takes a weighted sum of both these values for the final output.

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