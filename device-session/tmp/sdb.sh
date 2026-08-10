#!/system/bin/sh
DB=/data/user_de/0/com.pvr.configuration/databases/config.db
echo "=== db strings (keys) ==="
strings $DB 2>/dev/null | grep -iE "item|key_|power|perf|fps|refresh|foveat|eyebuffer|ffr|stencil|gesture|sixdof|otg|quick|boundary|screen|developer|debug|api|sdk_|support|cast|passthrough|seethrough" | head -80
