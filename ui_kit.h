// Shared look for every screen except the home scene: theme colours, title
// pill, panels, buttons, bars, page dots and a few vector icons. Header-only,
// include after ui_home.h. All colours follow gNight so cards, menus and games
// are dark in the evening like the home screen.
//
// Round-screen safety: the visible half-width at row y is sqrt(233^2-(y-233)^2),
// so content stays inside x 56..410 for y 96..350 and narrows towards the top
// and bottom. Text is size 2 (12x14 px) or larger; size 1 is not readable.
#pragma once
#include "ui_home.h"

// ---------- theme ----------

static uint16_t kBg() { return gNight ? UI_BG_NIGHT : UI_BG_DAY; }
static uint16_t kInk() { return inkColor(); }
static uint16_t kPanel() { return panelColor(); }
static uint16_t kSub() { return gNight ? C565(0x9a, 0xa6, 0xc8) : C565(0x6b, 0x72, 0x82); }  // secondary text
static uint16_t kEdge() { return gNight ? C565(0x3a, 0x44, 0x6a) : C565(0xc8, 0xc6, 0xd2); }
static uint16_t kShadow() { return gNight ? C565(0x08, 0x0c, 0x1a) : C565(0xe0, 0xd8, 0xc8); }
static uint16_t kOn(uint16_t c) { return lum565(c) >= 150 ? UI_INK : UI_WHITE; }  // text on a colour
static uint16_t kDim(uint16_t c) { return lerp565(c, kBg(), 2, 3); }               // washed-out accent
static uint16_t kGhost() { return gNight ? C565(0x2a, 0x32, 0x52) : C565(0xe4, 0xe2, 0xdc); }  // locked/empty fill

// accents per screen family
#define K_GOLD C565(0xf2, 0xb7, 0x2b)
#define K_BLUE C565(0x4c, 0x8c, 0xe0)
#define K_TEAL C565(0x2f, 0x9d, 0x8a)
#define K_PINK C565(0xd4, 0x52, 0x7e)
#define K_VIOLET C565(0x8a, 0x6a, 0xd0)

// ---------- text ----------

static int kTextW(const char *s, int size) { return (int)strlen(s) * 6 * size - size; }

static void kText(const char *s, int x, int y, int size, uint16_t col) {
  gfx->setTextColor(col);
  gfx->setTextSize(size);
  gfx->setCursor(x, y);
  gfx->print(s);
}

// centred on cx; drops to size 1 only when size 2 would still overflow maxW
static void kTextC(const char *s, int cx, int y, int size, uint16_t col, int maxW = 0) {
  if (maxW > 0) {
    while (size > 2 && kTextW(s, size) > maxW) size--;
    if (kTextW(s, size) > maxW) size = 1;
  }
  kText(s, cx - kTextW(s, size) / 2, y + (size == 1 ? 3 : 0), size, col);
}

static void kTextR(const char *s, int right, int y, int size, uint16_t col) {
  kText(s, right - kTextW(s, size), y, size, col);
}

// ---------- shapes ----------

static void kPanelRect(int x, int y, int w, int h, int r = 14, uint16_t fill = 0) {
  gfx->fillRoundRect(x, y + 3, w, h, r, kShadow());
  gfx->fillRoundRect(x, y, w, h, r, fill ? fill : kPanel());
  gfx->drawRoundRect(x, y, w, h, r, kEdge());
}

// pill-shaped button with a lower edge so it reads as pressable
static void kBtn(int x, int y, int w, int h, uint16_t col, const char *label, int size = 3) {
  int r = h / 2 > 18 ? 18 : h / 2;
  gfx->fillRoundRect(x, y + 4, w, h, r, lerp565(col, 0, 1, 4));
  gfx->fillRoundRect(x, y, w, h, r, col);
  kTextC(label, x + w / 2, y + (h - 7 * size) / 2, size, kOn(col), w - 16);
}

