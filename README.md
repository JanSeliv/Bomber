<a href="https://github.com/JanSeliv/Bomber/blob/main/LICENSE">![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)</a>
<a href="https://www.unrealengine.com/">![Unreal Engine](https://img.shields.io/badge/Unreal-5.4-dea309?style=flat&logo=unrealengine)</a>

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
<a href="https://trello.com/b/1jbKvyeh/bomber-kanban">Kanban Board</a>
<br/>
<br/>
<img src="https://github.com/user-attachments/assets/835bfb02-76ee-4373-a00b-543a0bde7057" width="1440">
</p>

## 🌟 About

Bomberrage is an open-source indie game developed on Unreal Engine 5 for Windows, MacOS and Linux.

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
- [🎮 Overview](#-overview)
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

- **Unreal Engine 5.4**
- **Project Disk Space:** 33GB

The project has been tested and launching the editor on the following platforms:

- **Windows 10 22H2**
- **macOS Sonoma 14.4** (Apple M2 hardware)
- **Ubuntu 22.04 LTS**

## 🛠 Key Features

This project could be useful for learners, demonstrating next features:

- Multiplayer
- Procedurally generated playfield
- Challenging AI
- Enhanced Input
- Modular Game Features
- Complex cinematics (Level Sequences)
- World Partition
- Model-View-ViewModel (MVVM) UI Pattern
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
- Implemented **[Progression System](https://github.com/h4rdmol/ProgressionSystem)** by [Valeriy Rotermel](https://github.com/h4rdmol) that unlocks new playable characters as you progress in the game:
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
 - Added the Bastet (Sphynx cat) and Roger (Pirate) characters by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
 - Fori and Hugo characters got additional second skins by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
 > ![](https://user-images.githubusercontent.com/20540872/106404153-23ff2c00-6432-11eb-8cb1-d3a7bc33b51b.gif)
 ---
#### `25.10.2020:
 - Added the Hugo and Fori characters by [Kateryna Shchetinina](https://www.artstation.com/kateseliv):
 > ![](https://user-images.githubusercontent.com/20540872/97118032-125a0a00-1708-11eb-8256-4bec419b1d48.gif)
 ---
#### `2019-10-15:` Uploaded first game-ready build.

## 🎮 Overview

**The level camera** that moves and zooms lens depending on the distance between players:

![GIF1](https://user-images.githubusercontent.com/20540872/62881283-b6d47400-bd2f-11e9-91bb-94d60942f8f8.gif)

## Level actors

- **Items** that affect the abilities of a player during gameplay:

Skate:  Increase the movement speed of the character.

Bomb: Increase the number of bombs that can be set at one time.

Fire: Increase the bomb blast radius.

- **Bombs:** are left by the character to destroy the level actors. Triggers other bombs and prevents players from
  moving through the bomb after it has been left behind.
- **Walls**: are not destroyed by a bomb explosion and stop the explosion.
- **Boxes** on destruction with some chances spawn an item.
- **Players and AI** - characters whose goal is to remain the last survivor for the win.

## Game interface

The number of items that are shown at the left side of the player’s avatar, the timer that is placed under and at the
right side is shown the number of alive players:

![GIF2](https://user-images.githubusercontent.com/20540872/63038224-f8e0ef80-bec0-11e9-9f32-711793cd9bee.gif)

The game menu is shown the result of the games match (win, lose, draw). If the match has not yet finished, it could be
minimized or opened out by ESC button in order to continue watching the game or restart the play, or to return to the
main menu:

- Shows the **win** notification when there is one character left:

![GIF3](https://user-images.githubusercontent.com/20540872/63024460-87487780-bea7-11e9-8573-b0950a040fe4.gif)

- Shows the **draw** when the last players are killed at the same time or at the end of game timer:

![GIF4](https://user-images.githubusercontent.com/20540872/63047128-12d7fd80-bed4-11e9-8c45-036ccb33fc97.gif)

- Shows the **lose** when the player was killed:

![GIF5](https://user-images.githubusercontent.com/20540872/63043291-38f99f80-becc-11e9-8234-765a402ab8f1.gif)

## Procedural generation

- Symmetrical regeneration for each new game:

![GIF6](https://user-images.githubusercontent.com/20540872/67123411-8659fc00-f1f0-11e9-8b71-f0b9072c34f8.gif)

- Scaling sides sizes in the editor or sizes selection in the start menu:

![GIF7](https://user-images.githubusercontent.com/20540872/63046685-45352b00-bed3-11e9-81f4-fea4fdf1f0c7.gif)

## Cells Data Structure

- Actors snapping to the center of the cell:

![GIF8](https://user-images.githubusercontent.com/20540872/63049470-0efaaa00-bed9-11e9-9f7d-9da1c16b69fd.gif)

- Searching of the nearest cell to an actor:

![GIF9](https://user-images.githubusercontent.com/20540872/63049762-ba0b6380-bed9-11e9-926f-2f82f621a130.gif)

## The Map Component

These components manage their owners and update this level actors in case of any changes on the map that allow to:

- Prepare in advance the level actors in the editor time:

_(Dragged from the Content Browser the wall, the character, the item, the box, and the bomb, that correctly exploded due
to Maps Components)_

![GIF10](https://user-images.githubusercontent.com/20540872/63053411-f5aa2b80-bee1-11e9-9328-79cf77609ec7.gif)

- Free location and rotation of the level map in the editor time:

![GIF11](https://user-images.githubusercontent.com/20540872/63057315-3f970f80-beea-11e9-979f-c7874042a382.gif)

## Artificial Intelligence

Bots behave like players with no use of the Unreal NavMesh:

- They can find items even around the corners and are able to manage their priorities correctly:

_(There are three items: A - the nearest under the bomb explosion, B - the item that is placed around the corner near
the enemy character. C - the farthest safe item. The bot does not risk and chooses to move to the C)_

![GIF12](https://user-images.githubusercontent.com/20540872/63061142-770ab980-bef4-11e9-9f34-d80e28fcbaaf.gif)

- Emergency Priority Change:

_(The bot runs for the item, the player otherwise set the bomb. Meanwhile, the bot changes direction and runs off the
bomb)_

![GIF13](https://user-images.githubusercontent.com/20540872/63061569-de753900-bef5-11e9-98dc-e12a57554dfc.gif)

- The bots even survive through in the midst of explosions:

![GIF14](https://user-images.githubusercontent.com/20540872/63062621-e46d1900-bef9-11e9-8e84-dbad3eb14dc6.gif)

- The editor preview visualization for selected bot automatically updates upon any changes on the map including addition
  from the Content Browser or drag-and-drop.

_Where is :_

_Green +: a safe crossway._

_Red +: crossway which has at least one enemy character._

_Yellow F: filtered cells for moving._

_Grey Х: the selected cell on which the bot moves to._

![GIF15](https://user-images.githubusercontent.com/20540872/63063848-aa524600-befe-11e9-93fb-ece39892ace5.gif)

## 🧑‍🤝‍🧑 Credits

- **Yevhenii Selivanov** - Programming - [GitHub](https://www.github.com/janseliv), [Telegram](https://t.me/JanSeliv)
- **Maksim Shashkov** - Level Design & Level Art - [Artstation](https://www.artstation.com/maksimshashkov)
- **Kateryna Shchetinina** - Characters & Animations - [Artstation](https://www.artstation.com/kateseliv)
- **Yevhenii Oksenchuk** - Game Design (Audio, UI, and Cinematics) - [Telegram](https://t.me/ComeThird)
- **Valeriy Rotermel** - [Progression System](https://github.com/h4rdmol/ProgressionSystem) - [GitHub](https://github.com/h4rdmol)
- **Anton Selivanov** - Foot Trails - [GitHub](https://github.com/antokior)

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
