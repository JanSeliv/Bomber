# About the Bomber Project

I would like to introduce my indie-game "Bomber". This game is lasting in fast speed rhythm. The mechanic of it is quite
easy but anyway, the game is worth to try. The last alive character is the winner. The gameplay includes strategically
placing down bombs to explode enemies and obstacles, power-ups gathering that help player to set multiple bombs at a
time, or explode bombs larger or become faster with speed item. The player dies if get caught up by enemies' bombs'
explosion including their own. The procedural generation of a level map with a given size makes the game a little more
diverse. Player competes with bots that will do everything but not let them win. The first rule is just to be the best
bomber and do not let bots to bomb player!

> **To download and play the game go to the [LINK](https://drive.google.com/open?id=1oxBUQwnQX322IxQUK8Y6A-L09WompiGi).**

## Content Team

> Levels created by Maksim Shashkov: [artstation.com/maksimshashkov](https://www.artstation.com/maksimshashkov)
>
> Characters created by Kateryna Selivanova: [artstation.com/kateseliv](https://www.artstation.com/kateseliv)

## Changelog

> `31.01.2021:`
>   - Added the Maya (Sphynx cat) character with two skins.
>   - Added the Pirate character with one skin.
>   - Fori and Hugo characters got additional second skins.
>   > ![](https://user-images.githubusercontent.com/20540872/106404153-23ff2c00-6432-11eb-8cb1-d3a7bc33b51b.gif)
>   ---
>   - `31.10.2020:` Added the third Level Map.
>   > ![](https://user-images.githubusercontent.com/20540872/97792191-2d7ebb00-1bdb-11eb-9a27-c50d64394caa.jpg)
>   ---
>   `25.10.2020:`, developed the new UI prototype.
>   - Added the second City Map.
>   - Added the Hugo character.
>   - Added the Fori character.
>   > ![](https://user-images.githubusercontent.com/20540872/97118032-125a0a00-1708-11eb-8256-4bec419b1d48.gif)
>   ---
>   - `15.10.2019:` Uploaded the game-ready build.
>

# Features

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

