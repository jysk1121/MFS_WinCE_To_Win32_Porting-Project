<#
.Synopsis
Build WinATM
.DESCRIPTION
Perform a build operation on the WinATM project with the specified parameters
.EXAMPLE
Build-ATM -Customer PAI -Territory US
.EXAMPLE
Build-ATM -Territory MX
#>
Param
(
   # Customer The customer for which to build
   [Parameter(ValueFromPipeline=$true,
      ValueFromPipelineByPropertyName=$true, 
      ValueFromRemainingArguments=$false, 
      Position=0,
      ParameterSetName='Parameter Set 1')]
   [ValidateNotNull()]
   [ValidateNotNullOrEmpty()]
   [ValidateSet("General", "PAI", "Cardtronics", "CashDepot", "CIBC (Mex)", "DYNAMICDCC")]
   [Alias("cust")] 
   $Customer = "General",

   # Territory The territory for the build
   [Parameter(Mandatory=$true, 
      ValueFromPipeline=$true,
      ValueFromPipelineByPropertyName=$true, 
      ValueFromRemainingArguments=$false, 
      Position=1,
      ParameterSetName='Parameter Set 1')]
   [ValidateNotNull()]
   [ValidateNotNullOrEmpty()]
   [ValidateSet("US", "CA", "MX", "AU")]
   [Alias("terr")]
   $Territory,

   # LogLevel sets the logging level for the build
   [Parameter(Mandatory=$true, 
      ValueFromPipeline=$true,
      ValueFromPipelineByPropertyName=$true, 
      ValueFromRemainingArguments=$false, 
      Position=2,
      ParameterSetName='Parameter Set 1')]
   [ValidateNotNull()]
   [ValidateNotNullOrEmpty()]
   [ValidateSet("Error", "Warn", "Info", "Debug")]
   [Alias("log")]
   $LogLevel = "Error",

   [Switch]
   $LocalMode
)

Function Setup-Env() {
   pushd 'C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC'
   cmd /c "vcvarsall.bat&set" |
   foreach {
      if ($_ -match "=") {
         $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
      }
   }
   popd
   $env:Path += ";$HOME/.sonar/build-wrapper-win-x86"
}

Setup-Env

$env:CL = "/DAPP_DEF /DAPP_CUSTOM_CARDTRONICS=$(If ($Customer -eq 'Cardtronics') { 1 } Else { 0 }) /DAPP_CUSTOM_PAI=$(If ($Customer -eq 'PAI') { 1 } Else { 0 }) /DAPP_CUSTOM_CIBC_MX=$(If ($Customer -eq 'CIBC (Mex)') { 1 } Else { 0 }) /DAPP_CUSTOM_CASHDEPOT=$(If ($Customer -eq 'CashDepot') { 1 } Else { 0 }) /DAPP_CUSTOM_DYNAMICDCC=$(If ($Customer -eq 'DYNAMICDCC') { 1 } Else { 0 })"
# $env:RC = "/DAPP_DEF /DAPP_CUSTOM_CARDTRONICS=$(If ($Customer -eq 'Cardtronics') { 1 } Else { 0 }) /DAPP_CUSTOM_PAI=$(If ($Customer -eq 'PAI') { 1 } Else { 0 }) /DAPP_CUSTOM_CIBC_MX=$(If ($Customer -eq 'CIBC (Mex)') { 1 } Else { 0 }) /DAPP_CUSTOM_CASHDEPOT=$(If ($Customer -eq 'CashDepot') { 1 } Else { 0 })"
$env:RC = $env:CL

switch ($LogLevel) {
   "Error" { $env:CL += " /DNH_LOG_LEVEL=1" }
   "Warn"  { $env:CL += " /DNH_LOG_LEVEL=2" }
   "Info"  { $env:CL += " /DNH_LOG_LEVEL=3" }
   "Debug" { $env:CL += " /DNH_LOG_LEVEL=4" }
   Default { $env:CL += " /DNH_LOG_LEVEL=1" }
}

