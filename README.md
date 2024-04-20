# WinUI 3 Xaml Preview (WinUI3XP)

[![nuget](https://img.shields.io/nuget/v/WinUI3XamlPreview)](https://www.nuget.org/packages/WinUI3XamlPreview/)
[![VS2022](https://img.shields.io/visual-studio-marketplace/v/Roxk.winui3xamlpreview.svg?label=Visual%20Studio%202022%20(Preview))](https://marketplace.visualstudio.com/items?itemName=Roxk.winui3xamlpreview)
[![CI](https://github.com/roxk/WinUI3XamlPreview/actions/workflows/ci.yaml/badge.svg)](https://github.com/roxk/WinUI3XamlPreview/actions/workflows/ci.yaml)

![Image showcasing how to use WinUI 3 Xaml Preview](hero-banner.gif)

WinUI3 Xaml Preview allows you to see a a live preview of your UI while authoring XAML. It is fast, reliable, and comes with basic tools like scaling and size configuration.

Contributions are welcome! :D

## Feature
- Interactive preview of your XAML code (powered by `XamlReader`)
- Integration with VS2022 17.9+
- Reload via opening/saving/switching
- Supports your own user control and custom control
- Supports controls in dependency (i.e. libraries)
- Supports control template previewing with control+style dropdown
- Supports DLL project (directly, or with a separate Sample App project)

## Feature/Language Support Table
||C++|C#|
|--|--|--|
| App | ✔️ | ✔️ |
| App packaged with WAP project | ✔️ | ✔️ |
| Dynamic WinRT Component (via separate Sample App project\*) | ✔️ | ✔️ |
| Static WinRT Component (via separate Sample App project\*) | ✔️ | ✔️ |
|Dynamic WinRT Component (Direct\*) | ✔️ | ❌ (blocked by cswinrt)|
|Static WinRT Component (Direct\*) | ❌ | ❌ |

\*See section below on Direct vs Sample App

### WinRT Component Support - Direct Mode and Sample App Mode

WinRT Component preview support comes in two mode. One is "Direct" mode, another is "Sample App" mode. Direct mode means you can preview directly in-project. Sample App mode means you preview via a separate "Sample App" project.

## Limitation
- Require building the project once before use
- Packaged apps require being deployed before use
- App project require a few lines of setup
- WinRT component project require some setup for any nuget/external dependencies

### WinRT Component Direct Mode's Limitation

The previewer tries its best to resolve your WinRT component's dynamic _project_, or _internal_ dependency and load them before use. This feature is known as Auto DLL Loading. Auto DLL Loading only works for project references whose output is a dynamic library.

If your WinRT component has any _external_ dynamic dependencies, e.g. nuget/external dll dependencies (transitive or direct), you'd need to copy them to your dll's output folder so the system can pick them up.

If your library has a complex dependency graph, it is suggested that you switch to Sample App mode.

### Non-Goal

The author does not believe in Designer so this project would stay only as a live preview. If you want a designer, fork this project and create your own.

## Getting Started

### App Project

1. Install the [VS extension](https://marketplace.visualstudio.com/items?itemName=Roxk.winui3xamlpreview). Require VS2022 17.9+

2. On your _WinUI 3 App project_ that uses this feature, install the nuget

```
nuget install WinUI3XamlPreview
```

3. If you are using C++, add `<winrt/WinUI3XamlPreview.h` to `pch.h`. C# users can go to the next step.

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

6. Viola! You have finished the setup for app project

### WinRT Component Project (Direct Mode)

1. Install the [VS extension](https://marketplace.visualstudio.com/items?itemName=Roxk.winui3xamlpreview). Require VS2022 17.9+

2. On your _WinUI 3 WinRT Component project_ that uses this feature, install the nuget

```
nuget install WinUI3XamlPreview
```

3. If you are using C++, add `<winrt/WinUI3XamlPreview.h` to `pch.h`. C# users can go to the next step.

4. If your project has no dependency/only project-reference dependencies, you have finished the setup. If you have any dynamic nuget/external dependencies, make sure they are copied to the output folder.

For example, suppose your project is `LibA` inside `Lib` folder with `Lib.sln`. With default output directory settings, you should copy any dll dependencies to `Lib\x64\Debug\LibA` if you want to preview `LibA` in `x64|Debug` configuration.

5. Viola! You have finished the setup for WinRT Component proejct Direct mode.

### WinRT Component Project (Sample App Mode)

1. Install the [VS extension](https://marketplace.visualstudio.com/items?itemName=Roxk.winui3xamlpreview). Require VS2022 17.9+

2. On your WinUI 3 WinRT Component's _Sample App project_ that uses this feature, install the nuget

```
nuget install WinUI3XamlPreview
```

3. If you are using C++, add `<winrt/WinUI3XamlPreview.h` to `pch.h` of your _Sample App Project_. C# users can go to the next step.

4. On your _WinUI 3 WinRT Component project_, add `<WinUI3XPLaunchProject>$SampleAppProjectName</WinUI3XPLaunchProject>` property. Make sure you replace `$SampleAppProjectName` with the name of the sample app project. E.g. if your WinRT Component's project name is `Lib`, your sample app (test app)'s project name is `LibTest`, the correct property value is `LibTest`. If you do not want to edit project file directly, you can (1) install the nuget on your _WinUI 3 WinRT Component project_, (2) edit the property by right clicking the component project in Visual Studio -> Properties -> WinUI3XamlPreview tab -> edit Launch Project. Make sure you disable the preview to save compile time. See [Configuration](#Configuration) section for more details

5. Viola! You have finished the setup for WinRT Component proejct Sample App mode.

## Usage

1. After setup, build your project again.
2. For packaged app, deploy your app once by righting click on the project -> deploy. If you are using WinRT Component Sample App mode, make sure you build/deploy your sample app project instead of just the component project. 
3. Locate the "Toggle WinUI 3 Xaml Preview" in the "Extensions" menu at the top.
4. Open any xaml file and click the toggle to launch preview, click it again to stop. You can also close the preview window to turn off live preview.

The preview renders according to the following triggers:
- Opening a XAML file
- Switching to a XAML file in the editor
- Saving a XAML file

## Configuration

![WinUI3XamlPreview's property page in Visual Studio](vs-property-page.png)

You can configure the behavior of WinUI3XamlPreview via a dedicated property page in Visual Studio. If you are comfortable editing project file directly, you can also insert/modify properties listed below.

|MSBuild Property|type|Behavior|
|--|--|--|
|`WinUI3XPEnabled`|`Bool`|`true` to enable WinUI3XamlPreview, `false` to disable. Default is true. When it's disabled, no winmd/dll are added to the project. Useful for CI/CD|
|`WinUI3XPLaunchProject`|`String`|Default is null. If specified, instead of using the XAML file's project to locate the preview app, use the specified project instead|

## Technical Details

TODO
