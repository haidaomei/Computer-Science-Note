# ============================================================================
# Run all probes.  Usage (PowerShell, from the Desktop directory):
#     .\run_lambda_probes.ps1
# Requires: clang / clang++ on PATH (llvm-mingw).
#
# NOTE: this runner is deliberately ASCII-only.  Windows PowerShell 5.1 reads
#       BOM-less .ps1 files as ANSI/GBK, which can corrupt non-ASCII literals.
#       The probe *programs* below print UTF-8 Chinese and are unaffected.
#
# Probe map:
#   01  C   : expressions that produce objects; lvalue nature; location & lifetime
#   02  C++ : prvalue / result object / materialization / init dispatch
#   03a C++17: guaranteed init compiles even with deleted copy AND move ctor
#   03b C++14: the very same code fails to compile
#   03c C++20: optional-elision path fails (NRVO still needs an accessible ctor)
#   04a C++20 default          : optional elision on, 0 copies
#   04b C++20 -fno-elide-...   : only the OPTIONAL case gains a copy
#   05  C++ : closure size; captured data stored once; lifetime set by the target
# ============================================================================
$ErrorActionPreference = 'Continue'
[Console]::OutputEncoding = [Text.Encoding]::UTF8

$d = $PSScriptRoot
if (-not $d) { $d = (Get-Location).Path }

function Run {
    param(
        [string]$Title,
        [string]$File,
        [string]$Compiler,
        [string[]]$Flags
    )
    Write-Host ""
    Write-Host ("=" * 72) -ForegroundColor Cyan
    Write-Host "  $Title" -ForegroundColor Cyan
    Write-Host ("=" * 72) -ForegroundColor Cyan

    $src  = Join-Path $d $File
    $base = [IO.Path]::GetFileNameWithoutExtension($File)
    $exe  = Join-Path $d ("build_" + $base + ".exe")
    $argv = @($Flags) + @('-o', $exe, $src)

    Write-Host ("  $ " + $Compiler + " " + ($Flags -join ' ')) -ForegroundColor DarkGray
    $out = & $Compiler @argv 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  [COMPILE FAILED -- this is the result being demonstrated]" -ForegroundColor Yellow
        $out | Select-String 'error' | Select-Object -First 3 | ForEach-Object { Write-Host "    $_" }
        return
    }
    & $exe
}

Run -Title 'PROBE 01  C: expressions producing objects; lvalue; location/lifetime' -File 'lambda_probe_01_C_objects.c' -Compiler 'clang' -Flags @('-std=c23','-O0','-Wno-writable-strings')

Run -Title 'PROBE 02  C++: prvalue / result object / materialization / init dispatch' -File 'lambda_probe_02_cpp_value_category.cpp' -Compiler 'clang++' -Flags @('-std=c++20','-O0')

Run -Title 'PROBE 03a  C++17: guaranteed init, copy AND move deleted' -File 'lambda_probe_03_elision.cpp' -Compiler 'clang++' -Flags @('-std=c++17','-O0')

Run -Title 'PROBE 03b  C++14: identical code fails' -File 'lambda_probe_03_elision.cpp' -Compiler 'clang++' -Flags @('-std=c++14','-O0')

Run -Title 'PROBE 03c  C++20 + optional path: fails (NRVO needs accessible ctor)' -File 'lambda_probe_03_elision.cpp' -Compiler 'clang++' -Flags @('-std=c++20','-O0','-DTRY_OPTIONAL')

Run -Title 'PROBE 04a  C++20 default (optional elision active)' -File 'lambda_probe_04_elision_switch.cpp' -Compiler 'clang++' -Flags @('-std=c++20','-O0')

Run -Title 'PROBE 04b  C++20 -fno-elide-constructors (only OPTIONAL case changes)' -File 'lambda_probe_04_elision_switch.cpp' -Compiler 'clang++' -Flags @('-std=c++20','-O0','-fno-elide-constructors')

Run -Title 'PROBE 05  closure size; single copy of captured data; lifetime by target' -File 'lambda_probe_05_closure_storage.cpp' -Compiler 'clang++' -Flags @('-std=c++20','-O0')

Write-Host ""
Write-Host "All probes finished." -ForegroundColor Green
