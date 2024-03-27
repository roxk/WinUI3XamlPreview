# WinUI 3 Xaml Preview (WinUI3XP)

[![nuget](https://img.shields.io/nuget/v/WinUI3XamlPreview)](https://www.nuget.org/packages/WinUI3XamlPreview/)
[![CI](https://github.com/roxk/WinUI3XamlPreview/actions/workflows/ci.yaml/badge.svg)](https://github.com/roxk/WinUI3XamlPreview/actions/workflows/ci.yaml)

WinUI3 Xaml Preview allows you to see a a live preview of your UI while authoring XAML. It is fast, reliable, and comes with basic tools like scaling and size configuration.

Featurs:
- Interactive preview of your XAML code (powered by `XamlReader`)
- Integration with VS2022 17.9+
- Reload via opening/saving/switching
- Supports your own user control and custom control
- Supports controls in dependency (i.e. libraries)

Limitation:
- Require building the project once before use
- Packaged apps require launching once after build, before use
- Require a few lines of setup
- Render errors are currently not shown. If you are not seeing your code updated, there is probably an error in your XAML. Check the code or build the project again

The author does not believe in Designer so this project would stay only as a live preview. If you want a designer, fork this project and create your own.

Otherwise, contributions are welcome! :D

## Getting Started

1. Install the [VS extension](https://marketplace.visualstudio.com/items?itemName=Roxk.winui3xamlpreview). Require VS2022 17.9+

2. On _each of your WinUI 3 project_ that uses this feature, install the nuget

```
nuget install WinUI3XamlPreview
```

3. If you are using C++, add `<winrt/WinUI3XamlPreview.h` to `pch.h`. C# users can go to the next step.

4. Make changes to the start of your `App.OnLaunched` method accoridng to the following diff:

### C++
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

### C#

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
