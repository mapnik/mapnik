$PSVersionTable
$PSVersionTable.PSVersion

$comp_name = $env:COMPUTERNAME
$user_name = $env:USERNAME
Write-Host $comp_name $user_name

$on_appveyor = $false
if($comp_name -like 'APPVYR*' -And $user_name -eq "appveyor"){
    $on_appveyor = $true
}


$SystemManaged  = Get-WmiObject -Class Win32_ComputerSystem | % {$_.AutomaticManagedPagefile} 
$total_physicalmem = gwmi Win32_ComputerSystem | % {[Math]::round($_.TotalPhysicalMemory/1MB,0)} 
$physical_mem = get-ciminstance -class 'cim_physicalmemory' | % { $_.Capacity/1024/1024}

$PF =gwmi Win32_PageFileUsage 
$PageFileLocation = $PF.Name;
$PageFileSize = $PF.AllocatedBaseSize 

Write-Host "physical memory          : "$physical_mem
Write-Host "total physical memory    : "$total_physicalmem
Write-Host "page file system managed : "$SystemManaged
Write-Host "page file location       : "$PageFileLocation
Write-Host "page file size           : "$PageFileSize
Write-Host "InitialSize              : "${CurrentPageFile}.InitialSize
Write-Host "MaximumSize              : "$CurrentPageFile.MaximumSize

if($on_appveyor -eq $true){

    Write-Host !!!!!!! on AppVeyor: changing page file settings !!!!!!!!!!

    $dirs = (
        "C:\qt",
        "C:\Users\appveyor\AppData\Local\Microsoft\Web Platform Installer",
        "C:\Program Files\Microsoft SQL Server",
        "C:\ProgramData\Package Cache"
    )
    Foreach($dir in $dirs){
        if(Test-Path $dir) {
            Write-Host found $dir
            Remove-Item $dir -Force -Recurse
        } else {
            Write-Host not found $dir
        }
    }

    #disable automatically managed page file settings
    $c = Get-WmiObject Win32_computersystem -EnableAllPrivileges
    if($c.AutomaticManagedPagefile){
        Write-Host disabling managed page file settings
        $c.AutomaticManagedPagefile = $false
        $c.Put() | Out-Null
    }

    $new_page_size=18000
    $CurrentPageFile = Get-WmiObject -Class Win32_PageFileSetting
    if($CurrentPageFile.InitialSize -ne $new_page_size){
        Write-Host "setting new page file size to $new_page_size"
        $CurrentPageFile.InitialSize=$new_page_size
        $CurrentPageFile.MaximumSize=$new_page_size
        $CurrentPageFile.Put() | Out-Null
    }

    Write-Host "new ------------ "
    Write-Host "system managed:" (Get-WmiObject -Class Win32_ComputerSystem | % {$_.AutomaticManagedPagefile})
    Write-Host "page file size:" (gwmi Win32_PageFileUsage).AllocatedBaseSize
    Write-Host "InitialSize: "${CurrentPageFile}.InitialSize
    Write-Host "MaximumSize: "$CurrentPageFile.MaximumSize
} else {
    Write-Host not on AppVeyor, leaving page file as is
}

#list drives
Get-WmiObject -Class Win32_LogicalDisk |
    Where-Object {$_.DriveType -ne 5} |
    Sort-Object -Property Name | 
    Select-Object Name, VolumeName, FileSystem, Description, VolumeDirty, `
        @{"Label"="DiskSize(GB)";"Expression"={"{0:N}" -f ($_.Size/1GB) -as [float]}}, `
        @{"Label"="FreeSpace(GB)";"Expression"={"{0:N}" -f ($_.FreeSpace/1GB) -as [float]}}, `
        @{"Label"="%Free";"Expression"={"{0:N}" -f ($_.FreeSpace/$_.Size*100) -as [float]}} |
    Format-Table -AutoSize

