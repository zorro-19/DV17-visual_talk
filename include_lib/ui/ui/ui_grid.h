#ifndef UI_GRID_H
#define UI_GRID_H


#include "ui/ui_core.h"
#include "ui/control.h"


struct ui_grid_item_info {
    u8 row;
    u8 col;
    u8 page_mode;
    u8 highlight_index;
    u16 interval;
    u8 touch_move_en;
    struct layout_info *info;
};

struct ui_grid {
    struct element elm;
    char hi_num;
    char hi_index;
    char new_hi_index;
    char touch_index;
    char onfocus;
    u8   touch_move_en;
    u8   auto_hi;
    u8   page_mode;
    u8   col_num;
    u8   row_num;
    u8   show_row;
    u8   show_col;
    u8   avail_item_num;
    u8   pix_scroll;
    int  rotate;
    int  max_right;
    int  max_bottom;
    struct layout *item;
    struct layout_info *item_info;
    struct element elm2;
    struct position pos;
    struct draw_context dc;
    const struct ui_grid_info *info;
    const struct element_event_handler *handler;
};

extern const struct element_event_handler grid_elm_handler;

static inline int ui_grid_cur_item(struct ui_grid *grid)
{
    if (grid->touch_index >= 0) {
        return grid->touch_index;
    }
    if (grid->auto_hi == 1) {
        return grid->hi_index;
    }
    return -1;
}

#define ui_grid_set_item(grid, index)  	(grid)->new_hi_index = index

#define ui_grid_touch_move_en(grid, en) (grid)->touch_move_en = en


void ui_grid_on_focus(struct ui_grid *grid);

void ui_grid_lose_focus(struct ui_grid *grid);

void ui_grid_state_reset(struct ui_grid *grid, int highlight_item);

int ui_grid_highlight_item(struct ui_grid *grid, int item, bool yes);

int ui_grid_highlight_item_by_id(int id, int item, bool yes);


struct ui_grid *__ui_grid_new(struct element_css *css, int id,
                              struct ui_grid_item_info *info, struct element *parent);

#endif

