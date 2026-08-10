#!/system/bin/sh
echo "=== 当前 foveation level / enable ==="
getprop persist.pvr.foveation.level
getprop persist.pvr.config.enable_ffr
echo ""
echo "=== config db 真实路径 ==="
su -c "find /data -name 'config.db' 2>/dev/null | grep -i pvr"
echo ""
echo "=== 尝试从 settings global 找 ffr/eyebuffer/fovea ==="
su -c "settings list global 2>/dev/null | grep -iE 'ffr|fovea|eyebuffer|eye_track|foveation'"
su -c "settings list system 2>/dev/null | grep -iE 'ffr|fovea|eyebuffer|foveation'"
echo ""
echo "=== PicoSystemConfig 配置 (disableApps/功能开关) ==="
su -c "find /data /system /vendor -iname '*systemconfig*' 2>/dev/null | head"
echo "--- 搜索 foveation/eyetrack 在 xml/prop 配置 ---"
su -c "find /system/product /system/etc /vendor/etc -iname '*.xml' 2>/dev/null | xargs grep -il 'foveat\|eyetrack\|eyebuffer' 2>/dev/null | head"
echo "done"