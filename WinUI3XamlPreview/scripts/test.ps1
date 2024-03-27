. "$PSScriptRoot\get-msbuild"
&$msbuild $PSScriptRoot\..\TestPacakged\TestPackaged.csproj -t:Restore
&$msbuild $PSScriptRoot\.. -t:TestPackaged -p:Platform=x64,Configuration=Debug
