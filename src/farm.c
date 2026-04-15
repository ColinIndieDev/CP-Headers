#define CPL_IMPLEMENTATION
#include "../cplibrary/cpl.h"

#define FARMLAND_SIZE 50

#define WHEAT_GROW_DUR 10

typedef enum { CROP_NONE, CROP_WHEAT } crop_type;

typedef struct {
    vec2f pos;
    crop_type crop;
    f32 planted_time;
    b8 harvest;
} farm;

VEC_DEF(farm, property)

b8 farm_exists(vec2f pos);
farm *get_farm_at(vec2f pos);

void init();
void handle_movement();
void update_loop();
void draw_loop();
void draw_ui_loop();
void cleanup();

typedef enum { STATE_PLOWING, STATE_PLANTING, STATE_HARVEST, STATE_SIZE } state;

font f;
property farmland;
state edit_mode = STATE_PLOWING;
crop_type crop_selected = CROP_WHEAT;
f32 money_balance = 5.0f;
f32 property_size = 10;

MAIN_PROG main(void) {
    cpl_init_window(800, 800, "Pendulum Simulation", OPENGL_VER_3_3);
    cpl_enable_vsync(false);

    init();

    while (!cpl_window_should_close()) {
        cpl_update();

        handle_movement();

        update_loop();
        draw_loop();
        draw_ui_loop();

        cpl_end_frame();
    }
    cpl_close_window();
}

void init() {
    cpl_create_font(&f, "assets/fonts/arial.ttf", "default", CPL_FILTER_LINEAR);

    property_reserve(&farmland, 100);
}

void handle_movement() {
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

    if (cpl_is_key_pressed(CPL_KEY_G)) {
        edit_mode = (edit_mode + 1) % STATE_SIZE;
    }
    if (cpl_is_key_pressed(CPL_KEY_B)) {
        if (edit_mode <= 0) {
            edit_mode = STATE_SIZE - 1;
        } else {
            edit_mode--;
        }
    }

    if (cpl_is_key_down(CPL_KEY_H)) {
        cpl_cam_2D.zoom += 2 * cpl_get_dt();
    }
    if (cpl_is_key_down(CPL_KEY_N)) {
        cpl_cam_2D.zoom -= 2 * cpl_get_dt();
    }
    cpl_cam_2D.zoom = CPM_CLAMP(cpl_cam_2D.zoom, 0.1f, 10);
}

void update_loop() {
    if (cpl_is_mouse_down(CPL_MOUSE_BUTTON_LEFT)) {
        vec2f mouse_world = cpl_get_screen_to_world_2D(cpl_get_mouse_pos());
        vec2f f = {
            mouse_world.x > 0 ? cpm_floorf(mouse_world.x / FARMLAND_SIZE)
                              : cpm_floorf(mouse_world.x / FARMLAND_SIZE) - 1,
            mouse_world.y > 0 ? cpm_floorf(mouse_world.y / FARMLAND_SIZE)
                              : cpm_floorf(mouse_world.y / FARMLAND_SIZE) - 1};
        if (edit_mode == STATE_PLOWING) {
            if (f.x < property_size / 2.0f && f.y < (property_size + 1) / 2.0f &&
                f.x > (-property_size - 1) / 2.0f &&
                f.y > -property_size / 2.0f && !farm_exists(f)) {
                property_push_back(&farmland, (farm){.pos = f,
                                                     .crop = CROP_NONE,
                                                     .planted_time = 0,
                                                     .harvest = false});
            }
        } else if (edit_mode == STATE_PLANTING) {
            farm *cur = get_farm_at(f);

            f32 cost = 0.0f;
            switch (crop_selected) {
            case CROP_WHEAT:
                cost = 0.5f;
                break;
            default:
                break;
            }

            if (cur && cur->crop == CROP_NONE && money_balance >= cost) {
                money_balance -= cost;
                cur->crop = crop_selected;
                cur->planted_time = cpl_get_time();
                cur->harvest = false;
            }
        } else if (edit_mode == STATE_HARVEST) {
            farm *cur = get_farm_at(f);
            if (cur && cur->harvest) {
                switch (cur->crop) {
                case CROP_WHEAT:
                    money_balance += 2.5f;
                    break;
                default:
                    break;
                }

                cur->crop = CROP_NONE;
                cur->planted_time = 0;
                cur->harvest = false;
            }
        }
    }
    FOREACH_VEC(farm, property, f, &farmland) {
        if (f->planted_time + WHEAT_GROW_DUR <= cpl_get_time()) {
            f->harvest = true;
        }
    }
}

void draw_loop() {
    cpl_clear_background(GREEN);

    cpl_begin_draw(CPL_SHAPE_2D_UNLIT, true);

    FOREACH_VEC(farm, property, f, &farmland) {
        vec4f color;
        switch (f->crop) {
        case CROP_NONE:
            color = BROWN;
            break;
        case CROP_WHEAT: {
            if (f->harvest) {
                color = YELLOW;
            } else {
                color = LIME_GREEN;
            }
            break;
        }
        }
        cpl_draw_rect(vec2f_f32_mul(&f->pos, FARMLAND_SIZE),
                      VEC2F_INIT(FARMLAND_SIZE), color, 0.0f);
    }

    {
        vec2f mouse_world = cpl_get_screen_to_world_2D(cpl_get_mouse_pos());
        cpl_draw_circle(mouse_world, 10.0f, RED);
    }
}

void draw_ui_loop() {
    cpl_begin_draw(CPL_TEXT, false);

    {
        char *mode;
        switch (edit_mode) {
        case STATE_PLOWING:
            mode = "Mode: Plowing";
            break;
        case STATE_PLANTING:
            mode = "Mode: Planting";
            break;
        case STATE_HARVEST:
            mode = "Mode: Harvest";
            break;
        default:
            mode = "Mode: ???";
            break;
        }
        cpl_draw_text(&f, mode, VEC2F_INIT(10), 0.5f, WHITE);
    }

    {
        char balance[50];
        snprintf(balance, sizeof(balance), "Money: $%.2f", money_balance);
        cpl_draw_text(&f, balance, (vec2f){10, 50}, 0.5f, WHITE);
    }
}

void cleanup() {
    property_destroy(&farmland);
    cpl_delete_font(&f);
}

b8 farm_exists(vec2f pos) {
    FOREACH_VEC(farm, property, f, &farmland) {
        if (pos.x == f->pos.x && pos.y == f->pos.y) {
            return true;
        }
    }
    return false;
}

farm *get_farm_at(vec2f pos) {
    FOREACH_VEC(farm, property, f, &farmland) {
        if (pos.x == f->pos.x && pos.y == f->pos.y) {
            return f;
        }
    }
    return NULLPTR;
}
