// Home-screen look: scene, header pills, HUD chips, ring-gauge dock, feed menu.
// Header-only. Include after Arduino_GFX_Library.h, species.h, pet.h and
// i18n.h. TamaPoke.ino uses it on the device; the same code also compiles on a
// PC against Arduino_GFX to render PNGs.
#pragma once
#include <Arduino.h>
#include <math.h>
#include <string.h>
#include "species.h"
#include "pet.h"
#include "i18n.h"
#include "dayphase.h"
#include "rtcbat.h"

extern Arduino_Canvas *gfx;
extern Pet pet;
extern uint32_t bathUntil;

#define CX 233  // centre of the round screen
#define CY 233
#define PET_CY 202  // vertical centre of the flash sprite
#define DOCK_Y 338  // top of the bottom panel

static const uint16_t INK_K = 0x18C4;  // spriteColor('k')

// round gauge buttons on the dock: the ring around each icon is its need
struct Btn {
  int16_t cx, cy;
  const char *const *icon;
};
Btn buttons[4] = {
  { 128, 382, SPR_ICON_BERRY },  // eat   (FOOD)
  { 198, 406, SPR_ICON_PLAY },   // play  (JOY)
  { 268, 406, SPR_ICON_LIGHT },  // light (ENE)
  { 338, 382, SPR_ICON_CLEAN },  // bath  (HYG)
};
#define BTN_R 26    // icon face; the gauge ring adds 5 px
#define BTN_HIT 34  // touch radius (neighbours are 70 px apart)

// night-mode stars (the card screen reuses the first set)
static const uint16_t STARS[][2] = { {120,140},{330,120},{370,210},{95,230},{280,90},{160,95} };
static const uint16_t STARS2[][2] = { {150,128},{205,160},{262,132},{318,176},{400,196},{40,206},{118,200},{355,128} };

bool gNight = false;  // real night (by hour) or sleeping: set by render()
uint16_t inkColor() { return gNight ? UI_INK_NIGHT : UI_INK; }

// ---------- background scene: biome of the type + real RTC hour ----------

#define C565(r, g, b) ((uint16_t)((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3)))
#define HORIZON 232  // line where the sky meets the ground

uint16_t panelColor() { return gNight ? C565(0x24, 0x2c, 0x4a) : UI_WHITE; }  // chips, pills
uint16_t edgeColor() { return gNight ? C565(0x3a, 0x44, 0x6a) : UI_TRACK; }

uint16_t lerp565(uint16_t a, uint16_t b, int i, int n) {
  if (n <= 0) return a;
  int ar = (a >> 11) & 31, ag = (a >> 5) & 63, ab = a & 31;
  int br = (b >> 11) & 31, bg = (b >> 5) & 63, bb = b & 31;
  return (uint16_t)((((ar + (br - ar) * i / n) << 11)) |
                    (((ag + (bg - ag) * i / n) << 5)) | (ab + (bb - ab) * i / n));
}

int lum565(uint16_t c) {  // 0..255, to pick dark or light text on a colour
  return ((((c >> 11) << 3) * 30) + ((((c >> 5) & 63) << 2) * 59) + ((c & 31) << 3) * 11) / 100;
}

int sceneHour() {
  return sceneHourFromEpoch(pet.lastSeenEpoch);
}

// ground of each biome by day (at night it blends towards the night blue)
static const uint16_t BIOME_SOIL[6] = {
  C565(0x7e, 0xc0, 0x7f),  // 0 meadow
  C565(0xdc, 0xca, 0x94),  // 1 beach (sand)
  C565(0x4f, 0x8a, 0x55),  // 2 forest
  C565(0x8a, 0x55, 0x44),  // 3 volcano
  C565(0xa8, 0x90, 0x6a),  // 4 mountain
  C565(0xe6, 0xee, 0xf5),  // 5 snow
};

uint16_t gSoil = BIOME_SOIL[0];  // ground colour of the last drawScene, for the pet shadow

