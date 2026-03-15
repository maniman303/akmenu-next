@echo off
set DIR=%CD:\=/%
docker run --rm ^
 -v "%DIR%:/workspace" ^
 -w /workspace/package ^
 devkitpro/devkitarm:20241104 ^
 bash ./package.sh