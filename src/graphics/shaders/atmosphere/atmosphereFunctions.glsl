#include "../src/graphics/shaders/atmosphere/atmosphereResources.glsl"

layout(set = 0, binding = 0) uniform sampler2D transmittanceLUT;
layout(set = 0, binding = 1) uniform sampler2D multiScatteringLUT;

float fromSubUVToUnit(float u, float resolution) {
    return (u - 0.5 / resolution) * (resolution / (resolution - 1.0));
}

float fromUnitToSubUV(float u, float resolution) {
    return (u + 0.5 / resolution) * (resolution / (resolution + 1.0));
}

float raySphereIntersectNearest(vec3 r0, vec3 rd, vec3 s0, float sR) {
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sR * sR);
    float delta = b * b - 4.0 * a * c;

    if (delta < 0.0 || a == 0.0) {
        return -1.0;
    }

    float sol0 = (-b - sqrt(delta)) / (2.0 * a);
    float sol1 = (-b + sqrt(delta)) / (2.0 * a);

    if (sol0 < 0.0 && sol1 < 0.0) {
        return -1.0;
    }
    if (sol0 < 0.0) {
        return max(0.0, sol1);
    }
    if (sol1 < 0.0) {
        return max(0.0, sol0);
    }
    return max(0.0, min(sol0, sol1));
}

float hgPhase(float g, float cosTheta) {
    float num = 1.0 - (g * g);
    float denom = 1.0 + (g * g) + 2.0 * g * cosTheta;
    return num / (4.0 * M_PI * denom * sqrt(denom));
}

float rayleighPhase(float cosTheta) {
    float factor = 3.0 / (16.0 * M_PI);
    return factor * (1.0 + (cosTheta * cosTheta));
}

vec3 getAlbedo(vec3 scattering, vec3 extinction) {
    return scattering / max(vec3(0.001), extinction);
}

MediumSampleRGB sampleMediumRGB(vec3 worldPos) {
    const float viewHeight = length(worldPos) - bottomRadius;

    const float densityMie = exp(mieDensity[1].w * viewHeight);
    const float densityRay = exp(rayleighDensity[1].w * viewHeight);
    const float densityOzo = clamp(viewHeight < absorptionDensity[0].x ?
    absorptionDensity[0].w * viewHeight + absorptionDensity[1].x :
    absorptionDensity[2].x * viewHeight + absorptionDensity[2].y, 0.0, 1.0);

    MediumSampleRGB s;

    s.scatteringMie = densityMie * mieScattering;
    s.absorptionMie = densityMie * mieAbsorption;
    s.extinctionMie = densityMie * mieExtinction;

    s.scatteringRay = densityRay * rayleighScattering;
    s.absorptionRay = vec3(0.0);
    s.extinctionRay = s.scatteringRay + s.absorptionRay;

    s.scatteringOzo = vec3(0.0);
    s.absorptionOzo = densityOzo * absorptionExtinction;
    s.extinctionOzo = s.scatteringOzo + s.absorptionOzo;

    s.scattering = s.scatteringMie + s.scatteringRay + s.scatteringOzo;
    s.absorption = s.absorptionMie + s.absorptionRay + s.absorptionOzo;
    s.extinction = s.extinctionMie + s.extinctionRay + s.extinctionOzo;

    s.albedo = getAlbedo(s.scattering, s.extinction);

    return s;
}

vec2 lutTransmittanceParamsToUV(float viewHeight, float viewZenithCosAngle) {
    float h = sqrt(max(0.0, (topRadius * topRadius) - (bottomRadius * bottomRadius)));
    float rho = sqrt(max(0.0, (viewHeight * viewHeight) - (bottomRadius * bottomRadius)));

    float discriminant = (viewHeight * viewHeight) * ((viewZenithCosAngle * viewZenithCosAngle) - 1.0) + (topRadius * topRadius);
    float d = max(0.0, (-viewHeight * viewZenithCosAngle + sqrt(discriminant)));

    float d_min = topRadius - viewHeight;
    float d_max = rho + h;
    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / h;

    return vec2(x_mu, x_r);
}

vec2 uvToSkyViewLutParams(float viewHeight, vec2 uv) {
    uv = vec2(fromSubUVToUnit(uv.x, 192.0), fromSubUVToUnit(uv.y, 108.0));

    float vHorizon = sqrt((viewHeight * viewHeight) - (bottomRadius * bottomRadius));
    float cosBeta = vHorizon / viewHeight;
    float beta = acos(cosBeta);
    float zenithHorizonAngle = M_PI - beta;
    float viewZenithCosAngle;

    if (uv.y < 0.5) {
        float coord = uv.y * 2.0;
        coord = 1.0 - coord;
        coord = 1.0 - coord;
        viewZenithCosAngle = cos(zenithHorizonAngle * coord);
    }
    else {
        float coord = uv.y * 2.0 - 1.0;
        viewZenithCosAngle = cos(zenithHorizonAngle + beta * coord);
    }

    float coord = uv.x;
    coord *= coord;
    float lightViewCosAngle = -(coord * 2.0 - 1.0);

    return vec2(viewZenithCosAngle, lightViewCosAngle);
}

