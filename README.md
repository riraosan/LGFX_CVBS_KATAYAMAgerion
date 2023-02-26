# LGFX_CVBS_KATAYAMAgerion

## はじめに

このリポジトリは、[moononournation](https://github.com/moononournation)さんが作成した[サンプル](https://github.com/moononournation/RGB565_video/tree/master/SPIFFS_MJPEG_JPEGDEC_MP3_audio)を用いて片山ゲリオン動画をコンポジットビデオで再生するためのプロジェクトです。

<blockquote class="twitter-tweet"><p lang="ja" dir="ltr">片山ゲリオンの解像度アップに成功しました！<br>アニメーションGIFからMotionJPEGにプロジェクトを変えました。<br>しかし、途中、負荷が上がりすぎてCPUが再起動してしまうので、15FPSに落としました。mp3は96kbpsとしました。<br>しかし、lovyanGFXはすげーな。。<br>あ、デバイスはATOM Liteを使っています。 <a href="https://t.co/0pwl0q8T8b">pic.twitter.com/0pwl0q8T8b</a></p>&mdash; riraosan (@riraosan_0901) <a href="https://twitter.com/riraosan_0901/status/1629469072613335043?ref_src=twsrc%5Etfw">February 25, 2023</a></blockquote> <script async src="https://platform.twitter.com/widgets.js" charset="utf-8"></script>

## ハード

次のハードを使用しました。

- M5Stack ATOM Lite
- SPK module（16bit DAC)
- RCA unit
- デジタルテレビ（コンポジット入力あり）

![image](/doc/IMG_3535.jpg)

## ソフト

次のライブラリを使用させていただきました。ライブラリ作者のみなさんに感謝します。

https://github.com/m5stack/M5GFX.git
https://github.com/bitbank2/JPEGDEC.git
https://github.com/earlephilhower/ESP8266Audio.git

## ライセンス

MIT

##　作者

@riraosan_0901 on Twitter

## 貢献者

- KATAYAMA Ryutarou
- OWADA Akira
- [moononournation](https://github.com/moononournation)

##　おわりに

Enjoy!