// full-width solid pill, no label (progress bars, tracks)
static void kBar(int x, int y, int w, int h, int val, int maxv, uint16_t col) {
  gfx->fillRoundRect(x, y, w, h, h / 2, kEdge());
  int fw = maxv > 0 ? (int)((long)val * w / maxv) : 0;
  if (fw > w) fw = w;
  if (fw < h && val > 0) fw = h;  // keep a visible round blob for tiny values
  if (fw > 0) gfx->fillRoundRect(x, y, fw, h, h / 2, col);
}

// accent pill at the top of a screen: the screen's name
// returns the pill width so callers can hang an icon next to it
static int kTitle(const char *s, uint16_t accent, int y = 36) {
  int n = (int)strlen(s), ts = n <= 10 ? 3 : 2;
  int w = kTextW(s, ts) + 36, x = CX - w / 2, h = 30;
  uint16_t fill = gNight ? lerp565(accent, UI_BG_NIGHT, 10, 16) : accent;
  gfx->fillRoundRect(x, y, w, h, 15, fill);
  if (gNight) gfx->drawRoundRect(x, y, w, h, 15, accent);
  kText(s, x + 18, y + (h - 7 * ts) / 2, ts, gNight ? UI_INK_NIGHT : kOn(accent));
  return w;
}

// page dots: the current page is a long accent pill
static void kDots(int page, int count, uint16_t accent, int y = 398) {
  int step = 22, x0 = CX - ((count - 1) * step) / 2;
  uint16_t off = gNight ? C565(0x4a, 0x56, 0x7e) : C565(0xc4, 0xc2, 0xcc);
  for (int i = 0; i < count; i++) {
    if (i == page) gfx->fillRoundRect(x0 + i * step - 9, y - 4, 18, 8, 4, accent);
    else gfx->fillCircle(x0 + i * step, y, 3, off);
  }
}

// ---------- icons ----------

static void kThickLine(int x0, int y0, int x1, int y1, bool vertical, uint16_t col) {
  for (int d = -1; d <= 1; d++) {
    if (vertical) gfx->drawLine(x0, y0 + d, x1, y1 + d, col);
    else gfx->drawLine(x0 + d, y0, x1 + d, y1, col);
  }
}

// dir: 0 left, 1 right, 2 down, 3 up
static void kChev(int cx, int cy, int dir, int s, uint16_t col) {
  if (dir < 2) {
    int a = dir == 0 ? s / 2 : -s / 2;
    kThickLine(cx + a, cy - s, cx - a, cy, false, col);
    kThickLine(cx - a, cy, cx + a, cy + s, false, col);
  } else {
    int a = dir == 2 ? s / 2 : -s / 2;
    kThickLine(cx - s, cy - a, cx, cy + a, true, col);
    kThickLine(cx, cy + a, cx + s, cy - a, true, col);
  }
}

static void kCheck(int cx, int cy, int s, uint16_t col) {
  kThickLine(cx - s, cy, cx - s / 3, cy + s * 2 / 3, true, col);
  kThickLine(cx - s / 3, cy + s * 2 / 3, cx + s, cy - s * 2 / 3, true, col);
}

// circle with a tick: goal done / medal got
static void kTick(int cx, int cy, int r, uint16_t fill) {
  gfx->fillCircle(cx, cy, r, fill);
  kCheck(cx, cy, r / 2, kOn(fill));
}

static void kHeart(int cx, int cy, int r, uint16_t col) {  // r = half width
  gfx->fillCircle(cx - r / 2, cy - r / 4, r / 2 + 1, col);
  gfx->fillCircle(cx + r / 2, cy - r / 4, r / 2 + 1, col);
  gfx->fillTriangle(cx - r, cy - r / 8, cx + r, cy - r / 8, cx, cy + r * 3 / 4 + 1, col);
}

static void kFlame(int x, int y, int s, uint16_t outer, uint16_t inner) {  // x,y = top of an s*1.4 tall flame
  gfx->fillTriangle(x + s / 2, y, x, y + s, x + s, y + s, outer);
  gfx->fillCircle(x + s / 2, y + s, s / 2, outer);
  gfx->fillTriangle(x + s / 2, y + s / 2, x + s / 4, y + s, x + s * 3 / 4, y + s, inner);
  gfx->fillCircle(x + s / 2, y + s + 1, s / 3, inner);
}

