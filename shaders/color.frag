#version 460

// Binding implicitly sets the uniform value (when applicable).
layout(binding=0) uniform sampler2D color;
layout(binding=1) uniform sampler2D depth;

in vec2 uv;
out vec4 frag_color;

// layout(binding = 0) uniform view {
//     int width;
//     int height;
// };
//
// layout(std430, binding = 2) buffer random {
//     int random_seed[];
// };

void main() {
    if (uv.x > 0) {
        if (uv.y > 0) {
            frag_color = texture(color, uv);
        } else {
            frag_color = texture(depth, uv);
        }
    }
}
