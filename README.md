# WinUI 3 Xaml Preview (WinUI3XP)

[![nuget](https://img.shields.io/nuget/v/WinUI3XamlPreview)](https://www.nuget.org/packages/WinUI3XamlPreview/)
[![VS2022](https://img.shields.io/visual-studio-marketplace/v/Roxk.winui3xamlpreview.svg?label=Visual%20Studio%202022%20(Preview))](https://marketplace.visualstudio.com/items?itemName=Roxk.winui3xamlpreview)
[![CI](https://github.com/roxk/WinUI3XamlPreview/actions/workflows/ci.yaml/badge.svg)](https://github.com/roxk/WinUI3XamlPreview/actions/workflows/ci.yaml)

![Image showcasing how to use WinUI 3 Xaml Preview](hero-banner.gif)

WinUI3 Xaml Preview allows you to see a a live preview of your UI while authoring XAML. It is fast, reliable, and comes with basic tools like scaling and size configuration.

Contributions are welcome! :D

## Features
- Interactive preview of your XAML code (powered by `XamlReader`)
- Integration with VS2022 17.9+
- Reload via opening/saving/switching
- Supports your own user control and custom control
- Supports controls in dependency (i.e. libraries)

Custom control's template previewing is not supported yet but is planned. Stay tuned.

## Language/Feature Support Table
|| App | WinRT Component (Dynamic) | WinRT Component (Static) |
|--|--|--|--|
|C++|✔️|✔️|❌|
|C#|✔️|❌ (blocked by cswinrt)|❌|

## Limitation
- Require building the project once before use
- Packaged apps require being deployed before use
- App project require a few lines of setup
- WinRT component project require some setup for any nuget/external dependencies

### WinRT Component Limitation

The previewer tries its best to resolve your WinRT component's dynamic _project_ dependency and load them before use. This feature is known as Auto DLL Loading. Auto DLL Loading only works for project references whose output is a dynamic library. If your WinRT component has any _external_ dynamic dependencies, e.g. nuget/external dll dependencies (transitive or direct), you'd need to copy them to your dll's output folder so the system can pick them up.

### Non-Goal

The author does not believe in Designer so this project would stay only as a live preview. If you want a designer, fork this project and create your own.

## Getting Started

1. Install the [VS extension](https://marketplace.visualstudio.com/items?itemName=Roxk.winui3xamlpreview). Require VS2022 17.9+

2. On _each of your WinUI 3 project_ that uses this feature, install the nuget

```
nuget install WinUI3XamlPreview
```

3. If you are using C++, add `<winrt/WinUI3XamlPreview.h` to `pch.h`. C# users can go to the next step.

Note that the next steps for app project and WinRT component project are different.

### App Project

4. Make changes to the start of your `App.OnLaunched` method accoridng to the following diff:

#### C++
```diff
- void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
+ winrt::fire_and_forget App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
+ {
+     if (co_await WinUI3XamlPreview::Preview::IsXamlPreviewLaunched())
+     {
+         co_return;
+     }
      // Your code...

```

#### C#

```diff
- protected override void OnLaunched(Microsoft.UI.Xaml.LaunchActivatedEventArgs args)
+ protected override async void OnLaunched(Microsoft.UI.Xaml.LaunchActivatedEventArgs args)
+ {
+     if (await WinUI3XamlPreview.Preview.IsXamlPreviewLaunched())
+     {
+         return;
+     }
      // Your code...
```

5. If you are developing non-packaged app, you have finished the setup. If you are developing packaged app, make changes to your package manifest according to the following diff:

6. Viola! You have finished the setup for app project

```diff
   <Applications>
     <Application>
+      <Extensions>
+        <uap5:Extension
+          Category="windows.appExecutionAlias">
+          <uap5:AppExecutionAlias>
+            <uap5:ExecutionAlias Alias="$YourTargetName-WinUI3XP.exe" />
+          </uap5:AppExecutionAlias>
+        </uap5:Extension>
+      </Extensions>
     </Application>
   </Applications>
```

Make sure to replace `$YourTargetName` with your build output name. E.g. if your build output is `MyApp.exe`, the correction execution alias to add is `MyApp-WinUI3XP.exe`.

### WinRT Component Project

4. If your project has no dependency/only project-reference dependencies, you have finished the setup. If you have any dynamic nuget/external dependencies, make sure they are copied to the output folder.

For example, suppose your project is `LibA` inside `Lib` folder with `Lib.sln`. With default output directory settings, you should copy any dll dependencies to `Lib\x64\Debug\LibA` if you want to preview `LibA` in `x64|Debug` configuration.

5. Viola! You have finished the setup for WinRT component project.

## Usage

1. After setup, build your project again.
2. For packaged app, deploy your app once by righting click on the project -> deploy.
3. Locate the "Toggle WinUI 3 Xaml Preview" in the "Extensions" menu at the top.
4. Open any xaml file and click the toggle to launch preview, click it again to stop. You can also close the preview window to turn off live preview.

The preview renders according to the following triggers:
- Opening a XAML file
- Switching to a XAML file in the editor
- Saving a XAML file

## Technical Details

TODO
