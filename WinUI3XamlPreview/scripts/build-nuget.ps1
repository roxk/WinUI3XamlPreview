param(
    [Parameter(Mandatory=$true)]
    [String]$version
)
$ErrorActionPreference = "Stop"
$template = Get-Content $PSScriptRoot\..\Nuget\WinUI3XamlPreview.nuspec.template
$nuspec = $template.Replace("VERSION", $version)
Set-Content -path $PSScriptRoot\..\Nuget\.nuspec -value $nuspec
. "$PSScriptRoot\get-msbuild"
nuget restore $PSScriptRoot\..\WinUI3XamlPreviewCs\WinUI3XamlPreviewCs.csproj
nuget restore $PSScriptRoot\..\WinUI3XamlPreview\WinUI3XamlPreview.vcxproj -PackagesDirectory $PSScriptRoot\..\packages
nuget restore $PSScriptRoot\..\WinUI3XamlPreview.DllLoader\WinUI3XamlPreview.DllLoader.vcxproj -PackagesDirectory $PSScriptRoot\..\packages
&$msbuild $PSScriptRoot\.. -t:WinUI3XamlPreview -p:Platform=x64,Configuration=Release
&$msbuild $PSScriptRoot\.. -t:WinUI3XamlPreviewCs -p:Platform="Any CPU" -p:Configuration=Release
&$msbuild $PSScriptRoot\.. -t:WinUI3XamlPreview_DllLoader -p:Platform=x64,Configuration=Release
# TODO: Fix ARM support. idlgen is attempting to use ARM64/idlgen.exe when the host is x64.
#&$msbuild $PSScriptRoot\..\WinUI3XamlPreview\WinUI3XamlPreview.vcxproj -p:Platform=ARM64,Configuration=Release
get-childitem $PSScriptRoot\..\x64\Release\WinUI3XamlPreview.DllLoader -exclude *.pdb | compress-archive -destinationpath $PSScriptRoot\..\Nuget\WinUI3XamlPreview.DllLoader.zip -force
nuget pack $PSScriptRoot\..\Nuget\.nuspec -outputDirectory $PSScriptRoot\..\Nuget