static void kStar(int cx, int cy, int r, uint16_t col) {
  int px[10], py[10];
  for (int i = 0; i < 10; i++) {
    float a = -1.5708f + i * 0.62832f, rr = (i & 1) ? r * 0.45f : (float)r;
    px[i] = cx + (int)(cosf(a) * rr);
    py[i] = cy + (int)(sinf(a) * rr);
  }
  for (int i = 0; i < 10; i++) gfx->fillTriangle(cx, cy, px[i], py[i], px[(i + 1) % 10], py[(i + 1) % 10], col);
}

static void kBolt(int cx, int cy, int s, uint16_t col) {
  gfx->fillTriangle(cx + s / 2, cy - s, cx - s / 2, cy + s / 8, cx + s / 8, cy + s / 8, col);
  gfx->fillTriangle(cx - s / 8, cy - s / 8, cx + s / 2, cy - s / 8, cx - s / 2, cy + s, col);
}

static void kFootprint(int x, int y, int s, uint16_t col) {  // sole + heel, s ~ sole height/2
  gfx->fillEllipse(x, y, s * 3 / 5, s, col);
  gfx->fillCircle(x, y + s + s / 2 + 1, s / 3 + 1, col);
}

static void kPencil(int cx, int cy, uint16_t col) {  // ~14px, tip bottom-left
  gfx->fillTriangle(cx - 7, cy + 7, cx - 7, cy + 2, cx - 2, cy + 7, col);
  kThickLine(cx - 4, cy + 2, cx + 6, cy - 8, false, col);
  kThickLine(cx - 1, cy + 5, cx + 8, cy - 5, false, col);
}

// round icon button (frame arrows, back): circle + chevron
static void kRoundChev(int cx, int cy, int r, int dir, uint16_t col) {
  gfx->fillCircle(cx, cy + 2, r, kShadow());
  gfx->fillCircle(cx, cy, r, kPanel());
  gfx->drawCircle(cx, cy, r, kEdge());
  kChev(cx, cy, dir, r / 2, col);
}

// icon-only pill button: chevron on a colour, or a ghost pill when !on
static void kBtnChev(int x, int y, int w, int h, uint16_t col, int dir, bool on) {
  if (on) gfx->fillRoundRect(x, y + 3, w, h, h / 2, lerp565(col, 0, 1, 4));
  gfx->fillRoundRect(x, y, w, h, h / 2, on ? col : kGhost());
  kChev(x + w / 2, y + h / 2, dir, 8, on ? kOn(col) : kSub());
}

// centred, breaks at spaces and after hyphens into up to 4 lines; returns the line count
static int kTextWrapC(const char *s, int cx, int y, int size, uint16_t col, int maxW, int lineH = 0) {
  if (!lineH) lineH = 9 * size;
  int per = (maxW + size) / (6 * size);  // chars per line
  char line[48];
  int lines = 0, n = (int)strlen(s);
  while (n > 0 && lines < 4) {
    int take = n <= per ? n : per;
    if (n > per) {
      int t = take;
      while (t > 0 && s[t] != ' ' && s[t - 1] != '-') t--;
      take = t ? t : per;
    }
    int len = take > 47 ? 47 : take;
    memcpy(line, s, len);
    line[len] = 0;
    kTextC(line, cx, y + lines * lineH, size, col);
    s += take;
    n -= take;
    while (*s == ' ') { s++; n--; }
    lines++;
  }
  return lines;
}

// small label pill centred on cx (marks, tags)
static void kChip(int cx, int y, const char *s, uint16_t col, int size = 2) {
  int w = kTextW(s, size) + 24, h = 7 * size + 10;
  gfx->fillRoundRect(cx - w / 2, y, w, h, h / 2, col);
  kText(s, cx - w / 2 + 12, y + 5, size, kOn(col));
}

// bottom chrome shared by all cards: page dots and a "pull down" affordance.
// Tapping y >= 400 or swiping up closes the card.
static void kCardFoot(int page, int count, uint16_t accent) {
  kDots(page, count, accent, 398);
  kChev(CX, 424, 2, 9, kSub());
}
