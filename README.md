# MetaHuman SDK Pixel Streaming Sample

This repo contains Metahuman chat bot sample project for Unreal Engine®. Photoreallistic avatar has Metahuman SDK powered facial expressions, speech and lip sync. Pixel Streaming works on tweaked version of Epic's Pixel Streaming and was developed specifically for creating chat bots. You can read about differences [here](./pixstreamjs.md).

**Why do I need this?**
This is good starting point for creating your own web based chat bots.
What's included:

* customizable web UI with npm and typescript compatibility
* tweakable **Browser ⇔ Unreal Engine** communication protocol
* ready for AI generated speech or audio files
* multilingual lip sync (powered by [MetaHuman SDK](https://www.unrealengine.com/marketplace/en-US/item/66b869fa0d3748e78d422e59716597b6)) with Microsoft Azure or Google voices
* scallable architecture

## What's Metahuman SDK?

MetaHumanSDK is a set of tools for creation of an immersive interaction with a digital human. Our service creates facial animation from an input audio file or text and the plugin includes connectivity modules of a synthesized voice from Google or Azure (text to speech), the creation of an interactive chat - connection to Dialogflow (Google) with the possibility of a live dialogue with a digital human

[Metahuman SDK on UE marketplace](https://www.unrealengine.com/marketplace/en-US/item/66b869fa0d3748e78d422e59716597b6)

## Demo

Here will be youtube video soon!

### Naming

**Streamer** – Unreal Engine® based application that we want to stream.
**Companion** – (aka Companion) server that arrange connections to *streamer*.
**Ballancer** – (aka Matchmaker) server that equally distribute load between pairs: *streamer* and *companion*.
**Website/UI** – frontend example of customizable chat bot written in pure typescript.

### Unreal Engine compatibility

|5.1|5.0|4.*|
|-------|-----|-----|
|ready  |in progress|N/A|

### Platforms

|Windows|macOS|Linux|
|-------|-----|-----|
|ready  |in progress|N/A|

### Prerequisites

* NodeJs 18.13+ ([official website](https://nodejs.org/en/download))

### Getting Started

1. open Unreal Engine project [PixelStreamingDemo.uproject](./unreal-streamer/PixelStreamingDemo.uproject) in unreal-streamer folder
2. run standalone game with the "Additional Launch Parameters" : PixelStreamingURL=ws://localhost:8888 -log (more details [here](./unreal-streamer/README.md#Launch))
3. default MetaHuman character is chatting out of the box, but you can in one step append chat functionality to your custom MetaHuman (more details [here](./unreal-streamer/README.md#MetaHuman-Chat-Setup))
4. run **start.bat** – it will automatically install all Node.js dependencies and start both companion and ballancer servers as well as sample website
5. open [http://localhost:9000](http://localhost:9000) in your browser

### Configuration

#### Default

By default UI uses 9000 port, streamer uses 8888 port, ballancer uses 5000 port for HTTP API and 5001 port for communication with companion and companion uses 5002 port.

#### Using code for configuration

You can change default configuration by editing both:
[DefaultBallancerConfig.ts](./ballancer-server/src/DefaultBallancerConfig.ts)
[DefaultCompanionServerConfig.ts](./companion-server/src/DefaultCompanionServerConfig.ts)

#### Using json configuration

Use **--config** terminal parameter to start server with custom configuration. Structure of the json file should be same as [DefaultBallancerConfig.ts](./ballancer-server/src/DefaultBallancerConfig.ts) and [DefaultCompanionServerConfig.ts](./companion-server/src/DefaultCompanionServerConfig.ts) respectively.

`node ./ballancer-server-bundled.js --config my-ballancer-config.json`

and/or

`node ./companion-server-bundled.js --config my-companion-config.json`
