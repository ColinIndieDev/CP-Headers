#define CPL_IMPLEMENTATION
#include "../cplibrary/cpl.h"

#define TIME_TO_X 25.0f
#define DELTA_Y_AXIS 0.1f
#define D 3.45f
#define m 1.0f
#define g 9.81f
#define k 8.9e-2f /* 8.9e-4 */

VEC_DEF(vec2f, trail_pos)

void handle_input();
void compute_pendulum();
void restart();
void draw_check_box(vec2f p, b8 *show);
void draw_graph(trail_pos *tp, vec4f color, f32 limit, b8 use);
void draw_loop();

b8 show_tx = true;
b8 show_tv = true;
b8 show_ta = true;
b8 show_tF = true;

const f32 origin = -0.2f;
const f32 scale = 1000.0f;
const f32 limit = -100.0f;
const f32 measure_dt = 0.1f;

f32 x = 0.0f;
f32 v = 0.0f;
f32 F_r = 0.0f;
f32 F = 0.0f;
f32 a = 0.0f;
f32 start = 0.0f;
f32 begin = 0.0f;
vec2f ball_pos = {.x = 0, .y = 0};

trail_pos tx_graph;
trail_pos tv_graph;
trail_pos ta_graph;
trail_pos tF_graph;

font f;
texture spring;
texture ball;

int main(void) {
    cpl_init_window(800, 800, "Pendulum Simulation");
    cpl_enable_vsync(false);

    x = origin;
    v = 0.0f;
    F_r = k * (v * CPM_ABS(v));
    F = -(D * x) - F_r;
    a = F / m;

    start = cpl_get_time();
    begin = cpl_get_time();

    trail_pos_reserve(&tx_graph, 1000);
    trail_pos_reserve(&tv_graph, 1000);
    trail_pos_reserve(&ta_graph, 1000);
    trail_pos_reserve(&tF_graph, 1000);

    cpl_set_time_scale(1.0f);

    cpl_create_font(&f, "assets/fonts/arial.ttf", "default", CPL_FILTER_LINEAR);
    cpl_load_texture(&spring, "assets/images/spring.png", CPL_FILTER_LINEAR);
    cpl_load_texture(&ball, "assets/images/ball.png", CPL_FILTER_LINEAR);

    while (!cpl_window_should_close()) {
        cpl_update();

        handle_input();

        compute_pendulum();

        draw_loop();

        cpl_end_frame();
    }
    trail_pos_destroy(&tx_graph);
    trail_pos_destroy(&tv_graph);
    trail_pos_destroy(&ta_graph);
    trail_pos_destroy(&tF_graph);
    cpl_close_window();
}

void handle_input() {
    f32 speed = 100.0f;

    if (cpl_is_key_down(CPL_KEY_W)) {
        cpl_cam_2D.pos.y -= speed * cpl_get_dt() * (1.0f / cpl_cam_2D.zoom);
    }
    if (cpl_is_key_down(CPL_KEY_S)) {
        cpl_cam_2D.pos.y += speed * cpl_get_dt() * (1.0f / cpl_cam_2D.zoom);
    }
    if (cpl_is_key_down(CPL_KEY_A)) {
        cpl_cam_2D.pos.x -= speed * cpl_get_dt() * (1.0f / cpl_cam_2D.zoom);
    }
    if (cpl_is_key_down(CPL_KEY_D)) {
        cpl_cam_2D.pos.x += speed * cpl_get_dt() * (1.0f / cpl_cam_2D.zoom);
    }
    if (cpl_is_key_down(CPL_KEY_ESCAPE)) {
        cpl_destroy_window();
    }

    if (cpl_is_key_pressed(CPL_KEY_R)) {
        restart();
    }

    if (cpl_is_key_down(CPL_KEY_H)) {
        cpl_cam_2D.zoom += 2 * cpl_get_dt();
    }
    if (cpl_is_key_down(CPL_KEY_N)) {
        cpl_cam_2D.zoom -= 2 * cpl_get_dt();
    }
    cpl_cam_2D.zoom = CPM_CLAMP(cpl_cam_2D.zoom, 0.1f, 10);
}