vec2 skyViewLutParamsToUV(bool intersectGround, float viewZenithCosAngle, float lightViewCosAngle, float viewHeight) {
    vec2 tmpUV;
    float vHorizon = sqrt((viewHeight * viewHeight) - (bottomRadius * bottomRadius));
    float cosBeta = vHorizon / viewHeight;
    float beta = acos(cosBeta);
    float zenithHorizonAngle = M_PI - beta;

    if (!intersectGround) {
        float coord = acos(viewZenithCosAngle) / zenithHorizonAngle;
        coord = 1.0 - coord;
        coord = 1.0 - coord;
        tmpUV.y = coord * 0.5;
    }
    else {
        float coord = (acos(viewZenithCosAngle) - zenithHorizonAngle) / beta;
        tmpUV.y = coord * 0.5 + 0.5;
    }

    float coord = -lightViewCosAngle * 0.5 + 0.5;
    coord = sqrt(coord);
    tmpUV.x = coord;

    return vec2(fromUnitToSubUV(tmpUV.x, 192.0), fromUnitToSubUV(tmpUV.y, 108.0));
}

vec4 moveToTopAtmosphere(vec3 worldPos, vec3 worldDir) {
    float viewHeight = length(worldPos);
    if (viewHeight > topRadius) {
        float tTop = raySphereIntersectNearest(worldPos, worldDir, vec3(0.0), topRadius);
        if (tTop >= 0.0) {
            vec3 upVector = worldPos / viewHeight;
            vec3 upOffset = upVector * -planetRadiusOffset;
            worldPos = worldPos + worldDir * tTop + upOffset;
        }
        else {
            return vec4(worldPos, 0.0);
        }
    }

    return vec4(worldPos, 1.0);
}

vec3 getSunLuminance(vec3 worldPos, vec3 worldDir, float planetRadius, vec3 sunDirection) {
    if (dot(worldDir, sunDirection) > cos (0.5 * 0.505 * 3.14159 / 180.0)) {
        float t = raySphereIntersectNearest(worldPos, worldDir, vec3(0.0, 0.0, 0.0), planetRadius);
        if (t < 0.0) {
            return vec3(1000000.0);
        }
    }

    return vec3(0.0); 
}

vec3 getMultiScattering(vec3 scattering, vec3 extinction, vec3 worldPos, float viewZenithCosAngle) {
    vec2 msUV = clamp(vec2(viewZenithCosAngle * 0.5 + 0.5, (length(worldPos) - bottomRadius) / (topRadius - bottomRadius)), 0.0, 1.0);
    msUV = vec2(fromUnitToSubUV(msUV.x, 32), fromUnitToSubUV(msUV.y, 32));

    vec3 multiScatteringLuminance = texture(multiScatteringLUT, msUV).rgb;
    return multiScatteringLuminance;
}

