<a href="https://github.com/JanSeliv/Bomber/blob/main/LICENSE">![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)</a>
<a href="https://www.unrealengine.com/">![Unreal Engine](https://img.shields.io/badge/Unreal-5.7-dea309?style=flat&logo=unrealengine)</a>

<br/>
<p align="center">
<a href="https://github.com/JanSeliv/Bomber">
</a>
<h3 align="center">💣 Bomberrage</h3>
<p align="center">
<a href="https://discord.gg/jbWgwDefnE"><strong>Join our Discord ››</strong></a>
<br/>
<a href="https://store.steampowered.com/app/1873240/Bomberrage/">Steam</a>
·
<a href="https://trello.com/b/1jbKvyeh/bomber-kanban">Board</a>
·
<a href="https://bigjangames.website/bomberrage">Website</a>
<br/>
<br/>
<img src="https://github.com/user-attachments/assets/835bfb02-76ee-4373-a00b-543a0bde7057" width="1440">
</p>

## 🌟 About

Bomberrage is an open-source Unreal Engine 5 game available on Steam for Windows, macOS and Linux.

Forget hidden exits and classic rules - Bomberrage is a fast, competitive game where you beat tough AI or friends in explosive multiplayer battles!

![Bomberrage](https://github.com/user-attachments/assets/e8774b8b-2f76-42f1-8eae-e6849658d2d3)

## Table of Contents

- [💣 About the Bomber Project](#-about-the-bomber-project)
- [🚀 Getting Started](#-getting-started)
- [💾 Play the Build](#-play-the-build)
- [💻 Unreal Project Requirements](#-platforms-and-requirements)
- [🛠 Key Features](#-key-features)
- [📋 Kanban Board](#-kanban-board)
- [📅 Changelog](#-changelog)
- [🧑‍🤝‍🧑 Credits](#-credits)
- [📫 Feedback & Contribution](#-feedback--contribution)
- [📜 License](#-license)

## 🚀 Getting Started

This project contains **submodules** and requires `--recurse-submodules` when cloning:
```sh
git clone --recurse-submodules https://github.com/JanSeliv/Bomber.git
```
If already cloned without submodules, you'll find empty folders in `Bomber\Plugins` and error on project startup. To download submodules separately, run:
```sh
git submodule update --init --recursive
```

## 💾 Play the Build

- **Download the latest build on Steam** via [**Download Bomberrage Demo**](https://store.steampowered.com/app/1873240/Bomberrage/):

[![Download on Steam](https://github.com/user-attachments/assets/39bbd233-fc1f-4a16-aec4-f33983a92cd8)](https://store.steampowered.com/app/1873240/Bomberrage/)

- Mirror link: [**GitHub Releases**](https://github.com/JanSeliv/Bomber/releases/)

## 💻 Unreal Project Requirements

- **Unreal Engine 5.7**
- **Project Size:** ~30GB (build ~3GB)

The project has been tested and launching the editor on the following platforms:

- **Windows 10 22H2**
- **macOS Sonoma 14.4** (Apple M2 hardware)
- **Ubuntu 22.04 LTS**
- **Android 14** (experimental with some issues)

## 🛠 Key Features

This project could be useful for learners, demonstrating next features:

- Gameplay Ability System (GAS)
- Mover 2.0
- Steam multiplayer support for 4 players (via Steam Friends)
- Modern networking: the Push Model and Iris replication
- Procedurally generated playfield
- Challenging AI
- Enhanced Input
- Mods and Modular Game Features
- Complex cinematics (Level Sequences)
- World Partition
- Model-View-ViewModel (MVVM) UI Pattern
- Localization
- Data-Driven Design (Data Assets, Data Tables, Data Registries, _see below_)

Despite this project is fully written in C++, it's extremely **blueprint-friendly**:

- **Data-Driven Design**: No hardcoded values. All data can be tweaked via Data Assets in editor as well as accessed in blueprints [[doc](https://trello.com/c/HGscMUdK)].
- **Fully Exposed**: Every class, property, and function is exposed to Blueprints allowing for heavy changes the logic with no code.
- **Well-Commented**: Every class, property and function is well-commented for easy understanding.
- **Utility Libraries**: Core static functions are accessible globally like Cell Utils [[doc](https://trello.com/c/b2IzcOhg)]. See more in the `Source\UtilityLibraries` [folder](https://github.com/JanSeliv/Bomber/tree/master/Source/Bomber/Public/UtilityLibraries).

Next [plugins](https://github.com/JanSeliv/Bomber/tree/master/Plugins) were developed for this project, but could be useful for other developers:

- [⚙️ Settings Widget Constructor](https://github.com/JanSeliv/SettingsWidgetConstructor)
- [🔄 Pool Manager](https://github.com/JanSeliv/PoolManager)
- [🎭 Morphs Player](https://github.com/JanSeliv/MorphsPlayer)
- [ƒ Function Picker](https://github.com/JanSeliv/FunctionPicker)
- [\>_ Meta Cheat Manager](https://github.com/JanSeliv/MetaCheatManager)
- [✂️ Level Sequencer Audio Trimmer](https://github.com/JanSeliv/LevelSequencerAudioTrimmer)

## 📋 Kanban Board

Stay updated with the current progress and plans on the [Trello board](https://trello.com/b/1jbKvyeh/bomber-kanban).

## 📅 Changelog
#### `2026-XX-XX:`
- Updated to **Unreal Engine 5.7**.
- Added sprint speed trail VFX when picked up maximum amount of skate powerups by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> ![SkateMax](https://github.com/user-attachments/assets/0ede3ba0-b26b-4cd0-b2d6-4eb4479b2a63)
- Added arrow indicator above local player during game start:
- ![PlayerArrowStart](https://github.com/user-attachments/assets/c1f2792d-44ff-4337-83c3-db087c73ddd0)
#### `2025-11-17:`
- Updated to **Unreal Engine 5.6**.
- Migrated the project to use **Gameplay Ability System (GAS)** for actions (bombs, damage, powerups) and **Mover 2.0** for movement, significantly improving the responsiveness in multiplayer for players with high ping.
- Implemented **pick up toast** for powerups, which is especially useful during intense gameplay to track easily the number of power-ups:
> ![PickupToast](https://github.com/user-attachments/assets/48dcb22d-91fd-4285-b695-0283db0f62c6)
- Players now have proper **death anims** when eliminated, animation by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> ![DeathAnim](https://github.com/user-attachments/assets/4d94969d-c115-4e81-b8c2-285a636b7968)
- Updated progression visualization to display player bombs instead of stars by [Maksim Shashkov](https://www.artstation.com/maksimshashkov) and [Valeriy Rotermel](https://github.com/moinm3uw)
> ![BombStars](https://github.com/user-attachments/assets/152730fa-677d-43cc-a5f8-5cbbeeff32dc)
- Optimized level generation with a single-pass algorithm for large level support, reducing 40x40 map creation time from >1000ms to under 1ms, allowing to build massive maps:
> ![LargeMap](https://github.com/user-attachments/assets/b0f2ab54-00a1-416d-aefb-706b3f3538a3)
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
> ![NewSkins](https://github.com/user-attachments/assets/11decad0-fa4c-45ff-ba33-9a6e2d805773)
- Added `Play Area Surrounder` mod on medium difficulty surrounding the play area with walls over time by [Anton Selivanov](https://github.com/antokior)
> ![PlayAreaSurrounder](https://github.com/user-attachments/assets/1ff184f3-ba25-4315-8ca1-df87d213dfb4)
- Added `Bomb Storm` mod on hard difficulty massively spawning bombs:
> ![BombStorm](https://github.com/user-attachments/assets/a7bed05d-0e83-4cf4-aa17-b45744aea124)
- New Bastet bomb by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
> ![BastetBomb](https://github.com/user-attachments/assets/27bcf3fe-2ea0-429b-8689-07b3ac4a482b)
- Progression System has been updated with new star mesh by [Kateryna Shchetinina](https://www.artstation.com/kateseliv) and implementation by [Valeriy Rotermel](https://github.com/moinm3uw):
> ![NewStars](https://github.com/user-attachments/assets/66160a94-d192-4bf7-8f99-cfa9854be7eb)
- Implemented the Loading Screen on launching the game and joining a multiplayer session:
> ![LoadingScreen](https://github.com/user-attachments/assets/084270ca-abc3-44c8-bd44-ae1ce26d1e25)
- Added the **Language setting** and fully localized the game in 30 languages, including Arabic, Chinese, Korean, and Thai:
> ![Localization](https://github.com/user-attachments/assets/68d3c9db-c850-4346-8ffb-231c5c6ec0e8)
- Implemented `Honor Loss` game result rewarding players who perform well despite losing by design from [Yevhenii Oksenchuk](https://t.me/ComeThird).
- Implemented unique starting attributes for each character (e.g., Bastet starts with 2 speed, Roger with 2 bombs, etc.)
- Improved nicknames display in the Main Menu and in-game UI.
 ---
#### `2024-12-29:`
- Updated to **Unreal Engine 5.4**.
- Added **Linux** support (tested on Ubuntu 22.04 LTS)
- Introduced **In-Game User Interface** with completely new look, utilizing the **Model-View-ViewModel** (MVVM) pattern:
> ![NewHUD](https://github.com/JanSeliv/Bomber/assets/20540872/73c3c7f7-02b5-4d54-b34f-b354201bfc06)
- Added cinematic for the Roger character on the Maya level by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> ![RogerCinematic](https://github.com/JanSeliv/Bomber/assets/20540872/9931d8da-e8cb-4cf5-ab61-361f48afa20b)
- Added cinematic for the Bastet character on the Maya level by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
> ![BastetCinematic](https://github.com/JanSeliv/Bomber/assets/20540872/0602cc7c-f68c-46fa-9400-46a8bc35c73c)
- Implemented **Switch Camera Transitions** between characters in Main Menu:
> ![Rails](https://github.com/JanSeliv/Bomber/assets/20540872/aa496ae1-a6bb-41d1-a578-566d1af48170)
- Unique **Bomb VFX** for each character:
> ![BombVFXs](https://github.com/JanSeliv/Bomber/assets/20540872/3163ade3-7f5f-40be-9c9e-69c0426b8a29)
- Implemented **[Progression System](https://github.com/moinm3uw/ProgressionSystem)** by [Valeriy Rotermel](https://github.com/moinm3uw) that unlocks new playable characters as you progress in the game:
> ![ProgressionSystem](https://github.com/user-attachments/assets/742ad861-f077-44f8-a6ae-048665b8a77f)
- New **Box and Wall meshes** for the Maya level by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
> ![NewBoxAndWall](https://github.com/user-attachments/assets/01e72eb6-ca89-4392-957c-92aba9663cdc)
- Implemented **Credits** screen by [Yevhenii Oksenchuk](https://t.me/ComeThird):
> ![Credits](https://github.com/user-attachments/assets/5ec5c208-9b3e-4b1c-be3d-711a973ce652)
- New **Splash** by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
> ![Splash](https://github.com/user-attachments/assets/8df95267-6c8d-4434-bb18-c7381c4ef601)
- Converted the Maya level to the **World Partition** to benefit from automatic streaming and External Data Layers.
 ---
#### `2024-01-13:`
- Updated to **Unreal Engine 5.3**.
- **New Main Menu** with completely different UI and complex cinematics for Hugo and Fori characters on starting the game:
> ![NewMainMenu](https://github.com/JanSeliv/Bomber/assets/20540872/9c960fa4-6760-4298-a55b-54d0cb8a0b13)
- **New Bomb meshes** for each character (shown from left to right: Bastet, Hugo, Fori, Roger) by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
> ![NewBombMeshes](https://github.com/JanSeliv/Bomber/assets/20540872/ce787e8c-d95c-4844-9282-e7aaff3dc243)
- **New game icon**: ![GameIcon](https://github.com/JanSeliv/Bomber/assets/20540872/ca239a66-b550-4a45-ba4f-182d85e3c460)
 ---
#### `2023-06-12:`
- Updated to **Unreal Engine 5.2**.
- Added **MacOS** support.
- Added **Ultra-wide** resolutions support.
- Extracted logic into plugins, so other developers can benefit from it in their projects
- Added Foot Trails for the Maya level by [Anton Selivanov](https://github.com/antokior):
>  <img width="560" alt="image" src="https://github.com/JanSeliv/Bomber/assets/20540872/a77c2e38-4fd6-4a04-988e-05d9613bd97e">
- New power-ups meshes for the Maya level (shown from left to right: move speed, bomb length, bomb quantity) by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
>  <img width="360" alt="image" src="https://github.com/JanSeliv/Bomber/assets/20540872/1e526fda-e51a-479c-b541-acccc8457725">
- Added new cheats such as: `Bomber.Level.SetSize 9x7` (find more on the [Bomber cheats page](https://trello.com/c/5PiHt7Ah/308-bomber-cheats))
- Updated Main-Menu background music.
 --- 
#### `2022-05-31:`
 - Added initial **multiplayer** support for 4 players (without Steam now, use 'Open' command to connect to each other).
 - Created the **Pool Manager** for the generated level to avoid spawning and destroying actors on each level reconstruction.
 - Added new **SteelMan** character for AI players with 3 different skins by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
>  ![SteelMan_31-05-22](https://user-images.githubusercontent.com/20540872/171299202-3422db3c-7061-4b75-b51c-a08a67d65ab5.gif)
 ---
#### `2021-12-31:`
 - The game migrated to the **Unreal Engine 5**.
 - Added sounds (background music, UI, in-game sounds) and sliders to tweak volumes in Settings Audio tab (Master, Music and SFX).
 - Added Controls tab in Settings to allow player remap input keys.
>  ![Settings_31-12-21](https://user-images.githubusercontent.com/20540872/147825296-ce7d33da-dfda-4757-b070-bfd08f700134.jpg)
 ---
#### `2021-06-03:`
 - Added the Maya level by [Maksim Shashkov](https://www.artstation.com/maksimshashkov):
>  ![](https://user-images.githubusercontent.com/20540872/120249537-8bf83e80-c27b-11eb-81be-583e8c30aa62.jpg)
 - Implemented `Settings` screen:
>  ![](https://user-images.githubusercontent.com/20540872/120127584-0e232d00-c1c0-11eb-8467-74633600c180.jpg)
 ---
#### `2021-01-31:`
 - Updated to **Unreal Engine 4.26**.
 - Added the Bastet (Sphynx cat) and Roger (Pirate) characters by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
 - Fori and Hugo characters got additional second skins by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
 > ![](https://user-images.githubusercontent.com/20540872/106404153-23ff2c00-6432-11eb-8cb1-d3a7bc33b51b.gif)
 ---
#### `2020-10-25`:
 - Updated to **Unreal Engine 4.25**.
 - Added the Hugo and Fori characters by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
 > ![](https://user-images.githubusercontent.com/20540872/97118032-125a0a00-1708-11eb-8256-4bec419b1d48.gif)
 ---
#### `2019-10-15:`
- Uploaded first game-ready build on **Unreal Engine 4.23**: [download from GDrive](https://drive.google.com/file/d/1DY4l9XEcazxouiTDTPcZhBv3XDKAfDBD)
- **The level camera** that moves and zooms lens depending on the distance between players:
> ![GIF1](https://user-images.githubusercontent.com/20540872/62881283-b6d47400-bd2f-11e9-91bb-94d60942f8f8.gif)
- First UI displaying the items (at the left side of the player’s avatar), the timer (that is placed under) and the number of alive players (at the
right side):
> ![GIF2](https://user-images.githubusercontent.com/20540872/63038224-f8e0ef80-bec0-11e9-9f32-711793cd9bee.gif)
- Symmetrical **Procedural generation** for each new game:
> ![GIF6](https://user-images.githubusercontent.com/20540872/67123411-8659fc00-f1f0-11e9-8b71-f0b9072c34f8.gif)
- Dynamic scaling in the editor:
> ![GIF7](https://user-images.githubusercontent.com/20540872/63046685-45352b00-bed3-11e9-81f4-fea4fdf1f0c7.gif)
- Dynamic placement of the actors on the level:
> ![GIF10](https://user-images.githubusercontent.com/20540872/63053411-f5aa2b80-bee1-11e9-9328-79cf77609ec7.gif)
- Free location and rotation of the level map in the editor:
> ![GIF11](https://user-images.githubusercontent.com/20540872/63057315-3f970f80-beea-11e9-979f-c7874042a382.gif)
- Smart **AI** surviving through any explosions:
> ![GIF14](https://user-images.githubusercontent.com/20540872/63062621-e46d1900-bef9-11e9-8e84-dbad3eb14dc6.gif)

## 🧑‍🤝‍🧑 Credits

- **Yevhenii Selivanov** - Programming - [GitHub](https://www.github.com/janseliv), [Telegram](https://t.me/JanSeliv)
- **Maksim Shashkov** - Level Design & Level Art - [Artstation](https://www.artstation.com/maksimshashkov)
- **Kateryna Shchetinina** - Characters & Animations - [Artstation](https://www.artstation.com/kateseliv)
- **Yevhenii Oksenchuk** - Game Design (Audio, UI, and Cinematics) - [Telegram](https://t.me/ComeThird)
- **Valeriy Rotermel** - [Progression System](https://github.com/moinm3uw/ProgressionSystem) - [GitHub](https://github.com/moinm3uw)
- **Anton Selivanov** - Foot Trails | Play Area Surrounder - [GitHub](https://github.com/antokior)

Special thanks to the following companies for providing their licenses to support our open source development:

- [JetBrains Rider](https://www.jetbrains.com/community/opensource/#support) - cross-platform .NET IDE.
- [PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.

## 📫 Feedback & Contribution

Feedback and contributions from the community are highly appreciated!

- **Tasks:** Check our [Google Sheets Tasks List](https://docs.google.com/spreadsheets/d/1BaElMO0IDiV7im5FNk19ewWBXH1vpg3-1TjiEX55Kw8/edit#gid=554015394) and [Bugs Backlog on Trello](https://trello.com/b/1jbKvyeh/bomber-kanban). Unassigned tasks and bugs are open for contribution.
- **Report & Suggest:** Found a bug or have a feature idea? Open an issue.
- **Fork & Pull:** Fork the project, make your changes, and submit a pull request to the `develop` branch.
- **Standards:** Adhere to the [Unreal Engine Coding Standards](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine) and [Naming Standards](https://github.com/Allar/ue5-style-guide) when contributing.
- **Blueprints & Assets:** If contributing blueprint logic or assets, attach screenshots to show what has changed.

## 📜 License

This project is licensed under the terms of the MIT license. See [LICENSE](LICENSE) for more details.

We hope you find this project useful and we look forward to your feedback and contributions.
