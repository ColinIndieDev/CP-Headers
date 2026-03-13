#version 330 core
out vec4 frag_color;

in vec2 tex_coord;

uniform sampler2D screen_tex;

void none() {
    frag_color = vec4(texture(screen_tex, tex_coord).rgb, 1.0);
}
void inverse() {
    frag_color = vec4(vec3(1.0 - texture(screen_tex, tex_coord)), 1.0);
}
void grayscale() {
    frag_color = texture(screen_tex, tex_coord);
    float avg = 0.2126 * frag_color.r + 0.7152 * frag_color.g + 0.0722 * frag_color.b;
    frag_color = vec4(avg, avg, avg, 1.0);
}

void kernel(int mode) {
    float offset = 1.0 / 300.0;
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );
    
    float kernel[9];

    if (mode == 0) {
        // Sharp
        kernel = float[](
            -1.0, -1.0, -1.0,
            -1.0,  9, -1.0,
            -1.0, -1.0, -1
        );
    }
    else if (mode == 1) {
        // Edge detection
        kernel = float[](
             1.0,  1.0,  1.0,
             1.0, -8,  1.0,
             1.0,  1.0,  1
        );
    }
    else {
        // Blur
        kernel = float[](
            1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
            2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
            1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0  
        );
    }
    vec3 sample_tex[9];
    for(int i = 0; i < 9; i++) {
        sample_tex[i] = vec3(texture(screen_tex, tex_coord.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++) {
        col += sample_tex[i] * kernel[i];
    }
    frag_color = vec4(col, 1.0);
}

void main() {
    kernel(2);
} 