// flat-bottomed puff
static void drawCloud(int x, int y, int r, uint16_t col) {
  gfx->fillCircle(x - r, y, r * 3 / 4, col);
  gfx->fillCircle(x, y - r / 3, r, col);
  gfx->fillCircle(x + r, y, r * 3 / 4, col);
  gfx->fillRect(x - r, y - r / 4, 2 * r + 1, r, col);
}

void drawClouds(uint32_t now, uint16_t col) {
  drawCloud((int)((now / 50) % 560) - 40, 134, 16, col);
  drawCloud((int)((now / 90 + 250) % 560) - 40, 176, 11, col);
}

void drawScene(uint8_t biome, uint32_t now, bool night) {
  int h = sceneHour();
  uint16_t top, bot;
  if (night)            { top = C565(0x0c, 0x12, 0x24); bot = C565(0x1e, 0x26, 0x46); }
  else if (h < 8)       { top = C565(0xd1, 0x6a, 0x86); bot = C565(0xf3, 0xb8, 0x7c); }  // dawn
  else if (h < 18)      { top = C565(0x8f, 0xc8, 0xea); bot = C565(0xdc, 0xee, 0xe6); }  // day
  else                  { top = C565(0xc7, 0x5a, 0x4a); bot = C565(0xf0, 0xae, 0x64); }  // dusk

  // sky in bands
  for (int y = 0; y < HORIZON; y += 4)
    gfx->fillRect(0, y, 466, 4, lerp565(top, bot, y, HORIZON));

  // sun or moon, left side: the right side is for the expedition chip and the sleep z's
  if (night) {
    uint16_t moon = C565(0xe8, 0xee, 0xf5), sky = lerp565(top, bot, 150, HORIZON);
    uint16_t halo = lerp565(sky, moon, 4, 16);
    gfx->fillCircle(84, 150, 36, lerp565(sky, moon, 2, 16));
    gfx->fillCircle(84, 150, 29, halo);
    gfx->fillCircle(84, 150, 22, moon);
    gfx->fillCircle(93, 145, 18, halo);  // crescent
    for (int i = 0; i < 14; i++) {
      int sx = i < 6 ? STARS[i][0] : STARS2[i - 6][0], sy = i < 6 ? STARS[i][1] : STARS2[i - 6][1];
      gfx->fillRect(sx, sy, 4, 4, UI_WHITE);
      if (sy >= 116 && (now / 500 + i * 3) % 7 == 0) {  // twinkle (not behind the header pills)
        gfx->fillRect(sx - 2, sy + 1, 8, 2, UI_WHITE);
        gfx->fillRect(sx + 1, sy - 2, 2, 8, UI_WHITE);
      }
    }
  } else {
    int sy = (h < 8 || h >= 18) ? 188 : 140;  // low sun at dawn and dusk
    uint16_t sun = h < 8 ? C565(0xff, 0xd9, 0x8a) : h < 18 ? C565(0xff, 0xe7, 0x9f) : C565(0xff, 0xf1, 0xc8);
    uint16_t sky = lerp565(top, bot, sy, HORIZON);
    gfx->fillCircle(84, sy, 36, lerp565(sky, sun, 3, 16));
    gfx->fillCircle(84, sy, 29, lerp565(sky, sun, 7, 16));
    gfx->fillCircle(84, sy, 22, sun);
    drawClouds(now, lerp565(UI_WHITE, bot, 3, 16));
  }

  uint16_t soil = BIOME_SOIL[biome < 6 ? biome : 0];
  if (night) soil = lerp565(soil, C565(0x16, 0x1c, 0x30), 9, 16);
  gSoil = soil;

  if (biome == 1) {  // beach sea: a strip of water over the sand
    uint16_t sea = night ? C565(0x1c, 0x34, 0x52) : C565(0x4f, 0x96, 0xc4);
    gfx->fillRect(0, HORIZON - 26, 466, 26, sea);
    for (int i = 0; i < 3; i++) {
      int wy = HORIZON - 22 + i * 7;
      uint16_t fc = night ? C565(0x3a, 0x58, 0x78) : C565(0xbf, 0xe6, 0xf5);
      gfx->fillRect(60 + ((now / 60 + i * 30) % 60), wy, 26, 2, fc);
      gfx->fillRect(300 - ((now / 60 + i * 20) % 60), wy, 26, 2, fc);
    }
  } else {  // far hills, hazy with the sky
    uint16_t far = lerp565(soil, bot, 7, 16);
    gfx->fillCircle(96, HORIZON + 16, 52, far);
    gfx->fillCircle(250, HORIZON + 30, 54, far);
    gfx->fillCircle(380, HORIZON + 18, 62, far);
  }

  // ground: light at the horizon, darker towards the viewer
  uint16_t lite = lerp565(soil, night ? C565(0x30, 0x3a, 0x5c) : UI_WHITE, 2, 16);
  uint16_t dkr = lerp565(soil, C565(0x0c, 0x12, 0x24), night ? 5 : 3, 16);
  for (int y = HORIZON; y < 466; y += 6)
    gfx->fillRect(0, y, 466, 6, lerp565(lite, dkr, y - HORIZON, 466 - HORIZON));
  uint16_t hill = lerp565(soil, night ? C565(0x0c, 0x12, 0x24) : UI_WHITE, night ? 3 : 4, 16);
  gfx->fillRoundRect(-60, HORIZON - 12, 586, 56, 28, hill);

  // biome details
  uint16_t dk = lerp565(soil, C565(0x10, 0x18, 0x20), night ? 11 : 7, 16);
  if (biome == 2) {  // forest: conifer silhouettes
    for (int tx : { 60, 150, 360, 416 }) {
      gfx->fillTriangle(tx, HORIZON - 46, tx - 16, HORIZON, tx + 16, HORIZON, dk);
      gfx->fillTriangle(tx, HORIZON - 60, tx - 12, HORIZON - 28, tx + 12, HORIZON - 28, dk);
    }
  } else if (biome == 3) {  // volcano: rocks and embers
    gfx->fillTriangle(70, HORIZON, 40, HORIZON + 30, 100, HORIZON + 30, dk);
    gfx->fillTriangle(400, HORIZON + 4, 372, HORIZON + 30, 430, HORIZON + 30, dk);
    if (!night)
      for (int e = 0; e < 4; e++)
        gfx->fillRect(120 + e * 70, HORIZON + 8 + (e % 2) * 6, 4, 4, C565(0xff, 0x9b, 0x3a));
  } else if (biome == 4) {  // mountain: peaks in the background
    gfx->fillTriangle(140, HORIZON - 50, 60, HORIZON, 220, HORIZON, dk);
    gfx->fillTriangle(330, HORIZON - 38, 250, HORIZON, 410, HORIZON, dk);
  } else if (biome == 5 && !night) {  // snow: falling flakes
    for (int f = 0; f < 10; f++) {
      int fx = (f * 53 + now / 40) % 466;
      int fy = (f * 90 + now / 18) % HORIZON;
      gfx->fillRect(fx, fy, 3, 3, UI_WHITE);
    }
  } else if (biome == 0) {  // meadow: grass tufts
    for (int gx : { 80, 175, 300, 395 })
      for (int b = -1; b <= 1; b++)
        gfx->fillRect(gx + b * 5, HORIZON + 6, 2, 8 + (b == 0 ? 4 : 0), dk);
  }
}

