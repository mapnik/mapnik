$ErrorActionPreference = 'Stop'
$pattern=$args[0]
$version='0'
Get-Content .\appveyor.yml |
    foreach {
        if ($_ -match $pattern){ $version = $_.split()[-1] }
    }
Write-Host $version