if ($LocalMode) {
   $env:CL += " /DAPP_LOCAL_MODE /DCDU_DECRYPT_MODE"
   $env:RC += " /DAPP_LOCAL_MODE /DCDU_DECRYPT_MODE"
}

# Interpolate Solution Location
$projectName = 'Project_WinCE60\WinATM_ITM'
If ($Territory -ne 'US') {
   $projectName = "$($projectName)_$($Territory)"
}
$projectName += ".sln"
$wince6_path = ".\ExeRele_60_${Territory}"
$wince7_path = ".\ExeRele_70_${Territory}"
$certificate_path = ".\DAT\PostBuild\MX5300CE2ndCert.pfx"
$masterfile_path = ".\DAT\MasterFiles\"

# Note: It seems that vcbuild (in parallel) is faster than MSBuild, but MSBuild has a prettier output
# Update: MSBuild doesn't respect the ENVVARs within the project files, so VCBuild it is!

# Days              : 0
# Hours             : 0
# Minutes           : 2
# Seconds           : 29
# Milliseconds      : 264
# Ticks             : 1492647871
# TotalDays         : 0.0017276017025463
# TotalHours        : 0.0414624408611111
# TotalMinutes      : 2.48774645166667
# TotalSeconds      : 149.2647871
# TotalMilliseconds : 149264.7871
vcbuild.exe /M4 /clean $projectName
build-wrapper-win-x86-64 --out-dir bw-output vcbuild.exe /M4 /rebuild $projectName "Release|CHA_SDK_CE6 (ARMV4I)"

if ($lastexitcode -ne 0)
{
   throw "Build Failed"
}


# Days              : 0
# Hours             : 0
# Minutes           : 2
# Seconds           : 46
# Milliseconds      : 658
# Ticks             : 1666588577
# TotalDays         : 0.00192892196412037
# TotalHours        : 0.0462941271388889
# TotalMinutes      : 2.77764762833333
# TotalSeconds      : 166.6588577
# TotalMilliseconds : 166658.8577
# MSBuild.exe $projectName /t:Clean
# MSBuild.exe $projectName /t:WinAtm /p:Configuration=Release /p:RC="$env:RC" 

Remove-Item -Force $wince6_path\*.lib
Remove-Item -Force $wince6_path\*.exp

# Copy in Masterfiles (unsigned)
Write-Host "Copying Masterfiles to 60 directory"
Copy-Item $masterfile_path* $wince6_path

Write-Host "Copying DLLRele"
Copy-Item ".\DllRele\${Territory}\*" $wince6_path

# Create WinCE 70 files
Write-Host "Creating WinCE 7 build"
If ( Test-Path $wince7_path ) {
   Remove-Item -Recurse $wince7_path
}

mkdir $wince7_path
Copy-Item $wince6_path\* $wince7_path

Write-Host "Signing WinCE 6.0 Binaries"
signtool.exe sign /f $certificate_path "$wince6_path\*.EXE"
signtool.exe sign /f $certificate_path "$wince6_path\*.DLL"
signtool.exe sign /f $certificate_path "$wince6_path\*.OCX"

Write-Host "Signing WinEC 7.0 Binaries"
$cert = Get-ChildItem -Path Cert:\LocalMachine\My\274a0b99256f7d8ce80acadb8d55b9ff706f323c
Set-AuthenticodeSignature -Certificate $cert -FilePath $wince7_path\*.exe -IncludeChain All -hashalgorithm "SHA256" -TimestampServer http://timestamp.globalsign.com/?signature=sha2
Set-AuthenticodeSignature -Certificate $cert -FilePath $wince7_path\*.dll -IncludeChain All -hashalgorithm "SHA256" -TimestampServer http://timestamp.globalsign.com/?signature=sha2
Set-AuthenticodeSignature -Certificate $cert -FilePath $wince7_path\*.ocx -IncludeChain All -hashalgorithm "SHA256" -TimestampServer http://timestamp.globalsign.com/?signature=sha2