SingleScatteringResult integrateScatteredLuminance(vec2 uv, mat4 viewProj, vec3 worldPos, vec3 worldDir, vec3 sunDir, bool ground, float sampleCountIni, float depthBufferValue, bool variableSampleCount, bool mieRayPhase, float tMaxMax) {
    SingleScatteringResult result;
    result.l = vec3(0.0);
    result.opticalDepth = vec3(0.0);
    result.transmittance = vec3(0.0);
    result.multiScatteringAs1 = vec3(0.0);
    result.newMultiScatteringStep0Out = vec3(0.0);
    result.newMultiScatteringStep1Out = vec3(0.0);
    
    vec3 clipSpace = vec3(uv * 2.0 - 1.0, 1.0);

    vec3 ground0 = vec3(0.0, 0.0, 0.0);
    float tBottom = raySphereIntersectNearest(worldPos, worldDir, ground0, bottomRadius);
    float tTop = raySphereIntersectNearest(worldPos, worldDir, ground0, topRadius);
    float tMax = 0.0;
    if (tBottom < 0.0) {
        if (tTop < 0.0) {
            tMax = 0.0;
            return result;
        }
        else {
            tMax = tTop;
        }
    }
    else {
        if (tTop > 0.0) {
            tMax = min(tTop, tBottom);
        }
    }

    if (depthBufferValue >= 0.0) {
        clipSpace.z = depthBufferValue;
        if (clipSpace.z < 1.0) {
            vec4 depthBufferWorldPos = inverse(viewProj) * vec4(clipSpace, 1.0);
            depthBufferWorldPos /= depthBufferWorldPos.w;

            float tDepth = length(depthBufferWorldPos.xyz - (worldPos + vec3(0.0, -bottomRadius, 0.0)));
            if (tDepth < tMax) {
                tMax = tDepth;
            }
        }
    }
    tMax = min(tMax, tMaxMax);

    float sampleCount = sampleCountIni;
    float sampleCountFloor = sampleCountIni;
    float tMaxFloor = tMax;
    if (variableSampleCount) {
        sampleCount = mix(SAMPLES_MIN, SAMPLES_MAX, clamp(tMax * 0.01, 0.0, 1.0));
        sampleCountFloor = floor(sampleCount);
        tMaxFloor = tMax * sampleCountFloor / sampleCount;
    }
    float dt = tMax / sampleCount;

    const float uniformPhase = 1.0 / (4.0 * M_PI);
    const vec3 wi = sunDir;
    const vec3 wo = worldDir;
    float cosTheta = dot(wi, wo);
    float miePhaseValue = hgPhase(miePhaseFunctionG, -cosTheta);
    float rayleighPhaseValue = rayleighPhase(cosTheta);

    vec3 globalL = vec3(1.0);

    vec3 l = vec3(0.0);
    vec3 throughput = vec3(1.0);
    vec3 opticalDepth = vec3(0.0);
    float t = 0.0;
    float tPrev = 0.0;
    const float sampleSegmentT = 0.3;
    for (float sampleIndex = 0.0; sampleIndex < sampleCount; sampleIndex += 1.0) {
        if (variableSampleCount) {
            float t0 = sampleIndex / sampleCountFloor;
            float t1 = (sampleIndex + 1.0) / sampleCountFloor;
            t0 = t0 * t0;
            t1 = t1 * t1;
            t0 = tMaxFloor * t0;
            if (t1 > 1.0) {
                t1 = tMax;
            }
            else {
                t1 = tMaxFloor * t1;
            }
            t = t0 + (t1 - t0) * sampleSegmentT;
            dt = t1 - t0;
        }
        else {
            float newT = tMax * (sampleIndex + sampleSegmentT) / sampleCount;
            dt = newT - t;
            t = newT;
        }
        vec3 p = worldPos + t * worldDir;

        MediumSampleRGB medium = sampleMediumRGB(p);
        const vec3 sampleOpticalDepth = medium.extinction * dt;
        const vec3 sampleTransmittance = exp(-sampleOpticalDepth);
        opticalDepth += sampleOpticalDepth;

        float pHeight = length(p);
        const vec3 upVector = p / pHeight;
        float sunZenithCosAngle = dot(sunDir, upVector);
        vec2 transmittanceUV = lutTransmittanceParamsToUV(pHeight, sunZenithCosAngle);
        vec3 transmittanceToSun = texture(transmittanceLUT, transmittanceUV).xyz;

        vec3 phaseTimesScattering;
        if (mieRayPhase) {
            phaseTimesScattering = medium.scatteringMie * miePhaseValue + medium.scatteringRay * rayleighPhaseValue;
        }
        else {
            phaseTimesScattering = medium.scattering * uniformPhase;
        }

        float tPlanet = raySphereIntersectNearest(p, sunDir, ground0 + planetRadiusOffset * upVector, bottomRadius);
        float planetShadow = tPlanet >= 0.0 ? 0.0 : 1.0;

        vec3 multiScatteredLuminance = vec3(0.0);
        multiScatteredLuminance = getMultiScattering(medium.scattering, medium.extinction, p, sunZenithCosAngle);

        float shadow = 1.0;

        vec3 s = globalL * (planetShadow * shadow * transmittanceToSun * phaseTimesScattering + multiScatteredLuminance * medium.scattering);

        vec3 ms = medium.scattering;
        vec3 msInt = (ms - ms * sampleTransmittance) / medium.extinction;
        result.multiScatteringAs1 += throughput * msInt;

        vec3 newMS = planetShadow * transmittanceToSun * medium.scattering * uniformPhase;
        result.newMultiScatteringStep0Out += throughput * (newMS - newMS * sampleTransmittance) / medium.extinction;

        newMS = medium.scattering * uniformPhase * multiScatteredLuminance;
        result.newMultiScatteringStep1Out += throughput * (newMS - newMS * sampleTransmittance) / medium.extinction;

        vec3 sInt = (s - s * sampleTransmittance) / medium.extinction;
        l += throughput * sInt;
        throughput *= sampleTransmittance;

        tPrev = t;
    }

    if (ground && tMax == tBottom && tBottom > 0.0) {
        vec3 p = worldPos + tBottom * worldDir;
        float pHeight = length(p);

        const vec3 upVector = p / pHeight;
        float sunZenithCosAngle = dot(sunDir, upVector);
        vec2 transmittanceUV = lutTransmittanceParamsToUV(pHeight, sunZenithCosAngle);
        vec3 transmittanceToSun = texture(transmittanceLUT, transmittanceUV).xyz;

        const float NdotL = clamp(dot(normalize(upVector), normalize(sunDir)), 0.0, 1.0);
        l += globalL * transmittanceToSun * throughput * NdotL * groundAlbedo / M_PI;
    }

    result.l = l;
    result.opticalDepth = opticalDepth;
    result.transmittance = throughput;
    
    return result;
}