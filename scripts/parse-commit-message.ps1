if($env:APPVEYOR_REPO_COMMIT_MESSAGE.ToLower().Contains($args[0].ToLower())) {
    Write-Host '1';
} else {
    Write-Host '0';
}
