/*
LGFX_CVBS_KATAYAMAgerion

Original Source:
https://github.com/moononournation/RGB565_video/tree/master/SPIFFS_MJPEG_JPEGDEC_MP3_audio

Licence:
MIT

Author:
[moononournation](https://github.com/moononournation)

Modified for LGFX CVBS Library: @riraosan_0901 2023-02-26
*/

#define MP3_FILENAME      "/mp3/katayama2.mp3"
#define FPS               15
#define MJPEG_FILENAME    "/jpg/katayama.mjpeg"
#define MJPEG_BUFFER_SIZE (320 * 240 * 2 / 14)

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <M5GFX.h>
#include <LGFX_8BIT_CVBS.h>
#include <WiFi.h>

static LGFX_8BIT_CVBS display;

/* MP3 Audio */
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
static AudioGeneratorMP3 *mp3;
static AudioFileSourceSD *aFile;
static AudioOutputI2S    *out;

/* MJPEG Video */
#include "MjpegClass.h"
static MjpegClass mjpeg;

/* variables */
static int           next_frame          = 0;
static int           skipped_frames      = 0;
static unsigned long total_play_audio_ms = 0;
static unsigned long total_read_video_ms = 0;
static unsigned long total_show_video_ms = 0;
static unsigned long start_ms, curr_ms, next_frame_ms;

// pixel drawing callback
static int drawMCU(JPEGDRAW *pDraw) {
  // Serial.printf("Draw pos = (%d, %d), size = %d x %d\n", pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  unsigned long s = millis();

  display.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels, true);

  total_show_video_ms += millis() - s;
  return 1;
} /* drawMCU() */

void setup() {
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);

  // Init Video
  display.init();
  display.fillScreen(TFT_BLACK);

  // Init FS
  SPI.begin(_CLK, _MISO, _MOSI, 4);
  SPI.setDataMode(SPI_MODE3);
  if (!SD.begin(4, SPI, 80000000)) {
    Serial.println(F("ERROR: File system mount failed!"));
    display.println(F("ERROR: File system mount failed!"));
  } else {
    aFile = new AudioFileSourceSD(MP3_FILENAME);
    out   = new AudioOutputI2S(I2S_NUM_1, 0, 64);  // Output to exDAC

    // from platfromio.ini
    out->SetPinout(_BCLK, _LRCLK, _DATA);
    out->SetGain(0.2);
    mp3 = new AudioGeneratorMP3();

    File vFile = SD.open(MJPEG_FILENAME);
    if (!vFile || vFile.isDirectory()) {
      Serial.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
      display.println(F("ERROR: Failed to open " MJPEG_FILENAME " file for reading"));
    } else {
      Serial.println(F("PCM audio MJPEG video start"));

      // init Video
      mjpeg.setup(&vFile, MJPEG_BUFFER_SIZE, drawMCU,
                  true /* enableDecodeMultiTask */,
                  true /* enableDrawMultiTask */,
                  true /* useBigEndian */);

      // init audio
      mp3->begin(aFile, out);

      start_ms      = millis();
      curr_ms       = start_ms;
      next_frame_ms = start_ms + (++next_frame * 1000 / FPS);
      while (vFile.available()) {
        // Read video
        mjpeg.readMjpegBuf();
        total_read_video_ms += millis() - curr_ms;

        if (millis() < next_frame_ms)  // check show frame or skip frame
        {
          // Play video
          mjpeg.drawJpg();
        } else {
          ++skipped_frames;
          Serial.println(F("Skip frame"));
        }
        curr_ms = millis();

        // Play audio
        if ((mp3->isRunning()) && (!mp3->loop())) {
          // mp3->stop();
        }
        total_play_audio_ms += millis() - curr_ms;

        while (millis() < next_frame_ms) {
          vTaskDelay(1);
        }

        curr_ms       = millis();
        next_frame_ms = start_ms + (++next_frame * 1000 / FPS);
      }
      int time_used    = millis() - start_ms;
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
      /*
      #define CHART_MARGIN   23
      #define LEGEND_A_COLOR 0xE0C3
      #define LEGEND_B_COLOR 0x33F7
      #define LEGEND_C_COLOR 0x4D69
      #define LEGEND_D_COLOR 0x9A74
      #define LEGEND_E_COLOR 0xFBE0
      #define LEGEND_F_COLOR 0xFFE6
      #define LEGEND_G_COLOR 0xA2A5
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
            display.printf("Play MP3:\n%0.1f %%\n", 100.0 * total_play_audio_ms / time_used);

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
      */
    }
  }
  display.fillScreen(TFT_BLACK);
  // esp_deep_sleep_start();
}

void loop() {
}
