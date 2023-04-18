# unreal-streamer

## Launch
At the Editor, you can play as standalone game with the "Additional Launch Parameters" at the settings: Editor Preferences / Level Editor / Play :
```
PixelStreamingURL=ws://localhost:8888 -log
```
After being launching server part (more details [here](../README.md)), you will get pixel streaming MetaHuman chat in a opened browser tab.

Check launching commands for packaged build in the example: [start-unreal-streamer.bat](./start-unreal-streamer.bat)

## MetaHuman Chat Setup
MetaHuman on the project map (./Content/Maps/Demo) works out the box.

For custom integration with your MetaHuman :
1) Add MetaHuman to the scene. You could get it from [Quixel Bridge](https://docs.metahuman.unrealengine.com/en-US/getting-started-with-metahumans-in-unreal-engine-5/).
2) From MetahumanSDK plugin, add BP_Talk_Component (./Plugins/MetahumanSDK/Content/Demo/Core/) in your MetaHuman.

By default, in the project for chatting - BP_PlayerController_Demo (./Content/Core/) gets the first actor with BP_Talk_Component on the scene.
