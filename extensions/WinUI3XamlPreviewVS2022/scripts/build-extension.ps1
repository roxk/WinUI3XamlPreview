param(
    [boolean]$buildClang,
    [string]$config
)
$ErrorActionPreference = "Stop"

if (!($config -eq "Release" -or $config -eq "Debug")) {
	echo "Uknown config: $config. -config [Release|Debug]"
	exit 1
}

$srcDir = "$PSScriptRoot"
$solutionDir = "$srcDir\..\"
msbuild "${solutionDir}WinUI3XamlPreviewVS2022\WinUI3XamlPreviewVS2022.csproj" -restore
msbuild $solutionDir -t:WinUI3XamlPreviewVS2022 -p:Configuration=$config
