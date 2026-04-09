<a href="https://github.com/JanSeliv/DataAssetsLoader/blob/main/LICENSE">![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)</a>
<a href="https://www.unrealengine.com/">![Unreal Engine](https://img.shields.io/badge/Unreal-5.7-dea309?style=flat&logo=unrealengine)</a>

<br/>
<p align="center">
<a href="https://github.com/JanSeliv/DataAssetsLoader">
<img src="https://github.com/JanSeliv/DataAssetsLoader/blob/main/Resources/Icon128.png?raw=true" alt="Logo" width="80" height="80">
</a>
<h3 align="center">📦 Data Assets Loader</h3>
<p align="center">
Auto-load and access any data asset from anywhere
<br/>
<br/>
<a href="https://discord.gg/jbWgwDefnE"><strong>Join our Discord ››</strong></a>
<br/>
<a href="https://github.com/JanSeliv/DataAssetsLoader/releases">Releases</a>
</p>

## 🌟 About

**Data Assets Loader** is a plugin that:
- automatically discovers, asynchronously loads, and unloads Primary Data Assets in your project via the Asset Manager, providing instant global access by class from both C++ and Blueprints
- Manages Data Registry consumer lifecycle so any Data Table can be injected externally (from plugins, DLC, Modular Game Features) while the project side stays boilerplate-free pulling data from Data Registry.

### Data Assets
- **Zero-Config Loading**: Data assets self-register via `PostLoad`, no manual setup or soft references needed.
- **Global Access by Class**: Retrieve any data asset instantly with `UDalSubsystem::GetDataAsset<UMyDataAsset>()`.
- **One-Shot Listeners**: Subscribe to a data asset class and get notified as soon as it is loaded, with automatic weak object safety.
- **Batch Listeners**: Wait for multiple data asset classes at once and fire a single callback when all are loaded.
- **Modular Game Features Support**: Automatically discovers and loads data assets from Game Feature plugins.

![DataAssetsLoader](https://github.com/user-attachments/assets/979e5393-995c-429b-8c43-0e029f22b230)

### Data Registries
- **Owner-Based Binding**: Subscribe to Data Registries via `UDalRegistrySubsystem` by owner pointer, auto async-loads soft references and fires callback on cache changes.
- **MGF Integration**: Modular Game Feature plugins inject their own Data Tables into registries; consumers bind once and automatically pick up new data when features activate.

![DataRegistries](https://github.com/user-attachments/assets/af317698-4e22-46c6-8cb6-92e881a8acee)

## 🎓 Sample Projects

Explore this [game project repository](https://github.com/JanSeliv/Bomber) to view the Data Assets Loader in action.

## 📅 Changelog
#### 2026-XX-XX
- [Editor] Listen node now has a Failed exec output that fires when the data asset unable to load
#### 2026-02-23
- 🎉 Initial public release on Unreal Engine 5.7

## 📫 Feedback & Contribution

Feedback and contributions from the community are highly appreciated!

If you'd like to contribute, please fork the project and create a pull request targeting the `develop` branch.

If you've found a bug or have an idea for a new feature, please open a new issue on GitHub or join our [Discord](https://discord.gg/jbWgwDefnE). Thank you!

## 📜 License

This project is licensed under the terms of the MIT license. See [LICENSE](LICENSE) for more details.

We hope you find this plugin useful and we look forward to your feedback and contributions.