uniform mediump vec4 uColor;

void main()
{
    gl_FragColor = vec4(uColor.rgb * uColor.a, uColor.a);
}
