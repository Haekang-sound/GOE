@echo off
chcp 65001 > nul

REM --- [1. 설정] ---
set GEMINI_MODEL=gemini-3-pro-preview
set GEMINI_API_KEY=AIzaSyCr8WQJz4OYl8jI25BoTdzZz2a_Wejx3C8
set GOOGLE_CLOUD_PROJECT=gen-lang-client-0015189838

REM --- [2. 실행] ---
REM 아래 줄이 있어야 설정된 모델로 채팅이 시작됩니다!
gemini

pause