// soft ground shadow under the pet; call before drawing the sprite
void drawPetShadow(int cx, int y, int halfW) {
  int ry = halfW / 5 < 4 ? 4 : halfW / 5;
  gfx->fillEllipse(cx, y, halfW, ry, lerp565(gSoil, 0, 3, 16));
  gfx->fillEllipse(cx, y, halfW * 6 / 10, ry * 6 / 10, lerp565(gSoil, 0, 5, 16));
}
#define UI_HAS_SHADOW 1

// draws an n x n pixel map scaled; silhouette=true paints it in ink
void drawMap(const char *const *map, int n, int x, int y, int s, bool silhouette) {
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < n; c++) {
      char ch = map[r][c];
      if (ch == '.') continue;
      gfx->fillRect(x + c * s, y + r * s, s, s, silhouette ? INK_K : spriteColor(ch));
    }
  }
}

// ---------- header and HUD ----------

void drawBattery() {
  int pc = batPercent();
  if (pc < 0) return;  // no battery connected
  int x = CX - 14, y = 10, w = 24, h = 11;
  bool charging = batCharging();
  uint16_t col = charging ? UI_BAR_OK
                 : (pc >= 40) ? inkColor()
                 : (pc >= 15) ? UI_BAR_WARN
                              : UI_BAR_BAD;
  gfx->drawRoundRect(x, y, w, h, 2, col);
  gfx->fillRect(x + w, y + 3, 3, 5, col);  // terminal
  if (charging) {
    // charging bolt (zigzag) instead of the level bar
    uint16_t bolt = C565(0xff, 0xd9, 0x4a);
    int bx = x + w / 2;
    gfx->fillTriangle(bx + 3, y + 1, bx - 4, y + 6, bx + 1, y + 6, bolt);
    gfx->fillTriangle(bx - 1, y + 5, bx + 4, y + 5, bx - 3, y + 10, bolt);
  } else {
    int fw = (w - 4) * pc / 100;
    if (fw > 0) gfx->fillRect(x + 2, y + 2, fw, h - 4, col);
  }
}

