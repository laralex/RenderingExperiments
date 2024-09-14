struct LightCoefficients {
    highp float radius;
    highp float reserved1;
    highp float reserved2;
    highp float reserved3;
};

struct Light {
    highp vec4 worldPosition;
    highp vec4 diffuseColor;
    highp vec4 ambientIntensity;
    LightCoefficients coefs;
};