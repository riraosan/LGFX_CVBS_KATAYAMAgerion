# LGFX_CVBS_KATAYAMAgerion

## はじめに

このリポジトリは、[moononournation](https://github.com/moononournation)さんが作成した[サンプル](https://github.com/moononournation/RGB565_video/tree/master/SPIFFS_MJPEG_JPEGDEC_MP3_audio)を用いて片山ゲリオン動画をコンポジットビデオで再生するためのプロジェクトです。

こちらがサンプル動画です。
https://twitter.com/riraosan_0901/status/1634766533313568768?s=20

## ハード

次のハードを使用しました。

- M5Stack ATOM Lite
- M5Stack SPK module（16bit DAC and TF-CARD)
- M5Stack RCA unit
- デジタルテレビ（コンポジット入力あり）
- MicroSDカード（mjpeg, mp3ファイルを格納してください）

![image](/doc/IMG_3535.jpg)

## ソフト

主に、次のライブラリを使用させていただきました。ライブラリ作者のみなさんに感謝します。

- https://github.com/m5stack/M5GFX.git
- https://github.com/bitbank2/JPEGDEC.git
- https://github.com/earlephilhower/ESP8266Audio.git

## ビルド

VSCodeを利用してください。VSCodeでPlatformIO拡張機能をインストールしてこのプロジェクトをフォルダ毎オープンしてください。
そうすると、ライブラリが自動的にプロジェクトへインストールされます。
PlatformIOの「Upload and Monitor」メニューを選択してビルドとファームウェアのアップロードを行ってください。

## ライセンス

MIT

## 作者

@riraosan_0901 on Twitter

## 貢献者

- KATAYAMA Ryutarou
- OWADA Akira
- [moononournation](https://github.com/moononournation)
- [らびやん](https://github.com/lovyan03)

## おわりに

Enjoy!