// dot colour of the status pill: what the pet needs most
static uint16_t moodColor() {
  if (pet.sleeping) return C565(0x5b, 0x8c, 0xe0);
  if (pet.isEgg() || pet.ceremony) return UI_BAR_OK;
  uint8_t lo = pet.fullness;
  if (pet.joy < lo) lo = pet.joy;
  if (pet.energy < lo) lo = pet.energy;
  if (pet.hygiene < lo) lo = pet.hygiene;
  if (lo < 25 || pet.poops > 0) return UI_BAR_BAD;
  return lo < 50 ? UI_BAR_WARN : UI_BAR_OK;
}

// species pill with the name, and under it a pill with the status message
void drawHeader(const char *name, uint16_t accent, const char *msg) {
  drawBattery();
  int n = (int)strlen(name), ts = n <= 14 ? 3 : 2;
  int w = n * 6 * ts + 28, x = CX - w / 2;
  gfx->fillRoundRect(x, 54, w, 30, 15, gNight ? lerp565(accent, UI_BG_NIGHT, 10, 16) : accent);
  if (gNight) gfx->drawRoundRect(x, 54, w, 30, 15, accent);
  gfx->setTextColor(gNight ? UI_INK_NIGHT : lum565(accent) >= 150 ? UI_INK : UI_WHITE);
  gfx->setTextSize(ts);
  gfx->setCursor(x + 14, 54 + (30 - 7 * ts) / 2);
  gfx->print(name);

  int m = (int)strlen(msg);
  if (!m) return;
  int ms = m * 12 + 48 > 340 ? 1 : 2;
  int pw = m * 6 * ms + 40, px = CX - pw / 2;
  gfx->fillRoundRect(px, 90, pw, 22, 11, panelColor());
  gfx->drawRoundRect(px, 90, pw, 22, 11, edgeColor());
  gfx->fillCircle(px + 13, 101, 4, moodColor());
  gfx->setTextColor(inkColor());
  gfx->setTextSize(ms);
  gfx->setCursor(px + 26, 90 + (22 - 7 * ms) / 2);
  gfx->print(msg);
}

// The top edge of the round display is not visible at the corners, so the
// two chips (steps, streak) sit side by side inside the circle above the name;
// the expedition hint stays free on the right.
static const int STEP_HUD_X = 128;
static const int STEP_HUD_Y = 28;
static const int STEP_HUD_W = 110;
static const int STEP_HUD_H = 22;
static const int STREAK_X = 244;
static const int STREAK_W = 94;

