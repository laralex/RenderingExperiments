struct MaterialCoefficients {
    highp float specularity;
    highp float specularPower;
    highp float reserved0;
    highp float reserved1;
};

struct Material {
    highp vec4 diffuseColor;
    MaterialCoefficients coefs;
};