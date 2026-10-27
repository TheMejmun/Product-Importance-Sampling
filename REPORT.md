# Motivation behind Product Importance Sampling

TODO: Explain Monte Carlo Integration, ray intersection, the choice of sampling direction,
light sampling and brdf sampling are heuristics, $L_e$ is just addition of constant, 

Multiple Importance Sampling approximates the rendering equation's integral (See figure below) by sampling the incoming light ($L_i$) and the BRDF ($f_r$) independently. It then takes a weighted sum of both these values for the final output.
![Rendering Equation](https://www.alex-jiang.com/wp-content/uploads/2025/02/renderequation.jpg)
TODO: graphic should parametrize L_i and f_r with omega

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

# Results

- Good in limited experiment
- Explain strengths and weaknesses of PIS

TODO: PDF, Results in Tables & Plots, with different cases shown