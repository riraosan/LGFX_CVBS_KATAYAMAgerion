/*
LGFX_CVBS_KATAYAMAgerion

Original Source:
https://github.com/moononournation/RGB565_video/tree/master/SPIFFS_MJPEG_JPEGDEC_MP3_audio

Licence:
MIT

Author:
[moononournation](https://github.com/moononournation)
Modified for LGFX CVBS Library: @riraosan_0901 2023-03-18

Contributors:
moononournation
Ryutarou KATAYAMA
Akira OWADA

*/

// #define KATAYAMA
// #define NON4
// #define NON5
// #define KANDENCH

#if defined(KATAYAMA)
#define FILENAME       "/wav/katayama.wav"
#define MJPEG_FILENAME "/jpg/katayama.mjpeg"
#elif defined(NON4)
#define FILENAME       "/wav/non4.wav"
#define MJPEG_FILENAME "/jpg/non4.mjpeg"
#elif defined(NON5)
#define FILENAME       "/wav/non5.wav"
#define MJPEG_FILENAME "/jpg/non5.mjpeg"
#elif defined(KANDENCH)
#define FILENAME       "/wav/kandenchflash.wav"
#define MJPEG_FILENAME "/jpg/kandenchflash.mjpeg"
#endif

#define FPS               24
#define MJPEG_BUFFER_SIZE (320 * 240 * 2 / 10)

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <M5GFX.h>
#include <LGFX_8BIT_CVBS.h>
#include <WiFi.h>
#include <Button2.h>

#define CHART_MARGIN   23
#define LEGEND_A_COLOR 0xE0C3
#define LEGEND_B_COLOR 0x33F7
#define LEGEND_C_COLOR 0x4D69
#define LEGEND_D_COLOR 0x9A74
#define LEGEND_E_COLOR 0xFBE0
#define LEGEND_F_COLOR 0xFFE6
#define LEGEND_G_COLOR 0xA2A5

static LGFX_8BIT_CVBS display;

#define TFCARD_CS_PIN 4  // dummy
#define LGFX          LGFX_8BIT_CVBS

#define LGFX_ONLY
#define USE_DISPLAY

#if defined(KATAYAMA)
#define SDU_APP_NAME "KATAYAMAgerion"
#define SDU_APP_PATH "/07_KATAYAMAgerion.bin"
#elif defined(NON4)
#define SDU_APP_NAME "NON-Chan_ep4"
#define SDU_APP_PATH "/04_NON-Chan_ep4.bin"
#elif defined(NON5)
#define SDU_APP_NAME "NON-Chan_ep5"
#define SDU_APP_PATH "/05_NON-Chan_ep5.bin"
#elif defined(KANDENCH)
#define SDU_APP_NAME "KANDENCH flash"
#define SDU_APP_PATH "/08_KANDENCH_flash.bin"
#endif

#include <M5StackUpdater.h>

/* MP3 Audio */
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>
static AudioGeneratorMP3 *mp3;
static AudioGeneratorWAV *wav;
static AudioFileSourceSD *aFile;
static AudioOutputI2S    *out;

/* MJPEG Video */
#include "MjpegClass.h"
static MjpegClass mjpeg;

#define BUTTON_GPIO_NUM 16
Button2 button;

/* variables */
static std::int_fast32_t next_frame          = 0;
static std::int_fast32_t skipped_frames      = 0;
static unsigned long     total_play_audio_ms = 0;
static unsigned long     total_read_video_ms = 0;
static unsigned long     total_show_video_ms = 0;
static unsigned long     start_ms, curr_ms, next_frame_ms;

bool bA = false;
bool bB = false;
bool bC = false;

void handler(Button2 &btn) {
  switch (btn.getType()) {
    case clickType::single_click:
      Serial.print("single ");
      bB = true;
      break;
    case clickType::double_click:
      Serial.print("double ");
      bC = true;
      break;
    case clickType::triple_click:
      Serial.print("triple ");
      break;
    case clickType::long_click:
      Serial.print("long ");
      bA = true;
      break;
    case clickType::empty:
      break;
    default:
      break;
  }

  Serial.print("click");
  Serial.print(" (");
  Serial.print(btn.getNumberOfClicks());
  Serial.println(")");
}

