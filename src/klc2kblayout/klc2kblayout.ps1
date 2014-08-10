<#
    .SYNOPSIS
    Generates a keyboard layout file to be used with kblayout from layouts
    created with Microsoft Keyboard Layout Creator
    
    .PARAMETER InputLayoutPath
    .klc file for the keyboard layout you want to type as (e.g. dvorak)
    
    .PARAMETER OutputLayoutPath
    .klc file for the keyboard layout you want windows to think you are
    typing as (e.g. qwerty)
    
    .PARAMETER OutputPath
    File to write keyboard layout to
#>
param(
    $InputLayoutPath,
    $OutputLayoutPath,
    $OutputPath
)

<#
    .SYNOPSIS
    Reads files created by Microsoft Keyboard Layout Creator into a format
    that facilitates mapping scancodes between two layouts
    
    .OUTPUT
    Hash using the keyboard key name ('A', 'Esc', etc) as the key, and scancode
    as the value
#>
function Parse-KLC() {
    param(
        $FilePath
    )
    $scanCodes = @{}
    $inScancodeBlock = $false
    
    foreach($line in Get-Content $FilePath) {
    
        if(-not $inScancodeBlock) {        
            # skip the early sections. We currently are only intersted in the block
            # of scancodes inside the LAYOUT section
            
            if($line.StartsWith("LAYOUT")) {                
                # skip the header for the layout section
                [void]$foreach.MoveNext() #
                [void]$foreach.MoveNext() # //SC	VK_		Cap	0	1	2
                [void]$foreach.MoveNext() # //--	----		----	----	----	----
                [void]$foreach.MoveNext() #
                
                $inScancodeBlock = $true
            }
            continue
        }elseif ($inScancodeBlock -and $line -eq "") {
            # the scancode section ends with an empty line
            # after that we are done
            break
        } else {
            # inside the scancode block
            # parse the scan code row
            $scanCode,$keyName,$void = $line -split '\s+'
            
            # and add it to the dictionary
            # NOTE: if the same key name has multiple codes the
            #       last one will be used.
            #       I haven't found a layout that does this
            #       yet though.
            $scanCodes[$keyName] = $scanCode
        }
    }    
    
    return $scanCodes
}

<#
    .SYNOPSIS    
    Maps the scancodes from one layout to another.
    
    .EXAMPLE
    Map-Layout -InputLayout $dvorak -OutputLayout $qwerty
    Creates a scancode map that reads incoming keys as dvorak and translates
    them to qwerty. e.g. the incoming scancode for 's' is output with the scancode for 'o'
    
    .PARAMETER InputLayout
    The layout to use as the input scancodes
    
    .PARAMETER OutputLayout
    The layout to use for the output scancodes
#>
function Map-Layouts(){
    param(          
          $InputLayout,
          $OutputLayout
          )
          
     $map = @{}     
     foreach($key in $InputLayout.GetEnumerator()){      
        if ($OutputLayout.Contains($key.Name)) {
            $map[$key.Value] = $OutputLayout[$key.Name]
        } else {
            $map[$key.Value] = $key.Value
        }
     }
          
     return $map   
}

<#
    .SYNOPSIS
    Writes a scancode map into a format that can be used by kblayout 
#>
function Write-LayoutMap(){
    param(
        $LayoutMap,
        $LayoutName,
        $OutputPath
        )
     
     # Output raw c right now, but it will be changed to a real config format later
         
     Out-File -FilePath $outputPath -InputObject @"
VOID KbLayoutLoadLayout$LayoutName(USHORT layout[]);
VOID KbLayoutLoadLayout$LayoutName(USHORT layout[])
{
"@
     
     foreach ($map in $LayoutMap.GetEnumerator()){
        Out-File -Append -FilePath $outputPath -InputObject "    layout[0x$($map.Name)] = 0x$($map.Value);"
     }
        
     Out-File -Append -FilePath $outputPath -InputObject @" 
}
"@
}

$outputLayout = Parse-KLC $OutputLayoutPath
$inputLayout = Parse-KLC $InputLayoutPath
$mapped = Map-Layouts $inputLayout $outputLayout
Write-LayoutMap $mapped ($inputLayoutPath -replace '\..+?$') $OutputPath
