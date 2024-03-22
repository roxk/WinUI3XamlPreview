param(
    [Parameter(Mandatory=$true)]
    [String]$placeholder,
    [Parameter(Mandatory=$true)]
    [String]$value,
    [Parameter(Mandatory=$true)]
    [String]$file,
    [String]$out
)
if ($out -eq "") {
    $finalOut = $file
} else {
    $finalOut = $out
}
(Get-Content ($file)) |
    Foreach-Object { $_ -replace "$placeholder", $value } |      
    Set-Content ($finalOut)
