//
// Created by Saman on 29.01.26.
//

#ifndef PIS_BRDF_H
#define PIS_BRDF_H
#include "../coords.h"

class BRDF {
protected:
    BRDF() = default;

    ~BRDF() = default;

public:
    // Should return the "color" -> in this case monochrome brightness
    [[nodiscard]] virtual double eval(const Polar &wi, const Polar &wo)  const = 0;

    [[nodiscard]] virtual double pdf(const Polar &wi, const Polar &wo) const= 0;

    [[nodiscard]] virtual Polar sample(const Polar &wi)const = 0;

    [[nodiscard]] virtual const char* name()const = 0;
};

#endif //PIS_BRDF_H
