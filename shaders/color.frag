#version 450

in vec2 uv;
out vec4 frag_color;

layout(binding = 0) uniform view {
    int width;
    int height;
};

layout(std430, binding = 2) buffer random {
    int random_seed[];
};

void main() {
    if (uv.x > 0) {
        frag_color = vec4(1, 0, 0, 1);
    }
}
