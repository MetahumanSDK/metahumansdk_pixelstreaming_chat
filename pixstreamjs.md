# Comparion to Epic's Pixel Streaming

Our version is heavily based on the Epic Games® pixel streaming [source code](https://github.com/EpicGames/PixelStreamingInfrastructure). But it's ported to Typescript and splited to different logic parts. Also it contains both server side and browser side logic. It's tweakable and customizable by design.

## Main differences

| Feature            | pixstream.js | Epic's Pixel Streaming |
|--------------------|--------------|------------------------|
| typescript support | ✅            | partial               |
| modularity         | ✅            | partial                |
| npm packages       | ✅            | partial                |
| chat bot logic     | ✅            | —                      |
| chat bot UI        | ✅            | —                      |
| single URL*         | ✅            | —                      |

\* Companion server doesn't have UI and only should be used for WebRTC and WebSocket connections. So user don't see Companion server url in web browser address field, and it's more difficult to directly connect to Companion server skipping Ballancer.

### Naming

**Streamer** – Unreal Engine® based application that we want to stream.
**Companion** – (aka Signalling) server that arrange connections to *streamer*.
**Ballancer** – (aka Matchmaker) server that equally distribute load between pairs: *streamer* and *companion*.

### Package stucture

- [@pixstream/universal](https://www.npmjs.com/package/@pixstream/universal) – contains logic and contracts shared between browser and server.
- [@pixstream/browser](https://www.npmjs.com/package/@pixstream/browser) – contains frontend logic for connecting and controlling streaming server.
- [@pixstream/server](https://www.npmjs.com/package/@pixstream/server) – contains server logic shared between companion and ballancer.
- [@pixstream/companion](https://www.npmjs.com/package/@pixstream/companion) – companion server logic
- [@pixstream/ballancer](https://www.npmjs.com/package/@pixstream/ballancer) – ballancer server logic
