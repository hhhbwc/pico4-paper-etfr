#!/system/bin/sh
echo "=== 所有可写 foveat/eyebuffer 属性 ==="
getprop | grep -iE "foveat|eyebuffer|ffr|gaze" 
echo ""
echo "=== Pxr_SetFoveationLevel 在 openxr_runtime 里的运行时地址 (libopenxr_api 0x21a74) ==="
su -c "cat /proc/2009/maps | grep libopenxr_api.so | head -1 | cut -d'-' -f1"
echo ""
echo "=== 现有 foveation 相关可写配置 (settings/属性) ==="
su -c "settings list global 2>/dev/null | grep -iE 'fovea|ffr|eyebuffer'"
echo "--- system ---"
su -c "settings list system 2>/dev/null | grep -iE 'fovea|ffr|eyebuffer'"
echo "--- secure ---"
su -c "settings list secure 2>/dev/null | grep -iE 'fovea|ffr|eyebuffer'"
echo ""
echo "=== 尝试直接 setprop 强制 foveation level ==="
setprop persist.pvr.foveation.level 6 2>&1
echo "set exit=$?"
getprop persist.pvr.foveation.level
echo "done"
