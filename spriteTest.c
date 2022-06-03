#include <gb/gb.h>
#include <gb/cgb.h>
#include "new_g.c"
#include "ghosty.c"
#include "wallpaper.c"
#include "floor_tileset.c"
#include "mush.c"
#include "mushbun.c"
#include "test_map.c"
#include <stdbool.h>
#include "enemy.c"
#include "enemy2.c"
#include "enemy03.c"
#include "number_set.c"
#include "map_tileset.c"
#include "castle_meta.c"
#include "castle.c"
#include <stdio.h>
UWORD empty_palette[]={
    RGB(6,	28,	31),
    RGB(18,	24,	27),
    RGB(22,	27,	30),
    RGB(27,	29,	30)
};
UWORD platform_palette[]={
    RGB(20,	20,	20),
    RGB(16,	16,	16),
    RGB(9,	16,	1),
    RGB(9,	19,	0) 
};
UWORD flower1_palette[]={
    RGB(12,	12,	12),
    RGB(16,	16,	16),
    RGB(7,	13,	1),
    RGB(27,	15,	15)
};
UWORD flower2_palette[]={
    RGB(29,	20,	0),
    RGB(8,	13,	1),
    RGB(9,	16,	1),
    RGB(9,	19,	0)
};
UWORD sky_palette[]={
    RGB(6,	28,	31),
    RGB(18,	24,	27),
    RGB(22,	27,	30),
    RGB(27,	29,	30)
};
UWORD bg_palette_set[] ={

    //Platform
    RGB(20,	20,	20),
    RGB(16,	16,	16),
    RGB(9,	16,	1),
    RGB(9,	19,	0),

    //Flower-1
    RGB(12,	12,	12),
    RGB(16,	16,	16),
    RGB(7,	13,	1),
    RGB(27,	15,	15),

    //Flower-2
    RGB(29,	20,	0),
    RGB(8,	13,	1),
    RGB(9,	16,	1),
    RGB(9,	19,	0),

    //SKY
    RGB(6,	28,	31),
    RGB(18,	24,	27),
    RGB(22,	27,	30),
    RGB(27,	29,	30),
    
    //Sky
    RGB(6,	28,	31),
    RGB(18,	24,	27),
    RGB(22,	27,	30),
    RGB(27,	29,	30)
};
#define PINK            RGB(31,  16,  23)
#define BLUE            RGB(10,  21, 31)
#define PURPLE          RGB(17,  13, 20)
#define RGB_LIGHTFLESH  RGB(30, 20, 15)
#define ANIMATION_SPEED 20
#define JUMP_GRAVITY    256
#define JUMP_SCALAR     2
#define GRAVITY         2
#define MAX_TIME        30

uint8_t timer = MAX_TIME;
bool playing = true;

uint8_t clamp(uint8_t n, uint8_t min, uint8_t max){
    if(n > max) n = max;
    else if(n < min) n = min;
    return n;
}

struct Player {
    uint8_t x;
    uint8_t y;
    bool jumping;
    uint8_t current_frame;
    uint8_t counter;
    uint8_t sprite_num;
    int8_t force_vec;
    uint8_t sprite_size; //1 = 8x8 or 8x16, 2 = 8x16 or 16x6, etc...
};

struct Map {
    uint8_t w;
    uint8_t h; 
};

UWORD wallpaper_palette[] = {
    
    RGB(16, 1, 3), RGB(31, 0, 11), RGB(4, 16, 6), RGB(0, 7, 5)
};

UWORD player_palette[] = {
    0, PINK, BLUE, PURPLE
};

bool check_collisions(struct Player *p, int8_t amountMoved){
    bool collision = false;
    if(p->x + amountMoved <= 0 || p->y + amountMoved <= 0) collision = true;
    else if(joypad() == J_LEFT || joypad() == J_RIGHT){
        if(get_bkg_tile_xy((p->x + amountMoved)/8, p->y/8) != 0x04){
            collision = true;
        }
    }
    else if(joypad() == J_UP || joypad() == J_DOWN){
        if(get_bkg_tile_xy(p->x/8, (p->y + amountMoved)/8) != 0x04){
            collision = true;
        }
    }

    return collision;
}

bool check_fall_collisions(struct Player *p, uint8_t n){
    bool collision = false;
    if(get_bkg_tile_xy(p->x/8, (p->y + n)/8) != 0x04) collision = true;
    //if(get_bkg_data_xy(p->x/8, (p->y - n)/8) != 0x04) collision = true;
    return collision;
}

void move_player(struct Player *p){//send ref to player struct
    int8_t n = 0;
    uint8_t key = joypad();
    if(key == J_LEFT || key == J_UP) n = -1;
    else n = 1;
    //check for collisions
    if(!check_collisions(p, n)){
        if(key & J_UP){
            if(!p->jumping && check_fall_collisions(p, GRAVITY)){
                        p->y-= GRAVITY;
                        p->force_vec = 1;
                        p->jumping = true;
            }
        }
        switch(key){
            case J_LEFT:
                p->x -= 1;
                //0x04 is the blank tile for sky and such i.e. no collision
                //if(get_bkg_tile_xy((p->x-1)/8, p->y/8) == 0x04) p->x-=1;
                //else p->x += 8;
                break;
            case J_RIGHT:
                p->x += 1;
                //if(get_bkg_tile_xy((p->x+1)/8, p->y/8) == 0x04) p->x+=1;
                //else p->x -= 8;
                break;
            case J_DOWN:
                p->y+=1;
                break;
        }
    }
    //set current tiles to use for player sprite
    for(uint8_t i = 0; i < 4; i++){
        set_sprite_tile(i, i + (p->current_frame * 4));
    }
    //draw sprite
    move_sprite(0, p->x, p->y);
    move_sprite(1, p->x, p->y + 8);  
    move_sprite(2, p->x+8, p->y);
    move_sprite(3 , p->x + 8, p->y + 8);  

    //SHOW_SPRITES;

}

