# 💣 About the Bomber Project

Bomber is an open-source indie game developed on Unreal Engine 5 for Windows and MacOS, offering fast-paced, bomb-laying action where the objective is to be the last one standing.

## Table of Contents

- [💣 About the Bomber Project](#-about-the-bomber-project)
- [🚀 Getting Started](#-getting-started)
- [🛠 Key Features](#-key-features)
- [💾 Play the Build](#-play-the-build)
- [📋 Kanban Board](#-kanban-board)
- [📅 Changelog](#-changelog)
- [🎮 Overview](#-overview)
- [🧑‍🤝‍🧑 Content Creation Team](#-content-creation-team)
- [📫 Feedback & Contribution](#-feedback--contribution)
- [📜 License](#-license)

## 🚀 Getting Started

This project uses **submodules**, downloaded via SSH. If you haven't set up SSH or skip submodules, you'll find empty folders in `Bomber\Plugins` and encounter [issues](https://github.com/JanSeliv/Bomber/issues/64) when running the project.

To clone the project properly, run:
```sh
git clone --recurse-submodules git@github.com:JanSeliv/Bomber.git
```
If already cloned without submodules, run:
```sh
git submodule update --init --recursive
```

## 🛠 Key Features

This project could be useful for learners, demonstrating next features:

- Multiplayer support
- Enhanced Input support
- Modular Game Features support
- Complex cinematics
- Procedurally generated level
- Challenging AI

Despite this project is fully written in C++, it's extremely **blueprint-friendly**:

- **Data-Driven Design**: No hardcoded values. All data can be tweaked via Data Assets in editor as well as accessed in blueprints [[doc](https://trello.com/c/HGscMUdK)].
- **Fully Exposed**: Every class, property, and function is exposed to Blueprints allowing for heavy changes the logic with no code.
- **Well-Commented**: Every class, property and function is well-commented for easy understanding.
- **Utility Libraries**: Core static functions are accessible globally like Cell Utils [[doc](https://trello.com/c/b2IzcOhg)]. See more in the `Source\UtilityLibraries` [folder](https://github.com/JanSeliv/Bomber/tree/master/Source/Bomber/Public/UtilityLibraries).

## 💾 Play the Build

To download and play the build, visit [GitHub Releases](https://github.com/JanSeliv/Bomber/releases/) or [GDrive](https://drive.google.com/open?id=1oxBUQwnQX322IxQUK8Y6A-L09WompiGi).

Want to test develop branch? [Message me](https://t.me/JanSeliv) for a Steam key.

## 📋 Kanban Board

Stay updated with the current progress and plans on the [Trello board](https://trello.com/b/1jbKvyeh/bomber-kanban).

## 📅 Changelog
####
- Updated to **Unreal Engine 5.3**.
- **New Main Menu** with completely different UI and complex cinematics on starting the game (only Hugo and Fori):
> ![NewMainMenu](https://github.com/JanSeliv/Bomber/assets/20540872/9c960fa4-6760-4298-a55b-54d0cb8a0b13)
#### `12.06.2023`
- Updated to **Unreal Engine 5.2**.
- Added **MacOS** support.
- Added **Ultra-wide** resolutions support.
- Extracted logic into plugins, so other developers can benefit from it in their projects:
-  [⚙️ Settings Widget Constructor](https://github.com/JanSeliv/SettingsWidgetConstructor)
-  [🔘️ Custom Shape Button](https://github.com/JanSeliv/CustomShapeButton)
-  [🔄 Pool Manager](https://github.com/JanSeliv/PoolManager)
-  [🎭 Morphs Player](https://github.com/JanSeliv/MorphsPlayer)
-  [ƒ Function Picker](https://github.com/JanSeliv/FunctionPicker)
-  [\>_ Meta Cheat Manager](https://github.com/JanSeliv/MetaCheatManager)
- Added Foot Trails for the Maya level as `Modular Game Feature`:
>  <img width="560" alt="image" src="https://github.com/JanSeliv/Bomber/assets/20540872/a77c2e38-4fd6-4a04-988e-05d9613bd97e">
- New power-ups meshes for the Maya level (shown from left to right: move speed, bomb length, bomb quantity):
>  <img width="360" alt="image" src="https://github.com/JanSeliv/Bomber/assets/20540872/1e526fda-e51a-479c-b541-acccc8457725">
- Added new cheats such as: `Bomber.Level.SetSize 9x7` (find more on the [Bomber cheats page](https://trello.com/c/5PiHt7Ah/308-bomber-cheats))
- Updated Main-Menu background music for all levels.
--- 
#### `31.05.2022:`
 - Added initial **multiplayer** support for 4 players (without Steam now, use 'Open' command to connect to each other).
 - Created the **Pool Manager** for the generated level to avoid spawning and destroying actors on each level reconstruction.
 - Added new **SteelMan** character for AI players with 3 different skins.
>  ![SteelMan_31-05-22](https://user-images.githubusercontent.com/20540872/171299202-3422db3c-7061-4b75-b51c-a08a67d65ab5.gif)
 ---
#### `31.12.2021:`
 - The game migrated to the **Unreal Engine 5**.
 - Added sounds (background music, UI, in-game sounds) and sliders to tweak volumes in Settings Audio tab (Master, Music and SFX).
 - Added Controls tab in Settings to allow player remap input keys.
>  ![Settings_31-12-21](https://user-images.githubusercontent.com/20540872/147825296-ce7d33da-dfda-4757-b070-bfd08f700134.jpg)
 ---
#### `03.06.2021:`
 - Completely updated the Maya level.
 - Added the Water level of the Pirate character.
 - Added settings.
>  ![](https://user-images.githubusercontent.com/20540872/120249537-8bf83e80-c27b-11eb-81be-583e8c30aa62.jpg)
>  ![](https://user-images.githubusercontent.com/20540872/120249541-8e5a9880-c27b-11eb-82cd-660878d33e6f.jpg)
>  ![](https://user-images.githubusercontent.com/20540872/120127584-0e232d00-c1c0-11eb-8467-74633600c180.jpg)
 ---
#### `31.01.2021:`
 - Added the Maya (Sphynx cat) character with two skins.
 - Added the Pirate character with one skin.
 - Fori and Hugo characters got additional second skins.
 > ![](https://user-images.githubusercontent.com/20540872/106404153-23ff2c00-6432-11eb-8cb1-d3a7bc33b51b.gif)
 ---
#### `31.10.2020:` Added the third Level Map.
 > ![](https://user-images.githubusercontent.com/20540872/97792191-2d7ebb00-1bdb-11eb-9a27-c50d64394caa.jpg)
 ---
#### `25.10.2020:`, developed the new UI prototype.
 - Added the second City Map.
 - Added the Hugo character.
 - Added the Fori character.
 > ![](https://user-images.githubusercontent.com/20540872/97118032-125a0a00-1708-11eb-8256-4bec419b1d48.gif)
 ---
#### `15.10.2019:` Uploaded first game-ready build.

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

## 🧑‍🤝‍🧑 Content Creation Team

- **Level Design and Level Art** by Maksim Shashkov: [Visit Artstation Profile](https://www.artstation.com/maksimshashkov)
- **Characters and Animations (Gameplay and Cinematics)** by Kateryna Shchetinina: [Visit Artstation Profile](https://www.artstation.com/kateseliv)
- **Game Design (including Audio, UI, and Cinematics)** by Yevhenii Oksenchuk: [Contact in Telegram](https://t.me/ComeThird)

## 📫 Feedback & Contribution

Feedback and contributions from the community are highly appreciated!

If you'd like to contribute, please fork the project and create a pull request targeting the `develop` branch.

If you've found a bug or have an idea for a new feature, please open a new issue on GitHub. Thank you!

## 📜 License

This project is licensed under the terms of the MIT license. See [LICENSE](LICENSE) for more details.

We hope you find this plugin useful and we look forward to your feedback and contributions.
