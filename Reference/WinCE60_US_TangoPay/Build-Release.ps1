# This script is the script that should be modified to create and upload the release media.

$QA = $True

function Publish {
    If ($QA) {
        Publish-QA
    } Else {
        Publish-QA
        Publish-Prod
    }
}

function Publish-Prod {
    pushd .\DAT\PostBuild\APBuilder
    .\publish_prod.ps1
    popd
}

function Publish-QA {
    pushd .\DAT\PostBuild\APBuilder
    .\publish_qa.ps1
    popd
}

.\Build-Atm.ps1 -Territory US -Customer General
Publish

.\Build-Atm.ps1 -Territory US -Customer PAI
Publish

.\Build-Atm.ps1 -Territory US -Customer Cardtronics
Publish

.\Build-Atm.ps1 -Territory CA -Customer General
Publish

.\Build-Atm.ps1 -Territory CA -Customer Cardtronics
Publish

.\Build-Atm.ps1 -Territory MX -Customer General
Publish

.\Build-Atm.ps1 -Territory MX -Customer Bansi
Publish
