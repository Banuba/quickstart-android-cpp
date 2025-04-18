## This sample is deprecated. Please, rely on [Java/Kotlin interfaces](https://github.com/Banuba/banuba-sdk-android-samples)

Quick start examples for integrating [Banuba SDK on Android](https://docs.banuba.com/face-ar-sdk-v1/android/android_overview) in C++ apps.
  
# Getting Started

1. Get the latest Banuba SDK archive for Android and the client token. Please fill in our form on [form on banuba.com](https://www.banuba.com/face-filters-sdk) website, or contact us via [info@banuba.com](mailto:info@banuba.com).
2. Copy `aar` and `include` files from the Banuba SDK archive into `libs` dir:
    `BNBEffectPlayer/include` => `quickstart-android-cpp/app/libs/include`
    `BNBEffectPlayer/banuba_effect_player-release.aar` => `quickstart-android-cpp/app/libs/banuba_effect_player-release.aar`
3. Copy and Paste your client token into appropriate section of `app/src/main/java/com/banuba/sdk/example/common/BanubaClientToken.kt`
4. Open the project in Android Studio and run the necessary target using the usual steps.

# Contributing

Contributions are what make the open source community such an amazing place to be learn, inspire, and create. Any contributions you make are **greatly appreciated**.

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request
