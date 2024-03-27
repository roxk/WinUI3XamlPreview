$formatCheck = "$PSScriptRoot\format.ps1"
$test = "$PSScriptRoot\test.ps1"

function run {
	param([ScriptBlock]$func)
	&$func
	if ($LASTEXITCODE -ne 0) {
		exit 1
	}
}

run -func {
	&$formatCheck -check
}
run -func {
	&$test
}
