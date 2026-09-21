$env:PYTHONUNBUFFERED = "true"

$os = $Args[0]
$path = Split-Path $MyInvocation.MyCommand.Path
Push-Location $path

# Publish Options
$publish = $true

$storage_type = 's3', '-b', 'nhasoftware'
$repo_root = ''
$hives = "retail SE$os `$territory `$lineage `$model"  # production
$latest = '-l'  # or '-l' for latest

If ($Args[1] -eq 'true') 
{
    $latest = '-l'
}

#
# Publish to Repo
#

$post_build_dir = ".\DAT\PostBuild"
$builder_dir = "$post_build_dir\APBuilder"

If ($publish) {
    Write-Host "Publishing..."

    python.exe $builder_dir\build.py publish -H "$hives" -p ".\UpdateFiles\$os\1500SE"   $latest $($repo_root) $($storage_type)
    python.exe $builder_dir\build.py publish -H "$hives" -p ".\UpdateFiles\$os\1800SE"   $latest $($repo_root) $($storage_type)
    python.exe $builder_dir\build.py publish -H "$hives" -p ".\UpdateFiles\$os\2700SE"   $latest $($repo_root) $($storage_type)
    python.exe $builder_dir\build.py publish -H "$hives" -p ".\UpdateFiles\$os\2700T"    $latest $($repo_root) $($storage_type)
    python.exe $builder_dir\build.py publish -H "$hives" -p ".\UpdateFiles\$os\2800SE"   $latest $($repo_root) $($storage_type)
    python.exe $builder_dir\build.py publish -H "$hives" -p ".\UpdateFiles\$os\2800T"    $latest $($repo_root) $($storage_type)
    python.exe $builder_dir\build.py publish -H "$hives" -p ".\UpdateFiles\$os\5200SE"   $latest $($repo_root) $($storage_type)
    python.exe $builder_dir\build.py publish -H "$hives" -p ".\UpdateFiles\$os\Combined" $latest $($repo_root) $($storage_type)
}

Pop-Location