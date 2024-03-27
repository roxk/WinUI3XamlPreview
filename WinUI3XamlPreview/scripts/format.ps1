param(
	[switch]$check
)

$setup = "$PSScriptRoot\setup.ps1"
&$setup
$passive = ""
if ($check.IsPresent) {
	$passive = "-passive"
}
$files = get-childitem -r $PSScriptRoot\.. | where { $_.FullName -notMatch 'Generated Files|x64|ARM64|obj|bin|packages' } | where { $_.FullName -Match '.xaml$' }
foreach ($file in $files) {
	dotnet xstyler -r -f $file.FullName $passive
}
