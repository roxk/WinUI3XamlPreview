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
- Supports packaged (single project, or with WAP project) or unpackaged app
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

### Packaged App with WAP (Windows Application Package) project Support

The VS2022 extension would automatically find the WAP project that packages your app. If you have multiple WAP packages packaging the same app, the behavior is undefined. In this scenario, it's recommended you use the `WinUI3XPLaunchProject` property to specify which project to launch. See [Configuration](#Configuration) section for more details.

### WinRT Component Support - Direct Mode and Sample App Mode

WinRT Component preview support comes in two mode. One is "Direct" mode, another is "Sample App" mode. Direct mode means you can preview directly in-project. Sample App mode means you preview via a separate "Sample App" project.

It's suggested you use Sample App mode for reliability, however Direct mode is also supported for a quicker setup.

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

### WinRT Component Project (Sample App Mode)

1. Install the [VS extension](https://marketplace.visualstudio.com/items?itemName=Roxk.winui3xamlpreview). Require VS2022 17.9+

2. On your WinUI 3 WinRT Component's _Sample App project_ that uses this feature, follow the steps in [App project](#AppProject) to set it up.

3. On your _WinUI 3 WinRT Component project_, add `<WinUI3XPLaunchProject>$SampleAppProjectName</WinUI3XPLaunchProject>` property. Make sure you replace `$SampleAppProjectName` with the name of the sample app project. E.g. if your WinRT Component's project name is `Lib`, your sample app (test app)'s project name is `LibTest`, the correct property value is `LibTest`. If you do not want to edit project file directly, you can (1) install the nuget on your _WinUI 3 WinRT Component project_, (2) edit the property by right clicking the component project in Visual Studio -> Properties -> WinUI3XamlPreview tab -> edit Launch Project. Make sure you disable the preview to save compile time. See [Configuration](#Configuration) section for more details

4. Viola! You have finished the setup for WinRT Component proejct Sample App mode.

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

WinUI3XamlPreview is possible thanks to the following facts:

1. `XamlReader` is powerful enough to read it any valid XAML (sans limitation, see below)
2. WASDK supports activation redirection
3. Windows support passing protocol directly to an app (instead of just shell executing the protocol)
4. WinUI 3 supports creating window manually (at least trivially)

### Challenges

#### Reliability

##### Dependencies

A complete XAML preview feature should be able to preview any valid XAML as if it has been compiled and shipped with the app. One limitation in the UWP's built-in support of preview is that it doesn't load any non-WinRT DLL. That is, if your control relies on any plain C/C++/C# library, the preview simply gives up and throws exception. 

The built-in preview has this limitation because it's building another app behind the scene and dynamically copy-and-pasting WinRT DLLs next to that app. _Any_ approach that tries to use another app to load XAML would have this issue. In fact, WinUI3XamlPreview's WinRT Component Direct mode has this exact issue. To solve this issue, WinUI3XamlPreview uses an _intrusive_ approach.

UWP's built-in preview is _nonintrusive_. That is, it tries to work without any user intervention. It doesn't inject code to your app [1], doesn't require users to configure anything. The upside is zero-effort from the user: the preview app is transparent to user's own app. The downside is there is a limit with this approach in terms of robustness, as described above. 

WinUI3XamlPreview is _intrusive_. It runs code in your app, requires some minimal setup for protocol activation. The downside is the app now needs to be aware of the preview functionality, users might need to conditionally disable/enable it. But the upside is a guarantee that if your app can load the XAML, the preview can preview it. Complete robustness and reliability. WinUI3XamlPreview explicitly choose reliability over zero-effort-from-user.

Since the downside of WinUI3XamlPreview is UX, plenty of solutions are available. We can borrow react native's solution of providing a wrapper application that handles the protocol activation. Or we can provide a set of templates that handles the minimal setup for the user. The fact that the preview runs in user's app remains, but these solution effectively achieves zero effort from the user.

##### XAML

`XamlReader` while powerful, comes with its own set of limitation. WinUI3XamlPreview essentially has to trim XAML of unsupported XAML before feeding to `XamlReader`.

The obvious offenders are anything `x:` related. Attributes with `x:` prefix are for compiled-binding, and obviously wouldn't work in `XamlReader`.

Another is event binding. The doc says XAML shouldn't contain any event binding, and in early test it's found that event binding are simply silently ignored. But as development unfolds, WinUI3XamlPreview had encountered bugs where binding of a seemingly innocent event would cause `XamlReader` to throw. So it turns out any custom event would cause `XamlReader` to freak out.

WinUI3XamlPreview tried to be clever and calls into `XamlMetadataProvider` to see if an event is custom, but it turns out there is no way to grab built-in control's provider, and even built-in control has unsupported events. To combat this, the solution is to parse each element and attribute _one by one_. If the attribute causes `XamlReader` to throw, it's discarded, otherwise it's kept.

One last piece of the puzzle comes from the fact to support control templates, we have to parse styles and find control template names and style names. So it turns out `Setter` cannot be used standalone and must be embeded as children of `Style` or `VisualStateManager`. Handling these exceptions make the parsing code feel more arbitrary, but is currently manageable.

I'd like to mention a random limitation related to control template support: `Resources.Uri` doesn't support file path. It only supports `ms-appx` Uri. This makes control template rendering needlessly boilerplate-y since WinUI3XamlPreview now have to manually load the resource dictionary, parse the XML, manipulate the XML tree, re-insert the dictionary to the tree containing the control, and so on.

But with all these efforts, the result is a fool-proof `XamlReader`. It seems WinUI3XamlPreview's trimmer sometimes prune valid attributes (e.g. in control templates), but the foundation should be reliable enough that fixing these doesn't require more ad-hoc element/attribute-specific handling.
