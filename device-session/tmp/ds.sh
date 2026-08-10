#!/system/bin/sh
echo "=== settings global 里 pvr/config 相关 ==="
settings list global 2>/dev/null | grep -iE "pvr|power|perf|fps|refresh|foveat|eyebuffer|ffr|stencil|gesture|sixdof|otg|quick" 
echo "=== DB config 库 ==="
find /data/user_de/0/com.pvr.configuration -name "*.db" 2>/dev/null
echo "=== config db 表 ==="
DB=$(find /data/user_de/0/com.pvr.configuration -name "config.db" 2>/dev/null | head -1)
echo "DB=$DB"
