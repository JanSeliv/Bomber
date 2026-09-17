<a href="https://github.com/JanSeliv/Bomber/blob/main/LICENSE">![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)</a>
<a href="https://www.unrealengine.com/">![Unreal Engine](https://img.shields.io/badge/Unreal-5.8-dea309?style=flat&logo=unrealengine)</a>
![Windows](https://img.shields.io/badge/Windows-0078D6?style=flat&logo=windows&logoColor=white)
![macOS](https://img.shields.io/badge/macOS-000000?style=flat&logo=apple&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat&logo=linux&logoColor=black)
![Android](https://img.shields.io/badge/Android_(experimental)-3DDC84?style=flat&logo=android&logoColor=white)

<br/>
<p align="center">
<h3 align="center">💣 Bomberrage</h3>
<p align="center">
<a href="https://discord.gg/jbWgwDefnE"><strong>Join our Discord ››</strong></a>
<br/>
<a href="https://store.steampowered.com/app/1873240/Bomberrage/">Steam</a>
·
<a href="https://trello.com/b/1jbKvyeh/bomber-kanban">Trello</a>
·
<a href="https://docs.google.com/document/d/1Dy2gNEdFfdDeJ3V-Gl8kbPuZu46TQg6wGnW3Tej22SI">Docs</a>
<br/>
<br/>
<img src="https://github.com/user-attachments/assets/835bfb02-76ee-4373-a00b-543a0bde7057" width="1440">
</p>

## 🌟 About

Bomberrage is an open-source Unreal Engine 5 game available on Steam for Windows, macOS and Linux, actively developed since 2019.

![Bomberrage](https://github.com/user-attachments/assets/e8774b8b-2f76-42f1-8eae-e6849658d2d3)

## 🚀 Getting Started

See the [Getting Started](https://docs.google.com/document/d/1Dy2gNEdFfdDeJ3V-Gl8kbPuZu46TQg6wGnW3Tej22SI/edit?tab=t.0#heading=h.p016an855xg8) on how to download UE project or get the packaged build.

## 🛠 Key Features

You might find this project useful if you are interested in examples of using any of the following Unreal Engine systems:

- Modding via Game Feature Plugins (GFP)
- Gameplay Ability System (GAS)
- Mover 2.0 (replaced Character Movement Component)
- Online Sessions (Steam)
- Efficient replication: Push Model, Iris and fast arrays
- State Trees
- Level Sequences (12 000+ frames | ~7 min)
- Primary Data Assets | Data Registries (Data Tables)
- Async Message System (replaced multicast delegates)
- Model-View-ViewModel (MVVM)
- External Data Layers (World Partition)
- Enhanced Input (remapping)
- Localization
- _And more..._

Despite this project is mostly written in C++, it's extremely **blueprint-friendly**:

- **Data-Driven Design**: No hardcoded values. 100% data can be tweaked in editor or via mods (Game Feature Plugins).
- **Fully Exposed**: Every class, property, and function is exposed to Blueprints allowing for heavy changes with no code.
- **Well-Commented**: Every class, property and function is well-commented for easy understanding.

## Table of Contents

- [📅 Changelog](#-changelog)
- [🧑‍🤝‍🧑 Credits](#-credits)
- [📫 Feedback & Contribution](#-feedback--contribution)

## 📅 Changelog
#### `2026-XX-XX:`
- Updated to **Unreal Engine 5.8**.
- Players now play pickup animation when collecting powerups by [Kateryna Shchetinina](https://www.artstation.com/kateseliv).
#### `2026-06-23:`
- Updated to **Unreal Engine 5.7**.
- Reduced repo size from ~20 GB to ~100 MB: added Blockout Map with minimal content (full content with Maya Map can be downloaded from [Releases](https://github.com/JanSeliv/Bomber/releases/))
> <img width="640" src="https://github.com/user-attachments/assets/70058b0b-0ca5-48ca-9079-7426206e383f" />
- Migrated game state management to **State Trees** for cleaner state transitions
> <img width="640" src="https://github.com/user-attachments/assets/9879e6df-c923-4b24-8137-a91ee933e537">
- Added arrow indicator above local player during game start:
> <img height="240" src="https://github.com/user-attachments/assets/c1f2792d-44ff-4337-83c3-db087c73ddd0">
- Added sprint speed trail VFX when picked up maximum amount of skate powerups by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> <img height="240" src="https://github.com/user-attachments/assets/0ede3ba0-b26b-4cd0-b2d6-4eb4479b2a63">
- Moved from multicast delegates to global tag-events via Async Message System plugin (aka Lyra's Gameplay Message Router)
- Migrated to **Data Assets Loader** plugin to feature modding freedom and automatically async load all assets.
- Added **mod packaging**: one button cooks any Game Feature Plugin separately, so a feature can be distributed and installed as its own standalone, removable mod.
- Rebalanced bots across all difficulty levels: Hard now runs previous AI behavior, Medium is friendlier now, on Easy no longer dodge bombs or steal power-ups
- Rebalanced progression rewards to get more stars by default
- Remapped gamepad controls to a new layout
 ---
#### `2025-11-17:`
- Updated to **Unreal Engine 5.6**.
- Migrated the project to use **Gameplay Ability System (GAS)** for actions (bombs, damage, powerups) and **Mover 2.0** for movement, significantly improving the responsiveness in multiplayer for players with high ping.
- Implemented **pick up toast** for powerups, which is especially useful during intense gameplay to track easily the number of power-ups:
> <img height="360" src="https://github.com/user-attachments/assets/48dcb22d-91fd-4285-b695-0283db0f62c6">
- Players now have proper **death anims** when eliminated, animation by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> <img height="240" src="https://github.com/user-attachments/assets/4d94969d-c115-4e81-b8c2-285a636b7968">
- Updated progression visualization to display player bombs instead of stars by [Maksim Shashkov](https://www.artstation.com/maksimshashkov) and [Valeriy Rotermel](https://github.com/moinm3uw)
> <img height="240" src="https://github.com/user-attachments/assets/152730fa-677d-43cc-a5f8-5cbbeeff32dc">
- Optimized level generation with a single-pass algorithm for large level support, reducing 40x40 map creation time from >1000ms to under 1ms, allowing to build massive maps:
> <img height="240" src="https://github.com/user-attachments/assets/b0f2ab54-00a1-416d-aefb-706b3f3538a3">
- Moved level generation to background thread, reducing main thread time from 60ms to 13ms.
 ---
#### `2025-06-30:`
- Updated to **Unreal Engine 5.5**.
- Uploaded the game to the **Steam** for public testing: [store page](https://store.steampowered.com/app/1873240/Bomberrage/).
- Added **Steam multiplayer** support, so players can invite and join each other via Steam Friends.
- Added **Android** support (experimental, with some issues).
- Improved performance with up to 300% gain → [results](https://docs.google.com/spreadsheets/d/10pPYJZAu-qeA9zKYOt6jn8ioxhOD58mFPgjdrATaN84/edit?usp=sharing).
- Improved networking efficiency by up to 642% with **Push Model** and **Iris** replication → [results](https://trello.com/c/A3kK1Uqj).
- Finished 4 skins for each character by [Kateryna Shchetinina](https://www.artstation.com/kateseliv), with skins unlock mechanic by [Valeriy Rotermel](https://github.com/moinm3uw):
> <img width="640" src="https://github.com/user-attachments/assets/11decad0-fa4c-45ff-ba33-9a6e2d805773">
- Added `Play Area Surrounder` mod on medium difficulty surrounding the play area with walls over time by [Anton Selivanov](https://github.com/antokior)
> <img height="240" src="https://github.com/user-attachments/assets/1ff184f3-ba25-4315-8ca1-df87d213dfb4">
- Added `Bomb Storm` mod on hard difficulty massively spawning bombs:
> <img height="240" src="https://github.com/user-attachments/assets/a7bed05d-0e83-4cf4-aa17-b45744aea124">
- New Bastet bomb by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
> <img height="240" src="https://github.com/user-attachments/assets/27bcf3fe-2ea0-429b-8689-07b3ac4a482b">
- Progression System has been updated with new star mesh by [Kateryna Shchetinina](https://www.artstation.com/kateseliv) and implementation by [Valeriy Rotermel](https://github.com/moinm3uw):
> <img height="240" src="https://github.com/user-attachments/assets/66160a94-d192-4bf7-8f99-cfa9854be7eb">
- Implemented the Loading Screen on launching the game and joining a multiplayer session:
> <img height="240" src="https://github.com/user-attachments/assets/084270ca-abc3-44c8-bd44-ae1ce26d1e25">
- Added the **Language setting** and fully localized the game in 30 languages, including Arabic, Chinese, Korean, and Thai:
> <img width="560" src="https://github.com/user-attachments/assets/68d3c9db-c850-4346-8ffb-231c5c6ec0e8">
- Implemented `Honor Loss` game result rewarding players who perform well despite losing by design from [Yevhenii Oksenchuk](https://t.me/ComeThird).
- Implemented unique starting attributes for each character (e.g., Bastet starts with 2 speed, Roger with 2 bombs, etc.)
- Improved nicknames display in the Main Menu and in-game UI.
 ---
#### `2024-12-29:`
- Updated to **Unreal Engine 5.4**.
- Added **Linux** support (tested on Ubuntu 22.04 LTS)
- Introduced **In-Game User Interface** with completely new look, utilizing the **Model-View-ViewModel** (MVVM) pattern:
> <img height="240" src="https://github.com/JanSeliv/Bomber/assets/20540872/73c3c7f7-02b5-4d54-b34f-b354201bfc06">
- Added cinematic for the Roger character on the Maya level by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> <img height="240" src="https://github.com/JanSeliv/Bomber/assets/20540872/9931d8da-e8cb-4cf5-ab61-361f48afa20b">
- Added cinematic for the Bastet character on the Maya level by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> <img height="240" src="https://github.com/JanSeliv/Bomber/assets/20540872/0602cc7c-f68c-46fa-9400-46a8bc35c73c">
- Implemented **Switch Camera Transitions** between characters in Main Menu:
> <img height="240" src="https://github.com/JanSeliv/Bomber/assets/20540872/aa496ae1-a6bb-41d1-a578-566d1af48170">
- Unique **Bomb VFX** for each character:
> <img height="240" src="https://github.com/JanSeliv/Bomber/assets/20540872/3163ade3-7f5f-40be-9c9e-69c0426b8a29">
- Implemented **[Progression System](https://github.com/moinm3uw/ProgressionSystem)** by [Valeriy Rotermel](https://github.com/moinm3uw) that unlocks new playable characters as you progress in the game:
> <img height="240" src="https://github.com/user-attachments/assets/742ad861-f077-44f8-a6ae-048665b8a77f">
- New **Box and Wall meshes** for the Maya level by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
> <img height="240" src="https://github.com/user-attachments/assets/01e72eb6-ca89-4392-957c-92aba9663cdc">
- Implemented **Credits** screen by [Yevhenii Oksenchuk](https://t.me/ComeThird):
> <img height="240" src="https://github.com/user-attachments/assets/5ec5c208-9b3e-4b1c-be3d-711a973ce652">
- New **Splash** by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
> <img height="240" src="https://github.com/user-attachments/assets/8df95267-6c8d-4434-bb18-c7381c4ef601">
- Converted the Maya level to the **World Partition** to benefit from automatic streaming and External Data Layers.
 ---
#### `2024-01-13:`
- Updated to **Unreal Engine 5.3**.
- **New Main Menu** with completely different UI and complex cinematics for Hugo and Fori characters on starting the game:
> <img height="240" src="https://github.com/JanSeliv/Bomber/assets/20540872/9c960fa4-6760-4298-a55b-54d0cb8a0b13">
- **New Bomb meshes** for each character (shown from left to right: Bastet, Hugo, Fori, Roger) by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
> <img height="240" src="https://github.com/JanSeliv/Bomber/assets/20540872/ce787e8c-d95c-4844-9282-e7aaff3dc243">
- **New game icon**: ![GameIcon](https://github.com/JanSeliv/Bomber/assets/20540872/ca239a66-b550-4a45-ba4f-182d85e3c460)
 ---
#### `2023-06-12:`
- Updated to **Unreal Engine 5.2**.
- Added **MacOS** support.
- Added **Ultra-wide** resolutions support.
- Extracted logic into plugins, so other developers can benefit from it in their projects
- Added Foot Trails for the Maya level by [Anton Selivanov](https://github.com/antokior):
>  <img height="240" alt="image" src="https://github.com/JanSeliv/Bomber/assets/20540872/a77c2e38-4fd6-4a04-988e-05d9613bd97e">
- New power-ups meshes for the Maya level (shown from left to right: move speed, bomb length, bomb quantity) by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
>  <img height="240" alt="image" src="https://github.com/JanSeliv/Bomber/assets/20540872/1e526fda-e51a-479c-b541-acccc8457725">
- Added new cheats such as: `Bomber.Level.SetSize 9x7` (find more on the [Bomber cheats page](https://trello.com/c/5PiHt7Ah/308-bomber-cheats))
- Updated Main-Menu background music.
 --- 
#### `2022-05-31:`
- Added initial **multiplayer** support for 4 players (without Steam now, use 'Open' command to connect to each other).
- Created the **Pool Manager** for the generated level to avoid spawning and destroying actors on each level reconstruction.
- Added new **SteelMan** character for AI players with 3 different skins by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
>  <img height="240" src="https://user-images.githubusercontent.com/20540872/171299202-3422db3c-7061-4b75-b51c-a08a67d65ab5.gif">
 ---
#### `2021-12-31:`
- The game migrated to the **Unreal Engine 5**.
- Added sounds (background music, UI, in-game sounds) and sliders to tweak volumes in Settings Audio tab (Master, Music and SFX).
- Added Controls tab in Settings to allow player remap input keys.
>  <img height="240" src="https://user-images.githubusercontent.com/20540872/147825296-ce7d33da-dfda-4757-b070-bfd08f700134.jpg">
 ---
#### `2021-06-03:`
- Added the Maya level by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
>  <img height="240" src="https://user-images.githubusercontent.com/20540872/120249537-8bf83e80-c27b-11eb-81be-583e8c30aa62.jpg">
- Implemented `Settings` screen:
>  <img height="240" src="https://user-images.githubusercontent.com/20540872/120127584-0e232d00-c1c0-11eb-8467-74632400c180.jpg">
 ---
#### `2021-01-31:`
- Updated to **Unreal Engine 4.26**.
- Added the Bastet (Sphynx cat) and Roger (Pirate) characters by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
- Fori and Hugo characters got additional second skins by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> <img height="240" src="https://user-images.githubusercontent.com/20540872/106404153-23ff2c00-6432-11eb-8cb1-d3a7bc33b51b.gif">
 ---
#### `2020-10-25`:
- Updated to **Unreal Engine 4.25**.
- Added the Hugo and Fori characters by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> <img height="240" src="https://user-images.githubusercontent.com/20540872/97118032-125a0a00-1708-11eb-8256-4bec419b1d48.gif">
 ---
#### `2019-10-15:`
- Uploaded first game-ready build on **Unreal Engine 4.23**: [download from GDrive](https://drive.google.com/file/d/1DY4l9XEcazxouiTDTPcZhBv3XDKAfDBD)
- **The level camera** that moves and zooms lens depending on the distance between players:
> <img height="240" src="https://user-images.githubusercontent.com/20540872/62881283-b6d47400-bd2f-11e9-91bb-94d60942f8f8.gif">
- First UI displaying the items (at the left side of the player’s avatar), the timer (that is placed under) and the number of alive players (at the
  right side):
> <img height="240" src="https://user-images.githubusercontent.com/20540872/63038224-f8e0ef80-bec0-11e9-9f32-711793cd9bee.gif">
- Symmetrical **Procedural generation** for each new game:
> <img height="240" src="https://user-images.githubusercontent.com/20540872/67123411-8659fc00-f1f0-11e9-8b71-f0b9072c34f8.gif">
- Dynamic scaling in the editor:
> <img height="240" src="https://user-images.githubusercontent.com/20540872/63046685-45352b00-bed3-11e9-81f4-fea4fdf1f0c7.gif">
- Dynamic placement of the actors on the level:
> <img height="240" src="https://user-images.githubusercontent.com/20540872/63053411-f5aa2b80-bee1-11e9-9328-79cf77609ec7.gif">
- Free location and rotation of the level map in the editor:
> <img height="240" src="https://user-images.githubusercontent.com/20540872/63057315-3f970f80-beea-11e9-979f-c7874042a382.gif">
- Smart **AI** surviving through any explosions:
> <img height="240" src="https://user-images.githubusercontent.com/20540872/63062621-e46d1900-bef9-11e9-8e84-dbad3eb14dc6.gif">

## 🧑‍🤝‍🧑 Credits

- **Yevhenii Selivanov** - Programming - [LinkedIn](https://www.linkedin.com/in/yevhenii-selivanov/), [Discord](https://discord.gg/jbWgwDefnE)
- **Maksim Shashkov** - Level Design & Level Art - [Artstation](https://www.artstation.com/maksimshashkov)
- **Kateryna Shchetinina** - Characters & Animations - [Artstation](https://www.artstation.com/kateseliv)
- **Yevhenii Oksenchuk** - Game Design (Audio, UI, and Cinematics) - [LinkedIn](https://www.linkedin.com/in/yevhenii-oksenchuk-358b99292/)
- **Valeriy Rotermel** - [Progression System](https://github.com/moinm3uw/ProgressionSystem) - [GitHub](https://github.com/moinm3uw)
- **Anton Selivanov** - Foot Trails | Play Area Surrounder - [GitHub](https://github.com/antokior)

## 📫 Feedback & Contribution

Feedback and contributions from the community are highly appreciated!

- **Tasks:** Check our [Trello](https://trello.com/b/1jbKvyeh/bomber-kanban) - unassigned tasks and bugs are open for contribution.
- **Report & Suggest:** Found a bug or have a feature idea? Open an issue.
- **Fork & Pull:** Fork the project, make your changes, and submit a pull request to the `develop` branch.
- **Standards:** Adhere to the [Unreal Engine Coding Standards](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine) and [Naming Standards](https://github.com/Allar/ue5-style-guide) when contributing.
- **Blueprints & Assets:** If contributing blueprint logic or assets, attach screenshots to show what has changed.