static void drawChip(int x, int w, uint16_t border) {
  gfx->fillRoundRect(x, STEP_HUD_Y, w, STEP_HUD_H, 11, panelColor());
  gfx->drawRoundRect(x, STEP_HUD_Y, w, STEP_HUD_H, 11, border);
}

// footprint: sole and heel
static void drawFoot(int x, int y, uint16_t col) {
  gfx->fillEllipse(x, y - 2, 3, 5, col);
  gfx->fillCircle(x, y + 5, 2, col);
}

void drawStepsHud() {
  uint16_t color = pet.stepsToday >= 5000UL ? UI_BAR_WARN :
                   pet.stepsToday >= 500UL ? UI_BAR_OK : edgeColor();
  drawChip(STEP_HUD_X, STEP_HUD_W, color);
  int my = STEP_HUD_Y + STEP_HUD_H / 2;
  drawFoot(STEP_HUD_X + 13, my + 2, inkColor());
  drawFoot(STEP_HUD_X + 21, my - 3, inkColor());
  char s[12];
  snprintf(s, sizeof(s), "%lu", (unsigned long)pet.stepsToday);
  gfx->setTextColor(inkColor());
  gfx->setTextSize(2);
  gfx->setCursor(STEP_HUD_X + 32, STEP_HUD_Y + 4);
  gfx->print(s);
}

// flame + streak days; always drawn so the header does not jump, grey at 0
void drawStreakBadge() {
  bool on = pet.streak >= 1;
  drawChip(STREAK_X, STREAK_W, on ? UI_BAR_WARN : edgeColor());
  int fx = STREAK_X + 9, fy = STEP_HUD_Y + 4;
  uint16_t outer = on ? UI_BAR_BAD : edgeColor();
  gfx->fillTriangle(fx + 6, fy, fx + 1, fy + 9, fx + 11, fy + 9, outer);
  gfx->fillCircle(fx + 6, fy + 9, 5, outer);
  if (on) {
    gfx->fillTriangle(fx + 6, fy + 5, fx + 4, fy + 10, fx + 8, fy + 10, UI_BAR_WARN);
    gfx->fillCircle(fx + 6, fy + 11, 3, UI_BAR_WARN);
  }
  char s[8];
  snprintf(s, sizeof(s), "%u", pet.streak);
  gfx->setTextColor(inkColor());
  gfx->setTextSize(2);
  gfx->setCursor(STREAK_X + 28, STEP_HUD_Y + 4);
  gfx->print(s);
}

// ascending z's right of the sleeping pet
void drawSleepZ(uint32_t now) {
  static const int8_t ZX[3] = { 62, 82, 108 }, ZY[3] = { 50, 28, 0 };  // +264, +156
  gfx->setTextColor(UI_INK_NIGHT);
  for (int i = 0; i < 3; i++) {
    gfx->setTextSize(2 + i);
    gfx->setCursor(264 + ZX[i], 156 + ZY[i] + (int)(sinf(now / 400.0f + i * 0.9f) * 4.0f));
    gfx->print("z");
  }
}

// ---------- bottom dock: need rings + buttons ----------

void drawPoops() {
  for (int i = 0; i < pet.poops; i++) {
    drawMap(SPR_POOP, 32, 36 + i * 46, 252, 2, false);
  }
}

// bottom panel base; the egg shows the Pokédex count on it instead of gauges
void drawDock() {
  gfx->fillRoundRect(0, DOCK_Y, 466, 128, 40, edgeColor());
  gfx->fillRoundRect(0, DOCK_Y + 2, 466, 126, 40, gNight ? UI_BG_NIGHT : UI_BG_DAY);
}

void drawEggDock(const char *label) {
  drawDock();
  gfx->setTextColor(inkColor());
  gfx->setTextSize(2);
  gfx->setCursor(CX - (int)strlen(label) * 6, 376);
  gfx->print(label);
}

