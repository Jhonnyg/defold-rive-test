
#define FILL_TYPE_NONE   0
#define FILL_TYPE_SOLID  1
#define FILL_TYPE_LINEAR 2
#define FILL_TYPE_RADIAL 3
#define MAX_STOPS        16

uniform mediump vec4 uFillData;        // x: type, y: stop count
uniform mediump vec4 uGradientLimits;  // xy: start, zw: end
uniform mediump vec4 uSolidColor;
uniform mediump vec4 uGradientColor0;
uniform mediump vec4 uGradientColor1;
uniform mediump vec4 uGradientColor2;
uniform mediump vec4 uGradientColor3;
uniform mediump vec4 uGradientColor4;
uniform mediump vec4 uGradientColor5;
uniform mediump vec4 uGradientColor6;
uniform mediump vec4 uGradientColor7;
uniform mediump vec4 uGradientColor8;
uniform mediump vec4 uGradientColor9;
uniform mediump vec4 uGradientColor10;
uniform mediump vec4 uGradientColor11;
uniform mediump vec4 uGradientColor12;
uniform mediump vec4 uGradientColor13;
uniform mediump vec4 uGradientColor14;
uniform mediump vec4 uGradientColor15;
uniform mediump vec4 uGradientStop0;
uniform mediump vec4 uGradientStop1;
uniform mediump vec4 uGradientStop2;
uniform mediump vec4 uGradientStop3;

varying mediump vec2 vPosition;

vec4 getColor(int i)
{
    if      (i == 0)  return uGradientColor0;
    else if (i == 1)  return uGradientColor1;
    else if (i == 2)  return uGradientColor2;
    else if (i == 3)  return uGradientColor3;
    else if (i == 4)  return uGradientColor4;
    else if (i == 5)  return uGradientColor5;
    else if (i == 6)  return uGradientColor6;
    else if (i == 7)  return uGradientColor7;
    else if (i == 8)  return uGradientColor8;
    else if (i == 9)  return uGradientColor9;
    else if (i == 10) return uGradientColor10;
    else if (i == 11) return uGradientColor11;
    else if (i == 12) return uGradientColor12;
    else if (i == 13) return uGradientColor13;
    else if (i == 14) return uGradientColor14;
    else if (i == 15) return uGradientColor15;
    return vec4(0.0);
}

float getStop(int i)
{
    if      (i == 0)  return uGradientStop0.x;
    else if (i == 1)  return uGradientStop0.y;
    else if (i == 2)  return uGradientStop0.z;
    else if (i == 3)  return uGradientStop0.w;
    else if (i == 4)  return uGradientStop1.x;
    else if (i == 5)  return uGradientStop1.y;
    else if (i == 6)  return uGradientStop1.z;
    else if (i == 7)  return uGradientStop1.w;
    else if (i == 8)  return uGradientStop2.x;
    else if (i == 9)  return uGradientStop2.y;
    else if (i == 10) return uGradientStop2.z;
    else if (i == 11) return uGradientStop2.w;
    else if (i == 12) return uGradientStop3.x;
    else if (i == 13) return uGradientStop3.y;
    else if (i == 14) return uGradientStop3.z;
    else if (i == 15) return uGradientStop3.w;
    return 0.0;
}

void main()
{
    int fillType = int(uFillData.x);
    int stopCount = int(uFillData.y);

    if (fillType == FILL_TYPE_SOLID)
    {
        gl_FragColor = vec4(uSolidColor.rgb * uSolidColor.a, uSolidColor.a);
    }
    else if (fillType == FILL_TYPE_LINEAR)
    {
        vec2 start          = uGradientLimits.xy;
        vec2 end            = uGradientLimits.zw;
        vec2 toEnd          = end - start;
        float lengthSquared = toEnd.x * toEnd.x + toEnd.y * toEnd.y;
        float f             = dot(vPosition - start, toEnd) / lengthSquared;
        vec4 color          = mix(uGradientColor0, uGradientColor1,
            smoothstep(uGradientStop0.x, uGradientStop0.y, f));

        for (int i=1; i < MAX_STOPS; ++i)
        {
            if(i >= stopCount-1)
            {
                break;
            }
            color = mix(color, getColor(i+1), smoothstep( getStop(i), getStop(i+1), f ));
        }

        gl_FragColor = vec4(color.xyz * color.w, color.w);
    }
    else if (fillType == FILL_TYPE_RADIAL)
    {
        vec2 start = uGradientLimits.xy;
        vec2 end   = uGradientLimits.zw;
        float f    = distance(start, vPosition)/distance(start, end);
        vec4 color = mix(uGradientColor0, uGradientColor1,
            smoothstep(uGradientStop0.x, uGradientStop0.y, f));
        for (int i=1; i < MAX_STOPS; ++i)
        {
            if(i >= stopCount-1)
            {
                break;
            }
            color = mix(color, getColor(i+1), smoothstep( getStop(i), getStop(i+1), f ));
        }

        gl_FragColor = vec4(color.xyz * color.w, color.w);
    }
    else
    {
        gl_FragColor = vec4(vec3(0.0), 1.0);
    }
}