bool buttonAPressed(void) {
  bool temp = bA;
  bA        = false;

  return temp;
}

bool buttonBPressed(void) {
  bool temp = bB;
  bB        = false;

  return temp;
}

bool buttonCPressed(void) {
  bool temp = bC;
  bC        = false;

  return temp;
}

void ButtonUpdate() {
  button.loop();
}

void setupButton(void) {
  button.setClickHandler(handler);
  button.setDoubleClickHandler(handler);
  button.setTripleClickHandler(handler);
  button.setLongClickHandler(handler);
  button.begin(BUTTON_GPIO_NUM);

  SDUCfg.setSDUBtnA(&buttonAPressed);
  SDUCfg.setSDUBtnB(&buttonBPressed);
  SDUCfg.setSDUBtnC(&buttonCPressed);
  SDUCfg.setSDUBtnPoller(&ButtonUpdate);
}

// pixel drawing callback
static int drawMCU(JPEGDRAW *pDraw) {
  // Serial.printf("Draw pos = (%d, %d), size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  unsigned long s = millis();

  display.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels, true);  // true!!

  total_show_video_ms += millis() - s;
  return 1;
} /* drawMCU() */

void setup() {
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);

  // Init Video
  display.init();
  display.fillScreen(TFT_BLACK);

  setupButton();

  setSDUGfx(&display);
  checkSDUpdater(
      SD,            // filesystem (default=SD)
      MENU_BIN,      // path to binary (default=/menu.bin, empty string=rollback only)
      2000,          // wait delay, (default=0, will be forced to 2000 upon ESP.restart() )
      TFCARD_CS_PIN  // (usually default=4 but your mileage may vary)
  );

  aFile = new AudioFileSourceSD(FILENAME);
  out   = new AudioOutputI2S(I2S_NUM_1, 0, 64);  // Output to exDAC

  // from platfromio.ini
  out->SetPinout(_BCLK, _LRCLK, _DATA);
  out->SetGain(0.3);

  wav = new AudioGeneratorWAV();

  File vFile = SD.open(MJPEG_FILENAME);
  if (!vFile || vFile.isDirectory()) {
    Serial.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
    display.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
  } else {
    Serial.println(F("PCM audio MJPEG video start"));

    // init Video
    mjpeg.setup(&vFile,
                MJPEG_BUFFER_SIZE,
                drawMCU,
                true,   // enableDecodeMultiTask
                true,   // enableDrawMultiTask
                true);  // useBigEndian

    wav->begin(aFile, out);

    start_ms      = lgfx::v1::millis();
    curr_ms       = start_ms;
    next_frame_ms = start_ms + (++next_frame * 1000 / FPS);
    while (vFile.available()) {
      // Read video
      mjpeg.readMjpegBuf();

      unsigned long read = lgfx::v1::millis();
      total_read_video_ms += read - curr_ms;

      if (read < next_frame_ms) {  // check show frame or skip frame
        // Play video
        mjpeg.drawJpg();
      } else {
        ++skipped_frames;
        // Serial.println(F("Skip frame"));
      }

      curr_ms = lgfx::v1::millis();

      // Play audio
      if ((wav->isRunning()) && (!wav->loop())) {
        // wav->stop();
      }

      total_play_audio_ms += lgfx::v1::millis() - curr_ms;

      while (lgfx::v1::millis() < next_frame_ms) {
        vTaskDelay(1);
      }

      curr_ms       = lgfx::v1::millis();
      next_frame_ms = start_ms + (++next_frame * 1000 / FPS);
    }
    int time_used    = lgfx::v1::millis() - start_ms;
    int total_frames = next_frame - 1;
    Serial.println(F("PCM audio MJPEG video end"));
    vFile.close();
    int   played_frames = total_frames - skipped_frames;
    float fps           = 1000.0 * played_frames / time_used;
    Serial.printf("Played frames: %d\n", played_frames);
    Serial.printf("Skipped frames: %d (%0.1f %%)\n", skipped_frames, 100.0 * skipped_frames / total_frames);
    Serial.printf("Time used: %d ms\n", time_used);
    Serial.printf("Expected FPS: %d\n", FPS);
    Serial.printf("Actual FPS: %0.1f\n", fps);
    Serial.printf("Play MP3: %lu ms (%0.1f %%)\n", total_play_audio_ms, 100.0 * total_play_audio_ms / time_used);
    Serial.printf("SDMMC read MJPEG: %lu ms (%0.1f %%)\n", total_read_video_ms, 100.0 * total_read_video_ms / time_used);
    Serial.printf("Decode video: %lu ms (%0.1f %%)\n", total_decode_video_ms, 100.0 * total_decode_video_ms / time_used);
    Serial.printf("Show video: %lu ms (%0.1f %%)\n", total_show_video_ms, 100.0 * total_show_video_ms / time_used);

    // wait last frame finished
    delay(200);

    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.printf("Played frames: %d\n", played_frames);
    display.printf("Skipped frames: %d (%0.1f %%)\n", skipped_frames, 100.0 * skipped_frames / total_frames);
    display.printf("Actual FPS: %0.1f\n\n", fps);
    int16_t r1 = ((display.height() - CHART_MARGIN - CHART_MARGIN) / 2);
    int16_t r2 = r1 / 2;
    int16_t cx = display.width() - display.height() + CHART_MARGIN + CHART_MARGIN - 1 + r1;
    int16_t cy = r1 + CHART_MARGIN;

    float arc_start1 = 0;
    float arc_end1   = arc_start1 + max(2.0, 360.0 * total_play_audio_ms / time_used);
    for (int i = arc_start1 + 1; i < arc_end1; i += 2) {
      display.fillArc(cx, cy, r1, r2, arc_start1 - 90.0, i - 90.0, LEGEND_A_COLOR);
    }
    display.fillArc(cx, cy, r1, r2, arc_start1 - 90.0, arc_end1 - 90.0, LEGEND_A_COLOR);
    display.setTextColor(LEGEND_A_COLOR);
    display.printf("Play WAV:\n%0.1f %%\n", 100.0 * total_play_audio_ms / time_used);

    float arc_start2 = arc_end1;
    float arc_end2   = arc_start2 + max(2.0, 360.0 * total_read_video_ms / time_used);
    for (int i = arc_start2 + 1; i < arc_end2; i += 2) {
      display.fillArc(cx, cy, r1, r2, arc_start2 - 90.0, i - 90.0, LEGEND_B_COLOR);
    }
    display.fillArc(cx, cy, r1, r2, arc_start2 - 90.0, arc_end2 - 90.0, LEGEND_B_COLOR);
    display.setTextColor(LEGEND_B_COLOR);
    display.printf("Read MJPEG:\n%0.1f %%\n", 100.0 * total_read_video_ms / time_used);

    float arc_start3 = arc_end2;
    float arc_end3   = arc_start3 + max(2.0, 360.0 * total_decode_video_ms / time_used);
    for (int i = arc_start3 + 1; i < arc_end3; i += 2) {
      display.fillArc(cx, cy, r2, 0, arc_start3 - 90.0, i - 90.0, LEGEND_C_COLOR);
    }
    display.fillArc(cx, cy, r2, 0, arc_start3 - 90.0, arc_end3 - 90.0, LEGEND_C_COLOR);
    display.setTextColor(LEGEND_C_COLOR);
    display.printf("Decode video:\n%0.1f %%\n", 100.0 * total_decode_video_ms / time_used);

    float arc_start4 = arc_end2;
    float arc_end4   = arc_start4 + max(2.0, 360.0 * total_show_video_ms / time_used);
    for (int i = arc_start4 + 1; i < arc_end4; i += 2) {
      display.fillArc(cx, cy, r1, r2, arc_start4 - 90.0, i - 90.0, LEGEND_D_COLOR);
    }
    display.fillArc(cx, cy, r1, r2, arc_start4 - 90.0, arc_end4 - 90.0, LEGEND_D_COLOR);
    display.setTextColor(LEGEND_D_COLOR);
    display.printf("Play video:\n%0.1f %%\n", 100.0 * total_show_video_ms / time_used);
  }

  delay(5000);
  display.fillScreen(TFT_BLACK);

  updateFromFS(SD);
  ESP.restart();
}

void loop() {
}
