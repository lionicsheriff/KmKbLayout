##
# How to install
################
#
# 1) Run this script to set up the registry entries
#    If you can't run the script you may need to execute:
#    Set-ExecutionPolicy -ExecutionPolicy Unrestricted
# 2) Reboot
#
##

$DRIVER_NAME = "kblayout"
$DRIVER_PATH = "$($ENV:WinDir)\System32\Drivers\$DRIVER_NAME.sys"

##
# Check if the user has correct permissions to install the driver
##
$isAdmin = [bool](([System.Security.Principal.WindowsIdentity]::GetCurrent()).groups -match "S-1-5-32-544")
if (-not $isAdmin){
    Write-Host "Can not install. Please run this script again as an administrator"
    Write-Host "Press any key to continue..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit
}

##
# Copy the driver to the installation folder
##
$arch = (gwmi win32_operatingsystem).osarchitecture
if ($arch -eq "64-bit"){
    Write-Host "Installing 64bit driver"
    Copy-Item -Path ".\amd64\$DRIVER_NAME.sys" -Destination $DRIVER_PATH

} else {
    Write-Host "Installing 32bit driver"
    Copy-Item -Path ".\i386\$DRIVER_NAME.sys" -Destination $DRIVER_PATH
}

##
# Set up the registry entries
##
Push-Location
Set-Location HKLM:

# Register the driver
if (-not (Test-Path ".\SYSTEM\CurrentControlSet\services\$DRIVER_NAME") ) {
    echo "Registering driver"

    # using the full path for the ImagePath property causes the driver to not load properly
    # however, a path relative to %winddir% works fine
    $imagePath = $DRIVER_PATH.Substring($Env:WinDir.Length + 1) # includes the / as well

    $kblayoutService = New-Item .\SYSTEM\CurrentControlSet\services -Name $DRIVER_NAME
    $kblayoutService | New-ItemProperty -Name ImagePath -Type ExpandString -Value $imagePath
    $kblayoutService | New-ItemProperty -Name Group -Type String -Value "Keyboard Class"
    $kblayoutService | New-ItemProperty -Name ErrorControl -Type Dword -Value 1
    $kblayoutService | New-ItemProperty -Name Start -Type Dword -Value 3
    $kblayoutService | New-ItemProperty -Name Type -Type Dword -Value 1
}

# Add the driver to the keyboard filters
$kbdClass = Get-Item ".\SYSTEM\CurrentControlSet\Control\Class\{4D36E96B-E325-11CE-BFC1-08002BE10318}"
$upperFilters = ($kbdClass | Get-ItemProperty -Name UpperFilters).UpperFilters
if (-not ($upperFilters -contains $DRIVER_NAME)) {
    Write-Host "Adding to keyboard filters"

    $newFilters = $upperFilters + $DRIVER_NAME
    $kbdClass | Set-ItemProperty -Name UpperFilters -Value $newFilters
}

Pop-Location

Write-Host "Installation complete"
Write-Host "Press any key to continue..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
