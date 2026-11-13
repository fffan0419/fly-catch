#include "splashkit.h"
#include "splashkit-arrays.h"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const double SPEED = 3;
const int INITIAL_FLY_COUNT = 5;

struct timespan
{
    long start;
    long duration;
};

struct entity
{
    bool enabled;
    point_2d pos;
    double radius;
    color clr;
};

struct spider_data
{
    entity ent;
};

struct fly_data
{
    entity ent;
    timespan caught_interval;
};

struct game_data
{
    spider_data spider;
    dynamic_array<fly_data> flies;
    int score;
};

/**
 * Create a new spider at screen centre.
 *
 * @return initialised spider_data
 */
spider_data new_spider()
{
    spider_data spider;
    spider.ent =
    {
        true,
        {SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0},
        20,
        COLOR_BLACK
    };
    return spider;
}

/**
 * Create a new fly with random position and active time.
 *
 * @param current_time current tick time
 * @return initialised fly_data
 */
fly_data new_fly(long current_time)
{
    fly_data fly;
    fly.ent =
    {
        false,
        {(double)rnd(SCREEN_WIDTH), (double)rnd(SCREEN_HEIGHT)},
        12,
        COLOR_YELLOW
    };
    fly.caught_interval = {current_time + 1000 + rnd(2000), 2000 + rnd(5000)};
    return fly;
}

/**
 * Draw entity if it is enabled.
 *
 * @param ent entity to draw
 */
void draw_entity(const entity &ent)
{
    if (!ent.enabled)
    {
        return;
    }
    fill_circle(ent.clr, ent.pos.x, ent.pos.y, ent.radius);
}

/**
 * Move entity using arrow keys and clamp inside screen bounds.
 *
 * @param ent entity to move
 * @param speed movement speed
 */
void move_with_keyboard(entity &ent, double speed)
{
    if (key_down(RIGHT_KEY))
    {
        ent.pos.x += speed;
    }
    if (key_down(LEFT_KEY))
    {
        ent.pos.x -= speed;
    }
    if (key_down(DOWN_KEY))
    {
        ent.pos.y += speed;
    }
    if (key_down(UP_KEY))
    {
        ent.pos.y -= speed;
    }
    if (ent.pos.x + ent.radius > SCREEN_WIDTH)
        ent.pos.x = SCREEN_WIDTH - ent.radius;
    if (ent.pos.x - ent.radius < 0)
        ent.pos.x = ent.radius;
    if (ent.pos.y + ent.radius > SCREEN_HEIGHT)
        ent.pos.y = SCREEN_HEIGHT - ent.radius;
    if (ent.pos.y - ent.radius < 0)
        ent.pos.y = ent.radius;
}

/**
 * Check if time is within the timespan.
 *
 * @param span target timespan
 * @param time time to test
 * @return true if time is inside span
 */
bool within_timespan(const timespan &span, long time)
{
    return time >= span.start && time < (span.start + span.duration);
}

/**
 * Check if time is after the end of timespan.
 *
 * @param span target timespan
 * @param time time to test
 * @return true if time past span end
 */
bool after_timespan(const timespan &span, long time)
{
    return time >= (span.start + span.duration);
}

/**
 * Check collision between two enabled circle entities.
 *
 * @param ent1 first entity
 * @param ent2 second entity
 * @return true when circles intersect
 */
bool entities_collide(const entity &ent1, const entity &ent2)
{
    return ent1.enabled && ent2.enabled && circles_intersect(
        ent1.pos.x, ent1.pos.y, ent1.radius,
        ent2.pos.x, ent2.pos.y, ent2.radius
    );
}

/**
 * Update fly active state, recreate fly when timespan ends.
 *
 * @param fly fly data to update
 * @param current_time current tick time
 */
void update_fly(fly_data &fly, long current_time)
{
    fly.ent.enabled = within_timespan(fly.caught_interval, current_time);
    if (after_timespan(fly.caught_interval, current_time))
    {
        fly = new_fly(current_time);
    }
}

/**
 * Run game update logic: move spider, check collisions, update flies and score.
 *
 * @param game main game state
 * @param current_time current tick time
 */
void update_game(game_data &game, long current_time)
{
    move_with_keyboard(game.spider.ent, SPEED);

    for(int i = 0; i < length(game.flies); i++)
    {
        fly_data &fly = game.flies[i];
        if (entities_collide(game.spider.ent, fly.ent))
        {
            fly = new_fly(current_time);
            game.score++;
        }
        update_fly(fly, current_time);
    }
}

/**
 * Render game screen with spider, flies and score text.
 *
 * @param game read‑only game state for drawing
 */
void draw_game(const game_data &game)
{
    clear_screen(COLOR_WHITE);
    draw_entity(game.spider.ent);

    for(int i = 0; i < length(game.flies); i++)
    {
        draw_entity(game.flies[i].ent);
    }

    draw_text("Score: " + to_string(game.score), COLOR_BLACK, "arial", 24, 10,10);
    refresh_screen();
}

/**
 * Run the program.
 *
 * @return 0 when the program finishes successfully
 */
int main()
{
    open_window("Fly Catch", SCREEN_WIDTH, SCREEN_HEIGHT);
    game_data game;
    game.spider = new_spider();
    game.score = 0;

    for(int i = 0; i < INITIAL_FLY_COUNT; i++)
    {
        add(game.flies, new_fly(current_ticks()));
    }

    while (!quit_requested())
    {
        process_events();
        update_game(game, current_ticks());
        draw_game(game);
    }
    return 0;
}