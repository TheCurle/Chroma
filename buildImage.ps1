#requires -RunAsAdministrator
param ([uint64]$diskSize=256MB, $driveLetter='Z')
# Thanks https://github.com/VoidNV/ZenithOS/blob/master/make.ps1!

$vhdPath = ".\Chroma.vhd"

if((Get-WindowsEdition -Online).Edition -eq "Home") {
    Write-Host -ForegroundColor Red `
    "Windows 10 Home does not support Hyper-V. This install requires Professional or higher."
    exit
}


if (-not (Get-Module -ListAvailable -Name "Hyper-V"))
{
	Write-Host -ForegroundColor Red "`nError: Hyper-V module not installed.`n"
	Write-Host "The following Windows Features need to be enabled under " -NoNewLine
	Write-Host -ForegroundColor Yellow "Hyper-V" -NoNewLine
	Write-Host ":`n"
	Write-Host -ForegroundColor Black -BackgroundColor Yellow "Hyper-V Module for Windows PowerShell" -NoNewLine
	Write-Host "  " -NoNewLine
	Write-Host -ForegroundColor Black -BackgroundColor Yellow "Hyper-V Services"

	$yes = New-Object System.Management.Automation.Host.ChoiceDescription "&Yes", "Launch the Windows Features dialog and exit."
	$no = New-Object System.Management.Automation.Host.ChoiceDescription "&No", "Exit. You can launch the Windows Features dialog yourself later."
	$options = [System.Management.Automation.Host.ChoiceDescription[]]($yes, $no)
	if ($Host.UI.PromptForChoice("", "`nLaunch the Windows Features dialog to enable them?", $options, 0) -eq 0)
	{
		Start-Process "OptionalFeatures"
	}
	exit
}

function New-FAT32VHD {
    $uuid = $null;
    if(Test-Path $vhdPath) {
        Remove-Item $vhdPath
    }

    Hyper-V\New-VHD $vhdPath -SizeBytes $diskSize -Fixed |
    Mount-VHD -Passthru -NoDriveLetter            |
    Initialize-Disk -PartitionStyle GPT -PassThru |
    New-Partition -UseMaximumSize                 |
    Format-Volume -FileSystem FAT32 -Force        |
    Get-Partition                                 |
    Set-Partition -NewDriveLetter $driveLetter

    Write-Host "Created disk of size" ($diskSize / 1MB) MiB.

    Copy-Item -Recurse -Force img\* ${driveLetter}:\

    Dismount-VHD $vhdPath

}

function Update-VHD {
   Mount-VHD $vhdPath -NoDriveLetter -PassThru |
   Get-Partition                               |
   Set-Partition -NewDriveLetter $driveLetter

   Copy-Item -Recurse -Force img\* ${driveLetter}:\

   Dismount-VHD $vhdPath
}