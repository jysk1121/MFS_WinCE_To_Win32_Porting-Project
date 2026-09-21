<#
.Synopsis
Package WinATM
.DESCRIPTION
Perform a packaging/signing of the compiled WinATM build artifacts into the ZIP files
.EXAMPLE
Package-ATM -Territory US -OS 60 -Severity 5 -ReleaseType security -Version "feature/test-build" -Build
#>
Param
(
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

    # Severity the severity of the build
    [Parameter(Mandatory=$true, 
        ValueFromPipeline=$true,
        ValueFromPipelineByPropertyName=$true, 
        ValueFromRemainingArguments=$false, 
        Position=2,
        ParameterSetName='Parameter Set 1')]
    [ValidateNotNull()]
    [ValidateNotNullOrEmpty()]
    [ValidateSet("0", "1", "2", "3", "4", "5", "6", "7", "8", "9")]
    [Alias("sev")]
    $Severity,

    # ReleaseType the type of the release
    [Parameter(Mandatory=$true, 
        ValueFromPipeline=$true,
        ValueFromPipelineByPropertyName=$true, 
        ValueFromRemainingArguments=$false, 
        Position=3,
        ParameterSetName='Parameter Set 1')]
    [ValidateNotNull()]
    [ValidateNotNullOrEmpty()]
    [ValidateSet("bugfix", "security", "feature")]
    [Alias("type")]
    $ReleaseType,

    # OS the WinCE operating system version
    [Parameter(Mandatory=$true, 
        ValueFromPipeline=$true,
        ValueFromPipelineByPropertyName=$true, 
        ValueFromRemainingArguments=$false, 
        Position=2,
        ParameterSetName='Parameter Set 1')]
    [ValidateNotNull()]
    [ValidateNotNullOrEmpty()]
    [ValidateSet("60", "70")]
    $OS,

    # Version the version of the AP
    [Parameter(Mandatory=$true, 
        ValueFromPipeline=$true,
        ValueFromPipelineByPropertyName=$true, 
        ValueFromRemainingArguments=$false, 
        Position=2,
        ParameterSetName='Parameter Set 1')]
    [ValidateNotNull()]
    [ValidateNotNullOrEmpty()]
    $Version,

    [Switch]
    $Build,

    [Switch]
    $Sign
)

$env:PYTHONUNBUFFERED = "true"

$path = Split-Path $MyInvocation.MyCommand.Path
Push-Location $path

# Builds all packages
$cert_serial = "RetailCodeSigning"

#
# Build Master Files
#

$post_build_dir = ".\DAT\PostBuild"
$builder_dir = "$post_build_dir\APBuilder"

If ($Build) {
    Write-Host "Building Master Zips..."

    # Build 1500SE only model
    py.exe $builder_dir\build.py build -t $Territory -o .\UpdateFiles -s .\ -v $Version -O $OS -m 1500SE

    # Build Combined Master File
    py.exe $builder_dir\build.py build -t $Territory -o .\UpdateFiles -s .\ -v $Version -O $OS -m 1800SE -m 2700SE -m 2700T -m 2800SE -m 2800T -m 5200SE
}

#
# Sign files
#

If ($Sign) {

    Write-Host "Signing Files..."

    $signer = "rsa"
    If ( $OS -Eq "70" ) {
        $signer = "nha_hsm"
        Copy-Item $builder_dir\libhsm.dll .
        Copy-Item $builder_dir\fxpkcs11.dll .
    }

    Start-Process "py.exe" -ArgumentList "$builder_dir\build.py sign -v 1.1.0 -V $Version -o .\UpdateFiles\$OS\1500SE\manifest.xml   -t $Territory -m 1500SE -S $Severity -T $ReleaseType -d $signer -f $builder_dir\certs -c $cert_serial -k $builder_dir\mxkey.pem .\UpdateFiles\$OS\1500SE" -NoNewWindow
    Start-Process "py.exe" -ArgumentList "$builder_dir\build.py sign -v 1.1.0 -V $Version -o .\UpdateFiles\$OS\1800SE\manifest.xml   -t $Territory -m 1800SE -S $Severity -T $ReleaseType -d $signer -f $builder_dir\certs -c $cert_serial -k $builder_dir\mxkey.pem .\UpdateFiles\$OS\1800SE" -NoNewWindow
    Start-Process "py.exe" -ArgumentList "$builder_dir\build.py sign -v 1.1.0 -V $Version -o .\UpdateFiles\$OS\2700SE\manifest.xml   -t $Territory -m 2700SE -S $Severity -T $ReleaseType -d $signer -f $builder_dir\certs -c $cert_serial -k $builder_dir\mxkey.pem .\UpdateFiles\$OS\2700SE" -NoNewWindow
    Start-Process "py.exe" -ArgumentList "$builder_dir\build.py sign -v 1.1.0 -V $Version -o .\UpdateFiles\$OS\2700T\manifest.xml    -t $Territory -m 2700T  -S $Severity -T $ReleaseType -d $signer -f $builder_dir\certs -c $cert_serial -k $builder_dir\mxkey.pem .\UpdateFiles\$OS\2700T" -NoNewWindow
    Start-Process "py.exe" -ArgumentList "$builder_dir\build.py sign -v 1.1.0 -V $Version -o .\UpdateFiles\$OS\2800SE\manifest.xml   -t $Territory -m 2800SE -S $Severity -T $ReleaseType -d $signer -f $builder_dir\certs -c $cert_serial -k $builder_dir\mxkey.pem .\UpdateFiles\$OS\2800SE" -NoNewWindow
    Start-Process "py.exe" -ArgumentList "$builder_dir\build.py sign -v 1.1.0 -V $Version -o .\UpdateFiles\$OS\2800T\manifest.xml    -t $Territory -m 2800T  -S $Severity -T $ReleaseType -d $signer -f $builder_dir\certs -c $cert_serial -k $builder_dir\mxkey.pem .\UpdateFiles\$OS\2800T" -NoNewWindow
    Start-Process "py.exe" -ArgumentList "$builder_dir\build.py sign -v 1.1.0 -V $Version -o .\UpdateFiles\$OS\5200SE\manifest.xml   -t $Territory -m 5200SE -S $Severity -T $ReleaseType -d $signer -f $builder_dir\certs -c $cert_serial -k $builder_dir\mxkey.pem .\UpdateFiles\$OS\5200SE" -NoNewWindow
    Start-Process "py.exe" -ArgumentList "$builder_dir\build.py sign -v 1.1.0 -V $Version -o .\UpdateFiles\$OS\Combined\manifest.xml -t $Territory -m none   -S $Severity -T $ReleaseType -d $signer -f $builder_dir\certs -c $cert_serial -k $builder_dir\mxkey.pem .\UpdateFiles\$OS\Combined" -NoNewWindow -Wait

    If ( $OS -Eq "70" ) {
        Remove-Item libhsm.dll
        Remove-Item fxpkcs11.dll
    }
}

Pop-Location
