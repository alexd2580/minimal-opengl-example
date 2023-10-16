#version 450

in vec2 uv;

// layout(location = 0) out vec4 frag_color;
// layout(location = 1) out float frag_depth;

out vec4 frag_color;

void main(){
    // frag_color = vec4(uv.x, 0, uv.y, 1);
    frag_color = vec4(1, 1, 1, 1);
}