void compute_pendulum() {
    F_r = k * (v * CPM_ABS(v));
    F = -(D * x) - F_r;
    a = F / m;
    v += (a * cpl_get_dt());
    x += (v * cpl_get_dt());

    ball_pos = (vec2f){.x = cpl_get_screen_width() / 2.0f,
                       .y = (x * scale) + (cpl_get_screen_height() / 2.0f)};

    if (begin + measure_dt <= cpl_get_time()) {
        trail_pos_push_back(&tx_graph, ball_pos);
        trail_pos_push_back(
            &tv_graph, (vec2f){ball_pos.x,
                               (v * scale) + (cpl_get_screen_height() / 2.0f)});
        trail_pos_push_back(
            &ta_graph, (vec2f){ball_pos.x,
                               (a * scale) + (cpl_get_screen_height() / 2.0f)});
        trail_pos_push_back(
            &tF_graph, (vec2f){ball_pos.x,
                               (F * scale) + (cpl_get_screen_height() / 2.0f)});

        begin = cpl_get_time();
    }
}

void restart() {
    start = cpl_get_time();
    trail_pos_clear(&tx_graph);
    trail_pos_clear(&tv_graph);
    trail_pos_clear(&ta_graph);
    trail_pos_clear(&tF_graph);
    x = origin;
    v = 0.0f;
    F_r = k * (v * CPM_ABS(v));
    F = -(D * x) - F_r;
    a = F / m;
}

void draw_graph(trail_pos *tp, vec4f color, f32 limit, b8 use) {
    FOREACH_VEC(vec2f, trail_pos, t, tp) { t->x -= TIME_TO_X * cpl_get_dt(); }
    VEC_ERASE_IF(tp, it.x < limit);

    if (!use) {
        return;
    }

    if (tp->size < 2) {
        return;
    }

    f32 *vertices = cp_malloc(3 * sizeof(f32) * tp->size);
    if (!vertices) {
        return;
    }

    u32 i = 0;
    FOREACH_VEC(vec2f, trail_pos, t, tp) {
        vertices[i++] = t->x;
        vertices[i++] = t->y;
        vertices[i++] = 0.0f;
    }

    u32 vao = 0;
    u32 vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (i32)(3 * sizeof(f32) * tp->size), vertices,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32),
                          (void *)NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    mat4f transform;
    mat4f_identity(&transform);
    mat4f_translate(&transform, &(vec3f){0.0f, 0.0f, 0.0f});

    cpl_shader_set_mat4f(&cpl_shaders[CPL_SHAPE_2D_UNLIT], "transform",
                         transform);
    cpl_shader_set_rgba(&cpl_shaders[CPL_SHAPE_2D_UNLIT], "input_color",
                        &color);
    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_STRIP, 0, (i32)tp->size);
    glBindVertexArray(0);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    cp_free(vertices);
    VEC_ERASE_IF(tp, it.x < limit);
}

