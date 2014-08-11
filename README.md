# KmKbLayout
> Kernel Mode Keyboard Layout

For when you want your keyboard layouts at a lower level

# What

KmKbLayout is a Windows filter driver that modifies keyboard scancodes before they get sent further up the layer. This allows you to implement a keyboard layout before the Windows keyboard layout engine.

# Why

I was getting tired of having to install my keyboard layouts on remote computers when using the built in RDP client. My colleagues were also getting sick of having to change the layout when they took over my sessions too. After a bit of tinkering with windows hooks, I realised that mstsc.exe must be sending the scancodes and this project was born.

Also, I wanted to play with a filter driver :)

# How Tos

## How to build

1) Install [Windows Driver Kit 7.1.0](http://www.microsoft.com/en-gb/download/details.aspx?id=11800)

2) Open "x86 Checked Build Environment" or "x64 Checked Build Environment"
   from the start menu (under Windows Driver Kits).
   Make certain you are running the version with the correct architecture and Windows version

3) Run make from the root directory

4) If you are building 64 bit you will also need to sign the driver
   I have been using [dseo](http://www.ngohq.com/?page=dseo)

## How to install

1) Open powershell as administrator

2) Browse to the directory with the built KmKbLayout files

3) Run ./install.ps1
    You may need to run ```Set-ExecutionPolicy -ExecutionPolicy Unrestricted```

## How to change the layout

1) Open regedit.exe

2) Browse to HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\kblayout\Layouts

3) Change the "Selected Layout" value to the name of the layout you want to use

or

1) Double click on the .reg file for the layout you want to use.
   They have been set up to change the selected layout on merge.

## How to build a new layout


1) Run [Microsoft Keyboard Layout Creator](http://msdn.microsoft.com/en-GB/goglobal/bb964665.aspx)

2) Click File->Load Existing Keyboard

3) Select your current keyboard layout

4) Save the layout somewhere
   This layout is your out layout and is what the new layout will map to (in my case it is qwerty)

5) Create a new layout in MKLC, and save it
   This is your input layout, and is what you want the keyboard to input as

6) Run klc2kblayout.ps1

    ./klc2kblayout -OutputLayout BaseLayout.klc -InputLayout NewLayout.klc -Output new.reg

7) Double click the resulting registry file

# License

MIT