@echo off
rem Starts the Jello task board for MSE Slim and opens it in your browser (http://localhost:5179/).
rem The board lives in claude\board\ as one JSON file per card; Claude reads and edits the same files.
rem Port 5179, so it can run next to Birb Box's board (5178). Needs the .NET 10 SDK.
rem Close this window to stop the board. Every change is saved the moment you make it.

cd /d "%~dp0"
title Jello board - MSE Slim
dotnet run --project "tools\JelloBoard" -c Release -- --board "%~dp0claude\board" --port 5179
if errorlevel 1 pause
