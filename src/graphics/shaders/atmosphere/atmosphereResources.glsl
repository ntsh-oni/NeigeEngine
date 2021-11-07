const vec3 solarIrradiance = vec3(1.0, 1.0, 1.0);
const float sunAngularRadius = 0.00467;

const float muSMin = -0.5;

const vec3 absorptionExtinction = vec3(0.00065, 0.00188, 0.00008);
const vec4 absorptionDensity[3] = vec4[3](vec4(25.0, 0.0, 0.0, 0.06667), vec4(-0.66667, 0.0, 0.0, 0.0), vec4(-0.06667, 2.66667, -0.00142, -0.00142));

const vec3 rayleighScattering = vec3(0.0058, 0.01356, 0.0331);
const vec4 rayleighDensity[3] = vec4[3](vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, -0.125), vec4(0.0, 0.0, -0.00142, -0.00142));

const float miePhaseFunctionG = 0.8;
const vec3 mieScattering = vec3(0.004, 0.004, 0.004);
const vec3 mieExtinction = vec3(0.00444, 0.00444, 0.00444);
const vec3 mieAbsorption = vec3(0.00044, 0.00044, 0.00044);
const vec4 mieDensity[3] = vec4[3](vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, -0.83333), vec4(0.0, 0.0, -0.00142, -0.00142));

const float topRadius = 6460.0;
const float bottomRadius = 6360.0;

const float planetRadiusOffset = 0.01;

const vec3 groundAlbedo = vec3(0.0, 0.0, 0.0);

#define SAMPLES_MIN 4
#define SAMPLES_MAX 14

#define M_PI 3.1415926535897932384626433832795

struct SingleScatteringResult {
    vec3 l;
    vec3 opticalDepth;
    vec3 transmittance;
    vec3 multiScatteringAs1;
    vec3 newMultiScatteringStep0Out;
    vec3 newMultiScatteringStep1Out;
};

struct MediumSampleRGB {
    vec3 scattering;
    vec3 absorption;
    vec3 extinction;

    vec3 scatteringMie;
    vec3 absorptionMie;
    vec3 extinctionMie;

    vec3 scatteringRay;
    vec3 absorptionRay;
    vec3 extinctionRay;

    vec3 scatteringOzo;
    vec3 absorptionOzo;
    vec3 extinctionOzo;
    
    vec3 albedo;
};