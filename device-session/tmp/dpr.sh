#!/system/bin/sh
echo "=== PXRuleValueFile.txt 各powerlevel段 ==="
awk 'BEGIN{RS="}"} /powerlevel/ {print "--- " $0 " ---"}' /system/etc/pxrconfig/rule/PXRuleValueFile.txt
