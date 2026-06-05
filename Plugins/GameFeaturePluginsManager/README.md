<a href="https://github.com/JanSeliv/GameFeaturePluginsManager/blob/main/LICENSE">![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)</a>
<a href="https://www.unrealengine.com/">![Unreal Engine](https://img.shields.io/badge/Unreal-5.7-dea309?style=flat&logo=unrealengine)</a>

<br/>
<p align="center">
<a href="https://github.com/JanSeliv/GameFeaturePluginsManager">
<img src="https://github.com/JanSeliv/GameFeaturePluginsManager/blob/main/Resources/Icon128.png?raw=true" alt="Logo" width="80" height="80">
</a>
<h3 align="center">🧩 Game Feature Plugins Manager</h3>
<p align="center">
Load and swap Game Feature Plugins automatically by gameplay tags
<br/>
<br/>
<a href="https://discord.gg/jbWgwDefnE"><strong>Join our Discord ››</strong></a>
<br/>
<a href="https://github.com/JanSeliv/GameFeaturePluginsManager/releases">Releases</a>
</p>

## 🌟 About

**Game Feature Plugins Manager** extends Unreal Engine's Modular Game Features, making Game Feature Plugins (GFP) load, unload, and swap themselves automatically instead of being toggled by hand.

- **Tag-Driven Activation**: A plugin declares which gameplay tags switch it on. Once the tag appears on the game state the plugin loads, and once it is gone the plugin unloads, with no manual activation calls.
- **Feature Chaining**: An active plugin can apply tags that switch other plugins on, so one feature triggers another. Exclusive tags make sibling features replace each other, for example selecting Map A turns Map B off.
- **Runtime In Every Mode**: Features activate, swap, and unload reliably during play across editor preview worlds, PIE, `-game`, and cooked builds, with each server and client managing its own features independently.
- **Plugins Switcher**: A built-in panel lists every registered Game Feature Plugin with an on/off toggle that redirects to the same activation API. It is shown as an editor tab docked next to the Scene Outliner by default, and can additionally be spawned in a shipping game.

<p align="center">
<img width="312" height="458" alt="Game Feature Plugins switcher editor tab, also spawnable in a shipping game" src="https://github.com/user-attachments/assets/8ade0942-7003-4f50-8729-00ce41b3ef99">
</p>

## 🎓 Sample Projects

Explore this [game project repository](https://github.com/JanSeliv/Bomber) to view the Game Feature Plugins Manager in action.

## 📅 Changelog
#### 2026-XX-XX
- 🎉 Initial public release on Unreal Engine 5.7

## 📫 Feedback & Contribution

Feedback and contributions from the community are highly appreciated!

If you'd like to contribute, please fork the project and create a pull request targeting the `develop` branch.

If you've found a bug or have an idea for a new feature, please open a new issue on GitHub or join our [Discord](https://discord.gg/jbWgwDefnE). Thank you!

## 📜 License

This project is licensed under the terms of the MIT license. See [LICENSE](LICENSE) for more details.

We hope you find this plugin useful and we look forward to your feedback and contributions.