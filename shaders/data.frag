#version 460

in vec2 uv;

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 frag_depth;

// layout(location = 0) out vec4 frag_color;
// layout(location = 1) out float frag_depth;

void main(){
    // frag_color = vec4(uv.x, 0, uv.y, 1);
    frag_color = vec4(abs(uv), 0, 1);
    frag_depth.r = abs(uv.y);
}
