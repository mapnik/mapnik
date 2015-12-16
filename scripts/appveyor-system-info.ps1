$SystemManaged  = Get-WmiObject -Class Win32_ComputerSystem | % {$_.AutomaticManagedPagefile} 
$total_physicalmem = gwmi Win32_ComputerSystem | % {[Math]::round($_.TotalPhysicalMemory/1MB,0)} 
$physical_mem = get-ciminstance -class 'cim_physicalmemory' | % { $_.Capacity/1024/1024}

$PF =gwmi Win32_PageFileUsage 
$PageFileLocation = $PF.Name;
$PageFileSize = $PF.AllocatedBaseSize 

Write-Host "physical memory          : $physical_mem"
Write-Host "total physical memory    : $total_physicalmem"
Write-Host "page file system managed : $SystemManaged"
Write-Host "page file location       : $PageFileLocation"
Write-Host "page file size           : $PageFileSize"