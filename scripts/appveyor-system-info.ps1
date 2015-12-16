$SystemManaged  = Get-WmiObject -Class Win32_ComputerSystem | % {$_.AutomaticManagedPagefile} 
$total_physicalmem = gwmi Win32_ComputerSystem | % {[Math]::round($_.TotalPhysicalMemory/1MB,0)} 
$physical_mem = get-ciminstance -class 'cim_physicalmemory' | % { $_.Capacity/1024/1024}

$PF =gwmi Win32_PageFileUsage 
$PageFileLocation = $PF.Name;
$PageFileSize = $PF.AllocatedBaseSize 
$CurrentPageFile = Get-WmiObject -Class Win32_PageFileSetting

Write-Host "physical memory          : $physical_mem"
Write-Host "total physical memory    : $total_physicalmem"
Write-Host "page file system managed : $SystemManaged"
Write-Host "page file location       : $PageFileLocation"
Write-Host "page file size           : $PageFileSize"
Write-Host page file min size       : $CurrentPageFile.InitialSize
Write-Host "page file max size       : $CurrentPageFile.MaximumSize"

#disable automatically managed page file settings
$c = Get-WmiObject Win32_computersystem -EnableAllPrivileges
if($c.AutomaticManagedPagefile){
    Write-Host disabling managed page file settings
    $c.AutomaticManagedPagefile = $false
    $c.Put() | Out-Null
}

$new_page_size=8192

if($PageFileSize -lt $new_page_size){
    Write-Host "settings new page file size to $new_page_size"
    $CurrentPageFile.InitialSize=$new_page_size
    $CurrentPageFile.MaximumSize=$new_page_size
    $CurrentPageFile.Put() | Out-Null
}
