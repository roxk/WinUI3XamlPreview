$ErrorActionPreference = "Stop"

. "$PSScriptRoot\get-msbuild"
nuget restore $PSScriptRoot\..\TestDep\TestDep.vcxproj -PackagesDirectory $PSScriptRoot\..\packages
nuget restore $PSScriptRoot\..\WinUI3XamlPreview\WinUI3XamlPreview.vcxproj -PackagesDirectory $PSScriptRoot\..\packages
nuget restore $PSScriptRoot\..\TestPackaged\TestPackaged.vcxproj -PackagesDirectory $PSScriptRoot\..\packages
nuget restore $PSScriptRoot\..\WinUI3XamlPreview.DllLoader\WinUI3XamlPreview.DllLoader.vcxproj -PackagesDirectory $PSScriptRoot\..\packages
&$msbuild $PSScriptRoot\.. -t:TestPackaged -p:Platform=x64,Configuration=Debug
&$msbuild $PSScriptRoot\.. -t:WinUI3XamlPreview_DllLoader -p:Platform=x64,Configuration=Debug
