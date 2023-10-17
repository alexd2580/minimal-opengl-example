#version 460

in vec2 uv;
out vec4 frag_color;

// layout(location = 0) out vec4 frag_color;
// layout(location = 1) out float frag_depth;

void main(){
    // frag_color = vec4(uv.x, 0, uv.y, 1);
    frag_color = vec4(abs(uv), 0, 1);
    gl_FragDepth = 0.5;
}
