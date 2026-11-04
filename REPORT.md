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

# Benchmarks

All the following benchmarks follow the same structure. 
In the _Equal Samples_ section, we compute the integral of the rendering equation with 1024 samples. 
In other words: We render a single pixel with a sample count of 1024. 
In the _Equal Time_ section, we limit the integral calculation not by sample count, but by a time of 0.001 seconds.

These _renders_ are performed 1024 times, with a new $\omega_o$ viewing direction each. 
For each of those 1024 renders, we calculate the error with a high-sample-count reference value.
From that, we calculate the Mean Squared Error.

### Diffuse + single uniform light
The first benchmark is a sanity check for the implementation. 
We render with a diffuse material and a single uniform light source. 

| Method         | Equal Samples 1024 x 1024 | Equal Time 1024 x 0.001s |
|----------------|--------------------------:|-------------------------:|
| BRDF Sampling  |                  0.000230 |                 0.000010 |
| Light Sampling |                  0.000221 |                 0.000030 |
| MIS            |                  0.000110 |                 0.000018 |
| PIS            |                  0.000226 |                 0.000036 |

As expected, all except MIS deliver similar results in the Equal Samples benchmark.
The score of MIS is half that, because it combines two samples, one for BRDF and one for light sampling, per resulting MIS sample.
The equal time benchmark is indirectly proportional to the calculation speed of each algorithm.

### Microfacet + zenith light

The following benchmarks all use the microfacet model instead of a diffuse material to better represent the state-of-the-art use cases.
The light sources from here on are all high-intensity light sources covering 2% of the hemicircle.
In this benchmark specifically, the light source is placed in the direction of the surface normal, i.e. at an angle of $90&deg;$.

| Method         | Equal Samples 1024 x 1024 | Equal Time 1024 x 0.001s |
|----------------|--------------------------:|-------------------------:|
| BRDF Sampling  |                  0.208543 |                 0.055958 |
| Light Sampling |                  0.004186 |                 0.001341 |
| MIS            |                  0.005747 |                 0.002155 |
| PIS            |                  0.005727 |                 0.002426 |

TODO

### Microfacet + front light

In this benchmark, the same light source is placed in the direction $\omega_o$, behind the camera.
The term front light is borrowed from photography, referring to a subject lit from the same direction as the camera.

| Method         | Equal Samples 1024 x 1024 | Equal Time 1024 x 0.001s |
|----------------|--------------------------:|-------------------------:|
| BRDF Sampling  |                  0.110784 |                 0.024245 |
| Light Sampling |                  0.010034 |                 0.004485 |
| MIS            |                  0.003876 |                 0.001846 |
| PIS            |                  0.004911 |                 0.001772 |

TODO

### Microfacet Back light

TODO

| Method         | Equal Samples 1024 x 1024 | Equal Time 1024 x 0.001s |
|----------------|--------------------------:|-------------------------:|
| BRDF Sampling  |                  1.164458 |                 0.290111 |
| Light Sampling |                  0.175121 |                 0.031702 |
| MIS            |                  0.060111 |                 0.029806 |
| PIS            |                  0.095535 |                 0.034527 |

TODO

### Microfacet Random light

TODO

| Method         | Equal Samples 4096 x 1024 | Equal Time 4096 x 0.001s |
|----------------|--------------------------:|-------------------------:|
| BRDF Sampling  |                  0.181180 |                 0.040210 |
| Light Sampling |                  0.010775 |                 0.002564 |
| MIS            |                  0.006222 |                 0.003048 |
| PIS            |                  0.009125 |                 0.004268 |

TODO

# Conclusion

- Good in limited experiment
- Explain strengths and weaknesses of PIS