void drawButtons() {
  uint32_t now = millis();
  int p = now % 1000, tri = p < 500 ? p : 1000 - p;
  uint16_t pulse = lerp565(UI_BAR_BAD, C565(0xff, 0xc0, 0xb0), tri, 500);
  for (int i = 0; i < 4; i++) {
    int cx = buttons[i].cx, cy = buttons[i].cy;
    uint8_t v = i == 0 ? pet.fullness : i == 1 ? pet.joy : i == 2 ? pet.energy : pet.hygiene;
    bool off = pet.sleeping && i != 2;  // while sleeping only LIGHT works
    bool need = !pet.sleeping && (v < 25 || (i == 3 && pet.poops > 0));
    uint16_t col = v >= 50 ? UI_BAR_OK : v >= 25 ? UI_BAR_WARN : UI_BAR_BAD;
    if (need) gfx->fillCircle(cx, cy, BTN_R + 8, pulse);  // pulsing halo
    gfx->fillCircle(cx, cy, BTN_R + 5, edgeColor());
    if (!off && v >= 100) gfx->fillCircle(cx, cy, BTN_R + 5, col);
    else if (!off && v > 0) gfx->fillArc(cx, cy, BTN_R + 5, BTN_R, 270, 270 + 3.6f * v, col);  // from 12 o'clock
    gfx->fillCircle(cx, cy, BTN_R, off ? C565(0x2a, 0x32, 0x52) : UI_WHITE);
    if (!off) drawMap(buttons[i].icon, 16, cx - 16, cy - 16, 2, false);
    if (need) {  // "!" badge
      int bx = cx + 22, by = cy - 22;
      gfx->fillCircle(bx, by, 9, UI_WHITE);
      gfx->fillCircle(bx, by, 7, UI_BAR_BAD);
      gfx->fillRect(bx - 1, by - 4, 2, 5, UI_WHITE);
      gfx->fillRect(bx - 1, by + 2, 2, 2, UI_WHITE);
    }
  }
}

// ---------- feed menu ----------

#define FEED_X 75
#define FEED_Y 258  // pet feet are at y 304: keep its head and body visible
#define FEED_W 314
#define FEED_H 84
#define FEED_STEP 76  // tile pitch: 70 px tile + 6 px gap

// food under a tap (0..3), or -1 outside the card
int feedItemAt(int x, int y) {
  if (x < FEED_X || x >= FEED_X + FEED_W || y < FEED_Y || y >= FEED_Y + FEED_H) return -1;
  int i = (x - (FEED_X + 8)) / FEED_STEP;
  return i < 0 ? 0 : i > 3 ? 3 : i;
}

void drawFeedMenu() {
  static const char *const *const FOOD_ICON[4] = { SPR_ICON_BERRY, SPR_ICON_BANANA, SPR_ICON_FISH, SPR_ICON_MEAT };
  static const StrId FOOD_NAME[4] = { S_FOOD_BERRY, S_FOOD_BANANA, S_FOOD_FISH, S_FOOD_MEAT };
  static const uint16_t TINT[4] = { C565(0xfd, 0xe0, 0xe3), C565(0xff, 0xf3, 0xc2), C565(0xd8, 0xee, 0xfa), C565(0xf6, 0xdc, 0xcf) };
  gfx->fillRoundRect(FEED_X, FEED_Y, FEED_W, FEED_H, 22, UI_WHITE);
  gfx->drawRoundRect(FEED_X, FEED_Y, FEED_W, FEED_H, 22, UI_INK);
  gfx->drawRoundRect(FEED_X + 1, FEED_Y + 1, FEED_W - 2, FEED_H - 2, 21, UI_INK);
  gfx->setTextColor(UI_INK);  // the card is white at night too
  gfx->setTextSize(1);
  for (int i = 0; i < 4; i++) {
    int tx = FEED_X + 8 + i * FEED_STEP, ty = FEED_Y + 8;
    gfx->fillRoundRect(tx, ty, 70, 68, 14, TINT[i]);
    drawMap(FOOD_ICON[i], 16, tx + 11, ty + 4, 3, false);
    const char *n = T(FOOD_NAME[i]);
    gfx->setCursor(tx + (70 - (int)strlen(n) * 6) / 2, ty + 56);
    gfx->print(n);
  }
}