void draw_loop() {
    cpl_clear_background(BLACK);

    cpl_begin_draw(CPL_SHAPE_2D_UNLIT, true);

    draw_graph(&tx_graph, GREEN, limit, show_tx);

    draw_graph(&tv_graph, RGB(0.0f, 255.0f, 255.0f), limit, show_tv);

    draw_graph(&ta_graph, RED, limit, show_ta);

    draw_graph(&tF_graph, RGB(255.0f, 255.0f, 0.0f), limit, show_tF);

    cpl_draw_line((vec2f){limit, cpl_cam_2D.pos.y},
                  (vec2f){limit, cpl_cam_2D.pos.y + (cpl_get_screen_height() *
                                                     (1.0f / cpl_cam_2D.zoom))},
                  2.0f * cpl_cam_2D.zoom, WHITE);

    cpl_draw_line((vec2f){limit, cpl_get_screen_height() / 2.0f},
                  (vec2f){ball_pos.x, cpl_get_screen_height() / 2.0f}, 1,
                  RGBA(255, 255, 255, 127));

    for (f32 i = ball_pos.x; i >= limit; i -= TIME_TO_X) {
        cpl_draw_line((vec2f){i, (cpl_get_screen_height() / 2.0f) - 2},
                      (vec2f){i, (cpl_get_screen_height() / 2.0f) + 2}, 0.5f,
                      WHITE);
    }

    cpl_begin_draw(CPL_TEXTURE_2D_UNLIT, true);

    vec2f spring_size = {0.03f * scale, ball_pos.y};
    cpl_draw_texture2D(&spring,
                       (vec2f){ball_pos.x - (spring_size.x / 2.0f), 0.0f},
                       spring_size, WHITE, 0.0f);

    f32 ball_size = 0.04f * scale;
    cpl_draw_texture2D(&ball,
                       (vec2f){ball_pos.x - (ball_size / 2.0f),
                               ball_pos.y - (ball_size / 2.0f)},
                       (vec2f){ball_size, ball_size}, WHITE, 0.0f);

    cpl_begin_draw(CPL_TEXT, true);

    u32 t = 0;
    for (f32 i = ball_pos.x; i >= limit; i -= TIME_TO_X, t++) {
        char t_str[50];
        snprintf(t_str, sizeof(t_str), "%d", t);
        cpl_draw_text(&f, t_str,
                      (vec2f){i, (cpl_get_screen_height() / 2.0f) + 5}, 0.2f,
                      WHITE);
    }

    cpl_begin_draw(CPL_SHAPE_2D_UNLIT, false);

    cpl_draw_line((vec2f){10.0f, 100.0f}, (vec2f){110.0f, 100.0f}, 1.0f, WHITE);

    draw_check_box(
        (vec2f){cpl_get_screen_width() - 50, cpl_get_screen_height() - 50},
        &show_tx);

    draw_check_box(
        (vec2f){cpl_get_screen_width() - 50, cpl_get_screen_height() - 100},
        &show_tv);

    draw_check_box(
        (vec2f){cpl_get_screen_width() - 50, cpl_get_screen_height() - 150},
        &show_ta);

    draw_check_box(
        (vec2f){cpl_get_screen_width() - 50, cpl_get_screen_height() - 200},
        &show_tF);

    cpl_begin_draw(CPL_TEXT, false);

    cpl_draw_text(
        &f, "t-x",
        (vec2f){cpl_get_screen_width() - 90, cpl_get_screen_height() - 50},
        0.5f, WHITE);

    cpl_draw_text(
        &f, "t-v",
        (vec2f){cpl_get_screen_width() - 90, cpl_get_screen_height() - 100},
        0.5f, WHITE);

    cpl_draw_text(
        &f, "t-a",
        (vec2f){cpl_get_screen_width() - 90, cpl_get_screen_height() - 150},
        0.5f, WHITE);

    cpl_draw_text(
        &f, "t-F",
        (vec2f){cpl_get_screen_width() - 90, cpl_get_screen_height() - 200},
        0.5f, WHITE);

    {
        char layout[50];
        snprintf(layout, sizeof(layout), "%.3fm",
                 100.0f / (scale * cpl_cam_2D.zoom));
        cpl_draw_text(&f, layout, (vec2f){10.0f, 120.0f}, 0.5f, WHITE);
    }

    {
        char layout[50];
        snprintf(layout, sizeof(layout), "1 : %fm",
                 1.0f / (scale * cpl_cam_2D.zoom));
        cpl_draw_text(&f, layout, (vec2f){10.0f, 10.0f}, 0.5f, WHITE);
    }

    {
        char elapsed[50];
        snprintf(elapsed, sizeof(elapsed), "Time: %.1fs",
                 cpl_get_time() - start);
        cpl_draw_text(&f, elapsed,
                      (vec2f){10.0f, cpl_get_screen_height() - 50.0f}, 0.5f,
                      WHITE);
    }
    {
        char mass[50];
        snprintf(mass, sizeof(mass), "Mass: %.3fkg", m);
        cpl_draw_text(&f, mass,
                      (vec2f){10.0f, cpl_get_screen_height() - 100.0f}, 0.5f,
                      WHITE);
    }
    {
        char acceleration[50];
        snprintf(acceleration, sizeof(acceleration), "Acceleration: %.2fm/s^2",
                 a);
        cpl_draw_text(&f, acceleration,
                      (vec2f){10.0f, cpl_get_screen_height() - 150.0f}, 0.5f,
                      WHITE);
    }
    {
        char speed[50];
        snprintf(speed, sizeof(speed), "Speed: %.3fm/s", v);
        cpl_draw_text(&f, speed,
                      (vec2f){10.0f, cpl_get_screen_height() - 200.0f}, 0.5f,
                      WHITE);
    }
    {
        char force[50];
        snprintf(force, sizeof(force), "Force: %.3fN", F);
        cpl_draw_text(&f, force,
                      (vec2f){10.0f, cpl_get_screen_height() - 250.0f}, 0.5f,
                      WHITE);
    }
}

void draw_check_box(vec2f p, b8 *show) {
    vec2f size = {.x = 25, .y = 25};
    cpl_draw_rect(p, size, WHITE, 0.0f);

    if (*show) {
        f32 thickness = 3.0f;
        cpl_draw_line(p, vec2f_add(&p, &size), thickness, BLACK);
        cpl_draw_line((vec2f){p.x, p.y + size.y}, (vec2f){p.x + size.x, p.y},
                      thickness, BLACK);
    }

    rect_collider box = {.pos = p, .size = size};
    if (cpl_check_collision_vec2f_rect(
            cpl_get_screen_to_world_2D(cpl_get_mouse_pos()), &box) &&
        cpl_is_mouse_pressed(CPL_MOUSE_BUTTON_LEFT)) {
        *show = !(*show);
    }
}
