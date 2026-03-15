@echo off
set DIR=%CD:\=/%
docker run --rm ^
 -v "%DIR%:/workspace" ^
 -w /workspace ^
 devkitpro/devkitarm:20241104 ^
 make %*