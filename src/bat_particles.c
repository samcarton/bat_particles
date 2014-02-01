#include <pebble.h>

#include "bat_rand.h"

static Window *window;

static AppTimer *timer;

static Layer *draw_layer;

static GRect window_frame;

#define NUM_PARTICLES 300
#define NUM_GENERATED 30

#define ACCEL_FACTOR 0.001f
#define VELOCITY_FACTOR 0.01f

#define TIMER_TIME 100

typedef struct Vec2
{
  float x;
  float y;
} Vec2;

typedef struct Particle
{
  Vec2 pos;
  Vec2 vel;
  unsigned short life;
} Particle;

static Particle g_particles[NUM_PARTICLES];

static float yAccel;
static float xAccel;
static int xCent;
static int yCent;
static int xSize;

// move emitter
static int xPos;
static uint32_t emitterTimer;
#define EMITTER_MOVE_TIME 50

// initialise particles to zero
static void particleInit(Particle *p)
{
  p->pos.x = 0;
  p->pos.y = 0;
  p->vel.x = 0;
  p->vel.y = 0;
  p->life = 0;
}

// draw a particle
static void drawParticle(Particle *p, GContext *ctx)
{
  graphics_draw_pixel(ctx,GPoint(p->pos.x,p->pos.y));
}

// apply particle motion
static void particleMotion(Particle *p)
{
  // include timer value?
  p->vel.x += xAccel;
  p->vel.y += yAccel;
  p->pos.x += p->vel.x;
  p->pos.y += p->vel.y;
}

// update the particles array
static void updateParticles(GContext *ctx)
{
  int i=0;
  for(;i<NUM_PARTICLES;++i)
  {
    if(g_particles[i].life>0)
    {
      // apply motion to particles
      particleMotion(&g_particles[i]);
      // draw particle
      drawParticle(&g_particles[i],ctx);

      // decrement life
      if(g_particles[i].life < TIMER_TIME)
      {
        g_particles[i].life = 0;
      }
      else
      {
        g_particles[i].life = g_particles[i].life - TIMER_TIME;
      }

    }
  }
}

// activate particle with random vel and life at center
static void activateParticle(Particle *p, float x, float y)
{
  p->pos.x = x;
  p->pos.y = y;
  p->vel.x = (((int)(bat_rand()%1001))-500)*VELOCITY_FACTOR;
  p->vel.y = (((int)(bat_rand()%1001))-500)*VELOCITY_FACTOR;
  p->life = 1000 + bat_rand()%1000;
}

// generate a new particle
static void generateParticle()
{
  int i=0;
  for(;i<NUM_PARTICLES;++i)
  {
    if(g_particles[i].life ==0)
    {
      activateParticle(&g_particles[i], xPos, yCent);
      return;
    }
  }
}


static void handle_accel(AccelData *accel_data, uint32_t num_samples) {
  // do nothing
}

static void draw_layer_callback(Layer *me, GContext *ctx){
  //draw stuff
  // 144 . 168

  graphics_context_set_stroke_color(ctx, GColorWhite);

  if(emitterTimer > EMITTER_MOVE_TIME)
  {
    emitterTimer = 0;
    xPos++;
    if(xPos > xSize)
    {
      xPos = 0;
    }
  }

  int i =0;
  for(;i<NUM_GENERATED;++i)
  {
    generateParticle();
  }

  updateParticles(ctx);  

}

static void timer_callback(void *data) {
  AccelData accel = { 0, 0, 0, 0, 0 };

  accel_service_peek(&accel);  
  yAccel = -accel.y * ACCEL_FACTOR;
  xAccel = accel.x * ACCEL_FACTOR;

  emitterTimer += TIMER_TIME;

  layer_mark_dirty(draw_layer);

  timer = app_timer_register(TIMER_TIME /* milliseconds */, timer_callback, NULL); 
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = window_frame = layer_get_frame(window_layer);

  // set center
  xCent = window_frame.size.w/2;
  yCent = window_frame.size.h/2;
  xSize = window_frame.size.w;

  draw_layer = layer_create(frame);
  layer_set_update_proc(draw_layer, draw_layer_callback);
  layer_add_child(window_layer, draw_layer);

  // init particles
  int i=0;
  for(i=0;i<NUM_PARTICLES;++i)
  {
    particleInit(&g_particles[i]);
  }

}

static void window_unload(Window *window) {
  layer_destroy(draw_layer);
}


static void init() {
  yAccel = 0;
  xAccel = 0;
  xPos = 0;
  emitterTimer = 0;
  xSize = 0;

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  accel_data_service_subscribe(0, handle_accel);

  timer = app_timer_register(TIMER_TIME /* milliseconds */, timer_callback, NULL);

}

static void deinit() {
  accel_data_service_unsubscribe();  
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
