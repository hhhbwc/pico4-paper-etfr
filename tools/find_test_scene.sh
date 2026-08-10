#!/system/bin/sh
echo "=== foveation / eyebuffer settings ==="
getprop | grep -iE "foveat|eyebuffer|ffr|gaze"
echo ""
echo "=== 系统里带 foveation/eyetrack 的 app (可能有测试/设置界面) ==="
pm list packages 2>/dev/null | grep -iE "eye|track|fovea|test|developer|vr" | head -20
echo ""
echo "=== 设置/开发者选项相关 activity ==="
dumpsys package com.pvr.developmentsettings 2>/dev/null | grep -iE "activity" | grep -iE "eye|fovea|track|ffr|test" | head
echo ""
echo "=== pvrfit/日志是否有 foveation 痕迹 ==="
su -c "logcat -d 2>/dev/null | grep -iE 'foveat|eyetrack|gaze|eyebuffer' | tail -20"
echo "done"