void physicsCheck(struct Player *p){
    int8_t sum_force = 0;
    if(p->jumping){
        sum_force = -(p->force_vec * p->force_vec) + (JUMP_GRAVITY * p->force_vec);
        sum_force = sum_force/8;
        //if(-(sum_force) < GRAVITY && (p->y - sum_force) > 0) p->y += -(sum_force);
        if(-(sum_force) < GRAVITY) p->y += JUMP_SCALAR * -(sum_force);
        else p->jumping = false;
        p->force_vec++;

        //printf("here");
        //p->y += GRAVITY;
        //formula for arc for jump h(x) = x^2-gx where g is force of gravity
        
        
    }
    else if(!check_fall_collisions(p, GRAVITY)) p->y+=GRAVITY; //falling becomes two pix per sec
    if(check_fall_collisions(p, GRAVITY) && p->jumping){
        p->jumping = false;
        //printf("here");
    } 

}
void draw_enemy(struct Player *e){//send ref to player struct
    uint8_t n = 0;
    //SPRITES_8x8;
    while(n < e->sprite_size){
        move_sprite(e->sprite_num, e->x, e->y);
        n++;
    }
    //SHOW_SPRITES;
}

void cycle_sprite(struct Player *p){
    uint8_t key = joypad();
    if(key & J_LEFT){
        if(p->counter >= ANIMATION_SPEED){
            p->counter = 0;
            p->current_frame = ((p->current_frame + 1) % 3);
        }
        if(p->current_frame == 0) p->current_frame = 1;
        p->counter++;
    }
    else if(key & J_RIGHT){
        if(p->counter >= ANIMATION_SPEED){
            p->counter = 0;
            p->current_frame = ((p->current_frame + 1) % 5);
        }
        if(p->current_frame < 3) p->current_frame = 3;
        p->counter++;
    }
    else{
        p->counter = 0;
        p->current_frame = 0;
    }
}
void cycle_enemy_sprite(struct Player *e){
    e->counter++;
    if(e->counter >= ANIMATION_SPEED){
        e->counter = 0;
        if(!check_fall_collisions(e, 1)){
            if(e->current_frame < 4) e->y--;
            else e->y++;
        }
        e->current_frame = (e->current_frame + 1) % 7;//replace with num_sprites parameter later
        set_sprite_tile(4, 20 + e->current_frame);
    }
}
void main(){
    //variables
    struct Player hero;
    uint8_t time_counter = 0;
    hero.x = 75;
    hero.y = 75;
    hero.sprite_size = 4;
    hero.sprite_num = 0;
    hero.counter = 0;
    hero.current_frame = 0;
    hero.force_vec = 0;
    hero.jumping = false;

    struct Player enemy_1;
    enemy_1.x = 132;
    enemy_1.y = 132;
    enemy_1.sprite_size = 1;
    enemy_1.sprite_num = hero.sprite_num + hero.sprite_size;

    struct Player enemy_2;
    enemy_2.x = 138;
    enemy_2.y = 138;
    enemy_2.sprite_size = 1;
    enemy_2.sprite_num = enemy_1.sprite_num + 1;

    //sand timer
    struct Player clock;
    clock.x = 8, clock.y = 8, clock.sprite_size = 1, clock.sprite_num = 28;

    SPRITES_8x8;

    //draw background
    set_bkg_palette(0, 5, bg_palette_set);
    set_bkg_data(0, 22, map_tileset);
    VBK_REG = 1; //sets Video Bank Reg to attributes
    set_bkg_tiles(0, 0, castle_metaWidth, castle_metaHeight, castle_meta);
    VBK_REG = 0; //sets Video Bank Reg to tile numbers
    set_bkg_tiles(0, 0, castle_tilemapWidth, castle_tilemapHeight, castle_tilemap);
    
    //draw player sprite
    set_sprite_data(0, 20, mushbun);
    for(int i = 0; i < 4; i++){
        set_sprite_tile(i, i);
    }
    move_player(&hero);

    SHOW_SPRITES;
    //SPRITES_8x8;
    //draw enemy
    set_sprite_data(20, 8, enemy1);
    set_sprite_tile(4, 20);
    move_sprite(4, enemy_1.x, enemy_1.y);

    //draw sand timer
    set_sprite_data(28, 10, number_set);
    set_sprite_tile(5, 28);
    set_sprite_tile(6, 29);

    SHOW_SPRITES;
    while(playing){
        uint8_t index = 0;
        if(time_counter > MAX_TIME){
            time_counter = 0;
            timer--;
            if(timer <= 0) playing = false;
        }
        physicsCheck(&hero);
        cycle_sprite(&hero);
        move_player(&hero);
        cycle_enemy_sprite(&enemy_1);
        draw_enemy(&enemy_1);
        
        SHOW_BKG;
        SHOW_SPRITES;
        //idles cpu and waits for screen isr to update before proceeding
        wait_vbl_done();
        delay(10);
        time_counter++;
    }
}