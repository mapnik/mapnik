$ErrorActionPreference = 'Stop'
$boost_version='0'
Get-Content .\appveyor.yml |
    foreach { 
        if ($_ -match "BOOST_VERSION: "){ $boost_version = $_.split()[-1] }
    }
Write-Host $boost_version
